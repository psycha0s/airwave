#include <QTextCodec>
#include "forms/mainform.h"
#include "qtsingleapplication/QtSingleApplication"


int main(int argc, char** argv)
{
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForTr(codec);

	QtSingleApplication application(argc, argv);
	if(application.isRunning()) {
		application.sendMessage(QString());
		qDebug("Found another instance, exiting..");
		return EXIT_SUCCESS;
	}

	MainForm mainForm;
	mainForm.show();

	application.setActivationWindow(&mainForm);

	return application.exec();
}
