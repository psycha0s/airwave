#include "mainform.h"

#include <QFile>
#include <QMessageBox>
#include "config.h"
#include "linkeditdialog.h"
#include "moduleinfo.h"
#include "ui_mainform.h"
#include "widgets/nofocusdelegate.h"


MainForm::MainForm(QWidget* parent) :
	QWidget(parent),
	ui_(new Ui::MainForm),
	linksModel_(new LinksModel(this)),
	sourceModel_(new DirListModel(this)),
	targetModel_(new DirListModel(this))
{
	ui_->setupUi(this);

	connect(ui_->showHiddenButton,
			SIGNAL(toggled(bool)),
			SLOT(onShowHiddenButtonToggled(bool)));

	connect(ui_->showLinkedButton,
			SIGNAL(clicked()),
			SLOT(onShowLinkedButtonClicked()));

	ui_->sourceLabel->setElideMode(Qt::ElideMiddle);
	ui_->targetLabel->setElideMode(Qt::ElideMiddle);

	connect(sourceModel_,
			SIGNAL(currentDirChanged(QString)),
			ui_->sourceLabel,
			SLOT(setText(QString)));

	connect(targetModel_,
			SIGNAL(currentDirChanged(QString)),
			ui_->targetLabel,
			SLOT(setText(QString)));

	ui_->linksView->setTextElideMode(Qt::ElideMiddle);
	ui_->linksView->setRootIsDecorated(false);
	ui_->linksView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui_->linksView->setModel(linksModel_);

	NoFocusDelegate* delegate = qobject_cast<NoFocusDelegate*>
			(ui_->linksView->itemDelegate());

	delegate->setExtraHeight(4);


	QHeaderView* header = ui_->linksView->header();
	header->setStretchLastSection(false);
	header->setResizeMode(0, QHeaderView::Stretch);
	header->setResizeMode(1, QHeaderView::Stretch);
	header->setResizeMode(2, QHeaderView::ResizeToContents);

	connect(ui_->linksView,
			SIGNAL(selectionUpdated(QItemSelection, QItemSelection)),
			SLOT(onCurrentLinkChanged()));


	sourceModel_->setNameFilters(QStringList() << "*.dll");
	sourceModel_->setDir(QDir::homePath());

	ui_->pluginsView->setModel(sourceModel_);
	ui_->pluginsView->setRootIsDecorated(false);

	connect(ui_->pluginsView,
			SIGNAL(doubleClicked(QModelIndex)),
			SLOT(onDirItemDoubleClicked(QModelIndex)));

	connect(ui_->pluginsView,
			SIGNAL(selectionUpdated(QItemSelection, QItemSelection)),
			SLOT(onCurrentPluginChanged()));


	targetModel_->setNameFilters(QStringList() << "*.so");
	QString vstDirPath = qgetenv("VST_PATH");
	if(vstDirPath.isEmpty())
		vstDirPath = QDir::homePath();

	targetModel_->setDir(vstDirPath);

	ui_->bridgesView->setModel(targetModel_);
	ui_->bridgesView->setRootIsDecorated(false);

	connect(ui_->bridgesView,
			SIGNAL(doubleClicked(QModelIndex)),
			SLOT(onDirItemDoubleClicked(QModelIndex)));


	setWindowTitle(PROJECT_NAME " utility " PROJECT_VERSION);

	ui_->splitter->setStretchFactor(0, 5);
	ui_->splitter->setStretchFactor(1, 4);


	connect(ui_->createLinkButton,
			SIGNAL(clicked()),
			SLOT(onCreateLinkButtonClicked()));

	connect(ui_->editLinkButton,
			SIGNAL(clicked()),
			SLOT(onEditLinkButtonClicked()));

	connect(ui_->removeLinkButton,
			SIGNAL(clicked()),
			SLOT(onRemoveLinkButtonClicked()));

	connect(ui_->recreateLinksButton,
			SIGNAL(clicked()),
			SLOT(onRecreateLinksButtonClicked()));

	onCurrentLinkChanged();
	onCurrentPluginChanged();


	if(!logSocket_.listen(LOG_SOCKET_PATH))
		qDebug("Unable to create logger socket.");

	connect(&logSocket_,
			SIGNAL(newMessage(quint64,QString,QString)),
			ui_->logView,
			SLOT(addMessage(quint64,QString,QString)));

	connect(ui_->addSeparatorButton,
			SIGNAL(clicked()),
			ui_->logView,
			SLOT(addSeparator()));

	connect(ui_->clearMessagesButton,
			SIGNAL(clicked()),
			ui_->logView,
			SLOT(clear()));

	connect(ui_->wordWrapButton,
			SIGNAL(toggled(bool)),
			ui_->logView,
			SLOT(setWordWrap(bool)));

	connect(ui_->autoScrollButton,
			SIGNAL(toggled(bool)),
			ui_->logView,
			SLOT(setAutoScroll(bool)));
}


MainForm::~MainForm()
{
	delete ui_;
}


QString MainForm::libraryPathFor(const QString& pluginPath) const
{
	QString libraryPath = PLUGIN_PATH "/" PLUGIN_BASENAME;

	ModuleInfo* moduleInfo = ModuleInfo::instance();
	ModuleInfo::Arch arch = moduleInfo->getArch(pluginPath);

	if(arch == ModuleInfo::kArch32) {
		return libraryPath + "-32.so";
	}
	else if(arch == ModuleInfo::kArch64) {
		return libraryPath + "-64.so";
	}

	return QString();
}


void MainForm::onShowHiddenButtonToggled(bool value)
{
	sourceModel_->setShowHidden(value);
	targetModel_->setShowHidden(value);
}


