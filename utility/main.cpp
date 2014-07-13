#include <QApplication>
#include <QTextCodec>
#include "forms/mainform.h"


int main(int argc, char** argv)
{
	QApplication application(argc, argv);

	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForTr(codec);

	MainForm mainForm;
	mainForm.show();

	return application.exec();
}
