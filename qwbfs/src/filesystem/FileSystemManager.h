#ifndef FILESYSTEMMANAGER_H
#define FILESYSTEMMANAGER_H

#include <QObject>
#include <QHash>

class AbstractFileSystem;

class FileSystemManager : public QObject
{
    Q_OBJECT
    
public:
    enum Type {
        NoChange = 0,
        WBFS = 1,
        Native = 2
    };
    
    FileSystemManager( QObject* parent = 0 );
    virtual ~FileSystemManager();
    
    AbstractFileSystem* open( const QString& filePath, FileSystemManager::Type type );
    int close( const QString& filePath );

protected:
    QHash<QString, AbstractFileSystem*> mInstances;
    
    QString resolvedFilePath( const QString& filePath ) const;
};

#endif // FILESYSTEMMANAGER_H
