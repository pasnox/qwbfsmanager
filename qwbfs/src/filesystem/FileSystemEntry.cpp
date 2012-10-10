#include "FileSystemEntry.h"

FileSystemEntry::FileSystemEntry()
    : mFilePath( QString::null ),
        mId( QString::null ),
        mSize( -1 ),
        mType( QWBFS::EntryTypeNone )
{
}

FileSystemEntry::FileSystemEntry( const QString& filePath, const QString& id, qint64 size, QWBFS::EntryType type )
    : mFilePath( filePath ),
        mId( id ),
        mSize( size ),
        mType( type )
{
}

bool FileSystemEntry::operator==( const FileSystemEntry& other ) const
{
    return
        mFilePath == other.mFilePath &&
        mId == other.mId &&
        mSize == other.mSize &&
        mType == other.mType
    ;
}

bool FileSystemEntry::operator!=( const FileSystemEntry& other ) const
{
    return !operator==( other );
}

FileSystemEntry::~FileSystemEntry()
{
}

QString FileSystemEntry::filePath() const
{
    return mFilePath;
}

QString FileSystemEntry::id() const
{
    return mId;
}

qint64 FileSystemEntry::size() const
{
    return mSize;
}

QWBFS::EntryType FileSystemEntry::type() const
{
    return mType;
}
