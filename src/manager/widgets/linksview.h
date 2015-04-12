#ifndef WIDGETS_LINKSVIEW_H
#define WIDGETS_LINKSVIEW_H

#include "models/linksmodel.h"
#include "widgets/generictreeview.h"


class LinksView : public GenericTreeView<LinksModel> {
	Q_OBJECT
public:
	LinksView(QWidget* parent = nullptr);

public slots:
	void setModel(LinksModel* model);

signals:
	void currentItemChanged(LinkItem* current, LinkItem* previous);

	void itemSelectionChanged(const QItemSelection& selected,
			const QItemSelection& deselected);

	void itemDoubleClicked(LinkItem* item);

protected:
	void currentChangeEvent(LinkItem* current, LinkItem* previous);

	void selectionChangeEvent(const QItemSelection& selected,
			const QItemSelection& deselected);

private slots:
	void onItemDoubleClicked(const QModelIndex& index);
};


#endif // WIDGETS_LINKSVIEW_H
