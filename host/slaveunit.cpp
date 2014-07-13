#include "slaveunit.h"

#include <cstring>
#include "common/logger.h"
#include "common/protocol.h"


namespace Airwave {


SlaveUnit* SlaveUnit::self_ = nullptr;


SlaveUnit::SlaveUnit() :
	isInitialized_(false),
	hwnd_(0),
	runAudio_(ATOMIC_FLAG_INIT),
	isEditorOpen_(false)
{
}


SlaveUnit::~SlaveUnit()
{
	if(isInitialized_) {
		LOG("Waiting for audio thread termination.");

		runAudio_.clear();
		WaitForSingleObject(audioThread_, INFINITE);

		destroyEditorWindow();

		FreeLibrary(module_);
	}
}


bool SlaveUnit::initialize(const char* fileName, int portId)
{
	if(isInitialized_) {
		LOG("Slave unit is already initialized.");
		return false;
	}

	module_ = LoadLibrary(fileName);
	if(!module_) {
		LOG("Unable to load '%s' shared library: %s", errorString().c_str());
		return false;
	}

	VstPluginMainProc vstMainProc = reinterpret_cast<VstPluginMainProc>(
			GetProcAddress(module_, "VSTPluginMain"));

	if(!vstMainProc) {
		vstMainProc = reinterpret_cast<VstPluginMainProc>(
				GetProcAddress(module_, "main"));

		if(!vstMainProc) {
			LOG("The '%s' is not a VST plugin.");
			FreeLibrary(module_);
			return false;
		}
	}

	if(!controlPort_.connect(portId)) {
		LOG("Unable to connect control port (id = %d).", portId);
		FreeLibrary(module_);
		return false;
	}

	LOG("Waiting for master unit request..");

	if(!controlPort_.waitRequest(3000)) {
		LOG("Unable to get initial request from master unit.");
		controlPort_.disconnect();
		FreeLibrary(module_);
		return false;
	}

	DataFrame* frame = controlPort_.frame<DataFrame>();

	if(frame->value != PROTOCOL_VERSION) {
		frame->value = PROTOCOL_VERSION;
		controlPort_.sendResponse();

		LOG("Master unit has incompatible protocol version: %d.", frame->value);
		controlPort_.disconnect();
		FreeLibrary(module_);
		return false;
	}

	if(!callbackPort_.connect(frame->opcode)) {
		LOG("Unable to connect callback port (id = %d).", frame->opcode);
		controlPort_.disconnect();
		FreeLibrary(module_);
		return false;
	}

	// When we call vstMainProc(), the audioMasterProc() can be called from
	// there with effect argument set to the nullptr. This is because VST plugin
	// object is not yet initialized at this point. Since we need the pointer to
	// our object inside of audioMasterProc(), we must store it in some
	// accessible place. Static member pointer is very ugly, but very effective
	// solution, since we don't need more than one instance of SlaveUnit anyway.
	self_ = this;

	effect_ = vstMainProc(audioMasterProc);
	if(!effect_) {
		LOG("Unable to initialize VST plugin.");
		controlPort_.disconnect();
		callbackPort_.disconnect();
		FreeLibrary(module_);
		return false;
	}

	std::memset(&timeInfo_, 0, sizeof(VstTimeInfo));

	frame->command = Command::PluginInfo;
	PluginInfo* info = reinterpret_cast<PluginInfo*>(frame->data);

	info->flags        = effect_->flags;
	info->programCount = effect_->numPrograms;
	info->paramCount   = effect_->numParams;
	info->inputCount   = effect_->numInputs;
	info->outputCount  = effect_->numOutputs;
	info->uniqueId     = effect_->uniqueID;
	info->version      = effect_->version;

	frame->value = PROTOCOL_VERSION;
	controlPort_.sendResponse();

	isInitialized_ = true;
	return true;
}


bool SlaveUnit::processRequest()
{
	if(!controlPort_.isConnected())
		return false;

	if(!controlPort_.waitRequest(20))
		return true;

	bool result = true;
	DataFrame* frame = controlPort_.frame<DataFrame>();

	switch(frame->command) {
	case Command::Dispatch:
		result = handleDispatch(frame);
		break;

	case Command::GetParameter:
		handleGetParameter();
		break;

	case Command::SetParameter:
		handleSetParameter();
		break;

	case Command::ShowWindow: {
		if(hwnd_) {
			ShowWindow(hwnd_, SW_SHOW);
			UpdateWindow(hwnd_);
		}
		break; }

	default:
		LOG("processRequest() unacceptable command: %d", frame->command);
		break;
	}

	frame->command = Command::Response;
	controlPort_.sendResponse();
	return result;
}


std::string SlaveUnit::errorString() const
{
	DWORD error = GetLastError();

	if(error) {
		LPVOID buffer;
		DWORD length = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
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


void SlaveUnit::destroyEditorWindow()
{
	if(hwnd_) {
		DestroyWindow(hwnd_);
		UnregisterClass(kWindowClass, module_);
		hwnd_ = 0;
	}
}


void SlaveUnit::audioThread()
{
	condition_.post();

	while(runAudio_.test_and_set()) {
		if(audioPort_.waitRequest(50)) {
			DataFrame* frame = audioPort_.frame<DataFrame>();

			if(frame->command == Command::ProcessSingle) {
				handleProcessSingle();
			}
			else if(frame->command == Command::ProcessDouble) {
				handleProcessDouble();
			}
			else if(frame->command == Command::Dispatch) {
				handleDispatch(frame);
			}
			else {
				LOG("audioThread() unacceptable command: %d", frame->command);
			}

			frame->command = Command::Response;
			audioPort_.sendResponse();
		}
	}
}


bool SlaveUnit::handleDispatch(DataFrame* frame)
{
//	LOG("handleDispatch: %s", kDispatchOpcodes[frame->opcode]);

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
		frame->value = effect_->dispatcher(effect_, frame->opcode,
				frame->index, frame->value, nullptr, frame->opt);
		break;

	case effEditClose:
		frame->value = effect_->dispatcher(effect_, frame->opcode,
				frame->index, frame->value, nullptr, frame->opt);

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
			LOG("Unable to connect audio port.");
			return false;
		}

		runAudio_.test_and_set();
		audioThread_ = CreateThread(nullptr, 0, audioThreadProc, this, 0,
				nullptr);

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
		wclass.hIcon         = LoadIcon(GetModuleHandle(nullptr), kWindowClass);
		wclass.hCursor       = LoadCursor(0, IDI_APPLICATION);
		wclass.lpszClassName = kWindowClass;

		if(!RegisterClassEx(&wclass)) {
			LOG("Unable to register window class: %s", errorString().c_str());
			return false;
		}

		hwnd_ = CreateWindowEx(WS_EX_TOOLWINDOW, kWindowClass, "Plugin",
				WS_POPUP, 0, 0, 200, 200, 0, 0, GetModuleHandle(nullptr), 0);

		if(!hwnd_) {
			LOG("Unable to create window: %s", errorString().c_str());
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

		SetWindowPos(hwnd_, 0, 0, 0, rect->right - rect->left,
				rect->bottom - rect->top, SWP_NOACTIVATE | SWP_NOMOVE);

		std::memcpy(&frame->data, rect, sizeof(ERect));

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
		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index,
				frame->value, frame->data, frame->opt);
		break;

	case effGetParameterProperties: {
		VstParameterProperties properties;
		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index,
				frame->value, &properties, frame->opt);

		std::memcpy(&frame->data, &properties, sizeof(VstParameterProperties));
		break; }

	case effGetMidiKeyName: {
		MidiKeyName midiKeyName;
		frame->value = effect_->dispatcher(effect_, frame->opcode, frame->index,
				frame->value, &midiKeyName, frame->opt);

		std::memcpy(&frame->data, &midiKeyName, sizeof(MidiKeyName));
		break; }

	case effProcessEvents: {
		VstEvent* events = reinterpret_cast<VstEvent*>(frame->data);
		lastEvents_.reload(frame->index, events);
		VstEvents* e = lastEvents_.events();

		frame->value = effect_->dispatcher(effect_, frame->opcode, 0,
				frame->value, e, frame->opt);
		break; }

	default:
		LOG("Unhandled dispatch event: %s", kDispatchEvents[frame->opcode]);
	}

