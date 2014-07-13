#ifndef UTILITY_MODELS_LINKSMODEL_H
#define UTILITY_MODELS_LINKSMODEL_H

#include "generictreemodel.h"
#include "common/linkmanager.h"


class LinkItem : public GenericTreeItem<LinkItem> {
public:
	LinkItem(const QString& pluginPath = QString(),
			const QString& bridgePath = QString(),
			const QString& arch = QString());

	QString pluginPath() const;
	QString bridgePath() const;
	QString architecture() const;

private:
	QString pluginPath_;
	QString bridgePath_;
	QString architecture_;
};


class LinksModel : public GenericTreeModel<LinkItem> {
public:
	LinksModel(QObject* parent = nullptr);

	int columnCount(const QModelIndex& parent = QModelIndex()) const;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	QVariant headerData(int section, Qt::Orientation orientation,
			int role = Qt::DisplayRole ) const;

	bool setLink(const QString& pluginPath, const QString& bridgePath);

	bool updateLink(const QString& bridgePath, const QString& newPath);

	bool removeLink(const QString& bridgePath);

private:
	Airwave::LinkManager linkManager_;

	void reload();
};


#endif // UTILITY_MODELS_LINKSMODEL_H
