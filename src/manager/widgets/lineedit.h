#ifndef WIDGETS_LINEEDIT_H
#define WIDGETS_LINEEDIT_H

#include <QLineEdit>
#include <QTimer>


class QToolButton;


class LineEdit: public QLineEdit {
	Q_OBJECT
public:
	enum ButtonStyle {
		kNormal,
		kAutoRaise,
		kLightAutoRaise,
		kTransparent
	};

	LineEdit(QWidget* parent = nullptr);

	bool hasButton() const;
	void setButtonEnabled(bool enabled);

	ButtonStyle buttonStyle() const;
	void setButtonStyle(ButtonStyle style);

	bool isAutoClearMode() const;
	void setAutoClearMode(bool enabled);

	QString buttonToolTip() const;
	void setButtonToolTip(const QString& toolTip);

	QIcon buttonIcon() const;
	void setButtonIcon(const QIcon& icon);

	uint editTimeout() const;
	void setEditTimeout(uint msecs);

	bool hasIcon() const;
	void setIcon(const QIcon& icon);

	QString prefix() const;
	void setPrefix(const QString& prefix);

	QColor prefixColor() const;
	void setPrefixColor(const QColor& color);

	QString suffix() const;
	void setSuffix(const QString& suffix);

	QColor suffixColor() const;
	void setSuffixColor(const QColor& color);

signals:
	void buttonClicked();
	void textEditTimeout(const QString& text);

protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);

private:
	bool hasButton_;
	QToolButton* button_;
	ButtonStyle buttonStyle_;
	bool isAutoClearMode_;
	QTimer timer_;
	QPixmap icon_;
	QString prefix_;
	QString suffix_;
	QColor prefixColor_;
	QColor suffixColor_;

	void updateLayout();

private slots:
	void onButtonClicked();
	void onTextChanged();
	void onTimeout();
};


#endif // WIDGETS_LINEEDIT_H
