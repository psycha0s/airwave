#ifndef GENERICTREEMODEL_H
#define GENERICTREEMODEL_H

#include <QAbstractItemModel>
#include "generictreeitem.h"


template<typename T>
class GenericTreeItem;


template<typename T>
class GenericTreeModel : public QAbstractItemModel {
public:
	/**
	 * Конструирует модель и устанавливает root в качестве фиктивного
	 * корневого элемента.
	 */
	GenericTreeModel(T* root = new T(), QObject* parent = nullptr);


	virtual ~GenericTreeModel();


	/**
	 * Уничтожает все элементы модели.
	 */
	void clear();


	/**
	 * Преобразует модельный индекс @index в элемент.
	 *
	 * В случае невалидного индекса возвращает @c rootItem().
	 */
	T* indexToItem(const QModelIndex& index) const;


	/**
	 * Преобразует элемент в модельный индекс.
	 *
	 * В случае, если @c item равен @c nullptr, или @c item не принадлежит
	 * модели, или @c item равен @c rootItem(), возвращается невалидный
	 * модельный индекс.
	 *
	 * @param item преобразуемый элемент.
	 * @param column колонка в преобразуемом элементе.
	 */
	QModelIndex itemToIndex(T* item, int column = 0) const;


	/**
	 * Возвращает фиктивный корневой элемент модели.
	 *
	 * Данный элемент является невидимым для всех работающих с моделью
	 * представлений и служит для унификации интерфейса
	 * добавления/перемещения/удаления дочерних элементов.
	 */
	T* rootItem() const;


	QModelIndex index(int row, int column,
			const QModelIndex& parent = QModelIndex()) const;

	QModelIndex parent(const QModelIndex& index = QModelIndex()) const;

	int rowCount(const QModelIndex& parent = QModelIndex()) const;

	bool hasChildren(const QModelIndex& parent = QModelIndex()) const;

	bool canFetchMore(const QModelIndex& parent = QModelIndex()) const;

	void fetchMore(const QModelIndex& parent = QModelIndex());

protected:
	/**
	 * Сбрасывает модель в ее первоначальное состояние.
	 *
	 * Базовая реализация просто уничтожает все элементы модели, при этом
	 * вызов AbstractItemModel::reset() не производится, поэтому функция
	 * является безопасной при использовании proxy моделей.
	 */
	void reset();

private:
	friend class GenericTreeItem<T>;

	T* rootItem_;


	void beginInsert(T* parentItem, int row, int count = 1);
	void endInsert();

	bool beginMove(T* srcParent, int srcRow, T* destParent, int destRow,
			int count = 1);

	void endMove();

	void beginRemove(T* parentItem, int row, int count = 1);
	void endRemove();

	void updateData(T* item);

	QModelIndex createNewIndex(int row, int column);

	void changePersistentIndexes(const QModelIndexList& from,
			const QModelIndexList& to);

};


template<typename T>
GenericTreeModel<T>::GenericTreeModel(T* root, QObject* parent) :
	QAbstractItemModel(parent),
	rootItem_(root)
{
	Q_ASSERT(root);
	rootItem_->model_ = this;

	if(rootItem_->hasChildren()) {
		beginInsert(rootItem_, 0, rootItem_->childCount());

		foreach(T* item, rootItem_->children_)
			item->model_ = this;

		endInsert();
	}
}


template<typename T>
GenericTreeModel<T>::~GenericTreeModel()
{
	delete rootItem_;
}


template<typename T>
void GenericTreeModel<T>::clear()
{
	beginResetModel();
	rootItem_->removeChildren();
	endResetModel();
}


template<typename T>
T* GenericTreeModel<T>::indexToItem(const QModelIndex& index) const
{
	if(!index.isValid())
		return rootItem_;

	return static_cast<T*>(index.internalPointer());
}


template<typename T>
QModelIndex GenericTreeModel<T>::itemToIndex(T* item, int column) const
{
	if(!item || item->model() != this || !item->parent())
		return QModelIndex();

	return createIndex(item->row(), column, item);
}


template<typename T>
T* GenericTreeModel<T>::rootItem() const
{
	return rootItem_;
}


template<typename T>
QModelIndex GenericTreeModel<T>::index(int row, int column,
		const QModelIndex& parent) const
{
	Q_UNUSED(column);

	T* parentItem = indexToItem(parent);
	if(!parentItem || (row < 0) || (parentItem->childCount() <= row))
		return QModelIndex();

	return itemToIndex(parentItem->childAt(row), column);
}


template<typename T>
QModelIndex GenericTreeModel<T>::parent(const QModelIndex& index) const
{
	return itemToIndex(indexToItem(index)->parent());
}


template<typename T>
int GenericTreeModel<T>::rowCount(const QModelIndex& parent) const
{
	T* parentItem = indexToItem(parent);
	return parentItem->childCount();
}


template<typename T>
bool GenericTreeModel<T>::hasChildren(const QModelIndex& parent) const
{
	T* item = indexToItem(parent);
	if(item->canFetchMore())
		return true;
	else
		return item->hasChildren();
}


template<typename T>
bool GenericTreeModel<T>::canFetchMore(const QModelIndex& parent) const
{
	T* item = indexToItem(parent);
	return item->canFetchMore();
}


template<typename T>
void GenericTreeModel<T>::fetchMore(const QModelIndex& parent)
{
	T* item = indexToItem(parent);
	if(item->canFetchMore())
		item->fetchMore();
}


template<typename T>
void GenericTreeModel<T>::reset()
{
	clear();
}


template<typename T>
void GenericTreeModel<T>::beginInsert(T* parentItem, int row, int count)
{
	beginInsertRows(itemToIndex(parentItem), row, row + count - 1);
}


template<typename T>
void GenericTreeModel<T>::endInsert()
{
	endInsertRows();
}


template<typename T>
bool GenericTreeModel<T>::beginMove(T* srcParent, int srcRow, T* destParent,
		int destRow, int count)
{
	return beginMoveRows(itemToIndex(srcParent), srcRow, srcRow + count - 1,
			itemToIndex(destParent), destRow);
}


template<typename T>
void GenericTreeModel<T>::endMove()
{
	endMoveRows();
}


template<typename T>
void GenericTreeModel<T>::beginRemove(T* parentItem, int row, int count)
{
	beginRemoveRows(itemToIndex(parentItem), row, row + count - 1);
}


template<typename T>
void GenericTreeModel<T>::endRemove()
{
	endRemoveRows();
}


template<typename T>
void GenericTreeModel<T>::updateData(T* item)
{
	QModelIndex from = itemToIndex(item);
	QModelIndex parent = from.parent();
	int last = qMax(columnCount() - 1, 0);
	QModelIndex to = index(from.row(), last, parent);
	emit dataChanged(from, to);
}


template<typename T>
inline QModelIndex GenericTreeModel<T>::createNewIndex(int row, int column)
{
	return createIndex(row, column);
}


template<typename T>
inline void GenericTreeModel<T>::changePersistentIndexes(
		const QModelIndexList& from, const QModelIndexList& to)
{
	changePersistentIndexList(from, to);
}


#endif // GENERICTREEMODEL_H
