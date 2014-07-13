#include "elidedlabel.h"

#include <QPainter>


ElidedLabel::ElidedLabel(QWidget* parent) :
	QFrame(parent),
	elideMode_(Qt::ElideRight)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}


const QString& ElidedLabel::text() const
{
	return text_;
}


Qt::TextElideMode ElidedLabel::elideMode() const
{
	return elideMode_;
}


void ElidedLabel::setText(const QString& newText)
{
	text_ = newText;
	update();
}


void ElidedLabel::setElideMode(Qt::TextElideMode mode)
{
	elideMode_ = mode;
	update();
}


void ElidedLabel::paintEvent(QPaintEvent* event)
{
	QFrame::paintEvent(event);

	QPainter painter(this);
	QFontMetrics fm = painter.fontMetrics();

	QString string = fm.elidedText(text_, elideMode_, width());
	painter.drawText(QPoint(0, fm.ascent()), string);
}
