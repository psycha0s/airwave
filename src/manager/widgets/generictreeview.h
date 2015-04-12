#ifndef WIDGETS_GENERICTREEVIEW_H
#define WIDGETS_GENERICTREEVIEW_H

#include <QMouseEvent>
#include <QTreeView>


template<typename Model>
class GenericTreeView : public QTreeView {
public:
	typedef typename Model::ItemType ItemType;
	typedef QList<ItemType> ItemList;

	GenericTreeView(QWidget* parent = nullptr);

	Model* model() const;
	void setModel(Model* model);

	ItemType* rootItem() const;

	void setAutoClearSelection(bool enabled);
	bool isAutoClearSelection() const;

	void setAutoExpanding(bool enabled);
	bool isAutoExpanding() const;

	bool hasSelection() const;
	void clearSelection();

	bool hasCurrentItem() const;
	ItemType* currentItem() const;
	void clearCurrentItem();

	ItemList selectedItems() const;
	bool isSelected(ItemType* item) const;
	void toggleSelection(ItemType* item);
	void setSelected(ItemType* item, bool selected);

	ItemType* itemAt(const QPoint& point) const;

protected:
	virtual void currentChangeEvent(ItemType* current, ItemType* previous);

	virtual void selectionChangeEvent(const QItemSelection& selected,
			const QItemSelection& deselected);

private:
	bool isAutoClearSelection_;
	bool isAutoExpanding_;

	void mousePressEvent(QMouseEvent* event);
	void rowsInserted(const QModelIndex& parent, int start, int end);

	void currentChanged(const QModelIndex& current,	const QModelIndex& previous);

	void selectionChanged(const QItemSelection& selected,
			const QItemSelection& deselected);
};


template<typename Model>
GenericTreeView<Model>::GenericTreeView(QWidget* parent) :
	QTreeView(parent),
	isAutoClearSelection_(false),
	isAutoExpanding_(false)
{
	setSelectionMode(SingleSelection);
	setSelectionBehavior(SelectRows);
	setDragEnabled(true);
	setDragDropMode(QAbstractItemView::InternalMove);
}


template<typename Model>
Model* GenericTreeView<Model>::model() const
{
	return static_cast<Model*>(QTreeView::model());
}


template<typename Model>
void GenericTreeView<Model>::setModel(Model* model)
{
	QTreeView::setModel(model);
	currentChangeEvent(nullptr, nullptr);
}


template<typename Model>
typename GenericTreeView<Model>::ItemType* GenericTreeView<Model>::rootItem() const
{
	if(model())
		return model()->root();

	return nullptr;
}


template<typename Model>
void GenericTreeView<Model>::setAutoClearSelection(bool enabled)
{
	isAutoClearSelection_ = enabled;
}


template<typename Model>
bool GenericTreeView<Model>::isAutoClearSelection() const
{
	return isAutoClearSelection_;
}


template<typename Model>
void GenericTreeView<Model>::setAutoExpanding(bool enabled)
{
	isAutoExpanding_ = enabled;
}


template<typename Model>
bool GenericTreeView<Model>::isAutoExpanding() const
{
	return isAutoExpanding_;
}


template<typename Model>
bool GenericTreeView<Model>::hasSelection() const
{
	if(selectionModel())
		return selectionModel()->hasSelection();

	return false;
}


template<typename Model>
void GenericTreeView<Model>::clearSelection()
{
	if(selectionModel()) {
		selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::NoUpdate);
		selectionModel()->clear();
	}
}


template<typename Model>
bool GenericTreeView<Model>::hasCurrentItem() const
{
	return currentIndex().isValid();
}


template<typename Model>
typename GenericTreeView<Model>::ItemType* GenericTreeView<Model>::currentItem() const
{
	if(!model())
		return nullptr;

	ItemType* item = model()->indexToItem(currentIndex());
	if(item == rootItem())
		return nullptr;

	return item;
}


template<typename Model>
void GenericTreeView<Model>::clearCurrentItem()
{
	if(selectionModel())
		selectionModel()->clearCurrentIndex();
}



