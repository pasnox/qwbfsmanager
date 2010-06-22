/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : DataNetworkCache.cpp
** Date      : 2010-06-16T14:19:29
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**
** In addition, as a special exception, the copyright holders give permission
** to link this program with the OpenSSL project's "OpenSSL" library (or with
** modified versions of it that use the same license as the "OpenSSL"
** library), and distribute the linked executables. You must obey the GNU
** General Public License in all respects for all of the code used other than
** "OpenSSL".  If you modify file(s), you may extend this exception to your
** version of the file(s), but you are not obligated to do so. If you do not
** wish to do so, delete this exception statement from your version.
**
****************************************************************************/
#include "DataNetworkCache.h"

#include <QDir>
#include <QBuffer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QHash>
#include <QPixmapCache>

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

QPixmap DataNetworkCache::cachedPixmap( const QUrl& url ) const
{
	QPixmap pixmap;
	
	if ( !QPixmapCache::find( url.toString(), &pixmap ) ) {
		const QByteArray* data = cachedData( url );
		
		if ( data ) {
			pixmap.loadFromData( *data );
			
			if ( !QPixmapCache::insert( url.toString(), pixmap ) ) {
				emit const_cast<DataNetworkCache*>( this )->error( tr( "Can't cache pixmap" ), url );
			}
		}
	}
	
	return pixmap;
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
