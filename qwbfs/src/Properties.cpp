#include "Properties.h"

#include <QSettings>
#include <QDir>

#define SETTINGS_CACHE_WORKING_PATH "cache/workingPath"
#define SETTINGS_CACHE_DISK_SIZE "cache/diskSize"
#define SETTINGS_CACHE_MEMORY_SIZE "cache/MemorySize"
#define SETTINGS_CACHE_USE_TEMPORARY_WORKING_PATH "cache/useTemporaryWorkingPath"

Properties::Properties( QObject* parent )
	: QObject( parent )
{
	mSettings = new QSettings( this );
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

int Properties::cacheDiskSize() const
{
	return mSettings->value( SETTINGS_CACHE_DISK_SIZE, 1024 *50 ).toInt();
}

void Properties::setCacheDiskSize( int sizeKb )
{
	mSettings->setValue( SETTINGS_CACHE_DISK_SIZE, sizeKb );
}

int Properties::cacheMemorySize() const
{
	return mSettings->value( SETTINGS_CACHE_MEMORY_SIZE, 1024 *5 ).toInt();
}

void Properties::setCacheMemorySize( int sizeKb )
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
