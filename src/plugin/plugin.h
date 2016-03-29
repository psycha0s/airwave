#ifndef PLUGIN_PLUGIN_H
#define PLUGIN_PLUGIN_H

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <X11/Xlib.h>
#include "common/dataport.h"
#include "common/event.h"
#include "common/vst24.h"
#include "common/vsteventkeeper.h"


namespace Airwave {


using RecursiveMutex = std::recursive_mutex;
using RecursiveLock = std::lock_guard<RecursiveMutex>;


class Plugin {
public:
	Plugin(const std::string& vstPath, const std::string& hostPath,
		   const std::string& prefixPath, const std::string& loaderPath,
		   const std::string& logSocketPath, AudioMasterProc masterProc);

	~Plugin();

	AEffect* effect();

private:
	AudioMasterProc masterProc_;
	AEffect* effect_;
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

	Event condition_;

	int childPid_;

	std::thread callbackThread_;
	std::atomic_flag processCallbacks_;
	std::thread::id mainThreadId_;

	bool isInAutomate_;
	float lastValue_;

	void callbackThread();

	intptr_t setBlockSize(DataPort* port, intptr_t frames);

	intptr_t handleAudioMaster();

	intptr_t dispatch(DataPort* port, i32 opcode, i32 index, intptr_t value, void* ptr,
			float opt);

	void sendXembedMessage(Display* display, Window window, long message, long detail,
			long data1, long data2);

	float getParameter(i32 index);
	void setParameter(i32 index, float value);

	void processReplacing(float** inputs, float** outputs, i32 count);
	void processDoubleReplacing(double** inputs, double** outputs, i32 count);

	static intptr_t dispatchProc(AEffect* effect, i32 opcode, i32 index, intptr_t value,
			void* ptr, float opt);

	static float getParameterProc(AEffect* effect, i32 index);
	static void setParameterProc(AEffect* effect, i32 index, float value);

	static void processReplacingProc(AEffect* effect, float** inputs, float** outputs,
			i32 sampleCount);

	static void processDoubleReplacingProc(AEffect* effect, double** inputs,
			double** outputs, i32 sampleCount);
};


} // namespace Airwave


#endif // PLUGIN_PLUGIN_H
