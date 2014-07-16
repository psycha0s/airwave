#include <string>
#include <dlfcn.h>
#include <signal.h>
#include "config.h"
#include "masterunit.h"
#include "common/filesystem.h"
#include "common/linkmanager.h"
#include "common/logger.h"
#include "common/vst24.h"


using namespace Airwave;

extern "C" {

AEffect* VSTPluginMain(AudioMasterProc audioMasterProc);
AEffect* mainStub(AudioMasterProc audioMasterProc) asm ("main");

}


void signalHandler(int signum)
{
	if(signum == SIGCHLD) {
		LOG("Child process terminated");
	}
	else {
		LOG("Received signal %d", signum);
	}
}


AEffect* VSTPluginMain(AudioMasterProc audioMasterProc)
{
	loggerInit(PLUGIN_BASENAME);

	LOG("Starting Airwave master unit %s", PROJECT_VERSION);

	// FIXME Without this signal handler the Renoise tracker is unable to start
	// child wine application.
	signal(SIGCHLD, signalHandler);


	// Get path to own binary
	Dl_info info;
	if(dladdr(reinterpret_cast<void*>(VSTPluginMain), &info) == 0) {
		LOG("Unable to get library filename");
		return nullptr;
	}

	loggerSetSenderId(FileSystem::baseName(info.dli_fname));

	std::string selfPath = FileSystem::realPath(info.dli_fname);
	LOG("Plugin binary: %s", selfPath.c_str());


	// Find path of the host binary
	std::string hostName;
	if(sizeof(void*) == 8) {
		hostName = HOST_BASENAME "-64.exe";
	}
	else {
		hostName = HOST_BASENAME "-32.exe";
	}

	// First try the path, specified at compile-time.
	std::string hostPath = FileSystem::realPath(HOST_PATH "/" + hostName);

	if(!FileSystem::isFileExists(hostPath)) {
		// At last resort, try to find host binary in the PATH.
		hostPath = FileSystem::fullNameFromPath(hostName);

		if(!FileSystem::isFileExists(hostPath)) {
			LOG("Host binary does not found in %s and PATH", HOST_PATH);
			return nullptr;
		}

	}

	LOG("Host binary:   %s", hostPath.c_str());


	// Get path to the linked VST plugin binary
	LinkManager linkManager;
	std::string pluginPath = linkManager.pluginPath(selfPath);

	if(pluginPath.empty()) {
		LOG("Bridge plugin is not linked with any VST plugin dll");
		return nullptr;
	}

	LOG("VST binary:    %s", pluginPath.c_str());


	// Initialize master unit.
	MasterUnit* masterUnit;
	masterUnit = new MasterUnit(pluginPath, hostPath, audioMasterProc);
	if(!masterUnit->effect()) {
		LOG("Unable to initialize master unit");
		return nullptr;
	}

	LOG("Master unit initialized");
	return masterUnit->effect();
}


// Derecated main() stub
AEffect* mainStub(AudioMasterProc audioMasterProc)
{
	return VSTPluginMain(audioMasterProc);
}
