#ifndef UTILITY_WIDGETS_LOGVIEW_H
#define UTILITY_WIDGETS_LOGVIEW_H

#include <QTextEdit>


class LogView : public QTextEdit {
	Q_OBJECT
public:
	LogView(QWidget* parent = nullptr);

	bool isAutoScroll() const;

public slots:
	void setAutoScroll(bool enabled);
	void setWordWrap(bool enabled);
	void addMessage(quint64 time, const QString& sender, const QString& text);
	void addSeparator();

private:
	bool isAutoScroll_;
	bool isWordWrap_;
};


#endif // UTILITY_WIDGETS_LOGVIEW_H
