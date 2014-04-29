#ifndef FILESYSTEMENTRY_H
#define FILESYSTEMENTRY_H

#include <QFileInfo>

#include "QWBFS.h"

class FileSystemEntry
{
public:
    typedef QList<FileSystemEntry> List;
    
    FileSystemEntry();
    FileSystemEntry( const QString& filePath, const QString& id, qint64 size, const QString& title, char region, QWBFS::EntryType type );
    virtual ~FileSystemEntry();
    
    bool operator==( const FileSystemEntry& other ) const;
    bool operator!=( const FileSystemEntry& other ) const;
    
    QString filePath() const;
    QString id() const;
    qint64 size() const;
    QWBFS::EntryType type() const;
    QWBFS::EntryState state() const;
    QString title() const;
    char region() const;
    
    QString toString() const
    {
        return QString(
            "--------------\n"
            "filePath: %1\n"
            "id: %2\n"
            "size: %3\n"
            "type: %4\n"
            "title: %5\n"
            "region: %6\n"
            "state: %7"
        )
            .arg( mFilePath )
            .arg( mId )
            .arg( mSize )
            .arg( mType )
            .arg( mTitle )
            .arg( mRegion )
            .arg( mState )
        ;
    }

protected:
    QString mFilePath;
    QString mId;
    qint64 mSize;
    QWBFS::EntryType mType;
    QWBFS::EntryState mState;
    QString mTitle;
    char mRegion;
    
};



#endif // FILESYSTEMENTRY_H
