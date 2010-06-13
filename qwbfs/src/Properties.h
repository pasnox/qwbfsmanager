#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QObject>
#include <QNetworkProxy>

class QSettings;
class UIMain;

class Properties : public QObject
{
	Q_OBJECT
	
public:
	Properties( QObject* parent = 0 );
	virtual ~Properties();
	
	QString temporaryPath() const;
	
	QString cacheWorkingPath() const;
	void setCacheWorkingPath( const QString& path );
	
	qint64 cacheDiskSize() const;
	void setCacheDiskSize( qint64 sizeKb );
	
	qint64 cacheMemorySize() const;
	void setCacheMemorySize( qint64 sizeKb );
	
	bool cacheUseTemporaryPath() const;
	void setCacheUseTemporaryPath( bool useTemporary );
	
	QNetworkProxy::ProxyType proxyType() const;
	void setProxyType( QNetworkProxy::ProxyType type );
	
	QString proxyServer() const;
	void setProxyServer( const QString& server );
	
	int proxyPort() const;
	void setProxyPort( int port );
	
	QString proxyLogin() const;
	void setProxyLogin( const QString& login );
	
	QString proxyPassword() const;
	void setProxyPassword( const QString& password );
	
	void restoreState( UIMain* window ) const;
	void saveState( UIMain* window );
	
	static QString decrypt( const QByteArray& data );
	static QByteArray crypt( const QString& string );

protected:
	QSettings* mSettings;
};

#endif // PROPERTIES_H
