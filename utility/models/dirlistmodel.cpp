#include "dirlistmodel.h"

#include <QApplication>
#include <QStyle>


DirListItem::DirListItem(const QFileInfo& info) :
	info_(info)
{
}


bool DirListItem::isDirectory() const
{
	return info_.isDir();
}


QString DirListItem::name() const
{
	return info_.fileName();
}


QString DirListItem::path() const
{
	return info_.filePath();
}


QString DirListItem::fullPath() const
{
	return info_.canonicalFilePath();
}


DirListModel::DirListModel(QObject* parent) :
	GenericTreeModel<DirListItem>(new DirListItem(), parent),
	showHidden_(false)
{
	connect(&watcher_,
			SIGNAL(directoryChanged(QString)),
			SLOT(update(QString)));
}


QString DirListModel::currentDir() const
{
	return dir_.canonicalPath();
}


void DirListModel::setDir(const QString& path)
{
	watcher_.removePath(dir_.canonicalPath());

	update(path);
	dir_ = QDir(path);

	watcher_.addPath(dir_.canonicalPath());

	emit currentDirChanged(dir_.canonicalPath());
}


void DirListModel::setShowHidden(bool enable)
{
	if(showHidden_ != enable) {
		showHidden_ = enable;
		setDir(currentDir());
	}
}


void DirListModel::setNameFilters(const QStringList& filters)
{
	nameFilters_ = filters;
	setDir(dir_.canonicalPath());
}


int DirListModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 1;
}


QVariant DirListModel::data(const QModelIndex& index, int role) const
{
	if(index.isValid()) {
		DirListItem* item = indexToItem(index);

		if(role == Qt::DisplayRole) {
			if(index.column() == 0) {
				return item->name();
			}
		}
		else if(role == Qt::DecorationRole) {
			if(index.column() == 0) {
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
	}

	return QVariant();
}


QVariant DirListModel::headerData(int section, Qt::Orientation orientation,
		int role) const
{
	Q_UNUSED(orientation);

	if(section == 0 && role == Qt::DisplayRole)
		return tr("Name");

	return QVariant();
}


void DirListModel::sort(int column, Qt::SortOrder order)
{
	Q_UNUSED(column);

	emit layoutAboutToBeChanged();

	if(order == Qt::AscendingOrder) {
		rootItem()->sortChildren(lessThan);
	}
	else {
		rootItem()->sortChildren(greaterThan);
	}

	emit layoutChanged();
}


bool DirListModel::lessThan(DirListItem* item1, DirListItem* item2)
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


bool DirListModel::greaterThan(DirListItem* item1, DirListItem* item2)
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


void DirListModel::update(const QString& path)
{
	QDir dir(path);
	if(!dir.exists())
		return;

	clear();

	dir = dir.canonicalPath();

	QDir::Filters filters = QDir::NoDot | QDir::AllDirs | QDir::Files;
	if(showHidden_)
		filters |= QDir::Hidden;

	dir.setFilter(filters);
	dir.setNameFilters(nameFilters_);

	for(const QFileInfo& info : dir.entryInfoList()) {
		if(dir.isRoot() && info.fileName() == "..")
			continue;

		DirListItem* item = new DirListItem(info);
		rootItem()->insertChild(item);
	}

	sort(0, Qt::AscendingOrder);
}
