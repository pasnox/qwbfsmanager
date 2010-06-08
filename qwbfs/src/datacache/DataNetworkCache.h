#ifndef DATANETWORKCACHE_H
#define DATANETWORKCACHE_H

#include <QObject>
#include <QUrl>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QCache>

class DataNetworkCache : public QObject
{
	Q_OBJECT
	
public:
	DataNetworkCache( QObject* parent = 0 );
	virtual ~DataNetworkCache();
	
	QString workingPath() const;
	int diskCacheSize() const;
	int memoryCacheSize() const;
	bool hasCachedData( const QUrl& url ) const;
	QByteArray* cachedData( const QUrl& url ) const;
	QIODevice* cachedDataDevice( const QUrl& url ) const;

public slots:
	void setWorkingPath( const QString& path );
	void setDiskCacheSize( int sizeKb );
	void setMemoryCacheSize( int sizeKb );
	void cacheData( const QUrl& url );
	void clear();
	void clearMemory();
	void clearDisk();

protected:
	QNetworkAccessManager* mManager;
	mutable QCache<uint, QByteArray> mCache;
	QHash<uint,QNetworkReply*> mRequests;
	QString mWorkingPath;
	int mDiskCacheSize;
	int mMemoryCacheSize;
	
	QString localFilePath( uint key ) const;
	QString localFilePath( const QUrl& url ) const;
	void updateCacheRestrictions();
	void clearMemory( bool emitSignal );
	void clearDisk( bool emitSignal );

protected slots:
	void networkManager_finished( QNetworkReply* reply );

signals:
	void dataCached( const QUrl& url );
	void error( const QString& message );
	void invalidated();
};

#endif // DATANETWORKCACHE_H
