#include "plugin.h"

#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "common/logger.h"
#include "common/protocol.h"


#define XEMBED_EMBEDDED_NOTIFY 0
#define XEMBED_FOCUS_OUT       5


namespace Airwave {


Plugin::Plugin(const std::string& vstPath, const std::string& hostPath,
		const std::string& prefixPath, const std::string& loaderPath,
		const std::string& logSocketPath, AudioMasterProc masterProc) :
	masterProc_(masterProc),
	effect_(nullptr),
	data_(nullptr),
	dataLength_(0),
	childPid_(-1),
	processCallbacks_(ATOMIC_FLAG_INIT),
	mainThreadId_(std::this_thread::get_id())
{
	// The constructor will return early when error occurs. In this case the effect()
	// fucntion will be returning nullptr, indicating the error.

	DEBUG("Main thread id: %p", mainThreadId_);

	// FIXME: frame size should be verified.
	if(!controlPort_.create(65536)) {
		ERROR("Unable to create control port");
		return;
	}

	// FIXME: frame size should be verified.
	if(!callbackPort_.create(1024)) {
		ERROR("Unable to create callback port");
		controlPort_.disconnect();
		return;
	}

	// Start the host endpoint's process.
	childPid_ = fork();
	if(childPid_ == -1) {
		ERROR("fork() call failed");
		controlPort_.disconnect();
		callbackPort_.disconnect();
		return;
	}
	else if(childPid_ == 0) {
		setenv("WINEPREFIX", prefixPath.c_str(), 1);
		setenv("WINELOADER", loaderPath.c_str(), 1);

		std::string id = std::to_string(controlPort_.id());
		std::string level = std::to_string(static_cast<int>(loggerLogLevel()));

		execl("/bin/sh", "/bin/sh", hostPath.c_str(), vstPath.c_str(), id.c_str(),
				level.c_str(), logSocketPath.c_str(), nullptr);

		// We should never reach this point on success child execution.
		ERROR("execl() call failed");
		return;
	}

	DEBUG("Child process started, pid=%d", childPid_);

	std::memset(&rect_, 0, sizeof(ERect));

	processCallbacks_.test_and_set();
	callbackThread_ = std::thread(&Plugin::callbackThread, this);

	condition_.wait();

	// Send host info to the host endpoint.
	DataFrame* frame = controlPort_.frame<DataFrame>();
	frame->command = Command::HostInfo;
	frame->opcode = callbackPort_.id();
	controlPort_.sendRequest();

	TRACE("Waiting response from host endpoint...");

	// Wait for the host endpoint initialization.
	if(!controlPort_.waitResponse()) {
		ERROR("Host endpoint is not responding");
		kill(childPid_, SIGKILL);
		controlPort_.disconnect();
		callbackPort_.disconnect();
		childPid_ = -1;
		return;
	}

	PluginInfo* info = reinterpret_cast<PluginInfo*>(frame->data);
	effect_ = new AEffect;
	std::memset(effect_, 0, sizeof(AEffect));

	effect_->magic                  = kEffectMagic;
	effect_->object                 = this;
	effect_->dispatcher             = dispatchProc;
	effect_->getParameter           = getParameterProc;
	effect_->setParameter           = setParameterProc;
	effect_->__processDeprecated    = nullptr;
	effect_->processReplacing       = processReplacingProc;
	effect_->processDoubleReplacing = processDoubleReplacingProc;
	effect_->flags                  = info->flags;
	effect_->numPrograms            = info->programCount;
	effect_->numParams              = info->paramCount;
	effect_->numInputs              = info->inputCount;
	effect_->numOutputs             = info->outputCount;
	effect_->uniqueID               = info->uniqueId;
	effect_->version                = info->version;

	DEBUG("VST plugin summary:");
	DEBUG("  flags:         0x%08X", effect_->flags);
	DEBUG("  program count: %d",     effect_->numPrograms);
	DEBUG("  param count:   %d",     effect_->numParams);
	DEBUG("  input count:   %d",     effect_->numInputs);
	DEBUG("  output count:  %d",     effect_->numOutputs);
	DEBUG("  unique ID:     0x%08X", effect_->uniqueID);
	DEBUG("  version:       %d",     effect_->version);
}


Plugin::~Plugin()
{
	TRACE("Waiting for callback thread termination...");

	processCallbacks_.clear();
	if(callbackThread_.joinable())
		callbackThread_.join();

	controlPort_.disconnect();
	callbackPort_.disconnect();
	audioPort_.disconnect();

	TRACE("Waiting for child process termination...");

	int status;
	waitpid(childPid_, &status, 0);

	if(effect_)
		delete effect_;

	TRACE("Plugin endpoint terminated");
}


AEffect* Plugin::effect()
{
	return effect_;
}


void Plugin::callbackThread()
{
	TRACE("Callback thread started");

	condition_.post();

	while(processCallbacks_.test_and_set()) {
		if(callbackPort_.waitRequest(100)) {
			DataFrame* frame = callbackPort_.frame<DataFrame>();
			frame->value = handleAudioMaster();
			callbackPort_.sendResponse();
		}
	}

	TRACE("Callback thread terminated");
}


intptr_t Plugin::handleAudioMaster()
{
	DataFrame* frame = callbackPort_.frame<DataFrame>();

	if(frame->opcode != audioMasterGetTime && frame->opcode != audioMasterIdle) {
		FLOOD("(%p) handleAudioMaster(opcode: %s, index: %d, value: %d, opt: %g)",
				std::this_thread::get_id(), kAudioMasterEvents[frame->opcode],
				frame->index, frame->value, frame->opt);
	}

	switch(frame->opcode) {
	case audioMasterVersion:
	case audioMasterAutomate:
	case __audioMasterWantMidiDeprecated:
	case audioMasterIdle:
	case audioMasterBeginEdit:
	case audioMasterEndEdit:
	case audioMasterUpdateDisplay:
	case audioMasterGetVendorVersion:
	case audioMasterIOChanged:
	case audioMasterSizeWindow:
	case audioMasterGetInputLatency:
	case audioMasterGetOutputLatency:
	case audioMasterGetCurrentProcessLevel:
	case audioMasterGetAutomationState:
		return masterProc_(effect_, frame->opcode, frame->index, frame->value,	nullptr,
				frame->opt);

	case audioMasterGetVendorString:
	case audioMasterGetProductString:
	case audioMasterCanDo:
		return masterProc_(effect_, frame->opcode, frame->index, frame->value,
				frame->data, frame->opt);

	case audioMasterGetTime: {
		intptr_t value = masterProc_(effect_, frame->opcode, frame->index, frame->value,
				nullptr, frame->opt);

		VstTimeInfo* timeInfo = reinterpret_cast<VstTimeInfo*>(value);
		if(timeInfo) {
			std::memcpy(frame->data, timeInfo, sizeof(VstTimeInfo));
			return 1;
		}

		return 0; }

	case audioMasterProcessEvents: {
		VstEvent* events = reinterpret_cast<VstEvent*>(frame->data);
		events_.reload(frame->index, events);
		VstEvents* e = events_.events();

		return masterProc_(effect_, frame->opcode, 0, 0, e, 0.0f); }
	}

	ERROR("Unhandled audio master event: %s", kAudioMasterEvents[frame->opcode]);
	return 0;
}


intptr_t Plugin::dispatch(DataPort* port, i32 opcode, i32 index, intptr_t value,
		void* ptr, float opt)
{
	if(opcode != effCanBeAutomated && opcode != effGetProgramNameIndexed &&
			opcode != effEditIdle && opcode != effGetParamDisplay &&
			opcode != effGetParamLabel && opcode != effGetParameterProperties) {
		FLOOD("dispatch: %s", kDispatchEvents[opcode]);
	}

	DataFrame* frame = port->frame<DataFrame>();
	frame->command = Command::Dispatch;
	frame->opcode  = opcode;
	frame->index   = index;
	frame->value   = value;
	frame->opt     = opt;

	switch(opcode) {

	// We will not transmit effEditIdle event because the host endpoint processes window
	// events continuously in its main thread.
	case effEditIdle:
		return 1;

	case effGetVstVersion:
	case effOpen:
	case effGetPlugCategory:
	case effSetSampleRate:
	case effGetVendorVersion:
	case effEditClose:
	case effMainsChanged:
	case effCanBeAutomated:
	case effGetProgram:
	case effStartProcess:
	case effSetProgram:
	case effBeginSetProgram:
	case effEndSetProgram:
	case effStopProcess:
	case effGetNumMidiInputChannels:
	case effGetNumMidiOutputChannels:
	case effSetPanLaw:
	case effGetTailSize:
		port->sendRequest();
		port->waitResponse();
		return frame->value;

	case effClose:
		port->sendRequest();
		port->waitResponse();

		TRACE("Closing plugin");
		delete this;
		loggerFree();
		return 1;

	case effSetBlockSize: {
		DEBUG("Setting block size to %d frames", value);
		audioPort_.disconnect();

		size_t frameSize = sizeof(DataFrame) + sizeof(double) *
				(value * effect_->numInputs + value * effect_->numOutputs);

		if(!audioPort_.create(frameSize)) {
			ERROR("Unable to create audio port");
			return 0;
		}

		frame->index = audioPort_.id();
		port->sendRequest();
		port->waitResponse();
		return frame->value; }

	case effEditOpen: {
		Display* display = XOpenDisplay(nullptr);
		Window parent = reinterpret_cast<Window>(ptr);

		port->sendRequest();
		port->waitResponse();

		union Cast {
			u8* data;
			ERect* rect;
		};

		Cast cast;
		cast.data = frame->data;
		rect_ = *cast.rect;

		int width = rect_.right - rect_.left;
		int height = rect_.bottom - rect_.top;

		DEBUG("Requested window size: %dx%d", width, height);

		XResizeWindow(display, parent, width, height);
		XSync(display, false);

		// FIXME without this delay, the VST window sometimes stays black.
		usleep(100000);

		Window child = frame->value;
		XReparentWindow(display, child, parent, 0, 0);

		sendXembedMessage(display, child, XEMBED_EMBEDDED_NOTIFY, 0, parent, 0);
		sendXembedMessage(display, child, XEMBED_FOCUS_OUT, 0, 0, 0);

		frame->command = Command::ShowWindow;
		port->sendRequest();
		port->waitResponse();

		// FIXME without this delay, the VST window sometimes stays black.
		usleep(100000);

		XMapWindow(display, child);
		XSync(display, false);

		XCloseDisplay(display);

		return frame->value; }

	case effEditGetRect: {
		port->sendRequest();
		port->waitResponse();

		union Cast {
			u8* data;
			ERect* rect;
		};

		Cast cast;
		cast.data = frame->data;
		rect_ = *cast.rect;

		ERect** rectPtr = static_cast<ERect**>(ptr);
		*rectPtr = &rect_;
		return frame->value; }

	case effCanDo: {
		const char* source = static_cast<const char*>(ptr);
		char* dest         = reinterpret_cast<char*>(frame->data);
		size_t maxLength   = port->frameSize() - sizeof(DataFrame);

		std::strncpy(dest, source, maxLength);
		dest[maxLength-1] = '\0';

		port->sendRequest();
		port->waitResponse();
		return frame->value; }

	case effGetProgramName: {
		port->sendRequest();
		port->waitResponse();

		const char* source = reinterpret_cast<const char*>(frame->data);
		char* dest         = static_cast<char*>(ptr);

		std::strncpy(dest, source, kVstMaxProgNameLen);
		dest[kVstMaxProgNameLen-1] = '\0';
		return frame->value; }

	case effSetProgramName: {
		const char* source = static_cast<const char*>(ptr);
		char* dest         = reinterpret_cast<char*>(frame->data);

		std::strncpy(dest, source, kVstMaxProgNameLen);
		dest[kVstMaxProgNameLen-1] = '\0';

		port->sendRequest();
		port->waitResponse();
		return frame->value; }

	case effGetVendorString:
	case effGetProductString: {
		port->sendRequest();
		port->waitResponse();

		const char* source = reinterpret_cast<const char*>(frame->data);
		char* dest         = static_cast<char*>(ptr);

		std::strncpy(dest, source, kVstMaxVendorStrLen);
		dest[kVstMaxVendorStrLen-1] = '\0';
		return frame->value; }

	case effGetParamName:
	case effGetParamLabel:
	case effGetParamDisplay: {
		port->sendRequest();
		port->waitResponse();

		const char* source = reinterpret_cast<const char*>(frame->data);
		char* dest         = static_cast<char*>(ptr);

		std::strncpy(dest, source, kVstMaxParamStrLen);
		dest[kVstMaxParamStrLen-1] = '\0';
		return frame->value; }

	case effGetEffectName: {
		port->sendRequest();
		port->waitResponse();

		const char* source = reinterpret_cast<const char*>(frame->data);
		char* dest         = static_cast<char*>(ptr);

		std::strncpy(dest, source, kVstMaxEffectNameLen);
		dest[kVstMaxEffectNameLen-1] = '\0';
		return frame->value; }

	case effGetParameterProperties:
		port->sendRequest();
		port->waitResponse();

		std::memcpy(ptr, frame->data, sizeof(VstParameterProperties));
		return frame->value;

	case effGetOutputProperties:
	case effGetInputProperties:
		port->sendRequest();
		port->waitResponse();

		std::memcpy(ptr, frame->data, sizeof(VstPinProperties));
		return frame->value;

	case effGetProgramNameIndexed: {
		port->sendRequest();
		port->waitResponse();

		const char* source = reinterpret_cast<const char*>(frame->data);
		char* dest         = static_cast<char*>(ptr);

		std::strncpy(dest, source, kVstMaxProgNameLen);
		dest[kVstMaxProgNameLen-1] = '\0';
		return frame->value; }

	case effGetMidiKeyName:
		port->sendRequest();
		port->waitResponse();

		std::memcpy(ptr, frame->data, sizeof(MidiKeyName));
		return frame->value;

	case effProcessEvents: {
		VstEvents* events = static_cast<VstEvents*>(ptr);
		VstEvent* event = reinterpret_cast<VstEvent*>(frame->data);
		frame->index = events->numEvents;

		for(int i = 0; i < events->numEvents; ++i)
			event[i] = *events->events[i];

		port->sendRequest();
		port->waitResponse();
		return frame->value; }

	case effGetChunk: {
		DEBUG("effGetChunk");

		// Tell the block size to the host endpoint.
		ptrdiff_t blockSize = port->frameSize() - sizeof(DataFrame);
		frame->value = blockSize;

		port->sendRequest();
		port->waitResponse();

		DEBUG("effGetChunk: chunk size %d bytes", frame->value);

		// If VST plugin supports the effGetChunk event, it has placed first data block
		// (or even the entire chunk) in the frame buffer.
		size_t chunkSize = frame->value;
		size_t count = frame->index;

		if(chunkSize == 0 || count == 0) {
			ERROR("effGetChunk is unsupported by the VST plugin");
			return 0;
		}

		chunk_.resize(chunkSize);

		auto it = chunk_.begin();
		it = std::copy(frame->data, frame->data + count, it);

		while(it != chunk_.end()) {
			frame->command = Command::GetDataBlock;
			frame->index = std::min(blockSize, chunk_.end() - it);

			DEBUG("effGetChunk: requesting next %d bytes", frame->index);

			port->sendRequest();
			port->waitResponse();

			size_t count = frame->index;
			if(count == 0) {
				ERROR("effGetChunk: premature end of data transmission");
				return 0;
			}

			it = std::copy(frame->data, frame->data + count, it);
		}

		DEBUG("effGetChunk: received %d bytes", chunkSize);

		void** chunk = static_cast<void**>(ptr);
		*chunk = static_cast<void*>(chunk_.data());
		return chunkSize; }

	case effSetChunk: {
		DEBUG("effSetChunk: %d bytes", frame->value);

		size_t chunkSize = frame->value;
		bool isPreset = frame->index;
		data_ = static_cast<uint8_t*>(ptr);
		dataLength_ = frame->value;
		size_t blockSize = port->frameSize() - sizeof(DataFrame);

		while(dataLength_) {
			frame->command = Command::SetDataBlock;
			size_t count = std::min(blockSize, dataLength_);
			frame->index = count;
			std::memcpy(frame->data, data_, count);

			DEBUG("effSetChunk: sending next %d bytes", count);

			port->sendRequest();
			port->waitResponse();

			data_ += count;
			dataLength_ -= count;
		}

		frame->command = Command::Dispatch;
		frame->opcode = effSetChunk;
		frame->index = isPreset;

		port->sendRequest();
		port->waitResponse();

		DEBUG("effSetChunk: sent %d bytes", chunkSize);

		return frame->value; }

	case effBeginLoadBank:
	case effBeginLoadProgram:
		std::memcpy(frame->data, ptr, sizeof(VstPatchChunkInfo));
		port->sendRequest();
		port->waitResponse();
		return frame->value;
	}

	ERROR("Unhandled dispatch event: %s", kDispatchEvents[opcode]);
	return 0;
}


void Plugin::sendXembedMessage(Display* display, Window window, long message, long detail,
		long data1, long data2)
{
	XEvent event;

	memset(&event, 0, sizeof(event));
	event.xclient.type = ClientMessage;
	event.xclient.window = window;
	event.xclient.message_type = XInternAtom(display, "_XEMBED", false);
	event.xclient.format = 32;
	event.xclient.data.l[0] = CurrentTime;
	event.xclient.data.l[1] = message;
	event.xclient.data.l[2] = detail;
	event.xclient.data.l[3] = data1;
	event.xclient.data.l[4] = data2;

	XSendEvent(display, window, false, NoEventMask, &event);
	XSync(display, false);
}


float Plugin::getParameter(i32 index)
{
	DataFrame* frame = controlPort_.frame<DataFrame>();
	frame->command = Command::GetParameter;
	frame->index = index;

	controlPort_.sendRequest();
	controlPort_.waitResponse();
	return frame->opt;
}


void Plugin::setParameter(i32 index, float value)
{
	DataFrame* frame = controlPort_.frame<DataFrame>();
	frame->command = Command::SetParameter;
	frame->index = index;
	frame->opt = value;

	controlPort_.sendRequest();
	controlPort_.waitResponse();
}


void Plugin::processReplacing(float** inputs, float** outputs, i32 count)
{
	DataFrame* frame = audioPort_.frame<DataFrame>();
	frame->command = Command::ProcessSingle;
	frame->value = count;
	float* data = reinterpret_cast<float*>(frame->data);

	for(int i = 0; i < effect_->numInputs; ++i) {
		std::memcpy(data, inputs[i], sizeof(float) * count);
		data += count;
	}

	audioPort_.sendRequest();
	audioPort_.waitResponse();

	data = reinterpret_cast<float*>(frame->data);

	for(int i = 0; i < effect_->numOutputs; ++i) {
		std::memcpy(outputs[i], data, sizeof(float) * count);
		data += count;
	}
}


void Plugin::processDoubleReplacing(double** inputs, double** outputs, i32 count)
{
	DataFrame* frame = audioPort_.frame<DataFrame>();
	frame->command = Command::ProcessDouble;
	frame->value = count;
	double* data = reinterpret_cast<double*>(frame->data);

	for(int i = 0; i < effect_->numInputs; ++i)
		data = std::copy(inputs[i], inputs[i] + count, data);

	audioPort_.sendRequest();
	audioPort_.waitResponse();

	data = reinterpret_cast<double*>(frame->data);

	for(int i = 0; i < effect_->numOutputs; ++i)
		data = std::copy(outputs[i], outputs[i] + count, data);
}


intptr_t Plugin::dispatchProc(AEffect* effect, i32 opcode, i32 index, intptr_t value,
		void* ptr, float opt)
{
	// Most of VST hosts send some dispatch events in separate threads. So, if the
	// current thread is different than the main thread, we will send this event through
	// the audio port for processing it inside the dedicated audio thread by the host
	// endpoint.

	Plugin* plugin = static_cast<Plugin*>(effect->object);
	DataPort* port;
	RecursiveMutex* guard;

	if(std::this_thread::get_id() == plugin->mainThreadId_) {
		port = &plugin->controlPort_;
		guard = &plugin->guard_;
	}
	else {
		port = &plugin->audioPort_;
		guard = &plugin->audioGuard_;
	}

	RecursiveLock lock(*guard);
	return plugin->dispatch(port, opcode, index, value, ptr, opt);
}


float Plugin::getParameterProc(AEffect* effect, i32 index)
{
	Plugin* plugin = static_cast<Plugin*>(effect->object);
	RecursiveLock lock(plugin->guard_);
	return plugin->getParameter(index);
}


void Plugin::setParameterProc(AEffect* effect, i32 index, float value)
{
	Plugin* plugin = static_cast<Plugin*>(effect->object);
	RecursiveLock lock(plugin->guard_);
	plugin->setParameter(index, value);
}


void Plugin::processReplacingProc(AEffect* effect, float** inputs, float** outputs,
		i32 sampleCount)
{
	Plugin* plugin = static_cast<Plugin*>(effect->object);
	RecursiveLock lock(plugin->audioGuard_);
	plugin->processReplacing(inputs, outputs, sampleCount);
}


void Plugin::processDoubleReplacingProc(AEffect* effect, double** inputs,
		double** outputs, i32 sampleCount)
{
	Plugin* plugin = static_cast<Plugin*>(effect->object);
	RecursiveLock lock(plugin->audioGuard_);
	plugin->processDoubleReplacing(inputs, outputs, sampleCount);
}


} // namespace Airwave
