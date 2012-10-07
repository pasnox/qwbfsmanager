#include "AbstractFileSystem.h"

#include <QTimer>
#include <QDebug>

// Entry

FileSystemEntry::FileSystemEntry()
    : mFilePath( QString::null ),
        mId( QString::null ),
        mSize( -1 ),
        mFormat( FileSystemEntry::None )
{
}

FileSystemEntry::FileSystemEntry( const QString& filePath, const QString& id, qint64 size, FileSystemEntry::Format format )
    : mFilePath( filePath ),
        mId( id ),
        mSize( size ),
        mFormat( format )
{
}

bool FileSystemEntry::operator==( const FileSystemEntry& other ) const
{
    return
        mFilePath == other.mFilePath &&
        mId == other.mId &&
        mSize == other.mSize &&
        mFormat == other.mFormat
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

FileSystemEntry::Format FileSystemEntry::format() const
{
    return mFormat;
}

// AbstractFileSystem

AbstractFileSystem::AbstractFileSystem( FileSystemManager* manager )
    : QObject( manager ),
        mRefCount( 0 ),
        mFilePath( QString::null ),
        mChangeTimeOut( new QTimer( this ) )
{
    mChangeTimeOut->setSingleShot( true );
    mChangeTimeOut->setInterval( 250 );
    connect( mChangeTimeOut, SIGNAL( timeout() ), this, SLOT( dataChangedTimeOut() ) );
    
    qWarning() << Q_FUNC_INFO << this;
}

AbstractFileSystem::~AbstractFileSystem()
{
    mChangeTimeOut->stop();
    
    qWarning() << Q_FUNC_INFO << this;
}

int AbstractFileSystem::ref()
{
    return ++mRefCount;
}

int AbstractFileSystem::unref()
{
    return --mRefCount;
}

int AbstractFileSystem::count()
{
    return mRefCount;
}

QString AbstractFileSystem::filePath() const
{
    return mFilePath;
}

void AbstractFileSystem::dataChanged()
{
    mChangeTimeOut->start();
}

void AbstractFileSystem::dataChangedTimeOut()
{
    emit changed();
}
