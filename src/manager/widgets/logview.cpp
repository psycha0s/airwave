#include <QScrollBar>
#include <QStringBuilder>
#include <QTime>
#include "logview.h"
#include "common/config.h"


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


bool LogView::isWordWrap() const
{
	return isWordWrap_;
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


void LogView::addMessage(quint64 time, const QString& sender, const QString& text)
{
	setTextColor(QColor(0x909090));
	insertPlainText(QString::number(time >> 32));
	insertPlainText(".");
	insertPlainText(QString::number(time & 0xFFFFFFFF).rightJustified(9, '0'));
	insertPlainText(" ");

	if(sender == HOST_BASENAME || sender.endsWith(".dll")) {
		setTextColor(QColor(0x804000));
	}
	else {
		setTextColor(QColor(0x004080));
	}

	insertPlainText(sender.rightJustified(20, ' ', true));

	insertPlainText(" : ");

	setTextColor(0x222222);
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
