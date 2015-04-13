#ifndef MODELS_DIRECTORYMODEL_H
#define MODELS_DIRECTORYMODEL_H

#include <QDir>
#include <QFileSystemWatcher>
#include "common/types.h"
#include "models/generictreemodel.h"


class DirectoryItem : public GenericTreeItem<DirectoryItem> {
public:
	DirectoryItem(const QFileInfo& info = QFileInfo());

	bool isDirectory() const;

	QString name() const;
	QString path() const;
	QString fullPath() const;
	i64 size() const;

	QString humanReadableSize() const;
	QString type() const;

private:
	QFileInfo info_;
	QString type_;

	QString getType() const;
};


class DirectoryModel : public GenericTreeModel<DirectoryItem> {
	Q_OBJECT
public:
	DirectoryModel(QObject* parent = nullptr);

	int columnCount(const QModelIndex& parent = QModelIndex()) const;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	QVariant headerData(int section, Qt::Orientation orientation,
			int role = Qt::DisplayRole) const;

	Qt::ItemFlags flags(const QModelIndex& index) const;

	QString directory() const;
	QStringList nameFilters() const;
	QDir::Filters filters() const;
	bool isFilesEnabled() const;

public slots:
	void setDirectory(const QString& path);
	void setNameFilters(const QStringList& filters);
	void setFilters(QDir::Filters filters);
	void setFilesEnabled(bool enabled);

signals:
	void directoryChanged(const QString& path);

private:
	QDir dir_;
	QFileSystemWatcher watcher_;
	QStringList nameFilters_;
	QDir::Filters filters_;
	bool isFilesEnabled_;

	void sort(int column, Qt::SortOrder order);

	static bool lessThan(DirectoryItem* item1, DirectoryItem* item2);
	static bool greaterThan(DirectoryItem* item1, DirectoryItem* item2);

private slots:

	void update(const QString& path);
};


#endif // MODELS_DIRECTORYMODEL_H
