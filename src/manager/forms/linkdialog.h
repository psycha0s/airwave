#ifndef FORMS_LINKEDITDIALOG_H
#define FORMS_LINKEDITDIALOG_H

#include <QDialog>


class QComboBox;
class QDialogButtonBox;
class LineEdit;
class LinkItem;


class LinkDialog : public QDialog {
	Q_OBJECT
public:
	LinkDialog(QWidget* parent = nullptr);

	LinkItem* item() const;
	void setItem(LinkItem* item);

private:
	QComboBox* loaderCombo_;
	QComboBox* prefixCombo_;
	QComboBox* logLevelCombo_;
	LineEdit* targetEdit_;
	LineEdit* locationEdit_;
	LineEdit* nameEdit_;
	QDialogButtonBox* buttons_;
	LinkItem* item_;

	void setupUi();
	QString currentPrefix() const;
	QString getPluginPath() const;

private slots:
	void browsePlugin();
	void browseLocation();
	void onPrefixChanged();
	void accept();
};


#endif // FORMS_LINKEDITDIALOG_H
