#ifndef NATIVEFILESYSTEM_H
#define NATIVEFILESYSTEM_H

#include "AbstractFileSystem.h"

#include <QHash>

class NativeFileSystem : public AbstractFileSystem
{
    Q_OBJECT
    
public:
    NativeFileSystem( FileSystemManager* manager );
    virtual ~NativeFileSystem();
    
    virtual bool mount( const QString& filePath );
    virtual bool umount();
    virtual bool isMounted() const;
    virtual bool format();
    
    virtual QWBFS::FileSystemType type() const;
    virtual QWBFS::EntryTypes supportedFormats() const;
    virtual QWBFS::EntryType preferredFormat() const;
    
    virtual FileSystemEntry::List entries() const;
    virtual FileSystemEntry entry( int row ) const;
    virtual FileSystemEntry entry( const QString& id ) const;
    virtual bool hasEntry( const QString& id ) const;
    
    virtual bool addEntry( const FileSystemEntry& entry, QWBFS::EntryType format = QWBFS::EntryTypeNone );
    virtual bool removeEntry( const FileSystemEntry& entry );
    virtual void clear();

protected:
    virtual FileSystemEntry createEntry( const QString& filePath ) const;
    
    void buildCache();

protected:
    bool mIsMounted;
    FileSystemEntry::List mEntries;
    QHash<QString, FileSystemEntry*> mCache;
};

#endif // NATIVEFILESYSTEM_H
