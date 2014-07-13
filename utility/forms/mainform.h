#ifndef UTILITY_FORMS_MAINFORM_H
#define UTILITY_FORMS_MAINFORM_H

#include <QWidget>
#include "models/dirlistmodel.h"
#include "models/linksmodel.h"


namespace Ui {
class MainForm;
}


class MainForm : public QWidget {
	Q_OBJECT
public:
	explicit MainForm(QWidget* parent = 0);
	~MainForm();

private:
	Ui::MainForm* ui_;

	LinksModel* linksModel_;
	DirListModel* sourceModel_;
	DirListModel* targetModel_;

private slots:
	void onShowHiddenButtonToggled(bool value);
	void onShowLinkedButtonClicked();

	void onDirItemDoubleClicked(const QModelIndex& index);
	void onCurrentLinkChanged();
	void onCurrentPluginChanged();

	void onCreateLinkButtonClicked();
	void onEditLinkButtonClicked();
	void onRemoveLinkButtonClicked();
	void onRecreateLinksButtonClicked();
};


#endif // UTILITY_FORMS_MAINFORM_H
