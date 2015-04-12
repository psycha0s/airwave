#include "mainform.h"

#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSettings>
#include <QSplitter>
#include <QToolBar>
#include "common/config.h"
#include "core/application.h"
#include "forms/linkdialog.h"
#include "forms/settingsdialog.h"
#include "models/linksmodel.h"
#include "widgets/linksview.h"
#include "widgets/logview.h"


MainForm::MainForm(QWidget* parent) :
	QMainWindow(parent)
{
	setupUi();
	loadSettings();
	updateToolbarButtons();

	QString logSocketPath = QString::fromStdString(qApp->storage()->logSocketPath());

	LogSocket* socket = qApp->logSocket();
	if(!socket->listen(logSocketPath))
		qDebug("Unable to create logger socket.");

	connect(socket,
			SIGNAL(newMessage(quint64,QString,QString)),
			logView_,
			SLOT(addMessage(quint64,QString,QString)));

	connect(qApp->links(),
			SIGNAL(rowsInserted(QModelIndex,int,int)),
			SLOT(updateToolbarButtons()));

	connect(qApp->links(),
			SIGNAL(rowsRemoved(QModelIndex,int,int)),
			SLOT(updateToolbarButtons()));

	connect(qApp->links(),
			SIGNAL(layoutChanged()),
			SLOT(updateToolbarButtons()));
}


MainForm::~MainForm()
{
	saveSettings();
}


void MainForm::loadSettings()
{
	QSettings settings;
	settings.beginGroup("mainForm");

	resize(settings.value("size", QSize(800, 600)).toSize());
	restoreState(settings.value("windowState").toByteArray());

	splitter_->restoreState(settings.value("mainSplitter").toByteArray());

	toggleWordWrap_->setChecked(settings.value("logWordWrap", true).toBool());
	toggleAutoScroll_->setChecked(settings.value("logAutoScroll", true).toBool());

	QHeaderView* header = linksView_->header();

	int width = settings.value("linkNameWidth", 150).toInt();
	header->resizeSection(0, width);

	width = settings.value("logLevelWidth", 90).toInt();
	header->resizeSection(1, width);

	width = settings.value("prefixNameWidth", 70).toInt();
	header->resizeSection(2, width);

	width = settings.value("loaderNameWidth", 70).toInt();
	header->resizeSection(3, width);

	settings.endGroup();
}


void MainForm::saveSettings()
{
	QSettings settings;
	settings.beginGroup("mainForm");

	settings.setValue("size", size());
	settings.setValue("windowState", saveState());

	settings.setValue("mainSplitter", splitter_->saveState());

	settings.setValue("logWordWrap", toggleWordWrap_->isChecked());
	settings.setValue("logAutoScroll", toggleAutoScroll_->isChecked());

	QHeaderView* header = linksView_->header();
	settings.setValue("linkNameWidth", header->sectionSize(0));
	settings.setValue("logLevelWidth", header->sectionSize(1));
	settings.setValue("prefixNameWidth", header->sectionSize(2));
	settings.setValue("loaderNameWidth", header->sectionSize(3));

	settings.endGroup();
}


