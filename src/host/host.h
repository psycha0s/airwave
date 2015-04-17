#ifndef HOST_HOST_H
#define HOST_HOST_H

#include <atomic>
#include <string>
#include <vector>
#include <wine/windows/windows.h>
#include "common/config.h"
#include "common/dataport.h"
#include "common/event.h"
#include "common/vst24.h"
#include "common/vsteventkeeper.h"


namespace Airwave {


struct DataFrame;


class Host {
public:
	Host();
	~Host();

	bool initialize(const char* fileName, int portId);
	bool processRequest();

private:
	bool isInitialized_;
	HMODULE module_;
	HWND hwnd_;
	CRITICAL_SECTION cs_;
	UINT_PTR timerId_;
	AEffect* effect_;
	VstTimeInfo timeInfo_;
	VstEventKeeper events_;

	u8* data_;
	size_t dataLength_;
	std::vector<u8> chunk_;

	DataPort controlPort_;
	DataPort callbackPort_;
	DataPort audioPort_;

	Event condition_;

	HANDLE audioThread_;
	std::atomic_flag runAudio_;

	bool isEditorOpen_;

	WNDPROC oldWndProc_;
	HWND childHwnd_;

	static Host* self_;
	static constexpr const char* kWindowClass = PROJECT_NAME;

	std::string errorString() const;
	void destroyEditorWindow();

	void audioThread();

	void handleGetDataBlock(DataFrame* frame);
	void handleSetDataBlock(DataFrame* frame);

	bool handleDispatch(DataFrame* frame);
	void handleGetParameter();
	void handleSetParameter();
	void handleProcessSingle();
	void handleProcessDouble();

	intptr_t audioMaster(i32 opcode, i32 index, intptr_t value, void* ptr, float opt);

	static intptr_t VSTCALLBACK audioMasterProc(AEffect* effect, i32 opcode, i32 index,
			intptr_t value, void* ptr, float opt);

	static DWORD CALLBACK audioThreadProc(void* param);

	static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam,
			LPARAM lParam);
};


} // namespace Airwave


#endif // HOST_HOST_H
