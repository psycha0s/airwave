#include "nofocusdelegate.h"


NoFocusDelegate::NoFocusDelegate(QWidget* parent) :
	QStyledItemDelegate(parent),
	extraHeight_(0)
{
}


void NoFocusDelegate::paint(QPainter* painter,
							const QStyleOptionViewItem& option,
							const QModelIndex& index) const
{
	QStyleOptionViewItemV4 opt = option;
	opt.state &= ~QStyle::State_HasFocus;

	QStyledItemDelegate::paint(painter, opt, index);
}


QSize NoFocusDelegate::sizeHint(const QStyleOptionViewItem& option,
								const QModelIndex& index) const
{
	QSize size = QStyledItemDelegate::sizeHint(option, index);
	size.setHeight(size.height() + extraHeight_);

	return size;
}


void NoFocusDelegate::setExtraHeight(int value)
{
	extraHeight_ = qMax(0, value);
}