void MainForm::setupUi()
{
	setWindowIcon(QIcon(":/" PROJECT_NAME "-manager.png"));
	setWindowTitle(PROJECT_NAME " manager " VERSION_STRING);

	setCentralWidget(new QWidget);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->setSpacing(0);
	layout->setMargin(1);

	linksView_ = new LinksView;
	linksView_->setModel(qApp->links());

	QHeaderView* header = linksView_->header();
	header->setStretchLastSection(false);
	header->setSectionResizeMode(0, QHeaderView::Interactive);
	header->setSectionResizeMode(1, QHeaderView::Interactive);
	header->setSectionResizeMode(2, QHeaderView::Interactive);
	header->setSectionResizeMode(3, QHeaderView::Interactive);
	header->setSectionResizeMode(4, QHeaderView::Stretch);

	connect(linksView_,
			SIGNAL(itemSelectionChanged(QItemSelection,QItemSelection)),
			SLOT(updateToolbarButtons()));

	connect(linksView_,
			SIGNAL(itemDoubleClicked(LinkItem*)),
			SLOT(editLink()));

	logView_ = new LogView;

	splitter_ = new QSplitter(Qt::Vertical);
	splitter_->addWidget(linksView_);
	splitter_->addWidget(logView_);

	int size = splitter_->height();
	splitter_->setSizes(QList<int>() << size * 0.618 << size * 0.382);

	layout->addWidget(splitter_);

	centralWidget()->setLayout(layout);

	//
	// Toolbar
	//
	toolBar_ = new QToolBar(this);
	toolBar_->setObjectName("toolBar_");
	toolBar_->setFloatable(false);
	toolBar_->setMovable(false);
	toolBar_->setIconSize(QSize(20, 20));
	addToolBar(toolBar_);

	// Create link action
	createLink_ = new QAction(QIcon(":/create_link.png"), "Create link", this);
	toolBar_->addAction(createLink_);
	connect(createLink_, SIGNAL(triggered()), SLOT(createLink()));

	// Edit link action
	editLink_ = new QAction(QIcon(":/edit.png"), "Edit link", this);
	editLink_->setEnabled(false);
	toolBar_->addAction(editLink_);
	connect(editLink_, SIGNAL(triggered()), SLOT(editLink()));

	// Remove link action
	removeLink_ = new QAction(QIcon(":/remove.png"), "Remove link", this);
	removeLink_->setEnabled(false);
	toolBar_->addAction(removeLink_);
	connect(removeLink_, SIGNAL(triggered()), SLOT(removeLink()));

	toolBar_->addSeparator();

	// Show in file browser action
	showInBrowser_ = new QAction(QIcon(":/open_in_browser.png"),
			"Open in file manager the WINE prefix directory of the link", this);

	showInBrowser_->setEnabled(false);
	toolBar_->addAction(showInBrowser_);
	connect(showInBrowser_, SIGNAL(triggered()), SLOT(showInBrowser()));

	// Update all links action
	updateLinks_ = new QAction(QIcon(":/update.png"), "Update links", this);
	toolBar_->addAction(updateLinks_);
	connect(updateLinks_, SIGNAL(triggered()), SLOT(updateLinks()));

	toolBar_->addSeparator();

	// Toggle word wrap action
	toggleWordWrap_ = new QAction(
			QIcon(":/outline.png"), "Wrap long lines in the log view", this);

	toggleWordWrap_->setCheckable(true);
	toolBar_->addAction(toggleWordWrap_);
	connect(toggleWordWrap_, SIGNAL(triggered(bool)), logView_, SLOT(setWordWrap(bool)));

	// Toggle auto scroll action
	toggleAutoScroll_ = new QAction(
			QIcon(":/download.png"), "Autoscroll log on new message", this);

	toggleAutoScroll_->setCheckable(true);
	toolBar_->addAction(toggleAutoScroll_);

	connect(toggleAutoScroll_,
			SIGNAL(triggered(bool)),
			logView_,
			SLOT(setAutoScroll(bool)));

	// Add separator action
	addSeparator_ = new QAction(QIcon(":/draw_line.png"), "Add separation line", this);

	toolBar_->addAction(addSeparator_);
	connect(addSeparator_, SIGNAL(triggered()), logView_, SLOT(addSeparator()));

	// Clear log action
	clearLog_ = new QAction(QIcon(":/erase.png"), "Clear log messages", this);
	toolBar_->addAction(clearLog_);
	connect(clearLog_, SIGNAL(triggered()), logView_, SLOT(clear()));

	toolBar_->addSeparator();

	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	toolBar_->addWidget(spacer);

	toolBar_->addSeparator();

	// Show about window action
//	showAbout_ = new QAction(QIcon(":/about.png"), "About", this);
//	toolBar_->addAction(showAbout_);
//	connect(showAbout_, SIGNAL(triggered()), SLOT(showAbout()));

	// Show settings action
	showSettings_ = new QAction(QIcon(":/settings.png"), "Settings", this);
	toolBar_->addAction(showSettings_);
	connect(showSettings_, SIGNAL(triggered()), SLOT(showSettings()));
}


