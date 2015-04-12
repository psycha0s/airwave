#ifndef WIDGETS_DIRECTORYVIEW_H
#define WIDGETS_DIRECTORYVIEW_H

#include "models/directorymodel.h"
#include "widgets/generictreeview.h"


class DirectoryView : public GenericTreeView<DirectoryModel> {
	Q_OBJECT
public:
	DirectoryView(QWidget* parent = nullptr);

public slots:
	void setModel(DirectoryModel* model);

signals:
	void currentItemChanged(DirectoryItem* current, DirectoryItem* previous);

	void itemSelectionChanged(const QItemSelection& selected,
			const QItemSelection& deselected);

	void itemDoubleClicked(DirectoryItem* current);

protected:
	void currentChangeEvent(DirectoryItem* current, DirectoryItem* previous);

	void selectionChangeEvent(const QItemSelection& selected,
			const QItemSelection& deselected);

	void mouseDoubleClickEvent(QMouseEvent* event);
};


#endif // WIDGETS_DIRECTORYVIEW_H
