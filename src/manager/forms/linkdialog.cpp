#include "linkdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include "common/config.h"
#include "core/application.h"
#include "core/moduleinfo.h"
#include "forms/filedialog.h"
#include "models/linksmodel.h"
#include "models/loadersmodel.h"
#include "models/prefixesmodel.h"
#include "widgets/lineedit.h"


LinkDialog::LinkDialog(QWidget* parent) :
	QDialog(parent),
	item_(nullptr)
{
	setupUi();
}


LinkItem* LinkDialog::item() const
{
	return item_;
}


void LinkDialog::setItem(LinkItem* item)
{
	item_ = item;

	if(item) {
		locationEdit_->setText(item->location());

		int index = prefixCombo_->findText(item->prefix());
		prefixCombo_->setCurrentIndex(index);

		index = loaderCombo_->findText(item->loader());
		loaderCombo_->setCurrentIndex(index);

		index = static_cast<int>(item->logLevel()) + 1;
		logLevelCombo_->setCurrentIndex(index);

		nameEdit_->setText(item->name());
		targetEdit_->setText(item->target());
	}
	else {
		nameEdit_->clear();
		locationEdit_->clear();
		targetEdit_->clear();

		int index = prefixCombo_->findText("default");
		prefixCombo_->setCurrentIndex(index);

		index = loaderCombo_->findText("default");
		loaderCombo_->setCurrentIndex(index);
	}
}


void LinkDialog::setupUi()
{
	setWindowIcon(QIcon(":/edit_link.png"));
	setWindowTitle("Link properties");
	setMinimumWidth(500);
	resize(600, 180);
	setFixedHeight(210);

	loaderCombo_ = new QComboBox;
	loaderCombo_->setModel(qApp->loaders());
	loaderCombo_->setCurrentIndex(loaderCombo_->findText("default"));

	prefixCombo_ = new QComboBox;
	prefixCombo_->setModel(qApp->prefixes());
	prefixCombo_->setCurrentIndex(prefixCombo_->findText("default"));
	connect(prefixCombo_, SIGNAL(currentIndexChanged(int)), SLOT(onPrefixChanged()));

	logLevelCombo_ = new QComboBox;
	logLevelCombo_->addItem(QIcon(":/star.png"), "default");
	logLevelCombo_->addItem(QIcon(":/mute.png"), "quiet");
	logLevelCombo_->addItem(QIcon(":/warning.png"), "error");
	logLevelCombo_->addItem(QIcon(":/trace.png"), "trace");
	logLevelCombo_->addItem(QIcon(":/bug.png"), "debug");
	logLevelCombo_->addItem(QIcon(":/scull.png"), "flood");

	targetEdit_ = new LineEdit;
	targetEdit_->setButtonEnabled(true);
	targetEdit_->setButtonStyle(LineEdit::kLightAutoRaise);
	targetEdit_->setButtonIcon(QIcon(":/open.png"));
	targetEdit_->setButtonToolTip("Browse");
	targetEdit_->setPrefix("${WINEPREFIX}/");
	targetEdit_->setPrefixColor(Qt::darkGreen);
	connect(targetEdit_, SIGNAL(buttonClicked()), SLOT(browsePlugin()));

	locationEdit_ = new LineEdit;
	locationEdit_->setButtonEnabled(true);
	locationEdit_->setButtonStyle(LineEdit::kLightAutoRaise);
	locationEdit_->setButtonIcon(QIcon(":/open.png"));
	locationEdit_->setButtonToolTip("Browse");

	QSettings settings;
	QString vstPath = settings.value("vstPath", qgetenv("VST_PATH")).toString();
	locationEdit_->setText(vstPath);
	connect(locationEdit_, SIGNAL(buttonClicked()), SLOT(browseLocation()));

	nameEdit_ = new LineEdit;

	buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttons_, SIGNAL(accepted()), SLOT(accept()));
	connect(buttons_, SIGNAL(rejected()), SLOT(reject()));

	QGridLayout* mainLayout = new QGridLayout;

	mainLayout->addWidget(new QLabel("WINE loader:"), 0, 0, Qt::AlignRight);
	mainLayout->addWidget(loaderCombo_, 0, 1, 1, 1);

	mainLayout->addWidget(new QLabel("WINE prefix:"), 1, 0, Qt::AlignRight);
	mainLayout->addWidget(prefixCombo_, 1, 1, 1, 1);

	mainLayout->addWidget(new QLabel("VST plugin:"), 2, 0, Qt::AlignRight);
	mainLayout->addWidget(targetEdit_, 2, 1, 1, 2);

	mainLayout->addWidget(new QLabel("Link location:"), 3, 0, Qt::AlignRight);
	mainLayout->addWidget(locationEdit_, 3, 1, 1, 2);

	mainLayout->addWidget(new QLabel("Link name:"), 4, 0, Qt::AlignRight);
	mainLayout->addWidget(nameEdit_, 4, 1, 1, 2);

	mainLayout->addWidget(new QLabel("Log level:"), 5, 0, Qt::AlignRight);
	mainLayout->addWidget(logLevelCombo_, 5, 1, 1, 1);

	mainLayout->addWidget(new QWidget, 6, 0);

	mainLayout->addWidget(buttons_, 7, 1, 1, 2);

	mainLayout->setRowStretch(5, 1);

	mainLayout->setColumnStretch(0, 0);
	mainLayout->setColumnStretch(1, 0);
	mainLayout->setColumnStretch(2, 1);

	setLayout(mainLayout);
}


