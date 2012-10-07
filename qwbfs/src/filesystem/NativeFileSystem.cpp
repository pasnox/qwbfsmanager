#include "NativeFileSystem.h"

#include <pCoreUtils.h>

#include <QFileInfo>

NativeFileSystem::NativeFileSystem( FileSystemManager* manager )
    : AbstractFileSystem( manager )
{
}

NativeFileSystem::~NativeFileSystem()
{
}

bool NativeFileSystem::mount( const QString& filePath )
{
    // TODO: Add QFileSystemWatcher on the directory to track manual change / delete / add ( from file manager )
    const QFileInfo fileInfo( filePath );
    
    if ( !fileInfo.isDir() || !fileInfo.isReadable() || !fileInfo.isWritable() ) {
        return false;
    }
    
    mFilePath = filePath;
    return true;
}

bool NativeFileSystem::umount()
{
    return true;
}

bool NativeFileSystem::format()
{
    return false;
}

FileSystemManager::Type NativeFileSystem::type() const
{
    return FileSystemManager::Native;
}

FileSystemEntry::Formats NativeFileSystem::supportedFormats() const
{
    return FileSystemEntry::ISO | FileSystemEntry::CISO | FileSystemEntry::WBFS;
}

FileSystemEntry::Format NativeFileSystem::preferredFormat() const
{
    return FileSystemEntry::WBFS;
}

FileSystemEntry::List NativeFileSystem::entries()
{
    if ( mCache.isEmpty() ) {
        buildCache();
    }
    
    return mCache.values();
}

FileSystemEntry NativeFileSystem::entry( const QString& id )
{
    return mCache.value( id.toUpper() );
}

bool NativeFileSystem::hasEntry( const QString& id )
{
    return mCache.contains( id.toUpper() );
}

bool NativeFileSystem::addEntry( const FileSystemEntry& entry, FileSystemEntry::Format format )
{
    //dataChanged();
    return false;
}

bool NativeFileSystem::removeEntry( const FileSystemEntry& entry )
{
    //dataChanged();
    return false;
}

FileSystemEntry NativeFileSystem::createEntry( const QString& filePath ) const
{
    return FileSystemEntry();
}

void NativeFileSystem::buildCache()
{
    QDir dir( mFilePath );
    const QStringList filters = QStringList()
        << "*.iso"
        << "*.ciso"
        << "*.wbfs"
    ;
    const QStringList filePaths = pCoreUtils::findFiles( dir, filters, true );
    
    mCache.clear();
    
    for ( int i = 0; i < filePaths.count(); i++ ) {
        const FileSystemEntry entry = createEntry( filePaths[ i ] );
        mCache[ entry.id() ] = entry;
    }
    
    dataChanged();
}
