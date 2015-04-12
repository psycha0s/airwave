#ifndef FORMS_PREFIXDIALOG_H
#define FORMS_PREFIXDIALOG_H

#include <QDialog>


class QDialogButtonBox;
class LineEdit;


class PrefixDialog : public QDialog {
	Q_OBJECT
public:
	PrefixDialog(QWidget* parent = nullptr);

	QString name() const;
	void setName(const QString& name);

	QString path() const;
	void setPath(const QString& path);


private:
	LineEdit* nameEdit_;
	LineEdit* pathEdit_;
	QDialogButtonBox* buttons_;

	void setupUi();

private slots:
	void browseForWinePrefix();
	void accept();
};


#endif // FORMS_PREFIXDIALOG_H
