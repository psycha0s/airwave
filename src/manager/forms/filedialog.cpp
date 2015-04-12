#include "filedialog.h"

#include <QIcon>
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include "forms/folderdialog.h"
#include "models/directorymodel.h"
#include "widgets/directoryview.h"
#include "widgets/lineedit.h"


FileDialog::FileDialog(DialogMode mode, QWidget* parent) :
	QDialog(parent),
	dialogMode_(mode),
	acceptMode_(kAcceptFile)
{
	setupUi();
	setRootDirectory(QDir::rootPath());
	setDirectory(QDir::homePath());
}


void FileDialog::setupUi()
{
	resize(600, 450);

	QLabel* label = new QLabel("Directory: ");
	currentDirEdit_ = new LineEdit;
	currentDirEdit_->setReadOnly(true);
	currentDirEdit_->setStyleSheet("QLineEdit { border: 1px solid #AAAAAA; }");

	goUpButton_ = new QToolButton;
	goUpButton_->setIconSize(QSize(20, 20));
	goUpButton_->setIcon(QIcon(":/go_up.png"));
	goUpButton_->setToolTip("Go to the upper folder");
	goUpButton_->setAutoRaise(true);
	connect(goUpButton_, SIGNAL(clicked()), SLOT(goUp()));

	createDirButton_ = new QToolButton;
	createDirButton_->setIconSize(QSize(20, 20));
	createDirButton_->setIcon(QIcon(":/create_folder.png"));
	createDirButton_->setToolTip("Create new folder");
	createDirButton_->setAutoRaise(true);
	connect(createDirButton_, SIGNAL(clicked()), SLOT(onCreateDirButtonClicked()));

	toggleHiddenButton_ = new QToolButton;
	toggleHiddenButton_->setIconSize(QSize(20, 20));
	toggleHiddenButton_->setIcon(QIcon(":/show.png"));
	toggleHiddenButton_->setToolTip("Show hidden files");
	toggleHiddenButton_->setAutoRaise(true);
	toggleHiddenButton_->setCheckable(true);
	connect(toggleHiddenButton_, SIGNAL(toggled(bool)), SLOT(setShowHidden(bool)));

	QHBoxLayout* currentDirLayout = new QHBoxLayout;
	currentDirLayout->setContentsMargins(2, 0, 2, 0);
	currentDirLayout->setSpacing(2);
	currentDirLayout->addWidget(label);
	currentDirLayout->addWidget(currentDirEdit_);
	currentDirLayout->addWidget(goUpButton_);
	currentDirLayout->addWidget(createDirButton_);
	currentDirLayout->addWidget(toggleHiddenButton_);

	model_ = new DirectoryModel;

	view_ = new DirectoryView;
	view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	view_->setModel(model_);

	connect(view_,
			SIGNAL(currentItemChanged(DirectoryItem*,DirectoryItem*)),
			SLOT(onItemChanged(DirectoryItem*)));

	connect(view_,
			SIGNAL(itemDoubleClicked(DirectoryItem*)),
			SLOT(onItemDoubleClicked(DirectoryItem*)));

	QHeaderView* header = view_->header();
	header->setStretchLastSection(false);
	header->setSectionResizeMode(0, QHeaderView::Stretch);

	nameEdit_ = new LineEdit;
	connect(nameEdit_, SIGNAL(textChanged(QString)), SLOT(onNameEditChanged(QString)));

	if(dialogMode_ == kOpenDialog) {
		actionButton_ = new QPushButton(QIcon(":/open.png"), "Open");
	}
	else {
		actionButton_ = new QPushButton(QIcon(":/save.png"), "Save");
	}

	actionButton_->setIconSize(QSize(16, 16));
	connect(actionButton_, SIGNAL(clicked()), SLOT(accept()));

	cancelButton_ = new QPushButton(QIcon(":/remove.png"), "Cancel");
	cancelButton_->setIconSize(QSize(16, 16));
	connect(cancelButton_, SIGNAL(clicked()), SLOT(reject()));

	QGridLayout* bottomLayout = new QGridLayout;
	bottomLayout->setContentsMargins(1, 4, 1, 4);
	bottomLayout->setSpacing(5);
	bottomLayout->addWidget(new QLabel("File name: "), 0, 0);
	bottomLayout->addWidget(nameEdit_, 0, 1);
	bottomLayout->addWidget(actionButton_, 0, 2);
	bottomLayout->addWidget(cancelButton_, 1, 2);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->setMargin(1);
	mainLayout->setSpacing(1);
	mainLayout->addLayout(currentDirLayout);
	mainLayout->addWidget(view_);
	mainLayout->addLayout(bottomLayout);

	setLayout(mainLayout);

	onNameEditChanged(QString());
}


