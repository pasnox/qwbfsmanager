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

QString Properties::temporaryDirectory() const
{
	return QDir::tempPath();
}

QString Properties::workingPath() const
{
	return mSettings->value( SETTINGS_CACHE_WORKING_PATH, temporaryDirectory() ).toString();
}

void Properties::setCacheWorkingPath( const QString& path )
{
	mSettings->setValue( SETTINGS_CACHE_WORKING_PATH, path );
}

int Properties::diskCacheSize() const
{
	return mSettings->value( SETTINGS_CACHE_DISK_SIZE, 1024 *50 ).toInt() *1024;
}

void Properties::setDiskCacheSize( int sizeKb )
{
	mSettings->setValue( SETTINGS_CACHE_DISK_SIZE, sizeKb );
}

int Properties::memoryCacheSize() const
{
	return mSettings->value( SETTINGS_CACHE_MEMORY_SIZE, 1024 *5 ).toInt() *1024;
}

void Properties::setMemoryCacheSize( int sizeKb )
{
	mSettings->setValue( SETTINGS_CACHE_MEMORY_SIZE, sizeKb );
}

bool Properties::useTemporaryCachePath() const
{
	return mSettings->value( SETTINGS_CACHE_USE_TEMPORARY_WORKING_PATH, false ).toBool();
}

void Properties::setUseTemporaryCachePath( bool useTemporary )
{
	mSettings->setValue( SETTINGS_CACHE_USE_TEMPORARY_WORKING_PATH, useTemporary );
}
