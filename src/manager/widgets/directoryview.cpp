#include "directoryview.h"

#include <QHeaderView>
#include "nofocusdelegate.h"


DirectoryView::DirectoryView(QWidget* parent) :
	GenericTreeView<DirectoryModel>(parent)
{
	setAutoClearSelection(true);
	setRootIsDecorated(false);

	NoFocusDelegate* delegate = new NoFocusDelegate(this);
	delegate->setExtraHeight(4);
	setItemDelegate(delegate);

	setContextMenuPolicy(Qt::CustomContextMenu);
}


void DirectoryView::setModel(DirectoryModel* model)
{
	GenericTreeView<DirectoryModel>::setModel(model);

	if(model) {
		QHeaderView* header = this->header();
		header->setStretchLastSection(false);
		header->setSectionResizeMode(0, QHeaderView::Stretch);
		header->setSectionResizeMode(1, QHeaderView::Interactive);
		header->setSectionResizeMode(2, QHeaderView::Interactive);
	}
}


void DirectoryView::currentChangeEvent(DirectoryItem* current, DirectoryItem* previous)
{
	emit currentItemChanged(current, previous);
}


void DirectoryView::selectionChangeEvent(const QItemSelection& selected,
		const QItemSelection& deselected)
{
	emit itemSelectionChanged(selected, deselected);
}


void DirectoryView::mouseDoubleClickEvent(QMouseEvent* event)
{
	QModelIndex index = indexAt(event->pos());
	DirectoryItem* item = model()->indexToItem(index);
	if(item)
		emit itemDoubleClicked(item);
}
