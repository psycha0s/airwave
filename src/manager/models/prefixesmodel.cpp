#include "prefixesmodel.h"

#include <QIcon>
#include "core/application.h"
#include "models/linksmodel.h"


PrefixItem::PrefixItem(Storage::Prefix prefix) :
	prefix_(prefix)
{
}


QString PrefixItem::name() const
{
	return QString::fromStdString(prefix_.name());
}


void PrefixItem::setName(const QString& name)
{
	if(prefix_.setName(name.toStdString())) {
		updateData();
		qApp->links()->update();
	}
}


QString PrefixItem::path() const
{
	return QString::fromStdString(prefix_.path());
}


void PrefixItem::setPath(const QString& path)
{
	prefix_.setPath(path.toStdString());
	updateData();
}


PrefixesModel::PrefixesModel(QObject* parent) :
	GenericTreeModel<PrefixItem>(new PrefixItem(), parent)
{
	Storage* s = qApp->storage();

	auto prefix = s->prefix();
	while(!prefix.isNull()) {
		PrefixItem* item = new PrefixItem(prefix);
		root()->insertChild(item);
		prefix = prefix.next();
	}
}


int PrefixesModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 2;
}


QVariant PrefixesModel::data(const QModelIndex& index, int role) const
{
	if(index.isValid()) {
		PrefixItem* item = indexToItem(index);

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


QVariant PrefixesModel::headerData(int section, Qt::Orientation orientation,
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


PrefixItem* PrefixesModel::createPrefix(const QString& name, const QString& path)
{
	Storage* s = qApp->storage();
	Storage::Prefix prefix = s->createPrefix(name.toStdString(), path.toStdString());
	if(!prefix)
		return nullptr;

	PrefixItem* item = new PrefixItem(prefix);
	root()->insertChild(item);
	return item;
}


bool PrefixesModel::removePrefix(PrefixItem* item)
{
	if(!item || item == root())
		return false;

	if(!qApp->storage()->removePrefix(item->prefix_))
		return false;

	delete item->takeFromParent();
	return true;
}
