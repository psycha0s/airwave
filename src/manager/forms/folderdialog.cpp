#include "folderdialog.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include "models/directorymodel.h"
#include "widgets/lineedit.h"


FolderDialog::FolderDialog(DirectoryModel* model, QWidget* parent) :
	QDialog(parent),
	model_(model)
{
	setupUi();
}


void FolderDialog::accept()
{
	QString name = nameEdit_->text();

	QDir dir(model_->directory() + '/' + name);
	if(dir.exists()) {
		QString message = QString("Directory '%1' is already exists").arg(name);
		QMessageBox::critical(this, "Error", message);
	}
	else {
		dir.mkpath(".");
		QDialog::accept();
	}
}


void FolderDialog::setupUi()
{
	setWindowTitle("New folder");

	nameEdit_ = new LineEdit;
	nameEdit_->setText("New folder");
	nameEdit_->selectAll();

	buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttons_, SIGNAL(accepted()), SLOT(accept()));
	connect(buttons_, SIGNAL(rejected()), SLOT(reject()));

	QGridLayout* layout = new QGridLayout;
	layout->addWidget(new QLabel("Folder name:"), 0, 0, Qt::AlignRight);
	layout->addWidget(nameEdit_, 0, 1);
	layout->addWidget(buttons_, 1, 0, 1, 2);

	setLayout(layout);
}
