#ifndef FORMS_LOADERDIALOG_H
#define FORMS_LOADERDIALOG_H

#include <QDialog>


class QDialogButtonBox;
class LineEdit;
class LoaderItem;


class LoaderDialog : public QDialog {
	Q_OBJECT
public:
	LoaderDialog(QWidget* parent = nullptr);

	LoaderItem* item() const;
	void setItem(LoaderItem* item);

private:
	LineEdit* nameEdit_;
	LineEdit* pathEdit_;
	QDialogButtonBox* buttons_;
	LoaderItem* item_;

	void setupUi();

private slots:
	void browseForWineLoader();
	void accept();
};


#endif // FORMS_LOADERDIALOG_H
