#ifndef MODELS_LOADERSMODEL_H
#define MODELS_LOADERSMODEL_H

#include "generictreemodel.h"
#include "common/storage.h"


using Airwave::Storage;


class LoaderItem : public GenericTreeItem<LoaderItem> {
public:
	LoaderItem(Storage::Loader loader = Storage::Loader());

	QString name() const;
	void setName(const QString& name);

	QString path() const;
	void setPath(const QString& path);

private:
	friend class LoadersModel;

	Storage::Loader loader_;
};


class LoadersModel : public GenericTreeModel<LoaderItem> {
	Q_OBJECT
public:
	LoadersModel(QObject* parent = nullptr);

	int columnCount(const QModelIndex& parent = QModelIndex()) const;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	QVariant headerData(int section, Qt::Orientation orientation,
			int role = Qt::DisplayRole) const;

	LoaderItem* createLoader(const QString& name, const QString& path);
	bool removeLoader(LoaderItem* item);
};


#endif // MODELS_LOADERSMODEL_H
