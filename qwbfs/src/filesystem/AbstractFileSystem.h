#ifndef ABSTRACTFILESYSTEM_H
#define ABSTRACTFILESYSTEM_H

#include <QObject>

#include "FileSystemManager.h"

class QTimer;

// FileSystemEntry

class FileSystemEntry
{
    Q_GADGET
    Q_FLAGS( Formats )
    friend class AbstractFileSystem;
    
public:
    enum Format {
        None = 0x0,
        ISO = 0x1,
        CISO = 0x2,
        WBFS = 0x4
    };
    
    Q_DECLARE_FLAGS( Formats, Format )
    
    typedef QList<FileSystemEntry> List;
    
    FileSystemEntry();
    virtual ~FileSystemEntry();
    
    bool operator==( const FileSystemEntry& other ) const;
    bool operator!=( const FileSystemEntry& other ) const;
    
    QString filePath() const;
    QString id() const;
    qint64 size() const;
    FileSystemEntry::Format format() const;

protected:
    QString mFilePath;
    QString mId;
    qint64 mSize;
    FileSystemEntry::Format mFormat;
    
    FileSystemEntry( const QString& filePath, const QString& id, qint64 size, FileSystemEntry::Format format );
};

Q_DECLARE_OPERATORS_FOR_FLAGS( FileSystemEntry::Formats )

// AbstractFileSystem

class AbstractFileSystem : public QObject
{
    Q_OBJECT
    friend class FileSystemManager;
    
public:
    AbstractFileSystem( FileSystemManager* manager );
    virtual ~AbstractFileSystem();
    
    QString filePath() const;
    
    virtual bool mount( const QString& filePath ) = 0;
    virtual bool umount() = 0;
    virtual bool format() = 0;
    
    virtual FileSystemManager::Type type() const = 0;
    virtual FileSystemEntry::Formats supportedFormats() const = 0;
    virtual FileSystemEntry::Format preferredFormat() const = 0;
    
    virtual FileSystemEntry::List entries() = 0;
    virtual FileSystemEntry entry( const QString& id ) = 0;
    virtual bool hasEntry( const QString& id ) = 0;
    
    virtual bool addEntry( const FileSystemEntry& entry, FileSystemEntry::Format format = FileSystemEntry::None ) = 0;
    virtual bool removeEntry( const FileSystemEntry& entry ) = 0;

protected:
    int ref();
    int unref();
    int count();
    void dataChanged();
    
    virtual FileSystemEntry createEntry( const QString& filePath ) const = 0;

protected slots:
    void dataChangedTimeOut();

protected:
    int mRefCount;
    QString mFilePath;
    QTimer* mChangeTimeOut;

signals:
    void changed();
    void progress( qint64 read, qint64 total );
};

#endif // ABSTRACTFILESYSTEM_H
