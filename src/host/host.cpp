#include "host.h"

#include <cstring>
#include "common/logger.h"
#include "common/protocol.h"


namespace Airwave {


Host* Host::self_ = nullptr;


Host::Host() :
	isInitialized_(false),
	hwnd_(0),
	data_(nullptr),
	dataLength_(0),
	runAudio_(ATOMIC_FLAG_INIT),
	isEditorOpen_(false),
	oldWndProc_(nullptr),
	childHwnd_(0)
{
	DEBUG("Main thread id: %p", GetCurrentThreadId());
}


Host::~Host()
{
	if(isInitialized_) {
		TRACE("Waiting for audio thread termination...");

		runAudio_.clear();
		WaitForSingleObject(audioThread_, INFINITE);

		destroyEditorWindow();

		DeleteCriticalSection(&cs_);
		FreeLibrary(module_);
	}
}


bool Host::initialize(const char* fileName, int portId)
{
	if(isInitialized_) {
		TRACE("Host endpoint is already initialized");
		return false;
	}

	module_ = LoadLibrary(fileName);
	if(!module_) {
		ERROR("Unable to load '%s' shared library: %s", fileName, errorString().c_str());
		return false;
	}

	if(!InitializeCriticalSectionAndSpinCount(&cs_, 0x00010000))  {
		FreeLibrary(module_);
		return false;
	}

	VstPluginMainProc vstMainProc = reinterpret_cast<VstPluginMainProc>(
			GetProcAddress(module_, "VSTPluginMain"));

	if(!vstMainProc) {
		vstMainProc = reinterpret_cast<VstPluginMainProc>(
				GetProcAddress(module_, "main"));

		if(!vstMainProc) {
			ERROR("The %s is not a VST plugin");
			DeleteCriticalSection(&cs_);
			FreeLibrary(module_);
			return false;
		}
	}

	if(!controlPort_.connect(portId)) {
		ERROR("Unable to connect control port (id = %d)", portId);
		DeleteCriticalSection(&cs_);
		FreeLibrary(module_);
		return false;
	}

	TRACE("Waiting for plugin endpoint request...");

	if(!controlPort_.waitRequest()) {
		ERROR("Unable to get initial request from plugin endpoint");
		controlPort_.disconnect();
		DeleteCriticalSection(&cs_);
		FreeLibrary(module_);
		return false;
	}

	TRACE("Request from plugin endpoint received, sending response");

	DataFrame* frame = controlPort_.frame<DataFrame>();
	if(!callbackPort_.connect(frame->opcode)) {
		ERROR("Unable to connect callback port (id = %d)", frame->opcode);
		controlPort_.disconnect();
		DeleteCriticalSection(&cs_);
		FreeLibrary(module_);
		return false;
	}

	// When we call vstMainProc(), the audioMasterProc() can be called from there with
	// effect argument set to the nullptr. This is because VST plugin object is not yet
	// initialized at this point. Since we need the pointer to our object inside of
	// audioMasterProc(), we must store it in some accessible place. Static member
	// pointer is very ugly, but very effective solution, since we don't need more than
	// one instance of host endpoint anyway.
	self_ = this;

	TRACE("Initializing VST plugin...");

	effect_ = vstMainProc(audioMasterProc);
	if(!effect_ || effect_->magic != kEffectMagic) {
		ERROR("Unable to initialize VST plugin");
		controlPort_.disconnect();
		callbackPort_.disconnect();
		DeleteCriticalSection(&cs_);
		FreeLibrary(module_);
		return false;
	}

	TRACE("VST plugin is initialized");

	std::memset(&timeInfo_, 0, sizeof(VstTimeInfo));

	frame->command = Command::PluginInfo;
	PluginInfo* info = reinterpret_cast<PluginInfo*>(frame->data);

	info->flags        = effect_->flags;
	info->programCount = effect_->numPrograms;
	info->paramCount   = effect_->numParams;
	info->inputCount   = effect_->numInputs;
	info->outputCount  = effect_->numOutputs;
	info->initialDelay = effect_->initialDelay;
	info->uniqueId     = effect_->uniqueID;
	info->version      = effect_->version;

	// Workaround for plugins from Waves
	char vendorName[kVstMaxVendorStrLen];
	if(effect_->dispatcher(effect_, effGetVendorString, 0, 0, &vendorName, 0.0f)) {
		if(strncmp(vendorName, "Waves", kVstMaxVendorStrLen) == 0)
			info->flags |= effFlagsHasEditor;
	}

	controlPort_.sendResponse();

	isInitialized_ = true;
	return true;
}


bool Host::processRequest()
{
	if(!controlPort_.isConnected()) {
		TRACE("Control port isn't connected anymore, exiting");
		return false;
	}

	if(!controlPort_.waitRequest(10))
		return true;

	bool result = true;
	DataFrame* frame = controlPort_.frame<DataFrame>();

	switch(frame->command) {
	case Command::Dispatch:
		result = handleDispatch(frame);
		break;

	case Command::GetDataBlock:
		handleGetDataBlock(frame);
		break;

	case Command::SetDataBlock:
		handleSetDataBlock(frame);
		break;

	case Command::ShowWindow: {
		if(hwnd_) {
			ShowWindow(hwnd_, SW_SHOW);
			UpdateWindow(hwnd_);
		}
		break; }

	default:
		ERROR("processRequest() unacceptable command: %d", frame->command);
		break;
	}

	frame->command = Command::Response;
	controlPort_.sendResponse();
	return result;
}


std::string Host::errorString() const
{
	DWORD error = GetLastError();

	if(error) {
		LPVOID buffer;
		DWORD length = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
				error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				reinterpret_cast<LPTSTR>(&buffer), 0, nullptr);

		if(length) {
			LPCSTR message = static_cast<LPCSTR>(buffer);
			std::string result(message, message + length);

			LocalFree(buffer);
			return result;
		}
	}

