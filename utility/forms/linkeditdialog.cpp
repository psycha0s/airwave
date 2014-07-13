#include "linkeditdialog.h"
#include "ui_linkeditdialog.h"


LinkEditDialog::LinkEditDialog(QWidget *parent) :
	QDialog(parent),
	ui_(new Ui::LinkEditDialog)
{
	ui_->setupUi(this);
	setFixedSize(size());
}


LinkEditDialog::~LinkEditDialog()
{
	delete ui_;
}


void LinkEditDialog::setLinkName(const QString& name)
{
	ui_->lineEdit->setText(name);
}


QString LinkEditDialog::linkName() const
{
	return ui_->lineEdit->text();
}
