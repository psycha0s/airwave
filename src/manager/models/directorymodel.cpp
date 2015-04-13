#include "directorymodel.h"

#include <QApplication>
#include <QIcon>
#include <QStyle>


DirectoryItem::DirectoryItem(const QFileInfo& info) :
	info_(info),
	type_(getType())
{
}


bool DirectoryItem::isDirectory() const
{
	return info_.isDir();
}


QString DirectoryItem::name() const
{
	return info_.fileName();
}


QString DirectoryItem::path() const
{
	return info_.filePath();
}


QString DirectoryItem::fullPath() const
{
	return info_.canonicalFilePath();
}


i64 DirectoryItem::size() const
{
	return i64(info_.size());
}


QString DirectoryItem::humanReadableSize() const
{
	const char* kUnitsTable[] = { " bytes", " KiB", " MiB", " GiB", " TiB" };
	double value = size();
	uint i = 0;

	while(value >= 1024.0 && i < sizeof(kUnitsTable)) {
		value /= 1024.0;
		++i;
	}

	return QString().setNum(value, 'f', 1) + kUnitsTable[i];
}


QString DirectoryItem::type() const
{
	return type_;
}


QString DirectoryItem::getType() const
{
	if(info_.isRoot())
		return "Drive";

	if(info_.isFile()) {
		if(!info_.suffix().isEmpty())
			return QString("%1 File").arg(info_.suffix());

		return "File";
	}

	if(info_.isDir())
		return "Folder";

	if(info_.isSymLink())
		return "Shortcut";

	return "Unknown";
}


DirectoryModel::DirectoryModel(QObject* parent) :
	GenericTreeModel<DirectoryItem>(new DirectoryItem(), parent),
	filters_(QDir::AllEntries),
	isFilesEnabled_(true)
{
	connect(&watcher_,
			SIGNAL(directoryChanged(QString)),
			SLOT(update(QString)));
}


int DirectoryModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 3;
}


QVariant DirectoryModel::data(const QModelIndex& index, int role) const
{
	if(index.isValid()) {
		DirectoryItem* item = indexToItem(index);

		if(role == Qt::DisplayRole) {
			int column = index.column();
			if(column == 0) {
				return item->name();
			}
			else if(column == 1) {
				if(!item->isDirectory()) {
					return item->humanReadableSize();
				}

				return "<DIR>";
			}
			else if(column == 2) {
				return item->type();
			}
		}
		else if(role == Qt::DecorationRole && index.column() == 0) {
			if(item->name() == "..") {
				return qApp->style()->standardIcon(QStyle::SP_ArrowUp);
			}
			else if(item->isDirectory()) {
				return qApp->style()->standardIcon(QStyle::SP_DirIcon);
			}
			else {
				return qApp->style()->standardIcon(QStyle::SP_FileIcon);
			}
		}
	}

	return QVariant();
}


QVariant DirectoryModel::headerData(int section, Qt::Orientation orientation,
		int role) const
{
	Q_UNUSED(orientation);

	if(role == Qt::DisplayRole) {
		if(section == 0) {
			return "Name";
		}
		else if(section == 1) {
			return "Size";
		}
		else if(section == 2) {
			return "Type";
		}
	}

	return QVariant();
}


Qt::ItemFlags DirectoryModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags = GenericTreeModel<DirectoryItem>::flags(index);

	DirectoryItem* item = indexToItem(index);
	if(!item->isDirectory() && !isFilesEnabled_)
		flags &= ~Qt::ItemIsEnabled;

	return flags;
}


void DirectoryModel::update(const QString& path)
{
	QDir dir(path);
	if(!dir.exists())
		return;

	clear();

	dir_ = dir.canonicalPath();
	dir_.setFilter(filters_);
	dir_.setNameFilters(nameFilters_);

	for(const QFileInfo& info : dir_.entryInfoList()) {
		if(dir.isRoot() && info.fileName() == "..")
			continue;

		DirectoryItem* item = new DirectoryItem(info);
		root()->insertChild(item);
	}

	sort(0, Qt::AscendingOrder);
}


QString DirectoryModel::directory() const
{
	return dir_.canonicalPath();
}


void DirectoryModel::setDirectory(const QString& path)
{
	watcher_.removePath(dir_.canonicalPath());

	update(path);

	watcher_.addPath(dir_.canonicalPath());
	emit directoryChanged(dir_.canonicalPath());
}


QStringList DirectoryModel::nameFilters() const
{
	return QStringList();
}


void DirectoryModel::setNameFilters(const QStringList& filters)
{
	nameFilters_ = filters;
	update(directory());
}


QDir::Filters DirectoryModel::filters() const
{
	return filters_;
}


void DirectoryModel::setFilters(QDir::Filters filters)
{
	filters_ = filters;
	update(directory());
}


void DirectoryModel::sort(int column, Qt::SortOrder order)
{
	Q_UNUSED(column);
	emit layoutAboutToBeChanged();

	if(order == Qt::AscendingOrder) {
		root()->sortChildren(lessThan);
	}
	else {
		root()->sortChildren(greaterThan);
	}

	emit layoutChanged();
}


bool DirectoryModel::lessThan(DirectoryItem* item1, DirectoryItem* item2)
{
	if(item1->name() == "..") {
		return true;
	}
	else if(item2->name() == "..") {
		return false;
	}

	if(item1->isDirectory() == item2->isDirectory())
		return item1->name().compare(item2->name(), Qt::CaseInsensitive) < 0;

	return item1->isDirectory();
}


bool DirectoryModel::greaterThan(DirectoryItem* item1, DirectoryItem* item2)
{
	if(item1->name() == "..") {
		return false;
	}
	else if(item2->name() == "..") {
		return true;
	}

	if(item1->isDirectory() == item2->isDirectory())
		return item1->name().compare(item2->name(), Qt::CaseInsensitive) > 0;

	return item2->isDirectory();
}


bool DirectoryModel::isFilesEnabled() const
{
	return isFilesEnabled_;
}


void DirectoryModel::setFilesEnabled(bool enabled)
{
	if(isFilesEnabled_ == enabled)
		return;

	isFilesEnabled_ = enabled;
	update(directory());
}
