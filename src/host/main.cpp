#include <cstdlib>
#include "host.h"
#include "common/config.h"
#include "common/filesystem.h"
#include "common/logger.h"


using namespace Airwave;


int main(int argc, const char* argv[])
{
	if(argc != 5) {
		fprintf(stderr, "Airwave host endpoint, version " VERSION_STRING);
		fprintf(stderr, "error: wrong number of arguments: %d", argc);
		fprintf(stderr, "usage: %s <vst path> <port id> <log level> <log socket path>",
				argv[0]);

		loggerFree();
		return -1;
	}

	loggerInit(argv[4], HOST_BASENAME);
	loggerSetSenderId(FileSystem::baseName(argv[1]));

	LogLevel level = static_cast<LogLevel>(atoi(argv[3]));
	if(level < LogLevel::kQuiet || level > LogLevel::kFlood) {
		loggerSetLogLevel(LogLevel::kTrace);
		ERROR("Invalid log level '%d', using log level 'trace' instead", argc);
	}
	else {
		loggerSetLogLevel(level);
	}

	TRACE("Initializing host endpoint %s", VERSION_STRING);

	Host* host = new Host;
	if(!host->initialize(argv[1], atoi(argv[2]))) {
		ERROR("Unable to initialize host endpoint");
		loggerFree();
		return -2;
	}

	TRACE("Host endpoint is initialized");

	while(host->processRequest()) {
		MSG message;

		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	delete host;

	TRACE("Host endpoint terminated");
	loggerFree();
	return 0;
}
