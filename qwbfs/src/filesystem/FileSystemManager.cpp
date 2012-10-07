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

AbstractFileSystem* FileSystemManager::open( const QString& _filePath, FileSystemManager::Type type )
{
    const QString filePath = resolvedFilePath( _filePath );
    
    if ( !QFile::exists( filePath ) ) {
        return 0;
    }
    
    AbstractFileSystem* fs = mInstances.value( filePath );
    
    if ( !fs ) {
        switch ( type ) {
            case FileSystemManager::WBFS: {
                break;
            }
            
            case FileSystemManager::Native: {
                fs = new NativeFileSystem( this );
                
                if ( fs->mount( filePath ) ) {
                    mInstances[ filePath ] = fs;
                }
                else {
                    delete fs;
                    fs = 0;
                }
                
                break;
            }
            
            case FileSystemManager::NoChange:
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
        
        if ( fs->count() == 0 ) {
            fs->umount();
            fs->deleteLater();
            mInstances.remove( filePath );
        }
        else {
            return fs->count();
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
