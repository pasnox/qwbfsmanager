/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : Properties.cpp
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
#include "Properties.h"
#include "datacache/DataNetworkCache.h"
#include "UIMain.h"

#include <QSettings>
#include <QDir>
#include <QDebug>

#define SETTINGS_CACHE_WORKING_PATH "cache/workingPath"
#define SETTINGS_CACHE_DISK_SIZE "cache/diskSize"
#define SETTINGS_CACHE_MEMORY_SIZE "cache/memorySize"
#define SETTINGS_CACHE_USE_TEMPORARY_WORKING_PATH "cache/useTemporaryWorkingPath"
#define SETTINGS_PROXY_TYPE "proxy/type"
#define SETTINGS_PROXY_SERVER "proxy/server"
#define SETTINGS_PROXY_PORT "proxy/port"
#define SETTINGS_PROXY_LOGIN "proxy/login"
#define SETTINGS_PROXY_PASSWORD "proxy/password"
#define SETTINGS_WINDOW_GEOMETRY "window/geometry"
#define SETTINGS_WINDOW_STATE "window/state"
#define SETTINGS_PARTITION_WIDGET_STATE "partitionWidget"

Properties::Properties( QObject* parent )
	: QObject( parent )
{
	mSettings = new QSettings( this );
	//qWarning() << mSettings->fileName();
}

Properties::~Properties()
{
}

QString Properties::temporaryPath() const
{
	return QDir::tempPath();
}

QString Properties::cacheWorkingPath() const
{
	return mSettings->value( SETTINGS_CACHE_WORKING_PATH, temporaryPath() ).toString();
}

void Properties::setCacheWorkingPath( const QString& path )
{
	mSettings->setValue( SETTINGS_CACHE_WORKING_PATH, path );
}

qint64 Properties::cacheDiskSize() const
{
	return mSettings->value( SETTINGS_CACHE_DISK_SIZE, DATA_NETWORK_CACHE_DEFAULT_DISK_SIZE ).value<qint64>();
}

void Properties::setCacheDiskSize( qint64 sizeKb )
{
	mSettings->setValue( SETTINGS_CACHE_DISK_SIZE, sizeKb );
}

qint64 Properties::cacheMemorySize() const
{
	return mSettings->value( SETTINGS_CACHE_MEMORY_SIZE, DATA_NETWORK_CACHE_DEFAULT_MEMORY_SIZE ).value<qint64>();
}

void Properties::setCacheMemorySize( qint64 sizeKb )
{
	mSettings->setValue( SETTINGS_CACHE_MEMORY_SIZE, sizeKb );
}

bool Properties::cacheUseTemporaryPath() const
{
	return mSettings->value( SETTINGS_CACHE_USE_TEMPORARY_WORKING_PATH, false ).toBool();
}

void Properties::setCacheUseTemporaryPath( bool useTemporary )
{
	mSettings->setValue( SETTINGS_CACHE_USE_TEMPORARY_WORKING_PATH, useTemporary );
}

QNetworkProxy::ProxyType Properties::proxyType() const
{
	return QNetworkProxy::ProxyType( mSettings->value( SETTINGS_PROXY_TYPE, QNetworkProxy::NoProxy ).toInt() );
}

void Properties::setProxyType( QNetworkProxy::ProxyType type )
{
	mSettings->setValue( SETTINGS_PROXY_TYPE, type );
}

QString Properties::proxyServer() const
{
	return mSettings->value( SETTINGS_PROXY_SERVER ).toString();
}

void Properties::setProxyServer( const QString& server )
{
	mSettings->setValue( SETTINGS_PROXY_SERVER, server );
}

int Properties::proxyPort() const
{
	return mSettings->value( SETTINGS_PROXY_PORT, -1 ).toInt();
}

void Properties::setProxyPort( int port )
{
	mSettings->setValue( SETTINGS_PROXY_PORT, port );
}

QString Properties::proxyLogin() const
{
	return mSettings->value( SETTINGS_PROXY_LOGIN ).toString();
}

void Properties::setProxyLogin( const QString& login )
{
	mSettings->setValue( SETTINGS_PROXY_LOGIN, login );
}

QString Properties::proxyPassword() const
{
	return decrypt( mSettings->value( SETTINGS_PROXY_PASSWORD ).toByteArray() );
}

void Properties::setProxyPassword( const QString& password )
{
	mSettings->setValue( SETTINGS_PROXY_PASSWORD, crypt( password ) );
}

void Properties::restoreState( UIMain* window ) const
{
	const QRect geometry = mSettings->value( SETTINGS_WINDOW_GEOMETRY ).toRect();
	const QByteArray state = mSettings->value( SETTINGS_WINDOW_STATE ).toByteArray();
	
	if ( geometry.isNull() ) {
		window->showMaximized();
	}
	else {
		window->setGeometry( geometry );
	}
	
	window->restoreState( state );
}

void Properties::saveState( UIMain* window )
{
	const QRect geometry = window->geometry();
	const QByteArray state = window->saveState();
	
	mSettings->setValue( SETTINGS_WINDOW_GEOMETRY, geometry );
	mSettings->setValue( SETTINGS_WINDOW_STATE, state );
}

QString Properties::decrypt( const QByteArray& data )
{
	return data.isEmpty() ? QString::null : QString::fromUtf8( qUncompress( QByteArray::fromBase64( data ) ) );
}

QByteArray Properties::crypt( const QString& string )
{
	return string.isEmpty() ? QByteArray() : qCompress( string.toUtf8(), 9 ).toBase64();
}
