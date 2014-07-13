#ifndef UTILITY_FORMS_LINKEDITDIALOG_H
#define UTILITY_FORMS_LINKEDITDIALOG_H

#include <QDialog>


namespace Ui {
class LinkEditDialog;
}


class LinkEditDialog : public QDialog {
	Q_OBJECT
public:
	explicit LinkEditDialog(QWidget* parent = nullptr);
	~LinkEditDialog();

	void setLinkName(const QString& name);
	QString linkName() const;

private:
	Ui::LinkEditDialog* ui_;
};


#endif // UTILITY_FORMS_LINKEDITDIALOG_H
