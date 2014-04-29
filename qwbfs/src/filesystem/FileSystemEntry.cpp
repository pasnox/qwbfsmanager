#include "FileSystemEntry.h"

FileSystemEntry::FileSystemEntry()
    : mFilePath( QString::null ),
        mId( QString::null ),
        mSize( -1 ),
        mType( QWBFS::EntryTypeUnknown ),
        mState( QWBFS::EntryStateNone ),
        mTitle( QString::null ),
        mRegion( 0 )
{
}

FileSystemEntry::FileSystemEntry( const QString& filePath, const QString& id, qint64 size, const QString& title, char region, QWBFS::EntryType type )
    : mFilePath( filePath ),
        mId( id ),
        mSize( size ),
        mType( type ),
        mState( QWBFS::EntryStateNone ),
        mTitle( title ),
        mRegion( region )
{
}

bool FileSystemEntry::operator==( const FileSystemEntry& other ) const
{
    return
        mFilePath == other.mFilePath &&
        mId == other.mId &&
        mSize == other.mSize &&
        mType == other.mType &&
        mTitle == other.mTitle &&
        mRegion == other.mRegion &&
        mState == other.mState
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

QWBFS::EntryState FileSystemEntry::state() const
{
    return mState;
}

QString FileSystemEntry::title() const
{
    return mTitle.isEmpty() ? "#NaN" : mTitle;
}

char FileSystemEntry::region() const
{
    return mRegion;
}
