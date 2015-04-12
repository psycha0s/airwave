#include "singleapplication.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QWidget>


SingleApplication::SingleApplication(int& argc, char** argv) :
	QApplication(argc, argv),
	server_(nullptr),
	window_(nullptr),
	isActivateOnMessage_(false)
{
	QString id = QApplication::applicationFilePath().replace('/', '.');
	initialize(id);
}


SingleApplication::SingleApplication(const QString& id, int& argc, char** argv) :
	QApplication(argc, argv),
	server_(nullptr),
	window_(nullptr),
	isActivateOnMessage_(false)
{
	initialize(id);
}


SingleApplication::~SingleApplication()
{
	finalize();
}


bool SingleApplication::isRunning() const
{
	return !server_;
}


QWidget* SingleApplication::activationWindow() const
{
	return window_;
}


void SingleApplication::setActivationWindow(QWidget* window)
{
	window_ = window;
}


bool SingleApplication::isActivateOnMessage() const
{
	return isActivateOnMessage_;
}


void SingleApplication::setActivateOnMessage(bool enable)
{
	isActivateOnMessage_ = enable;
}


void SingleApplication::activateWindow()
{
	if(window_) {
		window_->show();
		window_->activateWindow();
	}
}


bool SingleApplication::sendMessage(const QString& message)
{
	QLocalSocket socket;
	socket.connectToServer(id_);
	if(socket.waitForConnected(kConnectTimeout_)) {
		socket.write(message.toUtf8());
		return true;
	}

	return false;
}


void SingleApplication::initialize(const QString& id)
{
	id_ = id;

	QLocalSocket socket;
	socket.connectToServer(id);
	if(socket.waitForConnected(kConnectTimeout_))
		return;

	server_ = new QLocalServer(this);
	connect(server_, SIGNAL(newConnection()), SLOT(processClientConnection()));

	if(!server_->listen(id)) {
		if(server_->serverError() == QAbstractSocket::AddressInUseError) {
			QLocalServer::removeServer(id);

			if(!server_->listen(id))
				finalize();
		}
	}
}


void SingleApplication::finalize()
{
	if(server_) {
		delete server_;
		server_ = nullptr;
		id_.clear();
	}
}


void SingleApplication::processClientConnection()
{
	QLocalSocket* socket = server_->nextPendingConnection();
	if(socket) {
		QString message;
		if(socket->waitForReadyRead(kReadTimeout_))
			message = socket->readAll();

		delete socket;

		if(!message.isEmpty()) {
			if(isActivateOnMessage_)
				activateWindow();

			emit messageReceived(message);
		}
		else if(!isActivateOnMessage_) {
			activateWindow();
		}
	}
}
