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

void Properties::restoreState( UIMain* window ) const
{
	const QByteArray geometry = mSettings->value( SETTINGS_WINDOW_GEOMETRY ).toByteArray();
	const QByteArray state = mSettings->value( SETTINGS_WINDOW_STATE ).toByteArray();
	
	window->restoreGeometry( geometry );
	window->restoreState( state );
}

void Properties::saveState( UIMain* window )
{
	const QByteArray geometry = window->saveGeometry();
	const QByteArray state = window->saveState();
	
	mSettings->setValue( SETTINGS_WINDOW_GEOMETRY, geometry );
	mSettings->setValue( SETTINGS_WINDOW_STATE, state );
}
