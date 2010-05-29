#ifndef UIABOUT_H
#define UIABOUT_H

#include "ui_UIAbout.h"

class UIAbout : public QDialog, public Ui::UIAbout
{
	Q_OBJECT

public:
	UIAbout( QWidget* parent = 0 );

protected slots:
	void on_dbbButtons_clicked( QAbstractButton* button );
};

#endif // UIABOUT_H
