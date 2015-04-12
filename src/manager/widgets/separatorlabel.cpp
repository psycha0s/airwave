#include "separatorlabel.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>


SeparatorLabel::SeparatorLabel(const QString& text, QWidget* parent) :
	QWidget(parent)
{
	label_ = new QLabel(text);

	QFont font;
	font.setBold(true);
	label_->setFont(font);

	QFrame* line = new QFrame;
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QHBoxLayout* layout = new QHBoxLayout;
	layout->setMargin(0);
	layout->addWidget(label_);
	layout->addWidget(line);

	setLayout(layout);
}
