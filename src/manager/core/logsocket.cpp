#include "logsocket.h"

#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <linux/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <QByteArray>


LogSocket::LogSocket(QObject* parent) :
	QObject(parent),
	fd_(-1),
	notifier_(nullptr)
{
}


LogSocket::~LogSocket()
{
	close();
}


QString LogSocket::id() const
{
	return id_;
}


bool LogSocket::listen(const QString& id)
{
	fd_ = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(fd_ < 0) {
		qDebug("Unable to create socket: %s", strerror(errno));
		return false;
	}

	unlink(id.toUtf8().constData());

	struct sockaddr_un address;
	std::memset(&address, 0, sizeof(address));

	address.sun_family = AF_UNIX;
	std::snprintf(address.sun_path, UNIX_PATH_MAX, id.toUtf8().constData());

	if(bind(fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
		qDebug("Unable to bind socket: %s", strerror(errno));
		::close(fd_);
		fd_ = -1;
		return false;
	}

	id_ = id;
	notifier_ = new QSocketNotifier(fd_, QSocketNotifier::Read);
	connect(notifier_, SIGNAL(activated(int)), SLOT(handleDatagram()));
	return true;
}


void LogSocket::close()
{
	if(fd_ != -1) {
		delete notifier_;
		::close(fd_);
		unlink(id_.toUtf8().constData());
	}
}


void LogSocket::handleDatagram()
{
	size_t length = 0;
	if(ioctl(fd_, FIONREAD, &length) < 0) {
		qDebug("ioctl() call failed: %s", strerror(errno));
		return;
	}

	char* buffer = new char[length];

	if(recvfrom(fd_, buffer, length, 0, nullptr, nullptr) < 0) {
		qDebug("recvfrom() call failed: %s", strerror(errno));
		delete [] buffer;
		return;
	}

	quint64* timeStamp = reinterpret_cast<quint64*>(buffer);
	const char* sender = buffer + sizeof(quint64);

	const char* message;
	message = static_cast<const char*>(std::memchr(sender, 0x01, length));
	if(!message) {
		qDebug("Discarding invalid datagram.");
		delete [] buffer;
		return;
	}

	QString senderId;
	senderId = QString::fromUtf8(sender, message - sender);
	emit newMessage(*timeStamp, senderId, message + 1);
	delete [] buffer;
}
