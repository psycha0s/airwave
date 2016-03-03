#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionFrameV2>
#include <QToolButton>
#include <QtCore/qcoreapplication.h>
#include <QtWidgets/qcolordialog.h>
#include "lineedit.h"


LineEdit::LineEdit(QWidget* parent) :
	QLineEdit(parent),
	hasButton_(false),
	button_(new QToolButton(this)),
	buttonStyle_(kNormal),
	isAutoClearMode_(false),
	prefixColor_(qApp->palette().text().color()),
	suffixColor_(qApp->palette().text().color())
{
	button_->hide();
	button_->setCursor(Qt::ArrowCursor);
	button_->setToolButtonStyle(Qt::ToolButtonIconOnly);
	button_->setFocusPolicy(Qt::NoFocus);

	connect(button_,
			SIGNAL(clicked()),
			SLOT(onButtonClicked()));

	connect(this,
			SIGNAL(textChanged(QString)),
			SLOT(onTextChanged()));

	timer_.setInterval(200);
	timer_.setSingleShot(true);

	connect(&timer_,
			SIGNAL(timeout()),
			SLOT(onTimeout()));
}


bool LineEdit::hasButton() const
{
	return hasButton_;
}


void LineEdit::setButtonEnabled(bool enabled)
{
	if(enabled != hasButton_) {
		hasButton_ = enabled;
		button_->setVisible(enabled);
		updateLayout();
		update();
	}
}


LineEdit::ButtonStyle LineEdit::buttonStyle() const
{
	return buttonStyle_;
}


void LineEdit::setButtonStyle(ButtonStyle style)
{
	QColor col = palette().toolTipBase().color();
	QString sheet = "QToolButton { background-color: rgba(255, 255, 255, %1);} "
					"QToolButton QWidget { background-color: rgb(%2, %3, %4);}";
	switch(style) {
	case kNormal:
		button_->setAutoRaise(false);
		button_->setStyleSheet("");
		break;

	case kAutoRaise:
		button_->setAutoRaise(true);
		button_->setStyleSheet("");
		break;

	case kLightAutoRaise:
		button_->setAutoRaise(true);
		sheet = sheet.arg(80).arg(col.red()).arg(col.green()).arg(col.blue());
		button_->setStyleSheet(sheet);
		break;

	case kTransparent:
		button_->setAutoRaise(true);
		sheet = sheet.arg(0).arg(col.red()).arg(col.green()).arg(col.blue());
		button_->setStyleSheet(sheet);
		break;
	}
}


bool LineEdit::isAutoClearMode() const
{
	return isAutoClearMode_;
}


void LineEdit::setAutoClearMode(bool enabled)
{
	isAutoClearMode_ = enabled;
	button_->setToolTip(tr("Clear"));
	QIcon icon = style()->standardIcon(QStyle::SP_DockWidgetCloseButton);
	button_->setIcon(icon);
}


QIcon LineEdit::buttonIcon() const
{
	return button_->icon();
}


QString LineEdit::buttonToolTip() const
{
	return button_->toolTip();
}


void LineEdit::setButtonToolTip(const QString& toolTip)
{
	button_->setToolTip(toolTip);
}


void LineEdit::setButtonIcon(const QIcon& icon)
{
	button_->setIcon(icon);
}


uint LineEdit::editTimeout() const
{
	return timer_.interval();
}


void LineEdit::setEditTimeout(uint msecs)
{
	timer_.setInterval(msecs);
}


bool LineEdit::hasIcon() const
{
	return !icon_.isNull();
}


void LineEdit::setIcon(const QIcon& icon)
{
	icon_ = icon.pixmap(16, 16);
	updateLayout();
	update();
}


void LineEdit::updateLayout()
{
	int size = height();
	int leftMargin = 0;
	int rightMargin = 0;

	if(hasButton()) {
		rightMargin = size;
		button_->setGeometry(width() - size + 1, 1, size - 2, size - 2);
	}

	if(hasIcon())
		leftMargin = 16;

	if(!prefix_.isEmpty()) {
		QFontMetrics fm(font());
		leftMargin += fm.width(prefix_);
	}

	if(!suffix_.isEmpty()) {
		QFontMetrics fm(font());
		rightMargin += fm.width(suffix_);
	}

	setTextMargins(leftMargin, 0, rightMargin, 0);
}


void LineEdit::paintEvent(QPaintEvent* event)
{
    QLineEdit::paintEvent(event);

	if(hasIcon()) {
		QPainter painter(this);
		painter.drawPixmap(3, 2, icon_);
	}

	QPainter painter(this);
	QFontMetrics fm(font());
	QMargins margins = textMargins();

	int half = (height() - fm.height()) / 2;

	if(!prefix_.isEmpty()) {
		int x = margins.left() - fm.width(prefix_) + 6;
		int y = margins.top() + fm.ascent() + half;

		painter.setPen(prefixColor_);
		painter.drawText(x, y, prefix_);
	}

	if(!suffix_.isEmpty()) {
		int x = width() - margins.right();
		int y = margins.top() + fm.ascent() + half;

		painter.setPen(suffixColor_);
		painter.drawText(x, y, suffix_);
	}
}


void LineEdit::resizeEvent(QResizeEvent* event)
{
	QLineEdit::resizeEvent(event);
	updateLayout();
}


void LineEdit::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_Escape && isAutoClearMode()) {
		clear();
	}
	else {
		QLineEdit::keyPressEvent(event);
	}
}


void LineEdit::mouseMoveEvent(QMouseEvent* event)
{
	QLineEdit::mouseMoveEvent(event);

	if(hasIcon()) {
		if(event->x() < 16 + 2) {
			setCursor(Qt::PointingHandCursor);
		}
		else {
			setCursor(Qt::IBeamCursor);
		}
	}
}


void LineEdit::onButtonClicked()
{
	if(isAutoClearMode_)
		clear();

	emit buttonClicked();
}


void LineEdit::onTextChanged()
{
	timer_.start();
}


void LineEdit::onTimeout()
{
	emit textEditTimeout(text());
}


QString LineEdit::prefix() const
{
	return prefix_;
}


void LineEdit::setPrefix(const QString& prefix)
{
	prefix_ = prefix;
	updateLayout();
	update();
}


QColor LineEdit::prefixColor() const
{
	return prefixColor_;
}


void LineEdit::setPrefixColor(const QColor& color)
{
	prefixColor_ = color;
	update();
}


QString LineEdit::suffix() const
{
	return prefix_;
}


void LineEdit::setSuffix(const QString& suffix)
{
	suffix_ = suffix;
	updateLayout();
	update();
}


QColor LineEdit::suffixColor() const
{
	return suffixColor_;
}


void LineEdit::setSuffixColor(const QColor& color)
{
	suffixColor_ = color;
	update();
}
