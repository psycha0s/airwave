#include <cstdlib>
#include "config.h"
#include "slaveunit.h"
#include "common/logger.h"


int main(int argc, char const* argv[])
{
	LOG("VST bridge host %s -------------------------------", PROJECT_VERSION);

	if(argc != 3) {
		LOG("Usage: %s <VST plugin path> <control port id>", argv[0]);
		return -1;
	}

	Airwave::SlaveUnit slaveUnit;
	if(!slaveUnit.initialize(argv[1], atoi(argv[2]))) {
		LOG("Unable to initialize slave unit.");
		return -2;
	}

	LOG("Slave unit initialized, starting to process events.");

	while(slaveUnit.processRequest()) {
		MSG message;

		if(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	LOG("VST bridge host terminated.");
	return 0;
}
