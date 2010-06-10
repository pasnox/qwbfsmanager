#include "PropertiesDialog.h"
#include "Properties.h"

#include <QFileDialog>

PropertiesDialog::PropertiesDialog( QWidget* parent )
	: QDialog( parent )
{
	setupUi( this );
	setAttribute( Qt::WA_DeleteOnClose );
	mProperties = new Properties( this );
	
	leCachePath->setText( mProperties->cacheWorkingPath() );
	sbCacheDiskSize->setValue( mProperties->cacheDiskSize() /1024 );
	sbCacheMemorySize->setValue( mProperties->cacheMemorySize() /1024 );
	cbCacheUseTemporary->setChecked( mProperties->cacheUseTemporaryPath() );
}

PropertiesDialog::~PropertiesDialog()
{
}

void PropertiesDialog::on_tbCachePath_clicked()
{
	const QString path = QFileDialog::getExistingDirectory( this, tr( "Choose a folder for the data cache" ), mProperties->cacheWorkingPath() );
	
	if ( path.isEmpty() ) {
		return;
	}
	
	leCachePath->setText( path );
}

void PropertiesDialog::accept()
{
	mProperties->setCacheWorkingPath( leCachePath->text() );
	mProperties->setCacheDiskSize( sbCacheDiskSize->value() *1024 );
	mProperties->setCacheMemorySize( sbCacheMemorySize->value() *1024 );
	mProperties->setCacheUseTemporaryPath( cbCacheUseTemporary->isChecked() );
	
	QDialog::accept();
	
	emit propertiesChanged();
}
