#ifndef CORE_SINGLEAPPLICATION_H
#define CORE_SINGLEAPPLICATION_H

#include <QApplication>

#ifdef qApp
#undef qApp
#define qApp (static_cast<SingleApplication*>(QApplication::instance()))
#endif

class QLocalServer;


class SingleApplication : public QApplication {
	Q_OBJECT
public:
	SingleApplication(int& argc, char** argv);
	SingleApplication(const QString& id, int& argc, char** argv);

	~SingleApplication();

	bool isRunning() const;

	QWidget* activationWindow() const;
	void setActivationWindow(QWidget* window);

	bool isActivateOnMessage() const;
	void setActivateOnMessage(bool enable);

public slots:
	void activateWindow();
	bool sendMessage(const QString& message);

signals:
	void messageReceived(const QString& message);

private:
	static const int kConnectTimeout_ = 500;
	static const int kReadTimeout_ = 1000;

	QString id_;
	QLocalServer* server_;
	QWidget* window_;
	bool isActivateOnMessage_;

	void initialize(const QString& id);
	void finalize();

private slots:
	void processClientConnection();
};


#endif // CORE_SINGLEAPPLICATION_H
