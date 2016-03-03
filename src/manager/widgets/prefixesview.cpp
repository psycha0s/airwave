#include "prefixesview.h"

#include <QHeaderView>
#include "nofocusdelegate.h"


PrefixesView::PrefixesView(QWidget* parent) :
	GenericTreeView<PrefixesModel>(parent)
{
	setAutoClearSelection(true);
	setRootIsDecorated(false);

	NoFocusDelegate* delegate = new NoFocusDelegate(this);
//	delegate->setExtraHeight(4);
	setItemDelegate(delegate);

	setContextMenuPolicy(Qt::CustomContextMenu);
}


void PrefixesView::setModel(PrefixesModel* model)
{
	GenericTreeView<PrefixesModel>::setModel(model);

//	if(model) {
//		QHeaderView* header = this->header();
//		header->setStretchLastSection(false);
//		header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
//		header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
//		header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
//		header->setSectionResizeMode(3, QHeaderView::Stretch);
//	}
}


void PrefixesView::currentChangeEvent(PrefixItem* current, PrefixItem* previous)
{
	emit currentItemChanged(current, previous);
}


void PrefixesView::selectionChangeEvent(const QItemSelection& selected,
		const QItemSelection& deselected)
{
	emit itemSelectionChanged(selected, deselected);
}
