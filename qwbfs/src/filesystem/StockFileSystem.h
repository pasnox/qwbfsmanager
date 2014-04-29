#ifndef STOCKFILESYSTEM_H
#define STOCKFILESYSTEM_H

#include "AbstractFileSystem.h"

#include <QHash>

class StockFileSystem : public AbstractFileSystem
{
    Q_OBJECT
    
public:
    StockFileSystem( FileSystemManager* manager, const QString& name = QString::null );
    virtual ~StockFileSystem();
    
    virtual bool open( const QString& mountPoint );
    virtual bool close();
    virtual bool format();
    
    virtual QWBFS::FileSystemType type() const;
    virtual QWBFS::EntryTypes supportedFormats() const;
    virtual QWBFS::EntryType preferredFormat() const;
    
    virtual FileSystemEntry::List entries() const;
    virtual FileSystemEntry entry( int row ) const;
    virtual FileSystemEntry entry( const QString& id ) const;
    virtual bool hasEntry( const QString& id ) const;
    
    virtual bool addEntry( const FileSystemEntry& entry, QWBFS::EntryType format = QWBFS::EntryTypeUnknown );
    virtual bool removeEntry( const FileSystemEntry& entry );
    virtual void clear();

protected:
    virtual void setEntriesInternal( const FileSystemEntry::List& entries );

protected:
    FileSystemEntry::List mEntries;
    QHash<QString, FileSystemEntry*> mCache;
};

#endif // STOCKFILESYSTEM_H
