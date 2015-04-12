#include "prefixdialog.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include "core/application.h"
#include "forms/filedialog.h"
#include "models/prefixesmodel.h"
#include "widgets/lineedit.h"


PrefixDialog::PrefixDialog(QWidget* parent) :
	QDialog(parent)
{
	setupUi();
}


void PrefixDialog::setupUi()
{
	setWindowIcon(QIcon(":/windows.png"));
	setWindowTitle("WINE prefix properties");
	setMinimumWidth(300);
	setFixedHeight(100);
	resize(300, 100);

	nameEdit_ = new LineEdit;

	pathEdit_ = new LineEdit;
	pathEdit_->setButtonEnabled(true);
	pathEdit_->setButtonStyle(LineEdit::kLightAutoRaise);
	pathEdit_->setButtonIcon(QIcon(":/open.png"));
	pathEdit_->setButtonToolTip("Browse");
	connect(pathEdit_, SIGNAL(buttonClicked()), SLOT(browseForWinePrefix()));

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


void PrefixDialog::browseForWinePrefix()
{
	FileDialog dialog(FileDialog::kOpenDialog);
	dialog.setAcceptMode(FileDialog::kAcceptExistingDirectory);
	dialog.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
	dialog.setWindowTitle("Select the WINE prefix directory");

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


QString PrefixDialog::name() const
{
	return nameEdit_->text();
}


void PrefixDialog::setName(const QString& name)
{
	nameEdit_->setText(name);
}


QString PrefixDialog::path() const
{
	return pathEdit_->text();
}


void PrefixDialog::setPath(const QString& path)
{
	pathEdit_->setText(path);
}


void PrefixDialog::accept()
{
	if(!qApp->prefixes()->createPrefix(nameEdit_->text(), pathEdit_->text())) {
		// TODO messagebox
		return;
	}

	QDialog::accept();
}
