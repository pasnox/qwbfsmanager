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

protected slots:
	void on_tbCachePath_clicked();
	void on_cbProxyType_currentIndexChanged( int index );
	virtual void accept();

signals:
	void propertiesChanged();
};

#endif // PROPERTIESDIALOG_H
