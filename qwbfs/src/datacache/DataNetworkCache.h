/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : DataNetworkCache.h
** Date      : 2010-06-15T23:21:10
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR HARDWARE OR YOUR DATA
** BECAUSE OF IMPROPER USAGE OF THIS SOFTWARE.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef DATANETWORKCACHE_H
#define DATANETWORKCACHE_H

#include <QObject>
#include <QUrl>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QCache>

#define DATA_NETWORK_CACHE_DEFAULT_DISK_SIZE 1024 *50 // 50 MB
#define DATA_NETWORK_CACHE_DEFAULT_MEMORY_SIZE 1024 *5 // 5 MB

class DataNetworkCache : public QObject
{
	Q_OBJECT
	
public:
	DataNetworkCache( QObject* parent = 0 );
	virtual ~DataNetworkCache();
	
	QString workingPath() const;
	qint64 diskCacheSize() const;
	qint64 memoryCacheSize() const;
	bool hasCachedData( const QUrl& url ) const;
	QByteArray* cachedData( const QUrl& url ) const;
	QIODevice* cachedDataDevice( const QUrl& url ) const;

public slots:
	void setWorkingPath( const QString& path );
	void setDiskCacheSize( qint64 sizeKb );
	void setMemoryCacheSize( qint64 sizeKb );
	void cacheData( const QUrl& url );
	void clear();
	void clearMemory();
	void clearDisk();

protected:
	QNetworkAccessManager* mManager;
	mutable QCache<uint, QByteArray> mCache;
	QHash<uint,QNetworkReply*> mRequests;
	QString mWorkingPath;
	qint64 mDiskCacheSize;
	qint64 mMemoryCacheSize;
	
	QString localFilePath( uint key ) const;
	QString localFilePath( const QUrl& url ) const;
	void updateCacheRestrictions();
	void clearMemory( bool emitSignal );
	void clearDisk( bool emitSignal );

protected slots:
	void networkManager_authenticationRequired( QNetworkReply* reply, QAuthenticator* authenticator );
	void networkManager_finished( QNetworkReply* reply );
	void networkManager_proxyAuthenticationRequired( const QNetworkProxy& proxy, QAuthenticator* authenticator );
	void networkManager_sslErrors( QNetworkReply* reply, const QList<QSslError>& errors );

signals:
	void dataCached( const QUrl& url );
	void error( const QString& message, const QUrl& url = QUrl() );
	void invalidated();
};

#if QT_VERSION < 0x040700
uint qHash( const QUrl& url );
#endif

#endif // DATANETWORKCACHE_H
