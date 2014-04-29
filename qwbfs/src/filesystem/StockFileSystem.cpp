#include "StockFileSystem.h"

StockFileSystem::StockFileSystem( FileSystemManager* manager, const QString& name )
    : AbstractFileSystem( manager )
{
    if ( !name.isEmpty() ) {
        open( name );
    }
}

StockFileSystem::~StockFileSystem()
{
    close();
}

bool StockFileSystem::open( const QString& mountPoint )
{
    if ( isMounted() ) {
        if ( AbstractFileSystem::mountPoint() == mountPoint ) {
            return true;
        }
        
        if ( !close() ) {
            return false;
        }
    }
    
    mMountPoint = mountPoint;
    return true;
}

bool StockFileSystem::close()
{
    if ( isMounted() ) {
        // TODO: add sync() member in abstract class and move mount/umount as non virtual in abstract
        clear();
        mMountPoint.clear();
    }
    
    return true;
}

bool StockFileSystem::format()
{
    if ( !isMounted() ) {
        return false;
    }
    
    clear();
    return true;
}

QWBFS::FileSystemType StockFileSystem::type() const
{
    return QWBFS::FileSystemTypeStock;
}

QWBFS::EntryTypes StockFileSystem::supportedFormats() const
{
    return QWBFS::EntryTypeWiiISO | QWBFS::EntryTypeWiiCISO | QWBFS::EntryTypeWiiWBFS;
}

QWBFS::EntryType StockFileSystem::preferredFormat() const
{
    return QWBFS::EntryTypeWiiWBFS;
}

FileSystemEntry::List StockFileSystem::entries() const
{
    return mEntries;
}

FileSystemEntry StockFileSystem::entry( int row ) const
{
    return mEntries.value( row );
}

FileSystemEntry StockFileSystem::entry( const QString& id ) const
{
    FileSystemEntry* entry = mCache.value( id.toUpper(), 0 );
    return entry ? *entry : FileSystemEntry();
}

bool StockFileSystem::hasEntry( const QString& id ) const
{
    return mCache.contains( id.toUpper() );
}

bool StockFileSystem::addEntry( const FileSystemEntry& entry, QWBFS::EntryType format )
{
    Q_UNUSED( format );
    
    if ( hasEntry( entry.id() ) ) {
        return false;
    }
    
    const int count = rowCount();
    
    beginInsertRows( QModelIndex(), count, count );
    mEntries << entry;
    mCache[ entry.id().toUpper() ] = &mEntries.last();
    endInsertRows();
    
    dataChanged();
    return true;
}

bool StockFileSystem::removeEntry( const FileSystemEntry& entry )
{
    const int row = mEntries.indexOf( entry );
    
    if ( row == -1 ) {
        return false;
    }
    
    beginRemoveRows( QModelIndex(), row, row );
    mCache.remove( entry.id().toUpper() );
    mEntries.removeAt( row );
    endRemoveRows();
    
    dataChanged();
    return true;
}

void StockFileSystem::clear()
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

void StockFileSystem::setEntriesInternal( const FileSystemEntry::List& entries )
{
    mEntries = entries;
    mCache.clear();
    
    for ( int i = 0; i < mEntries.count(); i++ ) {
        FileSystemEntry& entry = mEntries[ i ];
        mCache[ entry.id().toUpper() ] = &entry;
    }
}