void LinkDialog::browsePlugin()
{
	QString prefix = currentPrefix();
	QFileInfo prefixInfo(prefix);

	if(!prefixInfo.isDir()) {
		QMessageBox::critical(this, "Error", "Selected prefix directory doesn't exists.");
		return;
	}

	FileDialog dialog(FileDialog::kOpenDialog);
	dialog.setAcceptMode(FileDialog::kAcceptExistingFile);
	dialog.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
	dialog.setNameFilter("*.dll");
	dialog.setWindowTitle("Select the source VST plugin");
	dialog.setRootDirectory(prefix);

	if(targetEdit_->text().isEmpty()) {
		QFileInfo info(prefixInfo.absoluteFilePath(), "drive_c");
		dialog.setDirectory(info.absoluteFilePath());
	}
	else {
		QFileInfo info(prefix, targetEdit_->text());
		while(!info.isDir())
			info = QFileInfo(info.absolutePath());

		dialog.setDirectory(info.absoluteFilePath());
	}

	if(dialog.exec()) {
		QString prefixPath = prefixInfo.absoluteFilePath();
		int length = prefixPath.length();
		if(!prefixPath.endsWith('/'))
			length++;

		targetEdit_->setText(dialog.selectedPath().mid(length));

		QString name = dialog.selectedName();
		name.chop(4); // Remove ".dll" extension
		nameEdit_->setText(name);
	}
}


void LinkDialog::browseLocation()
{
	FileDialog dialog(FileDialog::kOpenDialog);
	dialog.setAcceptMode(FileDialog::kAcceptExistingDirectory);
	dialog.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
	dialog.setWindowTitle("Select directory where the link will be placed");

	QString text = locationEdit_->text();
	if(text.isEmpty()) {
		dialog.setDirectory(QDir::homePath());
	}
	else {
		dialog.setDirectory(text);
	}

	if(dialog.exec()) {
		locationEdit_->setText(dialog.selectedPath());
	}
}


void LinkDialog::accept()
{
	QString prefix = currentPrefix();
	QFileInfo vstInfo(prefix, targetEdit_->text());

	if(!vstInfo.isFile() || vstInfo.suffix() != "dll") {
		QMessageBox::critical(this, "Error", "VST plugin is invalid or doesn't exists.");
		return;
	}

	QFileInfo locationInfo(locationEdit_->text());

	if(!locationInfo.isDir()) {
		QMessageBox::critical(this, "Error", "Location directory doesn't exists.");
		return;
	}

	QString name = nameEdit_->text();
	if(name.isEmpty()) {
		QMessageBox::critical(this, "Error", "Link name cannot be empty.");
		return;
	}

	QString pluginPath = getPluginPath();
	if(pluginPath.isEmpty()) {
		QMessageBox::critical(this, "Error", "VST plugin is corrupted.");
		return;
	}

	int index = logLevelCombo_->currentIndex();
	Airwave::LogLevel level = static_cast<Airwave::LogLevel>(index - 1);

	if(level == Airwave::LogLevel::kFlood) {
		QString message = "<b>WARNING!</b><br>"
				"By using the 'flood' log level, you will get an enormous count of log "
				"messages from this link, the plugin performance will be low and the "
				"audio playback may become very choppy.<br><br>"
				"Do you really want to proceed?";

		if(QMessageBox::question(this, "Question", message) == QMessageBox::No)
			return;
	}

	if(!item_) {
		item_ = qApp->links()->createLink(nameEdit_->text(), locationEdit_->text(),
				targetEdit_->text(), prefixCombo_->currentText(),
				loaderCombo_->currentText());

		if(!item_) {
			QMessageBox::critical(this, "Error", "Unable to create link.");
			return;
		}

		int value = logLevelCombo_->currentIndex() - 1;
		item_->setLogLevel(static_cast<LogLevel>(value));
	}
	else {
		if(item_->name() != name) {
			LinkItem* item = qApp->links()->root()->firstChild();

			while(item) {
				if(item->name() == name) {
					QString message = QString("Link with name '%1' is already exists.")
							.arg(name);

					QMessageBox::critical(this, "Error", message);
					return;
				}

				item = item->nextSibling();
			}
		}

		QDir dir(locationInfo.absoluteFilePath());
		dir.remove(item_->name() + ".so");

		item_->setName(nameEdit_->text());
		item_->setLocation(locationEdit_->text());
		item_->setTarget(targetEdit_->text());
		item_->setPrefix(prefixCombo_->currentText());
		item_->setLoader(loaderCombo_->currentText());

		int value = logLevelCombo_->currentIndex() - 1;
		item_->setLogLevel(static_cast<LogLevel>(value));
	}

	qApp->storage()->save();
	QFile::copy(pluginPath, locationInfo.absoluteFilePath() + '/' + name + ".so");

	QDialog::accept();
}


void LinkDialog::onPrefixChanged()
{
	targetEdit_->clear();
	nameEdit_->clear();
}


QString LinkDialog::currentPrefix() const
{
	int index = prefixCombo_->currentIndex();

	if(index != -1) {
		PrefixItem* item = qApp->prefixes()->root()->childAt(index);
		if(item)
			return item->path();
	}

	return QString();
}


QString LinkDialog::getPluginPath() const
{
	QString pluginPath = QString::fromStdString(qApp->storage()->binariesPath());
	return pluginPath + "/" PLUGIN_BASENAME ".so";
}
