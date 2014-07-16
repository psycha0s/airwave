#ifndef PLUGIN_MASTERUNIT_H
#define PLUGIN_MASTERUNIT_H

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <X11/Xlib.h>
#include "common/dataport.h"
#include "common/eventsignal.h"
#include "common/vst24.h"
#include "common/vsteventkeeper.h"


namespace Airwave {


using RecursiveMutex = std::recursive_mutex;
using RecursiveLock = std::lock_guard<RecursiveMutex>;


class MasterUnit {
public:
	MasterUnit(const std::string& pluginPath, const std::string& hostPath,
			AudioMasterProc masterProc);

	~MasterUnit();

	AEffect* effect();

private:
	AudioMasterProc masterProc_;
	AEffect effect_;
	ERect rect_;
	VstEventKeeper events_;

	uint8_t* data_;
	size_t dataLength_;
	std::vector<uint8_t> chunk_;

	RecursiveMutex guard_;
	RecursiveMutex audioGuard_;

	DataPort controlPort_;
	DataPort callbackPort_;
	DataPort audioPort_;

	EventSignal condition_;

	int childPid_;

	std::thread callbackThread_;
	std::atomic_flag processCallbacks_;
	std::thread::id mainThreadId_;

	void callbackThread();

	intptr_t handleAudioMaster();

	intptr_t dispatch(DataPort* port, int32_t opcode, int32_t index,
			intptr_t value, void* ptr, float opt);

	void sendXembedMessage(Display* display, Window window, long message,
			long detail, long data1, long data2);

	float getParameter(int32_t index);

	void setParameter(int32_t index, float value);

	void processReplacing(float** inputs, float** outputs, int32_t count);

	void processDoubleReplacing(double** inputs, double** outputs,
			int32_t count);

	static intptr_t dispatchProc(AEffect* effect, int32_t opcode, int32_t index,
			intptr_t value, void* ptr, float opt);

	static float getParameterProc(AEffect* effect, int32_t index);

	static void setParameterProc(AEffect* effect, int32_t index, float value);

	static void processReplacingProc(AEffect* effect, float** inputs,
			float** outputs, int32_t sampleCount);

	static void processDoubleReplacingProc(AEffect* effect, double** inputs,
			double** outputs, int32_t sampleCount);
};


} // namespace Airwave


#endif // PLUGIN_MASTERUNIT_H
