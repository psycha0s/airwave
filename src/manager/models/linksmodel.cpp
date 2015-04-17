#include "linksmodel.h"

#include <QDir>
#include <QIcon>
#include "core/application.h"


LinkItem::LinkItem(Storage::Link link) :
	link_(link)
{
	if(!link_.isNull()) {
		Storage* storage = qApp->storage();
		QString prefix = QString::fromStdString(storage->prefix(link_.prefix()).path());

		QFileInfo info(QDir(prefix), QString::fromStdString(link_.target()));
		arch_ = ModuleInfo::instance()->getArch(info.absoluteFilePath().toStdString());
	}
}


QString LinkItem::name() const
{
	QString path = QString::fromStdString(link_.path());
	int pos = path.lastIndexOf('/');
	if(pos == -1)
		return QString();

	return path.mid(pos + 1, path.count() - pos - 4);
}


void LinkItem::setName(const QString& name)
{
	QString path = QString::fromStdString(link_.path());
	int pos = path.lastIndexOf('/');
	if(pos == -1)
		return;

	path.truncate(pos + 1);
	path += name + ".so";
	if(link_.setPath(path.toStdString()))
		updateData();
}


QString LinkItem::location() const
{
	QString path = QString::fromStdString(link_.path());
	int pos = path.lastIndexOf('/');
	if(pos == -1)
		return QString();

	return path.left(pos);
}


void LinkItem::setLocation(const QString& path)
{
	QString location = path + '/' + name() + ".so";
	if(link_.setPath(location.toStdString()))
		updateData();
}


ModuleInfo::Arch LinkItem::arch() const
{
	return arch_;
}


QString LinkItem::prefix() const
{
	return QString::fromStdString(link_.prefix());
}


void LinkItem::setPrefix(const QString& prefix)
{
	link_.setPrefix(prefix.toStdString());
	updateData();
}


QString LinkItem::loader() const
{
	return QString::fromStdString(link_.loader());
}


void LinkItem::setLoader(const QString& loader)
{
	link_.setLoader(loader.toStdString());
	updateData();
}


QString LinkItem::target() const
{
	return QString::fromStdString(link_.target());
}


void LinkItem::setTarget(const QString& source)
{
	link_.setTarget(source.toStdString());
	updateData();
}


QString LinkItem::path() const
{
	return QString::fromStdString(link_.path());
}


void LinkItem::setPath(const QString& path)
{
	if(link_.setPath(path.toStdString()))
		updateData();
}


LogLevel LinkItem::logLevel() const
{
	return link_.logLevel();
}


void LinkItem::setLogLevel(LogLevel level)
{
	link_.setLogLevel(level);
	updateData();
}


LinksModel::LinksModel(QObject* parent) :
	GenericTreeModel<LinkItem>(new LinkItem(), parent)
{
	update();
}


int LinksModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 5;
}


QVariant LinksModel::data(const QModelIndex& index, int role) const
{
	if(index.isValid()) {
		LinkItem* item = indexToItem(index);

		if(role == Qt::DisplayRole) {
			int column = index.column();
			if(column == 0) {
				return item->name();
			}
			else if(column == 1) {
				return logLevelString(item->logLevel());
			}
			else if(column == 2) {
				return item->loader();
			}
			else if(column == 3) {
				return item->prefix();
			}
			else if(column == 4) {
				return item->target();
			}
		}
		else if(role == Qt::ToolTipRole) {
			int column = index.column();
			if(column == 0) {
				return item->path();
			}
			if(column == 1) {

			}
			if(column == 2) {
				auto loader = qApp->storage()->loader(item->loader().toStdString());
				return QString::fromStdString(loader.path());
			}
			if(column == 3) {
				auto prefix = qApp->storage()->prefix(item->prefix().toStdString());
				return QString::fromStdString(prefix.path());
			}
			if(column == 4) {
				return item->target();
			}
		}
		else if(role == Qt::DecorationRole) {
			int column = index.column();
			if(column == 0) {
				if(item->arch() == ModuleInfo::kArch32) {
					return QIcon(":/32bit.png");
				}
				else if(item->arch() == ModuleInfo::kArch64) {
					return QIcon(":/64bit.png");
				}
				else {
					return QIcon(":/unknown.png");
				}
			}
			else if(column == 1) {
				switch(item->logLevel()) {
				case LogLevel::kDefault:
					return QIcon(":/star.png");

				case LogLevel::kQuiet:
					return QIcon(":/mute.png");

				case LogLevel::kError:
					return QIcon(":/warning.png");

				case LogLevel::kTrace:
					return QIcon(":/trace.png");

				case LogLevel::kDebug:
					return QIcon(":/bug.png");

				case LogLevel::kFlood:
					return QIcon(":/scull.png");

				default:
					return QIcon();
				}
			}
		}
	}

	return QVariant();
}


QVariant LinksModel::headerData(int section, Qt::Orientation orientation,
		int role) const
{
	Q_UNUSED(orientation);

	if(role == Qt::DisplayRole) {
		if(section == 0) {
			return "Name";
		}
		else if(section == 1) {
			return "Log level";
		}
		else if(section == 2) {
			return "Loader";
		}
		else if(section == 3) {
			return "Prefix";
		}
		else if(section == 4) {
			return "VST plugin path (relative to prefix)";
		}
	}

	return QVariant();
}


LinkItem* LinksModel::createLink(const QString& name, const QString& location,
		const QString& target, const QString& prefix, const QString& loader)
{
	Storage* s = qApp->storage();

	QFileInfo info(QDir(location), name + ".so");
	std::string path = info.absoluteFilePath().toStdString();

	Storage::Link link = s->createLink(path, target.toStdString(), prefix.toStdString(),
			loader.toStdString());

	if(!link)
		return nullptr;

	LinkItem* item = new LinkItem(link);
	root()->insertChild(item);
	return item;
}


bool LinksModel::removeLink(LinkItem* item)
{
	if(!item || item->model() != this)
		return false;

	if(qApp->storage()->removeLink(item->link_)) {
		delete item->takeFromParent();
		return true;
	}

	return false;
}


void LinksModel::update()
{
	clear();

	Storage* s = qApp->storage();

	auto link = s->link();
	while(!link.isNull()) {
		LinkItem* item = new LinkItem(link);
		root()->insertChild(item);
		link = link.next();
	}
}


QString LinksModel::logLevelString(LogLevel level) const
{
	switch(level) {
	default:
	case LogLevel::kDefault:
		return "default";

	case LogLevel::kQuiet:
		return "quiet";

	case LogLevel::kError:
		return "error";

	case LogLevel::kTrace:
		return "trace";

	case LogLevel::kDebug:
		return "debug";

	case LogLevel::kFlood:
		return "flood";
	}
}