	return std::string();
}


void Host::destroyEditorWindow()
{
	if(hwnd_) {
		KillTimer(hwnd_, timerId_);
		DestroyWindow(hwnd_);
		UnregisterClass(kWindowClass, GetModuleHandle(nullptr));
		hwnd_ = 0;
	}
}


void Host::audioThread()
{
	condition_.post();

	while(runAudio_.test_and_set()) {
		if(audioPort_.waitRequest(100)) {
			DataFrame* frame = audioPort_.frame<DataFrame>();

			if(frame->command == Command::ProcessSingle) {
				handleProcessSingle();
			}
			else if(frame->command == Command::GetParameter) {
				handleGetParameter();
			}
			else if(frame->command == Command::SetParameter) {
				handleSetParameter();
			}
			else if(frame->command == Command::ProcessDouble) {
				handleProcessDouble();
			}
			else if(frame->command == Command::Dispatch) {
				handleDispatch(frame);
			}
			else {
				ERROR("audioThread() unacceptable command: %d", frame->command);
			}

			frame->command = Command::Response;
			audioPort_.sendResponse();
		}
	}
}


void Host::handleGetDataBlock(DataFrame* frame)
{
	size_t blockSize = frame->index;
	DEBUG("handleGetDataBlock: %d bytes", blockSize);
	frame->index = dataLength_ < blockSize ? dataLength_ : blockSize;
	std::copy(data_, data_ + frame->index, frame->data);
	data_ += frame->index;
	dataLength_ -= frame->index;
}


void Host::handleSetDataBlock(DataFrame* frame)
{
	chunk_.insert(chunk_.end(), frame->data, frame->data + frame->index);
}


