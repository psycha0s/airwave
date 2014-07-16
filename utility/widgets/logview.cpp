#include <QScrollBar>
#include <QStringBuilder>
#include <QTime>
#include "config.h"
#include "logview.h"


LogView::LogView(QWidget* parent) :
	QTextEdit(parent),
	isAutoScroll_(true),
	isWordWrap_(true)
{
	setReadOnly(true);

	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	setFont(font);

	setWordWrap(isWordWrap_);
}


bool LogView::isAutoScroll() const
{
	return isAutoScroll_;
}


void LogView::setAutoScroll(bool enabled)
{
	isAutoScroll_ = enabled;
}


void LogView::setWordWrap(bool enabled)
{
	isWordWrap_ = enabled;

	if(isWordWrap_) {
		setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	}
	else {
		setWordWrapMode(QTextOption::NoWrap);
	}
}


void LogView::addMessage(quint64 time, const QString& sender,
		const QString& text)
{
	setTextColor(QColor(0x808080));
	insertPlainText(QString::number(time >> 32));
	insertPlainText(".");
	insertPlainText(QString::number(time & 0xFFFFFFFF).rightJustified(9, '0'));
	insertPlainText(" ");

	if(sender == HOST_BASENAME || sender.endsWith(".dll")) {
		setTextColor(QColor(0xA0A000));
	}
	else {
		setTextColor(QColor(0x008000));
	}
	insertPlainText(sender.rightJustified(20, ' ', true));

	setTextColor(Qt::black);
	insertPlainText(" : ");

	insertPlainText(text);
	insertPlainText("\n");

	if(isAutoScroll_) {
		int maximum = verticalScrollBar()->maximum();
		verticalScrollBar()->setValue(maximum);
	}
}


void LogView::addSeparator()
{
	insertHtml("<hr><br>");
	if(isAutoScroll_) {
		int maximum = verticalScrollBar()->maximum();
		verticalScrollBar()->setValue(maximum);
	}
}
