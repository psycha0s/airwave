#ifndef MODELS_GENERICTREEMODEL_H
#define MODELS_GENERICTREEMODEL_H

#include <QAbstractItemModel>


template<typename T>
class GenericTreeModel;


template<typename Derived>
class GenericTreeItem {
public:
	typedef bool (*LessThanProc)(Derived*, Derived*);

	/**
	 * Конструирует новый элемент и устанавливает признак возможности отложенной загрузки
	 * информации о его дочерних элементах.
	 */
	GenericTreeItem(bool canFetchMore = false);

	virtual ~GenericTreeItem();

	/**
	 * Возвращает родительский элемент или @c nullptr, в случае его отсутствия.
	 */
	Derived* parent() const;

	/**
	 * Возвращает количество дочерниих элементов.
	 */
	int childCount() const;

	/**
	 * Возвращает @c true, если @p item является дочерним элементом для текущего.
	 */
	bool hasChild(Derived* item) const;

	/**
	 * Возвращает @c true, если элемент имеет хотя бы один дочерний элемент.
	 */
	bool hasChildren() const;

	/**
	 * Возвращает дочерний элемент на позиции @p row.
	 */
	Derived* childAt(int row) const;

	/**
	 * Возвращает первый дочерний элемент или @c nullptr, в случае его отсутствия.
	 */
	Derived* firstChild() const;

	/**
	 * Возвращает последний дочерний элемент или @c nullptr, в случае его отсутствия.
	 */
	Derived* lastChild() const;

	/**
	 * Возвращает следующий смежный элемент или @c nullptr, в случае его отсутствия.
	 */
	Derived* nextSibling() const;

	/**
	 * Возвращает предыдущий смежный элемент или @c nullptr, в случае его отсутствия.
	 */
	Derived* previousSibling() const;

	/**
	 * Производит вставку элемента @p item в дочерние элементы на позицию @p row. Если @p
	 * row меньше нуля или больше количества дочерних элементов, то элемент @p item
	 * помещается на последниюю позицию. Право владения элементом @p item передается
	 * текущему элементу.
	 *
	 * @param item вставляемый элемент.
	 * @param row позиция вставки.
	 */
	void insertChild(Derived* item, int row = -1);

	/**
	 * Производит перемещение элемента @p item в дочерние элементы на позицию @p row.
	 * Если @c row меньше нуля или больше количества дочерних элементов, то элемент @p
	 * item помещается на последнюю позицию. Если @p item является вышестоящим элементом
	 * или относится к другому экземпляру модели, то перемещения не происходит и функция
	 * возвращает @c false.
	 *
	 * @param item перемещаемый элемент.
	 * @param row позиция, на которую требуется переместить @p item.
	 * @return @c true в случае успеха.
	 */
	bool moveChild(Derived* item, int row = -1);

	/**
	 * Производит извлечение дочернего элемента с позиции @p row и возвращает этот
	 * элемент. Если @p row меньше нуля или больше количества дочерних элементов, то
	 * возвращается @c nullptr. Право владения элементом передается вызывающему коду.
	 *
	 * @param row позиция извлекаемого элемента.
	 * @return извлеченный элемент или @c nullptr.
	 */
	Derived* takeChild(int row);

	/**
	 * Производит извлечение дочернего элемента на позиции @p row с последующем его
	 * уничтожением.
	 *
	 * @param row позиция удаляемого элемента.
	 */
	void removeChild(int row);

	/**
	 * Рекурсивно уничтожает все дочерние элементы.
	 */
	void removeChildren();

	Derived* takeFromParent();

	/**
	 * Возвращает позицию элемента в списке дочерних элементов владельца.
	 */
	int row() const;

	/**
	 * Возвращает модель, к которой относится элемент, или @c nullptr, если элемент не
	 * относится ни к одной модели.
	 */
	GenericTreeModel<Derived>* model() const;

	void sortChildren(LessThanProc lessThan);

	/**
	 * Возвращает @c true, если информация о дочерних элементах не была получена.
	 */
	bool canFetchMore() const;

protected:
	/**
	 * Устанавливает признак возможности получения информации о дочерних элементах.
	 */
	void setCanFetchMore(bool canFetchMore);

	/**
	 * Получает информацию о дочерних элементах. Базовая реализация просто устанавливает
	 * @p canFetchMore в @c false. При необходимости выполнять отложенную загрузку
	 * потомки должны переопределить эту функцию и реализовать загрузку в ней.
	 */
	virtual void fetchMore();

