#ifndef FORMS_FILEDIALOG_H
#define FORMS_FILEDIALOG_H

#include <QDialog>
#include <QDir>


class QFileSystemModel;
class QItemSelection;
class QPushButton;
class QToolButton;
class DirectoryItem;
class DirectoryModel;
class DirectoryView;
class LineEdit;


class FileDialog : public QDialog {
	Q_OBJECT
public:
	enum DialogMode {
		kOpenDialog,
		kSaveDialog
	};

	enum AcceptMode {
		kAcceptFile,
		kAcceptExistingFile,
		kAcceptDirectory,
		kAcceptExistingDirectory
	};

	FileDialog(DialogMode mode, QWidget* parent = nullptr);

	AcceptMode acceptMode() const;
	void setAcceptMode(AcceptMode mode);

	QDir::Filters filters() const;
	void setFilter(QDir::Filters filters);

	QStringList	nameFilters() const;
	void setNameFilter(const QString& filter);
	void setNameFilters(const QStringList& filters);

	bool showHidden() const;
	QString directory() const;
	QString rootDirectory() const;
	QString selectedPath() const;
	QString selectedName() const;
	QString defaultSuffix() const;

public slots:
	void setShowHidden(bool enable);
	void setDirectory(const QString& path);
	void setRootDirectory(const QString& path);
	void setDefaultSuffix(const QString& suffix);
	void accept();
	int exec();

private:
	LineEdit* currentDirEdit_;
	QToolButton* goUpButton_;
	QToolButton* createDirButton_;
	QToolButton* toggleHiddenButton_;

	DirectoryModel* model_;
	DirectoryView* view_;

	LineEdit* nameEdit_;
	QPushButton* actionButton_;

	QPushButton* cancelButton_;

	DialogMode dialogMode_;
	AcceptMode acceptMode_;

	QString rootPath_;
	QString selectedPath_;
	QString suffix_;

	void setupUi();

private slots:
	void goUp();
	void onItemChanged(DirectoryItem* item);
	void onItemDoubleClicked(DirectoryItem* item);
	void onNameEditChanged(const QString& text);
	void onCreateDirButtonClicked();
};


#endif //FORMS_FILEDIALOG_H
