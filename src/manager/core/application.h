#ifndef CORE_APPLICATION_H
#define CORE_APPLICATION_H

#include "core/logsocket.h"
#include "core/singleapplication.h"

#ifdef qApp
#undef qApp
#define qApp (static_cast<Application*>(QApplication::instance()))
#endif


class LinksModel;
class LoadersModel;
class PrefixesModel;

namespace Airwave {
class Storage;
}


class Application : public SingleApplication {
public:
	Application(int& argc, char** argv);
	~Application();

	LogSocket* logSocket();
	Airwave::Storage* storage() const;
	LinksModel* links() const;
	LoadersModel* loaders() const;
	PrefixesModel* prefixes() const;

	QStringList checkMissingBinaries(const QString& path = QString()) const;

private:
	LogSocket logSocket_;
	Airwave::Storage* storage_;
	LinksModel* links_;
	LoadersModel* loaders_;
	PrefixesModel* prefixes_;
};


#endif // CORE_APPLICATION_H
