#ifndef WIDGET_SEPARATORLABEL_H_
#define WIDGET_SEPARATORLABEL_H_

#include <QWidget>


class QLabel;


class SeparatorLabel : public QWidget {
	Q_OBJECT
public:
	SeparatorLabel(const QString& text = QString(), QWidget* parent = nullptr);

private:
	QLabel* label_;
};


#endif //WIDGET_SEPARATORLABEL_H_
