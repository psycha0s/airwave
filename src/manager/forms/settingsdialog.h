#ifndef FORMS_SETTINGSDIALOG_H
#define FORMS_SETTINGSDIALOG_H

#include <QDialog>


class QComboBox;
class QDialogButtonBox;
class QLabel;
class QPushButton;
class LineEdit;
class LoadersModel;
class LoadersView;
class PrefixesModel;
class PrefixesView;


class SettingsDialog : public QDialog {
	Q_OBJECT
public:
	SettingsDialog(QWidget* parent = nullptr);
	~SettingsDialog();

private:
	LineEdit* vstPathEdit_;
	LineEdit* binariesPathEdit_;
	LineEdit* logSocketEdit_;
	QComboBox* logLevelCombo_;
	PrefixesView* prefixesView_;
	QPushButton* addPrefixButton_;
	QPushButton* editPrefixButton_;
	QPushButton* removePrefixButton_;
	LoadersView* loadersView_;
	QPushButton* addLoaderButton_;
	QPushButton* editLoaderButton_;
	QPushButton* removeLoaderButton_;
	QDialogButtonBox* buttons_;

	void setupUi();

private slots:
	void browseForVstPath();
	void browseForBinariesPath();
	void browseForSocketPath();
	void createPrefix();
	void editPrefix();
	void removePrefix();
	void createLoader();
	void editLoader();
	void removeLoader();
	void accept();
};


#endif // FORMS_SETTINGSDIALOG_H