	return true;
}


void SlaveUnit::handleGetParameter()
{
	DataFrame* frame = controlPort_.frame<DataFrame>();
	frame->opt = effect_->getParameter(effect_, frame->index);
}


void SlaveUnit::handleSetParameter()
{
	DataFrame* frame = controlPort_.frame<DataFrame>();
	effect_->setParameter(effect_, frame->index, frame->opt);
}


void SlaveUnit::handleProcessSingle()
{
	DataFrame* frame = audioPort_.frame<DataFrame>();

	float* inputs[effect_->numInputs];
	float* outputs[effect_->numOutputs];
	int32_t sampleCount = frame->value;
	float* data = reinterpret_cast<float*>(frame->data);

	for(int i = 0; i < effect_->numInputs; ++i)
		inputs[i] = data + i * sampleCount;

	for(int i = 0; i < effect_->numOutputs; ++i)
		outputs[i] = data + i * sampleCount;

	effect_->processReplacing(effect_, inputs, outputs, sampleCount);
}


void SlaveUnit::handleProcessDouble()
{
	DataFrame* frame = audioPort_.frame<DataFrame>();

	double* inputs[effect_->numInputs];
	double* outputs[effect_->numOutputs];
	int32_t sampleCount = frame->value;
	double* data = reinterpret_cast<double*>(frame->data);

	for(int i = 0; i < effect_->numInputs; ++i)
		inputs[i] = data + i * sampleCount;

	for(int i = 0; i < effect_->numOutputs; ++i)
		outputs[i] = data + i * sampleCount;

	effect_->processDoubleReplacing(effect_, inputs, outputs, sampleCount);
}


