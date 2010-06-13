#include "PropertiesDialog.h"
#include "Properties.h"

#include <QFileDialog>
#include <QNetworkProxy>

PropertiesDialog::PropertiesDialog( QWidget* parent )
	: QDialog( parent )
{
	setupUi( this );
	setAttribute( Qt::WA_DeleteOnClose );
	mProperties = new Properties( this );
	
	cbProxyType->addItem( tr( "No Proxy" ), QNetworkProxy::NoProxy );
	cbProxyType->addItem( tr( "Socks5" ), QNetworkProxy::Socks5Proxy );
	cbProxyType->addItem( tr( "Http" ), QNetworkProxy::HttpProxy );
	
	leCachePath->setText( mProperties->cacheWorkingPath() );
	sbCacheDiskSize->setValue( mProperties->cacheDiskSize() /1024 );
	sbCacheMemorySize->setValue( mProperties->cacheMemorySize() /1024 );
	cbCacheUseTemporary->setChecked( mProperties->cacheUseTemporaryPath() );
	
	cbProxyType->setCurrentIndex( cbProxyType->findData( mProperties->proxyType() ) );
	leProxyServer->setText( mProperties->proxyServer() );
	sbProxyPort->setValue( mProperties->proxyPort() );
	leProxyLogin->setText( mProperties->proxyLogin() );
	leProxyPassword->setText( mProperties->proxyPassword() );
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

void PropertiesDialog::on_cbProxyType_currentIndexChanged( int index )
{
	const QNetworkProxy::ProxyType type = QNetworkProxy::ProxyType( cbProxyType->itemData( index ).toInt() );
	
	switch ( type )
	{
		case QNetworkProxy::NoProxy:
			leProxyServer->setEnabled( false );
			sbProxyPort->setEnabled( false );
			leProxyLogin->setEnabled( false );
			leProxyPassword->setEnabled( false );
			break;
		case QNetworkProxy::Socks5Proxy:
		case QNetworkProxy::HttpProxy:
			leProxyServer->setEnabled( true );
			sbProxyPort->setEnabled( true );
			leProxyLogin->setEnabled( true );
			leProxyPassword->setEnabled( true );
			break;
		default:
			break;
	}
}

void PropertiesDialog::accept()
{
	mProperties->setCacheWorkingPath( leCachePath->text() );
	mProperties->setCacheDiskSize( Q_INT64_C( sbCacheDiskSize->value() *1024 ) );
	mProperties->setCacheMemorySize( Q_INT64_C( sbCacheMemorySize->value() *1024 ) );
	mProperties->setCacheUseTemporaryPath( cbCacheUseTemporary->isChecked() );
	
	mProperties->setProxyType( QNetworkProxy::ProxyType( cbProxyType->itemData( cbProxyType->currentIndex() ).toInt() ) );
	mProperties->setProxyServer( leProxyServer->text() );
	mProperties->setProxyPort( sbProxyPort->value() );
	mProperties->setProxyLogin( leProxyLogin->text() );
	mProperties->setProxyPassword( leProxyPassword->text() );
	
	QDialog::accept();
	
	emit propertiesChanged();
}
