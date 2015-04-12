#ifndef WIDGETS_LOADERSVIEW_H
#define WIDGETS_LOADERSVIEW_H

#include "generictreeview.h"
#include "models/loadersmodel.h"


class LoadersView : public GenericTreeView<LoadersModel> {
	Q_OBJECT
public:
	LoadersView(QWidget* parent = nullptr);

public slots:
	void setModel(LoadersModel* model);

signals:
	void currentItemChanged(LoaderItem* current, LoaderItem* previous);

	void itemSelectionChanged(const QItemSelection& selected,
			const QItemSelection& deselected);

protected:
	void currentChangeEvent(LoaderItem* current, LoaderItem* previous);

	void selectionChangeEvent(const QItemSelection& selected,
			const QItemSelection& deselected);
};


#endif // WIDGETS_LOADERSVIEW_H
