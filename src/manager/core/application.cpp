#include <QDir>
#include "application.h"
#include "common/config.h"
#include "common/storage.h"
#include "models/linksmodel.h"
#include "models/loadersmodel.h"
#include "models/prefixesmodel.h"


Application::Application(int& argc, char** argv) :
	SingleApplication(argc, argv),
	storage_(new Airwave::Storage),
	links_(new LinksModel(this)),
	loaders_(new LoadersModel(this)),
	prefixes_(new PrefixesModel(this))
{
}


Application::~Application()
{
	delete prefixes_;
	delete loaders_;
	delete links_;
	delete storage_;
}


LogSocket* Application::logSocket()
{
	return &logSocket_;
}


Storage* Application::storage() const
{
	return storage_;
}


LinksModel* Application::links() const
{
	return links_;
}


LoadersModel* Application::loaders() const
{
	return loaders_;
}


PrefixesModel* Application::prefixes() const
{
	return prefixes_;
}


QStringList Application::checkMissingBinaries(const QString& path) const
{
	QString binPath = path;

	if(binPath.isEmpty())
		binPath = QString::fromStdString(storage_->binariesPath());

	QDir binDir(binPath);

	QStringList fileList;
	fileList += HOST_BASENAME "-32.exe";
	fileList += PLUGIN_BASENAME ".so";
#ifdef PLATFORM_64BIT
	fileList += HOST_BASENAME "-64.exe";
#endif

	QStringList result;

	foreach(const QString& fileName, fileList) {
		if(!binDir.exists(fileName))
			result += fileName;
	}

	return result;
}
