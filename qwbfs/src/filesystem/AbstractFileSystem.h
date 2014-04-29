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
	virtual void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );
    
	QString mountPoint() const;
	bool isMounted() const;
	
    virtual bool open( const QString& mountPoint ) = 0;
    virtual bool close() = 0;
    virtual bool format() = 0;
    
    virtual QWBFS::FileSystemType type() const = 0;
    virtual QWBFS::EntryTypes supportedFormats() const = 0;
    virtual QWBFS::EntryType preferredFormat() const = 0;
    
    virtual FileSystemEntry::List entries() const = 0;
    virtual FileSystemEntry entry( int row ) const = 0;
    virtual FileSystemEntry entry( const QString& id ) const = 0;
    virtual bool hasEntry( const QString& id ) const = 0;
    
    virtual bool addEntry( const FileSystemEntry& entry, QWBFS::EntryType format = QWBFS::EntryTypeUnknown ) = 0;
    virtual bool removeEntry( const FileSystemEntry& entry ) = 0;
	virtual void clear() = 0;

protected:
	virtual void setEntriesInternal( const FileSystemEntry::List& entries ) = 0;
    int ref();
    int unref();
    int refCount() const;
    void dataChanged();

protected slots:
    void dataChangedTimeOut();

protected:
    int mRefCount;
    QString mMountPoint;
    QTimer* mChangeTimeOut;
	
	struct HashLessThanSorter {
		HashLessThanSorter( int column );
		
		bool operator()( const FileSystemEntry& left, const FileSystemEntry& right ) const;

		int c;
	};

	struct HashGreaterThanSorter {
		HashGreaterThanSorter( int column );
		
		bool operator()( const FileSystemEntry& left, const FileSystemEntry& right ) const;

		int c;
	};

signals:
    void changed();
    void progress( qint64 read, qint64 total );
};

#endif // ABSTRACTFILESYSTEM_H
