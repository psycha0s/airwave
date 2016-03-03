#include "loadersview.h"

#include <QHeaderView>
#include "nofocusdelegate.h"


LoadersView::LoadersView(QWidget* parent) :
	GenericTreeView<LoadersModel>(parent)
{
	setAutoClearSelection(true);
	setRootIsDecorated(false);

	NoFocusDelegate* delegate = new NoFocusDelegate(this);
//	delegate->setExtraHeight(4);
	setItemDelegate(delegate);

	setContextMenuPolicy(Qt::CustomContextMenu);
}


void LoadersView::setModel(LoadersModel* model)
{
	GenericTreeView<LoadersModel>::setModel(model);

//	if(model) {
//		QHeaderView* header = this->header();
//		header->setStretchLastSection(false);
//		header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
//		header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
//		header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
//		header->setSectionResizeMode(3, QHeaderView::Stretch);
//	}
}


void LoadersView::currentChangeEvent(LoaderItem* current, LoaderItem* previous)
{
	emit currentItemChanged(current, previous);
}


void LoadersView::selectionChangeEvent(const QItemSelection& selected,
		const QItemSelection& deselected)
{
	emit itemSelectionChanged(selected, deselected);
}
