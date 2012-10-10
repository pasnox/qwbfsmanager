#ifndef ABSTRACTFILESYSTEM_H
#define ABSTRACTFILESYSTEM_H

#include <QAbstractTableModel>

#include "FileSystemEntry.h"

class QTimer;

class FileSystemManager;

#define AbstractFileSystemColumnCount 6

class AbstractFileSystem : public QAbstractTableModel
{
    Q_OBJECT
    friend class FileSystemManager;
    
public:
    typedef QPair<int, int> PairIntInt;
	
	enum CustomRole {
		ListModeSizeHintRole = Qt::UserRole,
		IconModeSizeHintRole,
		CoverFlowModeSizeHintRole
	};
    
    AbstractFileSystem( FileSystemManager* manager );
    virtual ~AbstractFileSystem();
    
    virtual int columnCount( const QModelIndex& parent = QModelIndex() ) const;
    virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    
    
    
    QString filePath() const;
    
    virtual bool mount( const QString& filePath ) = 0;
    virtual bool umount() = 0;
	virtual bool isMounted() const = 0;
    virtual bool format() = 0;
    
    virtual QWBFS::FileSystemType type() const = 0;
    virtual QWBFS::EntryTypes supportedFormats() const = 0;
    virtual QWBFS::EntryType preferredFormat() const = 0;
    
    virtual FileSystemEntry::List entries() const = 0;
    virtual FileSystemEntry entry( int row ) const = 0;
    virtual FileSystemEntry entry( const QString& id ) const = 0;
    virtual bool hasEntry( const QString& id ) const = 0;
    
    virtual bool addEntry( const FileSystemEntry& entry, QWBFS::EntryType format = QWBFS::EntryTypeNone ) = 0;
    virtual bool removeEntry( const FileSystemEntry& entry ) = 0;

protected:
    int ref();
    int unref();
    int refCount() const;
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
