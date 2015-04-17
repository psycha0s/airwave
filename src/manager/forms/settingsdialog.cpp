#include "settingsdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QTreeWidget>
#include "common/config.h"
#include "core/application.h"
#include "core/logsocket.h"
#include "forms/filedialog.h"
#include "forms/loaderdialog.h"
#include "forms/prefixdialog.h"
#include "models/loadersmodel.h"
#include "models/prefixesmodel.h"
#include "widgets/lineedit.h"
#include "widgets/loadersview.h"
#include "widgets/prefixesview.h"
#include "widgets/separatorlabel.h"


SettingsDialog::SettingsDialog(QWidget* parent) :
	QDialog(parent)
{
	setupUi();

	QSettings settings;
	QString vstPath = settings.value("vstPath", qgetenv("VST_PATH")).toString();
	vstPathEdit_->setText(vstPath);

	Storage* storage = qApp->storage();
	binariesPathEdit_->setText(QString::fromStdString(storage->binariesPath()));
	logSocketEdit_->setText(QString::fromStdString(storage->logSocketPath()));

	int index = static_cast<int>(storage->defaultLogLevel());
	logLevelCombo_->setCurrentIndex(index);
}


SettingsDialog::~SettingsDialog()
{
	QSettings settings;
	settings.setValue("vstPath", vstPathEdit_->text());
}


void SettingsDialog::setupUi()
{
	setWindowIcon(QIcon(":/settings.png"));
	setMinimumWidth(400);
	resize(500, 450);

	vstPathEdit_ = new LineEdit;
	vstPathEdit_->setToolTip("Directory, where all of your native VSTs are located");
	vstPathEdit_->setButtonEnabled(true);
	vstPathEdit_->setButtonStyle(LineEdit::kLightAutoRaise);
	vstPathEdit_->setButtonIcon(QIcon(":/open.png"));
	vstPathEdit_->setButtonToolTip("Browse");
	connect(vstPathEdit_, SIGNAL(buttonClicked()), SLOT(browseForVstPath()));

	binariesPathEdit_ = new LineEdit;
	binariesPathEdit_->setToolTip("Directory, where airwave binaries are located");
	binariesPathEdit_->setButtonEnabled(true);
	binariesPathEdit_->setButtonStyle(LineEdit::kLightAutoRaise);
	binariesPathEdit_->setButtonIcon(QIcon(":/open.png"));
	binariesPathEdit_->setButtonToolTip("Browse");
	connect(binariesPathEdit_, SIGNAL(buttonClicked()), SLOT(browseForBinariesPath()));

	logSocketEdit_ = new LineEdit;
	logSocketEdit_->setToolTip("Socket file, used for logging");
	logSocketEdit_->setButtonEnabled(true);
	logSocketEdit_->setButtonStyle(LineEdit::kLightAutoRaise);
	logSocketEdit_->setButtonIcon(QIcon(":/open.png"));
	logSocketEdit_->setButtonToolTip("Browse");
	connect(logSocketEdit_, SIGNAL(buttonClicked()), SLOT(browseForSocketPath()));

	logLevelCombo_ = new QComboBox;
	logLevelCombo_->setToolTip("Log level, used for links with the 'default' log level.\n"
			"The higher the level, the more messages will appear.");

	logLevelCombo_->addItem(QIcon(":/mute.png"), "quiet");
	logLevelCombo_->addItem(QIcon(":/warning.png"), "error");
	logLevelCombo_->addItem(QIcon(":/trace.png"), "trace");
	logLevelCombo_->addItem(QIcon(":/bug.png"), "debug");
	logLevelCombo_->addItem(QIcon(":/scull.png"), "flood");

	QGridLayout* generalLayout = new QGridLayout;
	generalLayout->addWidget(new QLabel("VST location:"), 0, 0, Qt::AlignRight);
	generalLayout->addWidget(vstPathEdit_, 0, 1, 1, 4);
	generalLayout->addWidget(new QLabel("Binaries location:"), 1, 0, Qt::AlignRight);
	generalLayout->addWidget(binariesPathEdit_, 1, 1, 1, 4);
	generalLayout->addWidget(new QLabel("Log socket path:"), 2, 0, Qt::AlignRight);
	generalLayout->addWidget(logSocketEdit_, 2, 1, 1, 4);
	generalLayout->addWidget(new QLabel("Default log level:"), 3, 0, Qt::AlignRight);
	generalLayout->addWidget(logLevelCombo_, 3, 1);

	prefixesView_ = new PrefixesView;
	prefixesView_->setModel(qApp->prefixes());

	addPrefixButton_ = new QPushButton("Add");
	connect(addPrefixButton_, SIGNAL(clicked()), SLOT(createPrefix()));

	editPrefixButton_ = new QPushButton("Edit");
	connect(editPrefixButton_, SIGNAL(clicked()), SLOT(editPrefix()));

	removePrefixButton_ = new QPushButton("Remove");
	connect(removePrefixButton_, SIGNAL(clicked()), SLOT(removePrefix()));

	QVBoxLayout* vl = new QVBoxLayout;
	vl->addWidget(addPrefixButton_);
	vl->addWidget(editPrefixButton_);
	vl->addWidget(removePrefixButton_);
	vl->addStretch(1);

	QHBoxLayout* prefixLayout = new QHBoxLayout;
	prefixLayout->addWidget(prefixesView_);
	prefixLayout->addLayout(vl);

	loadersView_ = new LoadersView;
	loadersView_->setModel(qApp->loaders());

	addLoaderButton_ = new QPushButton("Add");
	connect(addLoaderButton_, SIGNAL(clicked()), SLOT(createLoader()));

	editLoaderButton_ = new QPushButton("Edit");
	connect(editLoaderButton_, SIGNAL(clicked()), SLOT(editLoader()));

	removeLoaderButton_ = new QPushButton("Remove");
	connect(removeLoaderButton_, SIGNAL(clicked()), SLOT(removeLoader()));

	vl = new QVBoxLayout;
	vl->addWidget(addLoaderButton_);
	vl->addWidget(editLoaderButton_);
	vl->addWidget(removeLoaderButton_);
	vl->addStretch(1);

	QHBoxLayout* loaderslayout = new QHBoxLayout;
	loaderslayout->addWidget(loadersView_);
	loaderslayout->addLayout(vl);

	buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttons_, SIGNAL(accepted()), SLOT(accept()));
	connect(buttons_, SIGNAL(rejected()), SLOT(reject()));

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(new SeparatorLabel("General:"));
	mainLayout->addLayout(generalLayout);
	mainLayout->addSpacing(10);
	mainLayout->addWidget(new SeparatorLabel("WINE prefixes:"));
	mainLayout->addLayout(prefixLayout);
	mainLayout->addSpacing(10);
	mainLayout->addWidget(new SeparatorLabel("WINE loaders:"));
	mainLayout->addLayout(loaderslayout);
	mainLayout->addSpacing(10);
	mainLayout->addWidget(buttons_);

	setLayout(mainLayout);
}