bool Host::handleDispatch(DataFrame* frame)
{
	FLOOD("handleDispatch: %s", kDispatchEvents[frame->opcode]);

	if(isEditorOpen_ && frame->opcode != effEditIdle) {
		// Postpone the effEditIdle event by 100 milliseconds
		SetTimer(hwnd_, timerId_, 100, nullptr);
	}

	switch(frame->opcode) {
	case effClose:
		// Some stupid hosts doesn't send the effEditClose event before sending
		// the effClose event. This leads to crashes of some stupid plugins. So
		// we just emulate correct behavior here to avoid these crashes.
		if(isEditorOpen_) {
			effect_->dispatcher(effect_, effEditClose, 0, 0, nullptr, 0.0f);
			destroyEditorWindow();
			isEditorOpen_ = false;
		}

		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index,
				frame->value, nullptr, frame->opt);
		break;

	case effGetVstVersion:
	case effOpen:
	case effGetPlugCategory:
	case effSetSampleRate:
	case effGetVendorVersion:
	case effEditIdle:
	case effMainsChanged:
	case effCanBeAutomated:
	case effGetProgram:
	case effStartProcess:
	case effSetProgram:
	case effBeginSetProgram:
	case effEndSetProgram:
	case effStopProcess:
	case effGetTailSize:
	case effSetEditKnobMode:
	case __effConnectInputDeprecated:
	case __effConnectOutputDeprecated:
	case __effKeysRequiredDeprecated:
	case __effIdentifyDeprecated:
		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index,
				frame->value, nullptr, frame->opt);
		break;

	case effEditClose:
		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index,
				frame->value, nullptr, frame->opt);

		destroyEditorWindow();
		isEditorOpen_ = false;
		break;

	case effSetBlockSize:
		if(runAudio_.test_and_set()) {
			runAudio_.clear();
			WaitForSingleObject(audioThread_, INFINITE);
		}

		audioPort_.disconnect();
		if(!audioPort_.connect(frame->index)) {
			ERROR("Unable to connect audio port");
			return false;
		}

		runAudio_.test_and_set();
		audioThread_ = CreateThread(nullptr, 0, audioThreadProc, this, 0, nullptr);

		condition_.wait();

		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index,
				frame->value, nullptr, frame->opt);
		break;

	case effEditOpen: {
		WNDCLASSEX wclass;
		std::memset(&wclass, 0, sizeof(WNDCLASSEX));

		wclass.cbSize        = sizeof(WNDCLASSEX);
		wclass.style         = CS_HREDRAW | CS_VREDRAW;
		wclass.lpfnWndProc   = windowProc;
		wclass.cbClsExtra    = 0;
		wclass.cbWndExtra    = 0;
		wclass.hInstance     = GetModuleHandle(nullptr);
		wclass.hIcon         = LoadIcon(nullptr, kWindowClass);
		wclass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
		wclass.lpszClassName = kWindowClass;

		if(!RegisterClassEx(&wclass)) {
			ERROR("Unable to register window class: %s", errorString().c_str());
			return false;
		}

		hwnd_ = CreateWindowEx(WS_EX_TOOLWINDOW, kWindowClass, "Plugin", WS_POPUP, 0, 0,
				200, 200, 0, 0, GetModuleHandle(nullptr), 0);

		if(!hwnd_) {
			ERROR("Unable to create window: %s", errorString().c_str());
			UnregisterClass(kWindowClass, module_);
			return false;
		}

		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index,
				frame->value, hwnd_, frame->opt);

		// Obtain the size of the VST window in advance.
		ERect* rect = nullptr;
		effect_->dispatcher(effect_, effEditGetRect, 0, 0, &rect, 0.0f);

		RECT wndRect = { rect->left, rect->top, rect->right, rect->bottom };

		AdjustWindowRectEx(&wndRect, GetWindowLong(hwnd_, GWL_STYLE),
				GetMenu(hwnd_) != nullptr, GetWindowLong(hwnd_, GWL_EXSTYLE));

		SetWindowPos(hwnd_, 0, 0, 0, rect->right - rect->left, rect->bottom - rect->top,
				SWP_NOACTIVATE | SWP_NOMOVE);

		std::memcpy(&frame->data, rect, sizeof(ERect));

		timerId_ = SetTimer(hwnd_, 0, 100, nullptr);

		HANDLE handle = GetPropA(hwnd_, "__wine_x11_whole_window");
		frame->value = reinterpret_cast<intptr_t>(handle);

		isEditorOpen_ = true;
		break; }

	case effEditGetRect: {
		ERect* rect = nullptr;
		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index,
				frame->value, &rect, frame->opt);

		std::memcpy(&frame->data, rect, sizeof(ERect));
		break; }

	case effCanDo:
	case effGetVendorString:
	case effGetProductString:
	case effGetParamName:
	case effGetParamLabel:
	case effGetProgramNameIndexed:
	case effGetParamDisplay:
	case effGetProgramName:
	case effSetProgramName:
	case effGetParameterProperties:
	case effGetOutputProperties:
	case effGetInputProperties:
	case effGetMidiKeyName:
	case effBeginLoadBank:
	case effBeginLoadProgram:
	case effGetEffectName:
	case effShellGetNextPlugin:
		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index,
				frame->value, frame->data, frame->opt);
		break;

	case effProcessEvents: {
		VstEvent* events = reinterpret_cast<VstEvent*>(frame->data);
		events_.reload(frame->index, events);
		VstEvents* e = events_.events();

		frame->value = effect_->dispatcher(effect_, frame->opcode, 0,
				frame->value, e, frame->opt);
		break; }

	case effGetChunk: {
		size_t blockSize = frame->value;

		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index, 0,
			&data_, frame->opt);

		dataLength_ = frame->value;

		DEBUG("effGetChunk: %d", dataLength_);
		if(dataLength_ == 0)
			break;

		frame->index = dataLength_ < blockSize ? dataLength_ : blockSize;
		std::copy(data_, data_ + frame->index, frame->data);
		data_ += frame->index;
		dataLength_ -= frame->index;
		break; }

	case effSetChunk: {
		DEBUG("effSetChunk: %d bytes", chunk_.size());
		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index,
				chunk_.size(), chunk_.data(), frame->opt);

		chunk_.clear();
		break; }

	case effSetSpeakerArrangement: {
		u8* data = frame->data;

		intptr_t value = reinterpret_cast<intptr_t>(data);
		void* ptr = data + sizeof(VstSpeakerArrangement);

		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index, value,
			ptr, frame->opt);

		break; }

	default:
		ERROR("Unhandled dispatch event: %s", kDispatchEvents[frame->opcode]);
	}

	return true;
}