	/**
	 * Производит уведомление модели о необходимости обновить данные элемента. Функция
	 * должна вызываться потомками после любого изменения данных элемента.
	 */
	void updateData();

	/**
	 * Вызывается при установке элементу новой модели. При необходимости выполнить
	 * дополнительные действия во время помещения элемента в модель, потомки должны
	 * переопределить эту функцию и выполнить эти действия в ней. В переопределенной
	 * функции запрещается производить действия над элементами модели, связанные со
	 * вставкой, удалением или перемещением, т.к. это может привести к рекурсии или
	 * непредсказуемому результату.
	 */
	virtual void attached() { }

	/**
	 * Вызывается при извлечении элемента из модели. При необходимости выполнить
	 * дополнительные действия во время извлечения элемента из модели, потомки должны
	 * переопределить эту функцию и выполнить эти действия в ней. В переопределенной
	 * функции запрещается производить действия над элементами модели, связанные со
	 * вставкой, удалением или перемещением, т.к. это может привести к рекурсии или
	 * непредсказуемому результату.
	 */
	virtual void detached() { }

	/**
	 * Вызывается при изменении положения элемента. При необходимости выполнить
	 * дополнительные действия по этому событию (например обновить parent id элемента в
	 * БД) потомки должны переопределить эту функцию и выполнить требуемые действия в
	 * ней. В переопределенной функции запрещается производить действия над элементами
	 * модели, связанные со вставкой, удалением или перемещением, т.к. это может привести
	 * к рекурсии или непредсказуемому результату.
	 */
	virtual void reattached() { }

	/**
	 * Вызывается сразу после вставки дочернего элемента @p item. Потомки могут
	 * переобпределить эту функцию, чтобы получать уведомления о событии. В
	 * переопределенной функции запрещается производить действия над элементами модели,
	 * связанные с вставкой, удалением или перемещением, т.к. это может привести к
	 * рекурсии или непредсказуемому результату.
	 *
	 * @param item новый дочерний элемент.
	 */
	virtual void childInserted(Derived* item) { Q_UNUSED(item); }

	/**
	 * Вызывается сразу после извлечения дочернего элемента @p item. Потомки могут
	 * переобпределить эту функцию, чтобы получать уведомления о событии. В
	 * переопределенной функции запрещается производить действия над элементами модели,
	 * связанные с вставкой, удалением или перемещением, т.к. это может привести к
	 * рекурсии или непредсказуемому результату.
	 *
	 * @param item извлеченный дочерний элемент.
	 */
	virtual void childRemoved(Derived* item) { Q_UNUSED(item); }

private:
	friend class GenericTreeModel<Derived>;

	Derived* parent_;
	QList<Derived*> children_;
	int row_;
	GenericTreeModel<Derived>* model_;
	bool canFetchMore_;


	void attach();
	void detach();
};


template<typename T>
class GenericTreeModel : public QAbstractItemModel {
public:
	typedef T ItemType;

	/**
	 * Конструирует модель и устанавливает root в качестве фиктивного корневого элемента.
	 */
	GenericTreeModel(T* root = new T(), QObject* parent = nullptr);

	virtual ~GenericTreeModel();

	/**
	 * Уничтожает все элементы модели.
	 */
	void clear();

	/**
	 * Преобразует модельный индекс @p index в элемент. В случае невалидного индекса
	 * возвращает @c rootItem().
	 */
	T* indexToItem(const QModelIndex& index) const;

	/**
	 * Преобразует элемент в модельный индекс. В случае, если @p item равен @c nullptr,
	 * или @p item не принадлежит модели, или @p item равен @c rootItem(), возвращается
	 * невалидный модельный индекс.
	 *
	 * @param item преобразуемый элемент.
	 * @param column колонка в преобразуемом элементе.
	 */
	QModelIndex itemToIndex(T* item, int column = 0) const;

	/**
	 * Возвращает фиктивный корневой элемент модели. Данный элемент является невидимым
	 * для всех работающих с моделью представлений и служит для унификации интерфейса
	 * добавления/перемещения/удаления дочерних элементов.
	 */
	T* root() const;

	virtual QModelIndex index(int row, int column,
			const QModelIndex& parent = QModelIndex()) const;

	virtual QModelIndex parent(const QModelIndex& index = QModelIndex()) const;

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

	virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;

	virtual bool canFetchMore(const QModelIndex& parent = QModelIndex()) const;