void SettingsDialog::browseForVstPath()
{
	FileDialog dialog(FileDialog::kOpenDialog);
	dialog.setAcceptMode(FileDialog::kAcceptExistingDirectory);
	dialog.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
	dialog.setWindowTitle("Select directory with the native VST plugins");

	QString path = vstPathEdit_->text();
	QFileInfo info(path);
	if(!info.exists(path) || !info.isDir())
		path = QDir::homePath();

	dialog.setDirectory(path);

	if(dialog.exec())
		vstPathEdit_->setText(dialog.selectedPath());
}


void SettingsDialog::browseForBinariesPath()
{
	FileDialog dialog(FileDialog::kOpenDialog);
	dialog.setAcceptMode(FileDialog::kAcceptExistingDirectory);
	dialog.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);

	QStringList nameFilters;
	nameFilters << HOST_BASENAME "-32.exe" << HOST_BASENAME "-64.exe";
	nameFilters << PLUGIN_BASENAME ".so";
	dialog.setNameFilters(nameFilters);

	dialog.setWindowTitle("Select directory containing airwave binaries");

	QString path = binariesPathEdit_->text();
	if(path.isEmpty())
		path = QString::fromStdString(qApp->storage()->binariesPath());

	dialog.setDirectory(path);

	if(dialog.exec())
		binariesPathEdit_->setText(dialog.selectedPath());
}


void SettingsDialog::browseForSocketPath()
{
	FileDialog dialog(FileDialog::kSaveDialog);
	dialog.setAcceptMode(FileDialog::kAcceptFile);
	dialog.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
	dialog.setDefaultSuffix(".sock");
	dialog.setWindowTitle("Enter file name of domain socket");

	QString path = logSocketEdit_->text();
	if(path.isEmpty())
		path = QString::fromStdString(qApp->storage()->logSocketPath());

	QFileInfo info(path);
	path = info.absolutePath();
	dialog.setDirectory(path);

	if(dialog.exec())
		logSocketEdit_->setText(dialog.selectedPath());
}


