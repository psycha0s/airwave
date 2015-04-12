#ifndef FORMS_FOLDERDIALOG_H
#define FORMS_FOLDERDIALOG_H

#include <QDialog>


class QDialogButtonBox;
class DirectoryModel;
class LineEdit;


class FolderDialog : public QDialog {
	Q_OBJECT
public:
	FolderDialog(DirectoryModel* model, QWidget* parent = nullptr);

public slots:
	void accept();

private:
	DirectoryModel* model_;
	LineEdit* nameEdit_;
	QDialogButtonBox* buttons_;

	void setupUi();
};


#endif // FORMS_FOLDERDIALOG_H
