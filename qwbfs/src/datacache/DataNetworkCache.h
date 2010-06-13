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
