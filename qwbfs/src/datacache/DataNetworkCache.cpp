#include "DataNetworkCache.h"

#include <QDir>
#include <QBuffer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QHash>

#if QT_VERSION < 0x040700
uint qHash( const QUrl& url )
{
	return qHash( url.toString() );
}
#endif

DataNetworkCache::DataNetworkCache( QObject* parent )
	: QObject( parent )
{
	mManager = new QNetworkAccessManager( this );
	mWorkingPath = QDir::tempPath();
	mDiskCacheSize = 0;
	mMemoryCacheSize = 0;
	
	setDiskCacheSize( DATA_NETWORK_CACHE_DEFAULT_DISK_SIZE );
	setMemoryCacheSize( DATA_NETWORK_CACHE_DEFAULT_MEMORY_SIZE );
	
	connect( mManager, SIGNAL( authenticationRequired( QNetworkReply*, QAuthenticator* ) ), this, SLOT( networkManager_authenticationRequired( QNetworkReply*, QAuthenticator* ) ) );
	connect( mManager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( networkManager_finished( QNetworkReply* ) ) );
	connect( mManager, SIGNAL( proxyAuthenticationRequired( const QNetworkProxy&, QAuthenticator* ) ), this, SLOT( networkManager_proxyAuthenticationRequired( const QNetworkProxy&, QAuthenticator* ) ) );
	connect( mManager, SIGNAL( sslErrors( QNetworkReply*, const QList<QSslError>& ) ), this, SLOT( networkManager_sslErrors( QNetworkReply*, const QList<QSslError>& ) ) );
}

DataNetworkCache::~DataNetworkCache()
{
}

QString DataNetworkCache::localFilePath( uint key ) const
{
	return QString( "%1/%2" ).arg( mWorkingPath ).arg( key );
}

QString DataNetworkCache::localFilePath( const QUrl& url ) const
{
	return localFilePath( qHash( url ) );
}

void DataNetworkCache::updateCacheRestrictions()
{
	//
}

void DataNetworkCache::networkManager_authenticationRequired( QNetworkReply* reply, QAuthenticator* authenticator )
{
	Q_UNUSED( reply );
	Q_UNUSED( authenticator );
	qWarning() << Q_FUNC_INFO;
}

void DataNetworkCache::networkManager_finished( QNetworkReply* reply )
{
	const QUrl url = reply->url();
	const uint key = qHash( url );
	const int code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
	
	mRequests.remove( key );
	reply->deleteLater();
	
	if ( reply->error() != QNetworkReply::NoError || code != 200 ) {
		emit error( tr( "Error when downloading the file '%1'." ).arg( url.toString() ), url );
		return;
	}
	
	QFile file( localFilePath( key ) );
	
	if ( !file.open( QIODevice::WriteOnly ) ) {
		emit error( tr( "Error when creating the cache for file '%1'" ).arg( url.toString() ), url );
		return;
	}
	
	if ( file.write( reply->readAll() ) == -1 ) {
		emit error( tr( "Error when writing the cache for file '%1'" ).arg( url.toString() ), url );
		return;
	}
	
	file.close();
	emit dataCached( url );
}

void DataNetworkCache::networkManager_proxyAuthenticationRequired( const QNetworkProxy& proxy, QAuthenticator* authenticator )
{
	Q_UNUSED( proxy );
	Q_UNUSED( authenticator );
	qWarning() << Q_FUNC_INFO;
}

void DataNetworkCache::networkManager_sslErrors( QNetworkReply* reply, const QList<QSslError>& errors )
{
	Q_UNUSED( reply );
	Q_UNUSED( errors );
	qWarning() << Q_FUNC_INFO;
	reply->ignoreSslErrors();
}

void DataNetworkCache::clearMemory( bool emitSignal )
{
	mCache.clear();
	
	if ( emitSignal ) {
		emit invalidated();
	}
}

void DataNetworkCache::clearDisk( bool emitSignal )
{
	if ( emitSignal ) {
		emit invalidated();
	}
}

QString DataNetworkCache::workingPath() const
{
	return mWorkingPath;
}

qint64 DataNetworkCache::diskCacheSize() const
{
	return mDiskCacheSize;
}

qint64 DataNetworkCache::memoryCacheSize() const
{
	return mMemoryCacheSize;
}

bool DataNetworkCache::hasCachedData( const QUrl& url ) const
{
	const uint key = qHash( url );
	return mCache.contains( key ) || QFile::exists( localFilePath( key ) );
}

QByteArray* DataNetworkCache::cachedData( const QUrl& url ) const
{
	const uint key = qHash( url );
	
	if ( mCache.contains( key ) ) {
		return mCache.object( key );
	}
	else {
		QFile file( localFilePath( key ) );
		
		if ( file.open( QIODevice::ReadOnly ) ) {
			QByteArray* data = new QByteArray( file.readAll() );
			
			if ( file.size() /1024 < mCache.maxCost() ){
				if ( mCache.insert( key, data, file.size() /1024 ) ) {
					return data;
				}
			}
			else {
				return data;
			}
		}
	}
	
	return 0;
}

QIODevice* DataNetworkCache::cachedDataDevice( const QUrl& url ) const
{
	const uint key = qHash( url );
	
	if ( mCache.contains( key ) ) {
		QBuffer* buffer = new QBuffer( mCache.object( key ) );
		buffer->open( QIODevice::ReadOnly );
		return buffer;
	}
	else {
		QFile* file = new QFile( localFilePath( key ) );
		
		if ( file->open( QIODevice::ReadOnly ) ) {
		
			if ( file->size() /1024 < mCache.maxCost() ) {
				QByteArray* data = new QByteArray( file->readAll() );
				
				file->reset();
				
				if ( mCache.insert( key, data, file->size() /1024 ) ) {
					return file;
				}
			}
			else {
				return file;
			}
		}
		else {
			delete file;
		}
	}
	
	return 0;
}

void DataNetworkCache::setWorkingPath( const QString& path )
{
	QFileInfo fi( path );
	
	if ( fi.exists() && !fi.isDir() ) {
		emit error( tr( "The working path '%1' exists and is not a directory." ) );
		return;
	}
	
	if ( !fi.exists() ) {
		QDir dir;
		
		if ( !dir.mkpath( path ) ) {
			emit error( tr( "Can't create the working path '%1'." ) );
			return;
		}
	}
	
	mWorkingPath = path;
}

void DataNetworkCache::setDiskCacheSize( qint64 sizeKb )
{
	mDiskCacheSize = sizeKb;
	updateCacheRestrictions();
}

void DataNetworkCache::setMemoryCacheSize( qint64 sizeKb )
{
	mMemoryCacheSize = 0 ? DATA_NETWORK_CACHE_DEFAULT_MEMORY_SIZE : sizeKb;
	mCache.setMaxCost( mMemoryCacheSize *1024 );
}

void DataNetworkCache::cacheData( const QUrl& url )
{
	const uint key = qHash( url );
	
	if ( mRequests.contains( key ) ) {
		return;
	}
	
	QNetworkRequest request( url );
	mRequests[ key ] = mManager->get( request );
}

void DataNetworkCache::clear()
{
	clearMemory( false );
	clearDisk( true );
}

void DataNetworkCache::clearMemory()
{
	clearMemory( true );
}

void DataNetworkCache::clearDisk()
{
	clearDisk( true );
}
