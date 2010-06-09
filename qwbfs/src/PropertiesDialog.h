#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include "ui_PropertiesDialog.h"

class Properties;

class PropertiesDialog : public QDialog, public Ui::PropertiesDialog
{
	Q_OBJECT

public:
	PropertiesDialog( QWidget* parent = 0 );
	virtual ~PropertiesDialog();

protected:
	Properties* mProperties;
};

#endif // PROPERTIESDIALOG_H
