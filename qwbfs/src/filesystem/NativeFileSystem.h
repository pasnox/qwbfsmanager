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
    virtual bool format();
    
    virtual FileSystemManager::Type type() const;
    virtual FileSystemEntry::Formats supportedFormats() const;
    virtual FileSystemEntry::Format preferredFormat() const;
    
    virtual FileSystemEntry::List entries();
    virtual FileSystemEntry entry( const QString& id );
    virtual bool hasEntry( const QString& id );
    
    virtual bool addEntry( const FileSystemEntry& entry, FileSystemEntry::Format format = FileSystemEntry::None );
    virtual bool removeEntry( const FileSystemEntry& entry );

protected:
    virtual FileSystemEntry createEntry( const QString& filePath ) const;
    void buildCache();

protected:
    QHash<QString, FileSystemEntry> mCache;
};

#endif // NATIVEFILESYSTEM_H
