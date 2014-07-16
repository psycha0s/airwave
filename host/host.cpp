#include <cstdlib>
#include "config.h"
#include "slaveunit.h"
#include "common/filesystem.h"
#include "common/logger.h"


int main(int argc, const char* argv[])
{
	using Airwave::FileSystem;

	loggerInit(HOST_BASENAME);

	LOG("Starting Airwave slave unit %s", PROJECT_VERSION);

	if(argc != 3) {
		LOG("Wrong number of arguments: %d", argc);
		loggerFree();
		return -1;
	}

	loggerSetSenderId(FileSystem::baseName(argv[1]));

	Airwave::SlaveUnit* slaveUnit = new Airwave::SlaveUnit;
	if(!slaveUnit->initialize(argv[1], atoi(argv[2]))) {
		LOG("Unable to initialize slave unit");
		loggerFree();
		return -2;
	}

	LOG("Slave unit initialized");

	while(slaveUnit->processRequest()) {
		MSG message;

		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	delete slaveUnit;

	LOG("Slave unit terminated");
	loggerFree();
	return 0;
}
