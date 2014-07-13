#ifndef UTILITY_WIDGETS_ELIDEDLABEL_H
#define UTILITY_WIDGETS_ELIDEDLABEL_H

#include <QFrame>


class ElidedLabel : public QFrame {
	Q_OBJECT
public:
	ElidedLabel(QWidget* parent = nullptr);

	const QString& text() const;
	Qt::TextElideMode elideMode() const;

public slots:
	void setText(const QString& text);
	void setElideMode(Qt::TextElideMode mode);

signals:
	void elisionChanged(bool elided);

protected:
	void paintEvent(QPaintEvent* event);

private:
	QString text_;
	Qt::TextElideMode elideMode_;
};


#endif // UTILITY_WIDGETS_ELIDEDLABEL_H
