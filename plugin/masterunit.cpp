#include "masterunit.h"

#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "config.h"
#include "common/filesystem.h"
#include "common/logger.h"
#include "common/protocol.h"


#define XEMBED_EMBEDDED_NOTIFY 0
#define XEMBED_FOCUS_OUT       5


namespace Airwave {


MasterUnit::MasterUnit(const std::string& pluginPath,
		const std::string& hostPath, AudioMasterProc masterProc) :
	masterProc_(masterProc),
	data_(nullptr),
	dataLength_(0),
	childPid_(-1),
	processCallbacks_(ATOMIC_FLAG_INIT),
	mainThreadId_(std::this_thread::get_id())
{
	// The constructor will return if it will find a error during his work. If
	// object was constructed successfully, then effect() returns a valid
	// pointer. In other case nullptr is returned.

	LOG("Main thread id: %p", mainThreadId_);

	// FIXME: frame size should be verified.
	if(!controlPort_.create(10240)) {
		LOG("Unable to create control port");
		return;
	}

	// FIXME: frame size should be verified.
	if(!callbackPort_.create(1024)) {
		LOG("Unable to create callback port");
		controlPort_.disconnect();
		return;
	}

	// Start the slave unit's process.
	childPid_ = fork();
	if(childPid_ == -1) {
		LOG("fork() call failed");
		controlPort_.disconnect();
		callbackPort_.disconnect();
		return;
	}
	else if(childPid_ == 0) {
		const char* wineLoader = getenv("WINELOADER");

		if(wineLoader) {
			LOG("WINELOADER: %s", wineLoader);
		}
		else {
			LOG("WINELOADER is not set, using wine from the PATH");
		}

		std::string id = std::to_string(controlPort_.id());

		execl("/bin/sh", "/bin/sh", hostPath.c_str(), pluginPath.c_str(),
				id.c_str(), nullptr);

		// We should never reach this point on success child execution.
		LOG("execl() call failed");
		return;
	}

	LOG("Child process started, pid=%d", childPid_);

	std::memset(&rect_, 0, sizeof(ERect));

	processCallbacks_.test_and_set();
	callbackThread_ = std::thread(&MasterUnit::callbackThread, this);

	condition_.wait();

	// Send host info to the slave unit.
	DataFrame* frame = controlPort_.frame<DataFrame>();
	frame->command = Command::HostInfo;
	frame->opcode = callbackPort_.id();
	frame->value = PROTOCOL_VERSION;
	controlPort_.sendRequest();

	LOG("Waiting for child response...");

	// Wait for the slave unit initialization.
	if(!controlPort_.waitResponse()) {
		LOG("Child process is not responding");
		kill(childPid_, SIGKILL);
		controlPort_.disconnect();
		callbackPort_.disconnect();
		childPid_ = -1;
		return;
	}

	// Slave unit should place his protocol version to frame->value.
	if(frame->value != PROTOCOL_VERSION) {
		// We cannot talk to the slave unit if his protocol differs from ours.
		// Note that we doesn't send SIGKILL to the child process here, because
		// we assume, that child also checks our protocol version and terminates
		// if it differs.
		LOG("Slave unit has incompatible protocol version: %d", frame->index);
		controlPort_.disconnect();
		callbackPort_.disconnect();
		childPid_ = -1;
		return;
	}

	PluginInfo* info = reinterpret_cast<PluginInfo*>(frame->data);
	std::memset(&effect_, 0, sizeof(effect_));

	effect_.magic                  = kEffectMagic;
	effect_.object                 = this;
	effect_.dispatcher             = dispatchProc;
	effect_.getParameter           = getParameterProc;
	effect_.setParameter           = setParameterProc;
	effect_.__processDeprecated    = nullptr;
	effect_.processReplacing       = processReplacingProc;
	effect_.processDoubleReplacing = processDoubleReplacingProc;
	effect_.flags                  = info->flags;
	effect_.numPrograms            = info->programCount;
	effect_.numParams              = info->paramCount;
	effect_.numInputs              = info->inputCount;
	effect_.numOutputs             = info->outputCount;
	effect_.uniqueID               = info->uniqueId;
	effect_.version                = info->version;

	LOG("VST plugin summary:");
	LOG("  flags:         0x%08X", effect_.flags);
	LOG("  program count: %d",     effect_.numPrograms);
	LOG("  param count:   %d",     effect_.numParams);
	LOG("  input count:   %d",     effect_.numInputs);
	LOG("  output count:  %d",     effect_.numOutputs);
	LOG("  unique ID:     0x%08X", effect_.uniqueID);
	LOG("  version:       %d",     effect_.version);
}


MasterUnit::~MasterUnit()
{
	LOG("Waiting for callback thread termination...");

	processCallbacks_.clear();
	if(callbackThread_.joinable())
		callbackThread_.join();

	controlPort_.disconnect();
	callbackPort_.disconnect();
	audioPort_.disconnect();

	LOG("Waiting for child process termination...");

	int status;
	waitpid(childPid_, &status, 0);

	LOG("Master unit terminated");
}


AEffect* MasterUnit::effect()
{
	if(childPid_ == -1)
		return nullptr;

	return &effect_;
}


void MasterUnit::callbackThread()
{
	LOG("Callback thread started");

	condition_.post();

	while(processCallbacks_.test_and_set()) {
		if(callbackPort_.waitRequest(100)) {
			DataFrame* frame = callbackPort_.frame<DataFrame>();
			frame->value = handleAudioMaster();
			callbackPort_.sendResponse();
		}
	}

	LOG("Callback thread terminated");
}


intptr_t MasterUnit::handleAudioMaster()
{
	DataFrame* frame = callbackPort_.frame<DataFrame>();

//	if(frame->opcode != audioMasterGetTime &&
//			frame->opcode != audioMasterIdle) {
//		LOG("(%p) handleAudioMaster(opcode: %s, index: %d, value: %d, opt: %g)",
//				std::this_thread::get_id(), kAudioMasterEvents[frame->opcode],
//				frame->index, frame->value, frame->opt);
//	}

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
		return masterProc_(&effect_, frame->opcode, frame->index, frame->value,
				nullptr, frame->opt);

	case audioMasterGetVendorString:
	case audioMasterGetProductString:
	case audioMasterCanDo:
		return masterProc_(&effect_, frame->opcode, frame->index,
						frame->value, frame->data, frame->opt);

	case audioMasterGetTime: {
		intptr_t value = masterProc_(&effect_, frame->opcode, frame->index,
				frame->value, nullptr, frame->opt);

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

		return masterProc_(&effect_, frame->opcode, 0, 0, e, 0.0f); }
	}

