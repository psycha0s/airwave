#include "loadersmodel.h"

#include <QIcon>
#include "core/application.h"
#include "models/linksmodel.h"


LoaderItem::LoaderItem(Storage::Loader loader) :
	loader_(loader)
{
}


QString LoaderItem::name() const
{
	return QString::fromStdString(loader_.name());
}


void LoaderItem::setName(const QString& name)
{
	if(loader_.setName(name.toStdString())) {
		updateData();
		qApp->links()->update();
	}
}


QString LoaderItem::path() const
{
	return QString::fromStdString(loader_.path());
}


void LoaderItem::setPath(const QString& path)
{
	loader_.setPath(path.toStdString());
	updateData();
}


LoadersModel::LoadersModel(QObject* parent) :
	GenericTreeModel<LoaderItem>(new LoaderItem(), parent)
{
	Storage* s = qApp->storage();

	auto loader = s->loader();
	while(!loader.isNull()) {
		LoaderItem* item = new LoaderItem(loader);
		root()->insertChild(item);
		loader = loader.next();
	}
}


int LoadersModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 2;
}


QVariant LoadersModel::data(const QModelIndex& index, int role) const
{
	if(index.isValid()) {
		LoaderItem* item = indexToItem(index);

		if(role == Qt::DisplayRole) {
			if(index.column() == 0) {
				return item->name();
			}
			else if(index.column() == 1) {
				return item->path();
			}
		}
	}

	return QVariant();
}


QVariant LoadersModel::headerData(int section, Qt::Orientation orientation,
		int role) const
{
	Q_UNUSED(orientation);

	if(role == Qt::DisplayRole) {
		if(section == 0) {
			return "Name";
		}
		else if(section == 1) {
			return "Path";
		}
	}

	return QVariant();
}


LoaderItem* LoadersModel::createLoader(const QString& name, const QString& path)
{
	Storage* s = qApp->storage();
	Storage::Loader loader = s->createLoader(name.toStdString(), path.toStdString());
	if(!loader)
		return nullptr;

	LoaderItem* item = new LoaderItem(loader);
	root()->insertChild(item);
	return item;
}


bool LoadersModel::removeLoader(LoaderItem* item)
{
	if(!item || item == root())
		return false;

	if(!qApp->storage()->removeLoader(item->loader_))
		return false;

	delete item->takeFromParent();
	return true;
}