FileDialog::AcceptMode FileDialog::acceptMode() const
{
	return acceptMode_;
}


void FileDialog::setAcceptMode(FileDialog::AcceptMode mode)
{
	acceptMode_ = mode;
	model_->setFilesEnabled(mode == kAcceptFile || mode == kAcceptExistingFile);
}


QDir::Filters FileDialog::filters() const
{
	return model_->filters();
}


void FileDialog::setFilter(QDir::Filters filters)
{
	model_->setFilters(filters);
}


QStringList FileDialog::nameFilters() const
{
	return model_->nameFilters();
}


void FileDialog::setNameFilter(const QString& filter)
{
	model_->setNameFilters(QStringList() << filter);
}


void FileDialog::setNameFilters(const QStringList& filters)
{
	model_->setNameFilters(filters);
}


QString FileDialog::rootDirectory() const
{
	return rootPath_;
}


void FileDialog::setRootDirectory(const QString& path)
{
	QDir dir(path);
	if(!dir.exists())
		return;

	rootPath_ = path;

	if(!model_->directory().startsWith(path))
		model_->setDirectory(path);
}


QString FileDialog::directory() const
{
	return model_->directory();
}


void FileDialog::setDirectory(const QString& path)
{
	if(!path.startsWith(rootPath_))
		return;

	if(QFileInfo::exists(path)) {
		model_->setDirectory(path);
		goUpButton_->setEnabled(path != rootPath_);
		currentDirEdit_->setText(path);
	}
}


void FileDialog::goUp()
{
	if(rootPath_ == model_->directory())
		return;

	QDir dir(model_->directory() + "/..");
	setDirectory(dir.absolutePath());
}


bool FileDialog::showHidden() const
{
	return model_->filters() & QDir::Hidden;
}


void FileDialog::setShowHidden(bool enable)
{
	QDir::Filters filters = model_->filters();

	if(enable) {
		filters |= QDir::Hidden;
	}
	else {
		filters &= ~QDir::Hidden;
	}

	model_->setFilters(filters);
}


void FileDialog::accept()
{
	if(!nameEdit_->text().contains('.'))
		nameEdit_->setText(nameEdit_->text() + suffix_);

	selectedPath_ = model_->directory() + '/' + nameEdit_->text();
	QDialog::accept();
}


int FileDialog::exec()
{
	selectedPath_.clear();
	return QDialog::exec();
}


QString FileDialog::selectedPath() const
{
	return selectedPath_;
}


QString FileDialog::selectedName() const
{
	DirectoryItem* item = view_->currentItem();
	if(!item)
		return QString();

	return item->name();
}


QString FileDialog::defaultSuffix() const
{
	return suffix_;
}


void FileDialog::setDefaultSuffix(const QString& suffix)
{
	if(suffix_.startsWith('.')) {
		suffix_ = suffix.mid(1);
	}
	else {
		suffix_ = suffix;
	}
}


void FileDialog::onItemChanged(DirectoryItem* item)
{
	if(!item || item->isDirectory()) {
		nameEdit_->clear();

		if(acceptMode_ == kAcceptFile || acceptMode_ == kAcceptExistingFile) {
			actionButton_->setEnabled(false);
		}

	}
	else {
		nameEdit_->setText(item->name());
		actionButton_->setEnabled(true);
	}

}


void FileDialog::onItemDoubleClicked(DirectoryItem* item)
{
	if(item->isDirectory()) {
		setDirectory(item->fullPath());
	}
	else if(acceptMode_ == kAcceptFile || acceptMode_ == kAcceptExistingFile) {
		accept();
	}
}


void FileDialog::onNameEditChanged(const QString& text)
{
	if(dialogMode_ == kSaveDialog) {
		actionButton_->setEnabled(!text.isEmpty());
	}
}


void FileDialog::onCreateDirButtonClicked()
{
	FolderDialog dialog(model_);
	dialog.exec();
}
