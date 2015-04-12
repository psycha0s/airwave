#ifndef WIDGETS_PREFIXESVIEW_H
#define WIDGETS_PREFIXESVIEW_H

#include "generictreeview.h"
#include "models/prefixesmodel.h"


class PrefixesView : public GenericTreeView<PrefixesModel> {
	Q_OBJECT
public:
	PrefixesView(QWidget* parent = nullptr);

public slots:
	void setModel(PrefixesModel* model);

signals:
	void currentItemChanged(PrefixItem* current, PrefixItem* previous);

	void itemSelectionChanged(const QItemSelection& selected,
			const QItemSelection& deselected);

protected:
	void currentChangeEvent(PrefixItem* current, PrefixItem* previous);

	void selectionChangeEvent(const QItemSelection& selected,
			const QItemSelection& deselected);
};


#endif // WIDGETS_PREFIXESVIEW_H
