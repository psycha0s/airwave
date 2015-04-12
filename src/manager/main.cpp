#include "common/config.h"
#include "core/application.h"
#include "forms/mainform.h"


int main(int argc, char** argv)
{
	Application application(argc, argv);

	if(application.isRunning())
		return EXIT_SUCCESS;

	application.setOrganizationName("phantom-code");
	application.setOrganizationDomain("darkhub.net");
	application.setApplicationName(PROJECT_NAME "-manager");
	application.setApplicationVersion(VERSION_STRING);

	MainForm mainForm;
	mainForm.show();

	application.setActivationWindow(&mainForm);
	return application.exec();
}