	virtual void fetchMore(const QModelIndex& parent = QModelIndex());

protected:
	/**
	 * Сбрасывает модель в ее первоначальное состояние. Базовая реализация просто
	 * уничтожает все элементы модели, при этом вызов AbstractItemModel::reset() не
	 * производится, поэтому функция является безопасной при использовании proxy моделей.
	 */
	void reset();

	void updateData(T* item);

private:
	friend class GenericTreeItem<T>;

	T* rootItem_;

	void beginInsert(T* parentItem, int row, int count = 1);
	void endInsert();

	bool beginMove(T* srcParent, int srcRow, T* destParent, int destRow, int count = 1);

	void endMove();

	void beginRemove(T* parentItem, int row, int count = 1);
	void endRemove();

	QModelIndex createNewIndex(int row, int column);

	void changePersistentIndexes(const QModelIndexList& from,
			const QModelIndexList& to);
};


//---------------------------------- GenericTreeItem -----------------------------------//


template<typename Derived>
GenericTreeItem<Derived>::GenericTreeItem(bool canFetchMore) :
	parent_(nullptr),
	row_(0),
	model_(nullptr),
	canFetchMore_(canFetchMore)
{
}


template<typename Derived>
GenericTreeItem<Derived>::~GenericTreeItem()
{
	qDeleteAll(children_);
}


template<typename Derived>
Derived* GenericTreeItem<Derived>::parent() const
{
	return parent_;
}


template<typename Derived>
int GenericTreeItem<Derived>::childCount() const
{
	return children_.count();
}


template<typename Derived>
bool GenericTreeItem<Derived>::hasChild(Derived* item) const
{
	return (children_.indexOf(item) != -1);
}


template<typename Derived>
bool GenericTreeItem<Derived>::hasChildren() const
{
	return !children_.isEmpty();
}


template<typename Derived>
Derived* GenericTreeItem<Derived>::childAt(int row) const
{
	return children_.at(row);
}


template<typename Derived>
Derived* GenericTreeItem<Derived>::firstChild() const
{
	if(children_.isEmpty())
		return nullptr;

	return children_.first();
}


template<typename Derived>
Derived* GenericTreeItem<Derived>::lastChild() const
{
	if(children_.isEmpty())
		return nullptr;

	return children_.last();
}


template<typename Derived>
Derived* GenericTreeItem<Derived>::nextSibling() const
{
	if(!parent_ || row_ + 1 == parent_->children_.count())
		return nullptr;

	return parent_->children_.at(row_ + 1);
}


template<typename Derived>
Derived* GenericTreeItem<Derived>::previousSibling() const
{
	if(!parent_ || row_ == 0)
		return nullptr;

	return parent_->children_.at(row_ - 1);
}


template<typename Derived>
void GenericTreeItem<Derived>::attach()
{
	model_ = parent_->model_;

	Derived* item = static_cast<Derived*>(this);
	item = item->firstChild();

	while(item) {
		item->attach();
		item = item->nextSibling();
	}

	attached();
}


template<typename Derived>
void GenericTreeItem<Derived>::detach()
{
	model_ = nullptr;

	Derived* item = static_cast<Derived*>(this);
	item = item->firstChild();

	while(item) {
		item->detach();
		item = item->nextSibling();
	}

	detached();
}


template<typename Derived>
void GenericTreeItem<Derived>::insertChild(Derived* item, int row)
{
	Q_ASSERT(item);

	if(item->parent_)
		item->parent_->takeChild(item->row());

	item->parent_ = static_cast<Derived*>(this);

	if((row < 0) || (row > children_.count()))
		item->row_ = children_.count();
	else
		item->row_ = row;

	if(model_)
		model_->beginInsert(item->parent_, item->row_);

	for(int i = item->row_; i < children_.count(); ++i)
		children_[i]->row_++;

	children_.insert(item->row_, item);

	if(model_) {
		item->attach();
		model_->endInsert();
	}

	childInserted(item);
}


