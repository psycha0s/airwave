#ifndef UTILITY_LOGSOCKET_H
#define UTILITY_LOGSOCKET_H

#include <QSocketNotifier>
#include <QString>


class LogSocket : public QObject {
	Q_OBJECT
public:
	LogSocket(QObject* parent = nullptr);
	~LogSocket();

	bool listen(const QString& id);

signals:
	void newMessage(quint64 time, const QString& sender, const QString& text);

private:
	int fd_;
	QSocketNotifier* notifier_;
	QString id_;

private slots:
	void handleDatagram();
};


#endif // UTILITY_LOGSOCKET_H
