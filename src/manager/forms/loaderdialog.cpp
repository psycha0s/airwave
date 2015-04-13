#include "loaderdialog.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include "core/application.h"
#include "forms/filedialog.h"
#include "models/loadersmodel.h"
#include "widgets/lineedit.h"


LoaderDialog::LoaderDialog(QWidget* parent) :
	QDialog(parent),
	item_(nullptr)
{
	setupUi();
}


void LoaderDialog::setupUi()
{
	setWindowIcon(QIcon(":/windows.png"));
	setWindowTitle("WINE loader properties");
	setMinimumWidth(300);
	setFixedHeight(100);
	resize(300, 100);

	nameEdit_ = new LineEdit;

	pathEdit_ = new LineEdit;
	pathEdit_->setButtonEnabled(true);
	pathEdit_->setButtonStyle(LineEdit::kLightAutoRaise);
	pathEdit_->setButtonIcon(QIcon(":/open.png"));
	pathEdit_->setButtonToolTip("Browse");
	connect(pathEdit_, SIGNAL(buttonClicked()), SLOT(browseForWineLoader()));

	buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttons_, SIGNAL(accepted()), SLOT(accept()));
	connect(buttons_, SIGNAL(rejected()), SLOT(reject()));

	QGridLayout* mainLayout = new QGridLayout;

	mainLayout->addWidget(new QLabel("Name:"), 0, 0, Qt::AlignRight);
	mainLayout->addWidget(nameEdit_, 0, 1);

	mainLayout->addWidget(new QLabel("Path:"), 1, 0, Qt::AlignRight);
	mainLayout->addWidget(pathEdit_, 1, 1);

	mainLayout->addWidget(new QWidget, 2, 1);
	mainLayout->setRowStretch(2, 1);

	mainLayout->addWidget(buttons_, 3, 0, 1, -1);

	setLayout(mainLayout);
}


void LoaderDialog::browseForWineLoader()
{
	FileDialog dialog(FileDialog::kOpenDialog);
	dialog.setAcceptMode(FileDialog::kAcceptExistingFile);
	dialog.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
	dialog.setWindowTitle("Select the WINE loader binary");

	QString text = pathEdit_->text();
	if(text.isEmpty()) {
		dialog.setDirectory(QDir::homePath());
	}
	else {
		dialog.setDirectory(text);
	}

	if(dialog.exec()) {
		pathEdit_->setText(dialog.selectedPath());
	}
}

LoaderItem* LoaderDialog::item() const
{
	return item_;
}


void LoaderDialog::setItem(LoaderItem* item)
{
	item_ = item;

	if(item_) {
		nameEdit_->setText(item->name());
		pathEdit_->setText(item->path());
	}
	else {
		nameEdit_->clear();
		pathEdit_->clear();
	}
}


void LoaderDialog::accept()
{
	QString name = nameEdit_->text();
	QString message = QString("The loader with name '%1' is already exist.").arg(name);

	if(!item_) {
		if(!qApp->loaders()->createLoader(nameEdit_->text(), pathEdit_->text())) {
			QMessageBox::critical(this, "Error", message);
			return;
		}
	}
	else if(name != item_->name()) {
		Storage::Loader loader = qApp->storage()->loader(name.toStdString());
		if(!loader.isNull()) {
			QMessageBox::critical(this, "Error", message);
			return;
		}

		item_->setName(nameEdit_->text());
		item_->setPath(pathEdit_->text());
	}
	else {
		item_->setPath(pathEdit_->text());
	}

	QDialog::accept();
}