intptr_t SlaveUnit::audioMaster(int32_t opcode, int32_t index,
		intptr_t value, void* ptr, float opt)
{
//	if(opcode != audioMasterGetTime) { // filter out audioMasterGetTime
//		LOG("master(opcode: %s, index: %d, value: %d, ptr: %p, opt: %g)",
//			kAudioMasterOpcodes[opcode], index, value, ptr, opt);
//	}

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
	case audioMasterGetCurrentProcessLevel:
//	case audioMasterUpdateDisplay:
	case audioMasterGetVendorVersion:
		callbackPort_.sendRequest();
		callbackPort_.waitResponse();
		return frame->value;

	case audioMasterIdle:
		// There is no need to translate this request to the VST host, because
		// we can simply call the dispatcher.
		effect_->dispatcher(effect_, effEditIdle, 0, 0, nullptr, 0.0f);
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

	LOG("Unhandled audio master request: %s", kAudioMasterEvents[opcode]);
	return 0;
}


intptr_t SlaveUnit::audioMasterProc(AEffect* effect, int32_t opcode,
		int32_t index, intptr_t value, void* ptr, float opt)
{
	UNUSED(effect);
	return self_->audioMaster(opcode, index, value, ptr, opt);
}


DWORD CALLBACK SlaveUnit::audioThreadProc(void* param)
{
	LOG("Audio thread started.");

	SlaveUnit* slaveUnit = static_cast<SlaveUnit*>(param);
	slaveUnit->audioThread();

	LOG("Audio thread terminated.");
	return 0;
}


LRESULT CALLBACK SlaveUnit::windowProc(HWND hwnd, UINT message,
		WPARAM wParam, LPARAM lParam)
{
	if(hwnd == self_->hwnd_) {
		switch(message) {
		case WM_CLOSE:
			LOG("Received WM_CLOSE event.");
			ShowWindow(hwnd, SW_HIDE);
			return 0;
		}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}


} //namespace Airwave