void Host::handleGetParameter()
{
//	DataFrame* frame = controlPort_.frame<DataFrame>();
	DataFrame* frame = audioPort_.frame<DataFrame>();
	frame->opt = effect_->getParameter(effect_, frame->index);
}


void Host::handleSetParameter()
{
//	DataFrame* frame = controlPort_.frame<DataFrame>();
	DataFrame* frame = audioPort_.frame<DataFrame>();
	effect_->setParameter(effect_, frame->index, frame->opt);
}


void Host::handleProcessSingle()
{
	DataFrame* frame = audioPort_.frame<DataFrame>();

	float* inputs[effect_->numInputs];
	float* outputs[effect_->numOutputs];
	i32 sampleCount = frame->value;
	float* data = reinterpret_cast<float*>(frame->data);

	for(int i = 0; i < effect_->numInputs; ++i)
		inputs[i] = data + i * sampleCount;

	for(int i = 0; i < effect_->numOutputs; ++i)
		outputs[i] = data + i * sampleCount;

	effect_->processReplacing(effect_, inputs, outputs, sampleCount);
}


void Host::handleProcessDouble()
{
	DataFrame* frame = audioPort_.frame<DataFrame>();

	double* inputs[effect_->numInputs];
	double* outputs[effect_->numOutputs];
	i32 sampleCount = frame->value;
	double* data = reinterpret_cast<double*>(frame->data);

	for(int i = 0; i < effect_->numInputs; ++i)
		inputs[i] = data + i * sampleCount;

	for(int i = 0; i < effect_->numOutputs; ++i)
		outputs[i] = data + i * sampleCount;

	effect_->processDoubleReplacing(effect_, inputs, outputs, sampleCount);
}


