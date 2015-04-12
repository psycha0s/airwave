#ifndef WIDGETS_NOFOCUSDELEGATE_H
#define WIDGETS_NOFOCUSDELEGATE_H

#include <QStyledItemDelegate>


class NoFocusDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	explicit NoFocusDelegate(QWidget* parent = nullptr);

	void paint(QPainter* painter, const QStyleOptionViewItem& option,
			   const QModelIndex& index) const;

	QSize sizeHint(const QStyleOptionViewItem& option,
				   const QModelIndex& index) const;

	void setExtraHeight(int value);

private:
	int extraHeight_;
};


#endif // WIDGETS_NOFOCUSDELEGATE_H
