#ifndef CORE_LOGSOCKET_H
#define CORE_LOGSOCKET_H

#include <QSocketNotifier>
#include <QString>


class LogSocket : public QObject {
	Q_OBJECT
public:
	LogSocket(QObject* parent = nullptr);
	~LogSocket();

	QString id() const;
	bool listen(const QString& id);
	void close();

signals:
	void newMessage(quint64 time, const QString& sender, const QString& text);

private:
	int fd_;
	QSocketNotifier* notifier_;
	QString id_;

private slots:
	void handleDatagram();
};


#endif // CORE_LOGSOCKET_H