	LOG("Unhandled audio master event: %s", kAudioMasterEvents[frame->opcode]);
	return 0;
}


intptr_t MasterUnit::dispatch(DataPort* port, int32_t opcode, int32_t index,
		intptr_t value, void* ptr, float opt)
{
/*	if(opcode != effCanBeAutomated && opcode != effGetProgramNameIndexed &&
			opcode != effEditIdle && opcode != effGetParamDisplay &&
			opcode != effGetParamLabel && opcode != effGetParameterProperties) {
		LOG("dispatch: %s", kDispatchEvents[opcode]);
	}*/

	DataFrame* frame = port->frame<DataFrame>();
	frame->command = Command::Dispatch;
	frame->opcode  = opcode;
	frame->index   = index;
	frame->value   = value;
	frame->opt     = opt;

	switch(opcode) {

	// We will not transmit effEditIdle event because slave unit processes
	// window events continuously in his main thread.
	case effEditIdle:
//		return 1;

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
		port->sendRequest();
		port->waitResponse();
		return frame->value;

	case effClose:
		port->sendRequest();
		port->waitResponse();

		LOG("Closing plugin");
		delete this;
		loggerFree();
		return 1;

	case effSetBlockSize: {
		LOG("Setting block size to %d frames", value);
		audioPort_.disconnect();

		size_t frameSize = sizeof(DataFrame) + sizeof(double) *
				(value * effect_.numInputs + value * effect_.numOutputs);

		if(!audioPort_.create(frameSize)) {
			LOG("Unable to create audio port");
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

		rect_ = *reinterpret_cast<ERect*>(frame->data);
		XResizeWindow(display, parent, rect_.right - rect_.left,
				rect_.bottom - rect_.top);

		XSync(display, false);

		// FIXME without this ugly delay, the VST window sometimes stays black.
		usleep(100000);

		Window child = frame->value;
		XReparentWindow(display, child, parent, 0, 0);

		sendXembedMessage(display, child, XEMBED_EMBEDDED_NOTIFY, 0, parent, 0);
		sendXembedMessage(display, child, XEMBED_FOCUS_OUT, 0, 0, 0);

		frame->command = Command::ShowWindow;
		port->sendRequest();
		port->waitResponse();

		// FIXME without this ugly delay, the VST window sometimes stays black.
		usleep(100000);

		XMapWindow(display, child);
		XSync(display, false);

		XCloseDisplay(display);

		return frame->value; }

	case effEditGetRect: {
		port->sendRequest();
		port->waitResponse();

		rect_ = *reinterpret_cast<ERect*>(frame->data);
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
		LOG("effGetChunk");

		// Tell the slave unit our block size.
		ptrdiff_t blockSize = port->frameSize() - sizeof(DataFrame);
		frame->value = blockSize;

		port->sendRequest();
		port->waitResponse();

		LOG("effGetChunk: chunk size %d bytes", frame->value);


		// If VST plugin supports the effGetChunk event, it has placed first
		// data block (or even the entire chunk) in the frame buffer.
		size_t chunkSize = frame->value;
		size_t count = frame->index;

		if(chunkSize == 0 || count == 0) {
			LOG("effGetChunk is unsupported by the VST plugin");
			return 0;
		}

		chunk_.resize(chunkSize);

		auto it = chunk_.begin();
		it = std::copy(frame->data, frame->data + count, it);

		while(it != chunk_.end()) {
			frame->command = Command::GetDataBlock;
			frame->index = std::min(blockSize, chunk_.end() - it);

//			LOG("effSetChunk: requesting next %d bytes", frame->index);

			port->sendRequest();
			port->waitResponse();

			size_t count = frame->index;
			if(count == 0) {
				LOG("effGetChunk: premature end of data transmission");
				return 0;
			}

			it = std::copy(frame->data, frame->data + count, it);
		}

		LOG("effGetChunk: received %d bytes", chunkSize);

		void** chunk = static_cast<void**>(ptr);
		*chunk = static_cast<void*>(chunk_.data());
		return chunkSize; }

	case effSetChunk: {
		LOG("effSetChunk: %d bytes", frame->value);

		bool isPreset = frame->index;
		data_ = static_cast<uint8_t*>(ptr);
		dataLength_ = frame->value;
		size_t blockSize = port->frameSize() - sizeof(DataFrame);

		while(dataLength_) {
			frame->command = Command::SetDataBlock;
			size_t count = std::min(blockSize, dataLength_);
			frame->index = count;
			std::memcpy(frame->data, data_, count);

//			LOG("effSetChunk: sending next %d bytes", count);

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

		return frame->value; }

	case effBeginLoadBank:
	case effBeginLoadProgram:
		std::memcpy(frame->data, ptr, sizeof(VstPatchChunkInfo));
		port->sendRequest();
		port->waitResponse();
		return frame->value;
	}

	LOG("Unhandled dispatch event: %s", kDispatchEvents[opcode]);
	return 0;
}


void MasterUnit::sendXembedMessage(Display* display, Window window,
		long message, long detail, long data1, long data2)
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


float MasterUnit::getParameter(int32_t index)
{
	DataFrame* frame = controlPort_.frame<DataFrame>();
	frame->command = Command::GetParameter;
	frame->index = index;

	controlPort_.sendRequest();
	controlPort_.waitResponse();
	return frame->opt;
}


void MasterUnit::setParameter(int32_t index, float value)
{
	DataFrame* frame = controlPort_.frame<DataFrame>();
	frame->command = Command::SetParameter;
	frame->index = index;
	frame->opt = value;

	controlPort_.sendRequest();
	controlPort_.waitResponse();
}


void MasterUnit::processReplacing(float** inputs, float** outputs,
		int32_t count)
{
	DataFrame* frame = audioPort_.frame<DataFrame>();
	frame->command = Command::ProcessSingle;
	frame->value = count;
	float* data = reinterpret_cast<float*>(frame->data);

	for(int i = 0; i < effect_.numInputs; ++i) {
		std::memcpy(data, inputs[i], sizeof(float) * count);
		data += count;
	}

	audioPort_.sendRequest();
	audioPort_.waitResponse();

	data = reinterpret_cast<float*>(frame->data);

	for(int i = 0; i < effect_.numOutputs; ++i) {
		std::memcpy(outputs[i], data, sizeof(float) * count);
		data += count;
	}
}


void MasterUnit::processDoubleReplacing(double** inputs, double** outputs,
		int32_t count)
{
	DataFrame* frame = audioPort_.frame<DataFrame>();
	frame->command = Command::ProcessDouble;
	frame->value = count;
	double* data = reinterpret_cast<double*>(frame->data);

	for(int i = 0; i < effect_.numInputs; ++i)
		data = std::copy(inputs[i], inputs[i] + count, data);

	audioPort_.sendRequest();
	audioPort_.waitResponse();

	data = reinterpret_cast<double*>(frame->data);

	for(int i = 0; i < effect_.numOutputs; ++i)
		data = std::copy(outputs[i], outputs[i] + count, data);
}


intptr_t MasterUnit::dispatchProc(AEffect* effect, int32_t opcode,
		int32_t index, intptr_t value, void* ptr, float opt)
{
//	if(opcode != effCanBeAutomated && opcode != effGetProgramNameIndexed &&
//			opcode != effEditIdle && opcode != effGetParamDisplay &&
//			opcode != effGetParamLabel && opcode != effGetParameterProperties) {
		LOG("(%p) dispatch: %s", std::this_thread::get_id(),
			kDispatchEvents[opcode]);
//	}

	// Most of VST hosts send some dispatch events in separate threads. So, if
	// the current thread is different than the main thread, we will send this
	// event through the audio port for processing it inside the dedicated audio
	// thread by the slave unit.

	MasterUnit* plugin = static_cast<MasterUnit*>(effect->object);
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


float MasterUnit::getParameterProc(AEffect* effect, int32_t index)
{
	MasterUnit* plugin = static_cast<MasterUnit*>(effect->object);
	RecursiveLock lock(plugin->guard_);
	return plugin->getParameter(index);
}


void MasterUnit::setParameterProc(AEffect* effect, int32_t index,
		float value)
{
	MasterUnit* plugin = static_cast<MasterUnit*>(effect->object);
	RecursiveLock lock(plugin->guard_);
	plugin->setParameter(index, value);
}


void MasterUnit::processReplacingProc(AEffect* effect, float** inputs,
		float** outputs, int32_t sampleCount)
{
	MasterUnit* plugin = static_cast<MasterUnit*>(effect->object);
	RecursiveLock lock(plugin->audioGuard_);
	plugin->processReplacing(inputs, outputs, sampleCount);
}


void MasterUnit::processDoubleReplacingProc(AEffect* effect,
		double** inputs, double** outputs, int32_t sampleCount)
{
	MasterUnit* plugin = static_cast<MasterUnit*>(effect->object);
	RecursiveLock lock(plugin->audioGuard_);
	plugin->processDoubleReplacing(inputs, outputs, sampleCount);
}


} // namespace Airwave