intptr_t Host::audioMaster(i32 opcode, i32 index, intptr_t value, void* ptr, float opt)
{
	if(opcode != audioMasterGetTime && opcode != audioMasterIdle)
		FLOOD("handleAudioMaster(%s)", kAudioMasterEvents[opcode]);

	DataFrame* frame = callbackPort_.frame<DataFrame>();
	frame->command = Command::AudioMaster;
	frame->opcode  = opcode;
	frame->index   = index;
	frame->value   = value;
	frame->opt     = opt;

	switch(opcode) {
	case audioMasterVersion:
	case __audioMasterWantMidiDeprecated:
	case audioMasterAutomate:
	case audioMasterBeginEdit:
	case audioMasterEndEdit:
	case audioMasterGetVendorVersion:
	case audioMasterSizeWindow:
	case audioMasterGetInputLatency:
	case audioMasterGetOutputLatency:
	case audioMasterGetCurrentProcessLevel:
	case audioMasterGetAutomationState:
	case audioMasterCurrentId:
		callbackPort_.sendRequest();
		callbackPort_.waitResponse();
		return frame->value;

	case audioMasterIOChanged: {
		PluginInfo* info = reinterpret_cast<PluginInfo*>(frame->data);
		info->flags        = effect_->flags;
		info->programCount = effect_->numPrograms;
		info->paramCount   = effect_->numParams;
		info->inputCount   = effect_->numInputs;
		info->outputCount  = effect_->numOutputs;
		info->initialDelay = effect_->initialDelay;
		info->uniqueId     = effect_->uniqueID;
		info->version      = effect_->version;

		callbackPort_.sendRequest();
		callbackPort_.waitResponse();
		return frame->value; }

	// FIXME Passing the audioMasterUpdateDisplay request to the plugin endpoint leads to
	// crash (or lock in Renoise) with some plugins (u-he TripleCheese).
	case audioMasterUpdateDisplay:
//		callbackPort_.sendRequest();
//		callbackPort_.waitResponse();
		return 1;

	case audioMasterIdle:
	case __audioMasterNeedIdleDeprecated:
		// There is no need to translate this request to the VST host, because we can
		// simply call the dispatcher.
//		if(isEditorOpen_)
//			effect_->dispatcher(effect_, effEditIdle, 0, 0, nullptr, 0.0f);

		// NOTE Currently we run effEditIdle periodically on timer event
		return 1;

	case audioMasterGetVendorString: {
		callbackPort_.sendRequest();
		callbackPort_.waitResponse();

		if(!frame->value)
			return 0;

		const char* source = reinterpret_cast<const char*>(frame->data);
		char* dest         = static_cast<char*>(ptr);

		std::strncpy(dest, source, kVstMaxVendorStrLen);
		dest[kVstMaxVendorStrLen-1] = '\0';
		return frame->value; }

	case audioMasterGetProductString: {
		callbackPort_.sendRequest();
		callbackPort_.waitResponse();

		if(!frame->value)
			return 0;

		const char* source = reinterpret_cast<const char*>(frame->data);
		char* dest         = static_cast<char*>(ptr);

		std::strncpy(dest, source, kVstMaxProductStrLen);
		dest[kVstMaxProductStrLen-1] = '\0';
		return frame->value; }

	case audioMasterCanDo: {
		const char* source = static_cast<const char*>(ptr);
		char* dest         = reinterpret_cast<char*>(frame->data);
		size_t maxLength   = callbackPort_.frameSize() - sizeof(DataFrame);

		std::strncpy(dest, source, maxLength);
		dest[maxLength-1] = '\0';

		callbackPort_.sendRequest();
		callbackPort_.waitResponse();
		return frame->value; }

	case audioMasterGetTime:
		callbackPort_.sendRequest();
		callbackPort_.waitResponse();

		if(!frame->value)
			return 0;

		std::memcpy(&timeInfo_, frame->data, sizeof(VstTimeInfo));
		return reinterpret_cast<intptr_t>(&timeInfo_);

	case audioMasterProcessEvents: {
		VstEvents* events = static_cast<VstEvents*>(ptr);
		VstEvent* event = reinterpret_cast<VstEvent*>(frame->data);
		frame->index = events->numEvents;

		for(int i = 0; i < events->numEvents; ++i)
			event[i] = *events->events[i];

		callbackPort_.sendRequest();
		callbackPort_.waitResponse();
		return frame->value; }
	}

	ERROR("Unhandled audio master request: %s", kAudioMasterEvents[opcode]);
	return 0;
}


intptr_t VSTCALLBACK Host::audioMasterProc(AEffect* effect, i32 opcode, i32 index,
		intptr_t value, void* ptr, float opt)
{
	UNUSED(effect);

	EnterCriticalSection(&self_->cs_);
	intptr_t result = self_->audioMaster(opcode, index, value, ptr, opt);

	LeaveCriticalSection(&self_->cs_);
	return result;
}


DWORD CALLBACK Host::audioThreadProc(void* param)
{
	TRACE("Audio thread started");

	Host* host = static_cast<Host*>(param);
	host->audioThread();

	TRACE("Audio thread terminated");
	return 0;
}


LRESULT CALLBACK Host::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(hwnd == self_->hwnd_) {
		switch(message) {
		case WM_CLOSE:
			DEBUG("Received WM_CLOSE event");
			ShowWindow(hwnd, SW_HIDE);
			return 0;

		case WM_PARENTNOTIFY:
			if(wParam == WM_CREATE) {
				self_->childHwnd_ = reinterpret_cast<HWND>(lParam);

				LONG_PTR value = SetWindowLongPtr(self_->childHwnd_, GWLP_WNDPROC,
						reinterpret_cast<LONG_PTR>(windowProc));

				self_->oldWndProc_ = reinterpret_cast<WNDPROC>(value);
			}
			break;

		case WM_TIMER:
			self_->effect_->dispatcher(self_->effect_, effEditIdle, 0, 0, nullptr, 0.0f);
			break;
		}
	}
	else if(self_->childHwnd_ && hwnd == self_->childHwnd_) {
		return CallWindowProc(self_->oldWndProc_, hwnd, message, wParam, lParam);
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


} //namespace Airwave
