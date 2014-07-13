#ifndef UTILITY_MODELS_DIRLISTMODEL_H
#define UTILITY_MODELS_DIRLISTMODEL_H

#include <QDir>
#include <QFileSystemWatcher>
#include <QStringList>
#include "generictreemodel.h"


class DirListItem : public GenericTreeItem<DirListItem> {
public:
	DirListItem(const QFileInfo& info = QFileInfo());

	bool isDirectory() const;
	QString name() const;
	QString path() const;
	QString fullPath() const;

private:
	QFileInfo info_;
};



class DirListModel : public GenericTreeModel<DirListItem> {
	Q_OBJECT
public:
	DirListModel(QObject* parent = nullptr);

	QString currentDir() const;
	void setDir(const QString& path);
	void setShowHidden(bool enable);
	void setNameFilters(const QStringList& filters);

	int columnCount(const QModelIndex& parent = QModelIndex()) const;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	QVariant headerData(int section, Qt::Orientation orientation,
			int role = Qt::DisplayRole ) const;

signals:
	void currentDirChanged(const QString& path);

private:
	QDir dir_;
	QFileSystemWatcher watcher_;
	bool showHidden_;
	QStringList nameFilters_;

	void sort(int column, Qt::SortOrder order);

	static bool lessThan(DirListItem* item1, DirListItem* item2);
	static bool greaterThan(DirListItem* item1, DirListItem* item2);

private slots:
	void update(const QString& path);
};


#endif // UTILITY_MODELS_DIRLISTMODEL_H
