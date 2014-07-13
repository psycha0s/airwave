#include "simpletreeview.h"

#include <QMouseEvent>
#include "nofocusdelegate.h"


SimpleTreeView::SimpleTreeView(QWidget* parent) :
	QTreeView(parent),
	isAutoExpanding_(false)
{
	setItemDelegate(new NoFocusDelegate);
	setSelectionMode(SingleSelection);
	setSelectionBehavior(SelectRows);
	setDragEnabled(true);
	setDragDropMode(QAbstractItemView::InternalMove);
}


QModelIndexList SimpleTreeView::selectedRows() const
{
	if(!selectionModel())
		return QModelIndexList();

	return selectionModel()->selectedRows();
}


void SimpleTreeView::selectRow(int row, const QModelIndex& parent)
{
	if(model() && selectionModel()) {
		row = qMax(0, row);
		QModelIndex index = model()->index(row, 0, parent);

		selectionModel()->select(index, QItemSelectionModel::Select |
				QItemSelectionModel::Rows);
	}
}


void SimpleTreeView::deselectRow(int row, const QModelIndex& parent)
{
	if(model() && selectionModel()) {
		row = qMax(0, row);
		QModelIndex index = model()->index(row, 0, parent);

		selectionModel()->select(index, QItemSelectionModel::Deselect |
				QItemSelectionModel::Rows);
	}
}


void SimpleTreeView::clearSelection()
{
	if(selectionModel())
		selectionModel()->clear();
}


bool SimpleTreeView::hasSelection() const
{
	if(selectionModel())
		return selectionModel()->hasSelection();

	return false;
}


int SimpleTreeView::firstSelectedRow() const
{
	if(hasSelection())
		return selectionModel()->selectedRows().first().row();

	return -1;
}


bool SimpleTreeView::isRowSelected(int row, const QModelIndex& parent) const
{
	if(selectionModel())
		return selectionModel()->isRowSelected(row, parent);

	return false;
}


void SimpleTreeView::setAutoExpanding(bool value)
{
	isAutoExpanding_ = value;
}


bool SimpleTreeView::isAutoExpanding() const
{
	return isAutoExpanding_;
}


void SimpleTreeView::mousePressEvent(QMouseEvent* event)
{
	QModelIndex index = indexAt(event->pos());
	if(!index.isValid() && selectionModel())
		selectionModel()->clearSelection();

	QTreeView::mousePressEvent(event);
	update();
}


void SimpleTreeView::currentChanged(const QModelIndex& current,
		const QModelIndex& previous)
{
	QTreeView::currentChanged(current, previous);
	emit currentChanged(current);
}


void SimpleTreeView::selectionChanged(const QItemSelection& selected,
		const QItemSelection& deselected)
{
	QTreeView::selectionChanged(selected, deselected);
	emit selectionUpdated(selected, deselected);
}


void SimpleTreeView::rowsInserted(const QModelIndex& parent, int start, int end)
{
	QTreeView::rowsInserted(parent, start, end);

	if(isAutoExpanding_) {
		for(int i = start; i <= end; ++i) {
			QModelIndex index = model()->index(i, 0, parent);
			setExpanded(index, true);
		}
	}
}
