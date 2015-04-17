#include <string>
#include <dlfcn.h>
#include <signal.h>
#include "plugin.h"
#include "common/config.h"
#include "common/filesystem.h"
#include "common/logger.h"
#include "common/moduleinfo.h"
#include "common/storage.h"


using namespace Airwave;


extern "C" {

AEffect* VSTPluginMain(AudioMasterProc audioMasterProc);
AEffect* mainStub(AudioMasterProc audioMasterProc) asm ("main");

}


void signalHandler(int signum)
{
	if(signum == SIGCHLD) {
		TRACE("Child process terminated");
	}
	else {
		TRACE("Received signal %d", signum);
	}
}


AEffect* VSTPluginMain(AudioMasterProc audioMasterProc)
{
	// FIXME Without this signal handler the Renoise tracker is unable to start the child
	// winelib application.
	signal(SIGCHLD, signalHandler);

	Storage storage;
	loggerInit(storage.logSocketPath(), PLUGIN_BASENAME);

	// Get path to own binary
	Dl_info info;
	if(dladdr(reinterpret_cast<void*>(VSTPluginMain), &info) == 0) {
		ERROR("Unable to get library filename");
		return nullptr;
	}

	std::string selfPath = FileSystem::realPath(info.dli_fname);
	if(selfPath.empty()) {
		ERROR("Unable to get an absolute path of the plugin binary", selfPath.c_str());
		return nullptr;
	}

	// Get path of the linked VST plugin binary
	Storage::Link link = storage.link(selfPath);
	if(!link) {
		ERROR("Link '%s' is corrupted", selfPath.c_str());
		return nullptr;
	}

	LogLevel level = link.logLevel();
	if(level == LogLevel::kDefault)
		level = storage.defaultLogLevel();

	loggerSetSenderId(FileSystem::baseName(info.dli_fname));
	loggerSetLogLevel(level);

	TRACE("Initializing plugin endpoint %s", VERSION_STRING);
	TRACE("Plugin binary: %s", selfPath.c_str());

	std::string winePrefix = link.prefix();
	Storage::Prefix prefix = storage.prefix(winePrefix);
	if(!prefix) {
		ERROR("Invalid WINE prefix '%s'", winePrefix.c_str());
		return nullptr;
	}

	std::string prefixPath = FileSystem::realPath(prefix.path());
	if(!FileSystem::isDirExists(prefixPath)) {
		ERROR("WINE prefix directory '%s' doesn't exists", prefixPath.c_str());
		return nullptr;
	}

	TRACE("WINE prefix:   %s", prefixPath.c_str());

	std::string wineLoader = link.loader();
	Storage::Loader loader = storage.loader(wineLoader);
	if(!loader) {
		ERROR("Invalid WINE loader '%s'", wineLoader.c_str());
		return nullptr;
	}

	std::string loaderPath = FileSystem::realPath(loader.path());
	if(!FileSystem::isFileExists(loaderPath)) {
		ERROR("WINE loader binary '%s' doesn't exists", loaderPath.c_str());
		return nullptr;
	}

	TRACE("WINE loader:   %s", loaderPath.c_str());

	std::string vstPath = prefixPath + '/' + link.target();
	if(!FileSystem::isFileExists(vstPath)) {
		ERROR("VST binary '%s' doesn't exists", vstPath.c_str());
		return nullptr;
	}

	TRACE("VST binary:    %s", vstPath.c_str());

	// Find host binary path
	ModuleInfo::Arch arch = ModuleInfo::instance()->getArch(vstPath);

	std::string hostName;
	if(arch == ModuleInfo::kArch64) {
		hostName = HOST_BASENAME "-64.exe";
	}
	else if(arch == ModuleInfo::kArch32) {
		hostName = HOST_BASENAME "-32.exe";
	}
	else {
		ERROR("Unable to determine VST plugin architecture");
		return nullptr;
	}

	std::string hostPath = FileSystem::realPath(storage.binariesPath() + '/' + hostName);
	if(!FileSystem::isFileExists(hostPath)) {
		ERROR("Host binary '%s' doesn't exists", hostPath.c_str());
		return nullptr;
	}

	TRACE("Host binary:   %s", hostPath.c_str());

	// We process only two cases, because messages with log levels lower than 'trace'
	// wouldn't be logged anyway.
	if(level == LogLevel::kTrace) {
		TRACE("Log level:     trace");
	}
	else if(level == LogLevel::kDebug) {
		TRACE("Log level:     debug");
	}

	// Initialize plugin endpoint
	Plugin* plugin;
	plugin = new Plugin(vstPath, hostPath, prefixPath, loaderPath,
			storage.logSocketPath(), audioMasterProc);
	if(!plugin->effect()) {
		ERROR("Unable to initialize plugin endpoint");
		return nullptr;
	}

	TRACE("Plugin endpoint is initialized");
	return plugin->effect();
}


// Deprecated main() stub which is still used by some hosts
AEffect* mainStub(AudioMasterProc audioMasterProc)
{
	return VSTPluginMain(audioMasterProc);
}
