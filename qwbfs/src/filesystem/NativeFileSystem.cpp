#include "NativeFileSystem.h"

#include <FreshCore/pCoreUtils>

#include <QFileInfo>
#include <QDebug>

NativeFileSystem::NativeFileSystem( FileSystemManager* manager )
    : AbstractFileSystem( manager ),
        mIsMounted( false )
{
}

NativeFileSystem::~NativeFileSystem()
{
    umount();
}

bool NativeFileSystem::mount( const QString& filePath )
{
    if ( isMounted() ) {
        if ( mFilePath == filePath ) {
            return true;
        }
        
        if ( !umount() ) {
            return false;
        }
    }
    
    // TODO: Add QFileSystemWatcher on the directory to track manual change / delete / add ( from file manager )
    const QFileInfo fileInfo( filePath );
    
    if (
        !fileInfo.isDir() ||
        !fileInfo.isReadable() ||
        !fileInfo.isWritable() /*||
        fileInfo.fileName() != "wbfs"*/
    ) {
        return false;
    }
    
    mFilePath = filePath;
    buildCache();
    return true;
}

bool NativeFileSystem::umount()
{
    if ( isMounted() ) {
        // TODO: add sync() member in abstract class and move mount/umount as non virtual in abstract
        clear();
        mFilePath.clear();
        mIsMounted = false;
    }
    
    return true;
}

bool NativeFileSystem::isMounted() const
{
    return mIsMounted;
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
    return QWBFS::EntryTypeISO | QWBFS::EntryTypeCISO | QWBFS::EntryTypeWBFS;
}

QWBFS::EntryType NativeFileSystem::preferredFormat() const
{
    return QWBFS::EntryTypeWBFS;
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
    return *mCache.value( id.toUpper() );
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

FileSystemEntry NativeFileSystem::createEntry( const QString& filePath ) const
{
    const QFileInfo fileInfo( filePath );
    const QString suffix = fileInfo.suffix().toLower();
    QWBFS::EntryType format = QWBFS::EntryTypeNone;
    
    if ( suffix.compare( "iso", Qt::CaseInsensitive ) == 0 ) {
        format = QWBFS::EntryTypeISO;
    }
    else if ( suffix.compare( "ciso", Qt::CaseInsensitive ) == 0 ) {
        format = QWBFS::EntryTypeCISO;
    }
    else if ( suffix.compare( "wbfs", Qt::CaseInsensitive ) == 0 ) {
        format = QWBFS::EntryTypeWBFS;
    }
    
    return FileSystemEntry( filePath, fileInfo.fileName(), fileInfo.size(), format );
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
    
    clear();
    
    if ( filePaths.isEmpty() ) {
        return;
    }
    
    beginInsertRows( QModelIndex(), 0, filePaths.count() -1 );
    for ( int i = 0; i < filePaths.count(); i++ ) {
        mEntries << createEntry( filePaths[ i ] );
        mCache[ mEntries.last().id() ] = &mEntries.last();
    }
    endInsertRows();
}
