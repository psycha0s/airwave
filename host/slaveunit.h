#ifndef HOST_SLAVEUNIT_H
#define HOST_SLAVEUNIT_H

#include <atomic>
#include <string>
#include <vector>
#include <wine/windows/windows.h>
#include "config.h"
#include "common/dataport.h"
#include "common/eventsignal.h"
#include "common/vst24.h"
#include "common/vsteventkeeper.h"


namespace Airwave {


struct DataFrame;


class SlaveUnit {
public:
	SlaveUnit();
	~SlaveUnit();

	bool initialize(const char* fileName, int portId);
	bool processRequest();

private:
	bool isInitialized_;
	HMODULE module_;
	HWND hwnd_;
	AEffect* effect_;
	VstTimeInfo timeInfo_;
	VstEventKeeper events_;

	uint8_t* data_;
	size_t dataLength_;
	std::vector<uint8_t> chunk_;

	DataPort controlPort_;
	DataPort callbackPort_;
	DataPort audioPort_;

	EventSignal condition_;

	HANDLE audioThread_;
	std::atomic_flag runAudio_;

	bool isEditorOpen_;

	static SlaveUnit* self_;
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

	intptr_t audioMaster(int32_t opcode, int32_t index, intptr_t value,
			void* ptr, float opt);

	static intptr_t VSTCALLBACK audioMasterProc(AEffect* effect, int32_t opcode,
			int32_t index, intptr_t value, void* ptr, float opt);

	static DWORD CALLBACK audioThreadProc(void* param);

	static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam,
			LPARAM lParam);
};


} // namespace Airwave


#endif // HOST_SLAVEUNIT_H