template<typename Derived>
bool GenericTreeItem<Derived>::moveChild(Derived* item, int row)
{
	Q_ASSERT(item);

	// Перемещать элементы можно только в пределах одной модели
	if(item->model_ != model_)
		return false;

	if((row < 0) || (row > children_.count()))
		row = children_.count();

	int modelRow = row;

	Derived* oldParent = item->parent_;
	Derived* newParent = static_cast<Derived*>(this);

	if(oldParent == newParent) {
		// Если перемещение идет на свое собственно место, дополнительные действия не
		// требуются
		if(item->row_ == row)
			return true;

		// Перемещение вниз
		if(item->row_ < row)
			++modelRow;
	}

	// Нельзя перемещать элемент в один из его дочерних элементов
	Derived* temp = static_cast<Derived*>(this);
	while(temp) {
		if(item == temp)
			return false;

		temp = temp->parent_;
	}

	if(model_ && !model_->beginMove(oldParent, item->row_, newParent, modelRow))
		return false;

	if(oldParent == newParent) {
		if(row > item->row_) {
			for(int i = item->row_ + 1; i < row; ++i)
				children_[i]->row_--;

			children_.move(item->row_, row - 1);
			item->row_ = row - 1;
		}
		else if(row < item->row_) {
			for(int i = row; i < item->row_; ++i)
				children_[i]->row_++;

			children_.move(item->row_, row);
			item->row_ = row;
		}
	}
	else {
		oldParent->children_.takeAt(item->row_);

		for(int i = item->row_; i < oldParent->children_.count(); ++i)
			oldParent->children_[i]->row_--;

		item->row_ = row;
		item->parent_ = newParent;

		for(int i = item->row_; i < newParent->children_.count(); ++i)
			newParent->children_[i]->row_++;

		children_.insert(row, item);
	}

	if(model_)
		model_->endMove();

	if(oldParent != newParent) {
		oldParent->childRemoved(item);
		newParent->childInserted(item);
	}

	item->reattached();
	return true;
}


template<typename Derived>
Derived* GenericTreeItem<Derived>::takeChild(int row)
{
	if(model_)
		model_->beginRemove(static_cast<Derived*>(this), row);

	Derived* item = children_.takeAt(row);

	for(int i = item->row_; i < children_.count(); ++i)
		children_[i]->row_--;

	item->parent_ = nullptr;
	item->row_ = 0;
	item->model_ = nullptr;

	item->detach();

	if(model_)
		model_->endRemove();

	childRemoved(item);
	return item;
}


template<typename Derived>
void GenericTreeItem<Derived>::removeChild(int row)
{
	delete takeChild(row);
}


template<typename Derived>
void GenericTreeItem<Derived>::removeChildren()
{
	int count = children_.count();

	// Удаляем дочерние элементы от последнего к первому, чтобы избежать лишних
	// копирований элементов внутри children_.
	while(count--)
		removeChild(count);
}


template<typename Derived>
Derived* GenericTreeItem<Derived>::takeFromParent()
{
	if(parent_)
		return parent_->takeChild(row_);

	return static_cast<Derived*>(this);
}


template<typename Derived>
int GenericTreeItem<Derived>::row() const
{
	return row_;
}


template<typename Derived>
bool GenericTreeItem<Derived>::canFetchMore() const
{
	return canFetchMore_;
}


template<typename Derived>
void GenericTreeItem<Derived>::setCanFetchMore(bool canFetchMore)
{
	canFetchMore_ = canFetchMore;
}


template<typename Derived>
void GenericTreeItem<Derived>::fetchMore()
{
	canFetchMore_ = false;
}


template<typename Derived>
GenericTreeModel<Derived>* GenericTreeItem<Derived>::model() const
{
	return model_;
}


template<typename Derived>
void GenericTreeItem<Derived>::updateData()
{
	if(model_)
		model_->updateData(static_cast<Derived*>(this));
}


template<typename Derived>
void GenericTreeItem<Derived>::sortChildren(LessThanProc lessThan)
{
	qSort(children_.begin(), children_.end(), lessThan);

	if(model_) {
		QModelIndexList fromIndexes;
		QModelIndexList toIndexes;

		for(int row = 0; row < children_.count(); ++row) {
			Derived* item = children_[row];

			int oldRow = item->row_;
			item->row_ = row;

			for(int i = 0; i < model()->columnCount(); ++i) {
				fromIndexes.append(model_->createNewIndex(oldRow, i));
				toIndexes.append(model_->createNewIndex(row, i));
			}
		}

		model_->changePersistentIndexes(fromIndexes, toIndexes);
	}
}


//---------------------------------- GenericTreeModel ----------------------------------//


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
T* GenericTreeModel<T>::root() const
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
void GenericTreeModel<T>::updateData(T* item)
{
	QModelIndex from = itemToIndex(item);
	QModelIndex parent = from.parent();
	int last = qMax(columnCount() - 1, 0);
	QModelIndex to = index(from.row(), last, parent);
	emit dataChanged(from, to);
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
bool GenericTreeModel<T>::beginMove(T* srcParent, int srcRow, T* destParent, int destRow,
		int count)
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


#endif // MODELS_GENERICTREEMODEL_H