bool MainForm::checkBinaries()
{
	QStringList files = qApp->checkMissingBinaries();
	if(!files.isEmpty()) {
		QString message = "Some binaries aren't found, please choose the correct\n"
				"binaries location in settings dialog!\n\nThe missed binaries are:\n\n";
		foreach(const QString& fileName, files)
			message += fileName + '\n';

		QMessageBox::critical(this, "Error", message);
		return false;
	}

	return true;
}



void MainForm::createLink()
{
	if(checkBinaries()) {
		LinkDialog dialog;
		dialog.exec();
	}
	else {
		SettingsDialog dialog;
		dialog.exec();
	}
}


void MainForm::editLink()
{
	if(checkBinaries()) {
		LinkDialog dialog;
		dialog.setItem(linksView_->currentItem());
		dialog.exec();
	}
	else {
		SettingsDialog dialog;
		dialog.exec();
	}
}


void MainForm::removeLink()
{
	LinkItem* item = linksView_->currentItem();
	if(!item)
		return;

	QString message = QString("Do you really want to remove the '%1' link?")
			.arg(item->name());

	if(QMessageBox::question(this, "Question", message) == QMessageBox::Yes) {
		QFileInfo info(item->path());
		QString fileName = item->name() + ".so";

		if(qApp->links()->removeLink(linksView_->currentItem())) {
			info.dir().remove(fileName);
			qApp->storage()->save();
		}
	}
}


void MainForm::updateLinks()
{
	QString pluginPath = QString::fromStdString(qApp->storage()->binariesPath());

	LinkItem* item = qApp->links()->root()->firstChild();
	while(item) {
		auto prefix = qApp->storage()->prefix(item->prefix().toStdString());
		if(!prefix)
			continue;

		QString prefixPath = QString::fromStdString(prefix.path());
		QFileInfo vstInfo(prefixPath + '/' + item->target());

		ModuleInfo* moduleInfo = ModuleInfo::instance();
		ModuleInfo::Arch arch = moduleInfo->getArch(vstInfo.absoluteFilePath());

		QString pluginName;
		if(arch == ModuleInfo::kArch32) {
			pluginName = PLUGIN_BASENAME "-32.so";
		}
		else if(arch == ModuleInfo::kArch64) {
			pluginName = PLUGIN_BASENAME "-64.so";
		}
		else {
			continue;
		}

		QFileInfo linkInfo(item->path());
		QDir dir(linkInfo.absoluteDir());
		dir.remove(linkInfo.fileName());

		QFile::copy(pluginPath + '/' + pluginName, linkInfo.absoluteFilePath());

		item = item->nextSibling();
	}
}


void MainForm::showInBrowser()
{
	LinkItem* item = linksView_->currentItem();
	if(!item)
		return;

	auto prefix = qApp->storage()->prefix(item->prefix().toStdString());
	if(!prefix) {
		QMessageBox::critical(this, "Error", "WINE prefix is corrupted");
		return;
	}

	QString path = QString::fromStdString(prefix.path());
	if(!QDir(path).exists()) {
		QMessageBox::critical(this, "Error", "WINE prefix directory doesn't exist");
		return;
	}

	path = QDir::toNativeSeparators(path);
	QDesktopServices::openUrl(QUrl("file:///" + path));
}


void MainForm::showAbout()
{

}


void MainForm::showSettings()
{
	SettingsDialog dialog;
	dialog.exec();
}


void MainForm::updateToolbarButtons()
{
	bool enable = linksView_->hasSelection();

	editLink_->setEnabled(enable);
	showInBrowser_->setEnabled(enable);
	removeLink_->setEnabled(enable);

	updateLinks_->setEnabled(linksView_->model()->root()->childCount());
}
