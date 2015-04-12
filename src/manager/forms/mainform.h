#ifndef FORMS_MAINFORM_H
#define FORMS_MAINFORM_H

#include <QMainWindow>


class QAction;
class QSplitter;
class LinksModel;
class LinksView;
class LogView;


class MainForm : public QMainWindow {
	Q_OBJECT
public:
	explicit MainForm(QWidget* parent = nullptr);
	~MainForm();

public slots:
	void loadSettings();
	void saveSettings();

private:
	QToolBar* toolBar_;

	QAction* createLink_;
	QAction* editLink_;
	QAction* showInBrowser_;
	QAction* removeLink_;
	QAction* updateLinks_;

	QAction* toggleWordWrap_;
	QAction* toggleAutoScroll_;
	QAction* addSeparator_;
	QAction* clearLog_;

	QAction* showAbout_;
	QAction* showSettings_;

	QSplitter* splitter_;
	LinksView* linksView_;
	LogView* logView_;

	void setupUi();
	bool checkBinaries();

private slots:
	void createLink();
	void editLink();
	void removeLink();
	void updateLinks();
	void showInBrowser();
	void showAbout();
	void showSettings();

	void updateToolbarButtons();
};


#endif // FORMS_MAINFORM_H