template<typename Model>
typename GenericTreeView<Model>::ItemList GenericTreeView<Model>::selectedItems() const
{
	if(!selectionModel())
		return ItemList();

	ItemList result;
	foreach(const QModelIndex& index, selectionModel()->selectedRows())
		result += model()->indexToItem(index);

	return result;
}


template<typename Model>
bool GenericTreeView<Model>::isSelected(ItemType* item) const
{
	if(!selectionModel() || !item)
		return false;

	QModelIndex index = model()->itemToIndex(item->parent());
	int row = item->row();
	return selectionModel()->isRowSelected(row, index);
}


template<typename Model>
void GenericTreeView<Model>::toggleSelection(ItemType* item)
{
	if(!selectionModel() || !item)
		return;

	QModelIndex index = model()->itemToIndex(item);
	if(index.isValid()) {
		QItemSelectionModel::SelectionFlags flags;
		flags = QItemSelectionModel::Toggle | QItemSelectionModel::Rows;
		selectionModel()->select(index, flags);
	}
}


template<typename Model>
void GenericTreeView<Model>::setSelected(ItemType* item, bool selected)
{
	if(!selectionModel() || !item)
		return;

	QModelIndex index = model()->itemToIndex(item);
	if(index.isValid()) {
		QItemSelectionModel::SelectionFlags flags;
		flags = QItemSelectionModel::Rows;

		if(selected) {
			flags |= QItemSelectionModel::Select;
		}
		else {
			flags |= QItemSelectionModel::Deselect;
		}

		selectionModel()->select(index, flags);
	}
}


template<typename Model>
typename GenericTreeView<Model>::ItemType* GenericTreeView<Model>::itemAt(
		const QPoint& point) const
{
	if(!model())
		return nullptr;

	ItemType* item = model()->indexToItem(indexAt(point));
	if(item == rootItem())
		return nullptr;

	return item;
}


template<typename Model>
void GenericTreeView<Model>::mousePressEvent(QMouseEvent* event)
{
	if(isAutoClearSelection_) {
		QModelIndex index = indexAt(event->pos());
		if(!index.isValid())
			clearSelection();
	}

	QTreeView::mousePressEvent(event);
}


template<typename Model>
void GenericTreeView<Model>::rowsInserted(const QModelIndex& parent, int start, int end)
{
	QTreeView::rowsInserted(parent, start, end);

	// При добавлении первого элемента, он делается текущим в QAbstractItemView. Чтобы
	// синхронизовать текущий элемент с выделением (которого после создания первого
	// элемента нет), мы делаем текущий элемент невалидным.
	if(rootItem()->childCount() == 1)
		setCurrentIndex(QModelIndex());

	if(isAutoExpanding_) {
		for(int i = start; i <= end; ++i) {
			QModelIndex index = model()->index(i, 0, parent);
			setExpanded(index, true);
		}
	}
}


template<typename Model>
void GenericTreeView<Model>::currentChangeEvent(ItemType* current, ItemType* previous)
{
	Q_UNUSED(current);
	Q_UNUSED(previous);
}


template<typename Model>
void GenericTreeView<Model>::selectionChangeEvent(const QItemSelection& selected,
		const QItemSelection& deselected)
{
	Q_UNUSED(selected);
	Q_UNUSED(deselected);
}


template<typename Model>
void GenericTreeView<Model>::currentChanged(const QModelIndex& current,
		const QModelIndex& previous)
{
	QTreeView::currentChanged(current, previous);

	ItemType* currentItem = model()->indexToItem(current);
	if(currentItem == rootItem())
		currentItem = nullptr;

	ItemType* previousItem = model()->indexToItem(previous);
	if(previousItem == rootItem())
		previousItem = nullptr;

	currentChangeEvent(currentItem, previousItem);
}


template<typename Model>
void GenericTreeView<Model>::selectionChanged(const QItemSelection& selected,
		const QItemSelection& deselected)
{
	QTreeView::selectionChanged(selected, deselected);
	selectionChangeEvent(selected, deselected);
}


#endif // WIDGETS_GENERICTREEVIEW_H
