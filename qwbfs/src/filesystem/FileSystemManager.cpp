#include "FileSystemManager.h"
#include "NativeFileSystem.h"

#include <QDir>
#include <QFileInfo>

FileSystemManager::FileSystemManager( QObject* parent )
    : QObject( parent )
{
}

FileSystemManager::~FileSystemManager()
{
}

AbstractFileSystem* FileSystemManager::open( const QString& _filePath, QWBFS::FileSystemType type )
{
    const QString filePath = resolvedFilePath( _filePath );
    
    if ( !QFile::exists( filePath ) ) {
        return 0;
    }
    
    AbstractFileSystem* fs = mInstances.value( filePath );
    
    if ( !fs ) {
        switch ( type ) {
            case QWBFS::FileSystemTypeWBFS: {
                break;
            }
            
            case QWBFS::FileSystemTypeNative: {
                fs = new NativeFileSystem( this );
                
                if ( fs->open( filePath ) ) {
                    mInstances[ filePath ] = fs;
                }
                else {
                    delete fs;
                    fs = 0;
                }
                
                break;
            }
            
            case QWBFS::FileSystemTypeNone:
                break;
        }
    }
    
    if ( fs ) {
        fs->ref();
    }
    
    return fs;
}

int FileSystemManager::close( const QString& _filePath )
{
    const QString filePath = resolvedFilePath( _filePath );
    AbstractFileSystem* fs = mInstances.value( filePath );
    
    if ( fs ) {
        fs->unref();
        
        if ( fs->refCount() == 0 ) {
            fs->close();
            fs->deleteLater();
            mInstances.remove( filePath );
        }
        else {
            return fs->refCount();
        }
    }
    
    return 0;
}

QString FileSystemManager::resolvedFilePath( const QString& filePath ) const
{
    QFileInfo fileInfo( filePath );
    
    while ( fileInfo.isSymLink() ) {
        fileInfo.setFile( fileInfo.symLinkTarget() );
    }
    
    return QDir::toNativeSeparators( QDir::cleanPath( fileInfo.absoluteFilePath() ) );
}
