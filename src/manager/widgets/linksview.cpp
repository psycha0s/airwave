#include "linksview.h"

#include <QHeaderView>
#include "widgets/nofocusdelegate.h"


LinksView::LinksView(QWidget* parent) :
	GenericTreeView<LinksModel>(parent)
{
	setAutoClearSelection(true);
	setRootIsDecorated(false);

	NoFocusDelegate* delegate = new NoFocusDelegate(this);
//	delegate->setExtraHeight(4);
	setItemDelegate(delegate);

	setContextMenuPolicy(Qt::CustomContextMenu);

	connect(this,
			SIGNAL(doubleClicked(QModelIndex)),
			SLOT(onItemDoubleClicked(QModelIndex)));
}


void LinksView::setModel(LinksModel* model)
{
	GenericTreeView<LinksModel>::setModel(model);

//	if(model) {
//		QHeaderView* header = this->header();
//		header->setStretchLastSection(false);
//		header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
//		header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
//		header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
//		header->setSectionResizeMode(3, QHeaderView::Stretch);
//	}
}


void LinksView::currentChangeEvent(LinkItem* current, LinkItem* previous)
{
	emit currentItemChanged(current, previous);
}


void LinksView::selectionChangeEvent(const QItemSelection& selected,
		const QItemSelection& deselected)
{
	emit itemSelectionChanged(selected, deselected);
}


void LinksView::onItemDoubleClicked(const QModelIndex& index)
{
	emit itemDoubleClicked(model()->indexToItem(index));
}
