#ifndef FORMS_PREFIXDIALOG_H
#define FORMS_PREFIXDIALOG_H

#include <QDialog>


class QDialogButtonBox;
class LineEdit;
class PrefixItem;


class PrefixDialog : public QDialog {
	Q_OBJECT
public:
	PrefixDialog(QWidget* parent = nullptr);

	PrefixItem* item() const;
	void setItem(PrefixItem* item);

private:
	LineEdit* nameEdit_;
	LineEdit* pathEdit_;
	QDialogButtonBox* buttons_;
	PrefixItem* item_;

	void setupUi();

private slots:
	void browseForWinePrefix();
	void accept();
};


#endif // FORMS_PREFIXDIALOG_H
