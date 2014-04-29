#ifndef FILESYSTEMMANAGER_H
#define FILESYSTEMMANAGER_H

#include <QObject>
#include <QHash>

#include "QWBFS.h"
#include "AbstractFileSystem.h"

class FileSystemManager : public QObject
{
    Q_OBJECT
    
public:
    FileSystemManager( QObject* parent = 0 );
    virtual ~FileSystemManager();
    
    AbstractFileSystem* open( const QString& filePath, QWBFS::FileSystemType type );
    int close( const QString& filePath );

protected:
    QHash<QString, AbstractFileSystem*> mInstances;
    
    QString resolvedFilePath( const QString& filePath ) const;
};

#endif // FILESYSTEMMANAGER_H