void MainForm::onShowLinkedButtonClicked()
{
	int row = ui_->linksView->firstSelectedRow();
	if(row < 0)
		return;

	LinkItem* linkItem = linksModel_->rootItem()->childAt(row);
	if(linkItem) {
		QString dir = linkItem->pluginPath();
		dir = dir.left(dir.lastIndexOf('/'));
		sourceModel_->setDir(dir);

		dir = linkItem->bridgePath();
		dir = dir.left(dir.lastIndexOf('/'));
		targetModel_->setDir(dir);

		DirListItem* item = sourceModel_->rootItem()->firstChild();
		while(item) {
			if(item->fullPath() == linkItem->pluginPath()) {
				ui_->pluginsView->selectRow(item->row());
				break;
			}

			item = item->nextSibling();
		}

		item = targetModel_->rootItem()->firstChild();
		while(item) {
			if(item->fullPath() == linkItem->bridgePath()) {
				ui_->bridgesView->selectRow(item->row());
				break;
			}

			item = item->nextSibling();
		}
	}
}


void MainForm::onDirItemDoubleClicked(const QModelIndex& index)
{
	QTreeView* view = static_cast<QTreeView*>(sender());
	DirListModel* model = static_cast<DirListModel*>(view->model());

	DirListItem* item = model->indexToItem(index);
	if(item && item->isDirectory()) {
		model->setDir(item->path());
	}
}


void MainForm::onCurrentLinkChanged()
{
	bool hasSelection = ui_->linksView->hasSelection();

	ui_->editLinkButton->setEnabled(hasSelection);
	ui_->removeLinkButton->setEnabled(hasSelection);
	ui_->showLinkedButton->setEnabled(hasSelection);
}


void MainForm::onCurrentPluginChanged()
{
	int row = ui_->pluginsView->firstSelectedRow();
	if(row < 0) {
		ui_->createLinkButton->setEnabled(false);
		return;
	}

	DirListItem* item = sourceModel_->rootItem()->childAt(row);
	ui_->createLinkButton->setEnabled(!item->isDirectory());
}


void MainForm::onCreateLinkButtonClicked()
{
	int row = ui_->pluginsView->firstSelectedRow();
	if(row < 0)
		return;

	QString pluginPath = sourceModel_->rootItem()->childAt(row)->name();
	if(!pluginPath.endsWith(".dll"))
		return;

	QString name = pluginPath;
	name.chop(4);	// Cut the ".dll" extension.

	QString bridgePath = targetModel_->currentDir() + '/';

	pluginPath = sourceModel_->rootItem()->childAt(row)->fullPath();

	QString libraryPath = libraryPathFor(pluginPath);
	if(libraryPath.isEmpty()) {
		QString message = "Unable to determine architecture of " + pluginPath;
		QMessageBox::critical(this, tr("Error"), tr(message.toAscii()));
		return;
	}

	if(!QFile::exists(libraryPath)) {
		QString message = "Unable to find airwave template: " + libraryPath;
		QMessageBox::critical(this, tr("Error"), tr(message.toAscii()));
		return;
	}

	LinkEditDialog dialog;
	dialog.setLinkName(name);
	if(dialog.exec()) {
		QString fileName = dialog.linkName() + ".so";

		QFile::copy(libraryPath, bridgePath + fileName);
		linksModel_->setLink(pluginPath, bridgePath + fileName);
	}
}


void MainForm::onEditLinkButtonClicked()
{
	int row = ui_->linksView->firstSelectedRow();
	if(row < 0)
		return;

	QString bridgePath = linksModel_->rootItem()->childAt(row)->bridgePath();

	int pos = bridgePath.lastIndexOf('/');
	QString bridgeName = bridgePath.mid(pos + 1);
	bridgeName.chop(3);	// Cut the ".so" extension.

	LinkEditDialog dialog;
	dialog.setLinkName(bridgeName);
	if(dialog.exec()) {
		QDir dir(bridgePath.left(pos));
		dir.rename(bridgeName + ".so", dialog.linkName() + ".so");

		QString newPath = dir.canonicalPath() + "/" + dialog.linkName() + ".so";
		linksModel_->updateLink(bridgePath, newPath);
	}
}


void MainForm::onRemoveLinkButtonClicked()
{
	int row = ui_->linksView->firstSelectedRow();
	if(row < 0)
		return;

	QString bridgePath = linksModel_->rootItem()->childAt(row)->bridgePath();
	int pos = bridgePath.lastIndexOf('/');

	QString bridgeName = bridgePath.mid(pos + 1);
	QDir dir(bridgePath.left(pos));

	dir.remove(bridgeName);
	linksModel_->removeLink(bridgePath);
}


void MainForm::onRecreateLinksButtonClicked()
{
	LinkItem* item = linksModel_->rootItem()->firstChild();

	while(item) {
		QFile bridgeFile(item->bridgePath());

		QString plugin = item->pluginPath();
		QString libraryPath = libraryPathFor(plugin);
		if(libraryPath.isEmpty()) {
			QString message = "Unable to determine architecture of " + plugin;
			QMessageBox::critical(this, tr("Error"), tr(message.toAscii()));
			return;
		}

		if(!QFile::exists(libraryPath)) {
			QString message = "Unable to find airwave template: " + libraryPath;
			QMessageBox::critical(this, tr("Error"), tr(message.toAscii()));
			return;
		}

		if(bridgeFile.exists())
			bridgeFile.remove();

		QFile::copy(libraryPath, item->bridgePath());
		item = item->nextSibling();
	}
}
