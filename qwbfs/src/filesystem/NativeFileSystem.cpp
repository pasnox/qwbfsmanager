#include "NativeFileSystem.h"

#include <FreshCore/pCoreUtils>

#include <QFileInfo>
#include <QDebug>

NativeFileSystem::NativeFileSystem( FileSystemManager* manager )
    : AbstractFileSystem( manager )
{
}

NativeFileSystem::~NativeFileSystem()
{
    close();
}

bool NativeFileSystem::open( const QString& mountPoint )
{
    if ( isMounted() ) {
        if ( AbstractFileSystem::mountPoint() == mountPoint ) {
            return true;
        }
        
        if ( !close() ) {
            return false;
        }
    }
    
    // TODO: Add QFileSystemWatcher on the directory to track manual change / delete / add ( from file manager )
    const QFileInfo fileInfo( mountPoint );
    
    if (
        !fileInfo.isDir() ||
        !fileInfo.isReadable() ||
        !fileInfo.isWritable() /*||
        fileInfo.fileName() != "wbfs"*/
    ) {
        return false;
    }
    
    mMountPoint = mountPoint;
    loadEntries();
    return true;
}

bool NativeFileSystem::close()
{
    if ( isMounted() ) {
        // TODO: add sync() member in abstract class and move mount/umount as non virtual in abstract
        clear();
        mMountPoint.clear();
    }
    
    return true;
}

bool NativeFileSystem::format()
{
    return false;
}

QWBFS::FileSystemType NativeFileSystem::type() const
{
    return QWBFS::FileSystemTypeNative;
}

QWBFS::EntryTypes NativeFileSystem::supportedFormats() const
{
    return QWBFS::EntryTypeWiiISO | QWBFS::EntryTypeWiiCISO | QWBFS::EntryTypeWiiWBFS;
}

QWBFS::EntryType NativeFileSystem::preferredFormat() const
{
    return QWBFS::EntryTypeWiiWBFS;
}

FileSystemEntry::List NativeFileSystem::entries() const
{
    return mEntries;
}

FileSystemEntry NativeFileSystem::entry( int row ) const
{
    return mEntries.value( row );
}

FileSystemEntry NativeFileSystem::entry( const QString& id ) const
{
    FileSystemEntry* entry = mCache.value( id.toUpper(), 0 );
    return entry ? *entry : FileSystemEntry();
}

bool NativeFileSystem::hasEntry( const QString& id ) const
{
    return mCache.contains( id.toUpper() );
}

bool NativeFileSystem::addEntry( const FileSystemEntry& entry, QWBFS::EntryType format )
{
    //dataChanged();
    return false;
}

bool NativeFileSystem::removeEntry( const FileSystemEntry& entry )
{
    //dataChanged();
    return false;
}

void NativeFileSystem::clear()
{
    const int count = rowCount();
    
    if ( count == 0 ) {
        return;
    }
    
    beginRemoveRows( QModelIndex(), 0, count -1 );
    mCache.clear();
    mEntries.clear();
    endRemoveRows();
}

void NativeFileSystem::setEntriesInternal( const FileSystemEntry::List& entries )
{
    mEntries = entries;
    buildCache();
}

void NativeFileSystem::loadEntries()
{
    QDir dir( mountPoint() );
    const QStringList filters = QStringList()
        << "*.iso"
        << "*.ciso"
        << "*.wbfs"
    ;
    const QStringList filePaths = pCoreUtils::findFiles( dir, filters, true );
    
    clear();
    
    if ( filePaths.isEmpty() ) {
        return;
    }
    
    beginInsertRows( QModelIndex(), 0, filePaths.count() -1 );
    
    for ( int i = 0; i < filePaths.count(); i++ ) {
        mEntries << QWBFS::createEntry( filePaths[ i ] );
    }
    
    buildCache();
    
    endInsertRows();
}

void NativeFileSystem::buildCache()
{
    mCache.clear();
    
    for ( int i = 0; i < mEntries.count(); i++ ) {
        FileSystemEntry& entry = mEntries[ i ];
        mCache.insertMulti( entry.id().toUpper(), &entry );
    }
}
