#ifndef UTILITY_WIDGETS_SIMPLETREEVIEW_H
#define UTILITY_WIDGETS_SIMPLETREEVIEW_H

#include <QTreeView>
#include <QItemSelection>


class SimpleTreeView : public QTreeView {
	Q_OBJECT
public:
	explicit SimpleTreeView(QWidget* parent = 0);

	QModelIndexList selectedRows() const;
	void selectRow(int row, const QModelIndex& parent = QModelIndex());
	void deselectRow(int row, const QModelIndex &parent = QModelIndex());
	void clearSelection();

	bool hasSelection() const;

	int firstSelectedRow() const;

	bool isRowSelected(int row,
			const QModelIndex& parent = QModelIndex()) const;

	void setAutoExpanding(bool value);
	bool isAutoExpanding() const;

signals:
	void currentChanged(const QModelIndex& current);

	void selectionUpdated(const QItemSelection &selected,
			const QItemSelection &deselected);

protected:
	void mousePressEvent(QMouseEvent* event);

	void currentChanged(const QModelIndex& current,
			const QModelIndex& previous);

	void selectionChanged(const QItemSelection& selected,
			const QItemSelection& deselected);

	void rowsInserted(const QModelIndex& parent, int start, int end);


private:
	bool isAutoExpanding_;
};


#endif // UTILITY_WIDGETS_SIMPLETREEVIEW_H
