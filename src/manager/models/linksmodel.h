#ifndef MODELS_LINKSMODEL_H
#define MODELS_LINKSMODEL_H

#include "common/logger.h"
#include "common/storage.h"
#include "core/moduleinfo.h"
#include "models/generictreemodel.h"


using Airwave::Storage;
using Airwave::LogLevel;


class LinkItem : public GenericTreeItem<LinkItem> {
public:
	LinkItem(Storage::Link link = Storage::Link());

	QString name() const;
	void setName(const QString& name);

	QString location() const;
	void setLocation(const QString& path);

	ModuleInfo::Arch arch() const;

	QString prefix() const;
	void setPrefix(const QString& prefix);

	QString loader() const;
	void setLoader(const QString& loader);

	QString target() const;
	void setTarget(const QString& source);

	QString path() const;
	void setPath(const QString& path);

	LogLevel logLevel() const;
	void setLogLevel(LogLevel level);

private:
	friend class LinksModel;

	Storage::Link link_;
	ModuleInfo::Arch arch_;
	LogLevel level_;
};


class LinksModel : public GenericTreeModel<LinkItem> {
	Q_OBJECT
public:
	LinksModel(QObject* parent = nullptr);

	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	QVariant headerData(int section, Qt::Orientation orientation,
			int role = Qt::DisplayRole) const;

	LinkItem* createLink(const QString& name, const QString& location,
			const QString& target, const QString& prefix, const QString& loader);

	bool removeLink(LinkItem* item);

	void update();

private:
	QString logLevelString(LogLevel level) const;
};


#endif // MODELS_LINKSMODEL_H