void SettingsDialog::createPrefix()
{
	PrefixDialog dialog;
	dialog.exec();
}


void SettingsDialog::editPrefix()
{
	PrefixItem* item = prefixesView_->currentItem();
	if(!item) {
		QMessageBox::critical(this, "Error", "You should select prefix item to edit.");
		return;
	}

	if(item->name() == "default") {
		QMessageBox::critical(this, "Error", "You cannot edit default WINE prefix.");
		return;
	}

	PrefixDialog dialog;
	dialog.setItem(item);
	dialog.exec();
}


void SettingsDialog::removePrefix()
{
	PrefixItem* item = prefixesView_->currentItem();
	if(!item) {
		QMessageBox::critical(this, "Error", "You should select prefix item to remove.");
		return;
	}

	QString prefix = item->name();

	if(prefix == "default") {
		QMessageBox::critical(this, "Error", "You cannot remove default WINE prefix.");
		return;
	}

	QString text = QString("Do you really want to remove the '%1' prefix?").arg(prefix);
	if(QMessageBox::question(this, "Question", text) == QMessageBox::Yes) {
		if(!qApp->prefixes()->removePrefix(item))
			QMessageBox::critical(this, "Error", "Unable to delete selected prefix.");
	}
}


void SettingsDialog::createLoader()
{
	LoaderDialog dialog;
	dialog.exec();
}


void SettingsDialog::editLoader()
{
	LoaderItem* item = loadersView_->currentItem();
	if(!item) {
		QMessageBox::critical(this, "Error", "You should select loader item to edit.");
		return;
	}

	if(item->name() == "default") {
		QMessageBox::critical(this, "Error", "You cannot edit default WINE loader.");
		return;
	}

	LoaderDialog dialog;
	dialog.setItem(item);
	dialog.exec();
}


void SettingsDialog::removeLoader()
{
	LoaderItem* item = loadersView_->currentItem();
	if(!item) {
		QMessageBox::critical(this, "Error", "You should select loader item to remove.");
		return;
	}

	QString loader = item->name();

	if(loader == "default") {
		QMessageBox::critical(this, "Error", "You cannot remove default WINE loader.");
		return;
	}

	QString text = QString("Do you really want to remove the '%1' loader?").arg(loader);
	if(QMessageBox::question(this, "Question", text) == QMessageBox::Yes) {
		if(!qApp->loaders()->removeLoader(item))
			QMessageBox::critical(this, "Error", "Unable to delete selected loader.");
	}
}


void SettingsDialog::accept()
{
	if(binariesPathEdit_->text().isEmpty()) {
		QMessageBox::critical(this, "Error", "You should select the binaries location.");
		return;
	}

	if(!QDir().exists(binariesPathEdit_->text())) {
		QMessageBox::critical(this, "Error", "Binaries location doesn't exists.");
		return;
	}

	QFileInfo fileInfo(binariesPathEdit_->text());
	if(!fileInfo.isDir()) {
		QMessageBox::critical(this, "Error", "Binaries location is not a directory.");
		return;
	}

	QStringList files = qApp->checkMissingBinaries(binariesPathEdit_->text());
	if(!files.isEmpty()) {
		QString message = "Some binaries aren't found, please choose the correct\n"
				"binaries location!\n\nThe missed binaries are:\n\n";
		foreach(const QString& fileName, files)
			message += fileName + '\n';

		QMessageBox::critical(this, "Error", message);
		return;
	}

	if(logSocketEdit_->text().isEmpty()) {
		QMessageBox::critical(this, "Error", "Log socket is not defined");
		return;
	}

	Airwave::Storage* storage = qApp->storage();

	int index = logLevelCombo_->currentIndex();
	Airwave::LogLevel level = static_cast<Airwave::LogLevel>(index);

	if(level == Airwave::LogLevel::kFlood) {
		QString message = "<b>WARNING!</b><br>"
				"By using the 'flood' log level as default, you will get an enormous "
				"count of log messages from links with 'default' log level, the "
				"performance will be low and the audio playback may become very choppy."
				"<br><br>"
				"Do you really want to proceed?";

		if(QMessageBox::question(this, "Question", message) == QMessageBox::No)
			return;
	}

	LogSocket* socket = qApp->logSocket();
	if(logSocketEdit_->text() != socket->id()) {
		socket->close();
		socket->listen(logSocketEdit_->text());
	}

	storage->setDefaultLogLevel(level);
	storage->setBinariesPath(binariesPathEdit_->text().toStdString());

	storage->save();
	QDialog::accept();
}
