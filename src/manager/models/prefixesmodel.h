#ifndef MODELS_PREFIXMODEL_H
#define MODELS_PREFIXMODEL_H

#include "generictreemodel.h"
#include "common/storage.h"


using Airwave::Storage;


class PrefixItem : public GenericTreeItem<PrefixItem> {
public:
	PrefixItem(Storage::Prefix prefix = Storage::Prefix());

	QString name() const;
	void setName(const QString& name);

	QString path() const;
	void setPath(const QString& path);

private:
	friend class PrefixesModel;

	Storage::Prefix prefix_;
};


class PrefixesModel : public GenericTreeModel<PrefixItem> {
	Q_OBJECT
public:
	PrefixesModel(QObject* parent = nullptr);

	int columnCount(const QModelIndex& parent = QModelIndex()) const;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	QVariant headerData(int section, Qt::Orientation orientation,
			int role = Qt::DisplayRole) const;

	PrefixItem* createPrefix(const QString& name, const QString& path);
	bool removePrefix(PrefixItem* item);
};


#endif // MODELS_PREFIXMODEL_H
