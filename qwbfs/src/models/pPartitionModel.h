#ifndef PPARTITIONMODEL_H
#define PPARTITIONMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>
#include <QIcon>

class pPartitionModel : public QAbstractTableModel
{
	Q_OBJECT
	friend class PartitionComboBox;
	
public:
	struct Partition {
		enum Type {
			Unknown = 0,
			NotMounted = 1,
			Removable = 2,
			Fixed = 3,
			Remote = 4,
			CdRom = 5,
			RamDisk = 6
		};
		
		Partition( const QString& _origin = QString::null )
		{
			origin = _origin;
			free = -1;
			used = -1;
			total = -1;
			type = Unknown;
			fileSystemMark = 0x0;
		}
		
		bool operator==( const Partition& other ) const
		{
			return origin == other.origin;
		}
		
		QIcon icon() const
		{
			if ( fileSystem.contains( "swap", Qt::CaseInsensitive ) || fileSystem.contains( "linux", Qt::CaseInsensitive ) ) {
				return QIcon( ":/icons/256/linux.png" );
			}
			else if ( fileSystem.contains( "hfs", Qt::CaseInsensitive ) || fileSystem.contains( "hpfs", Qt::CaseInsensitive ) ) {
				return QIcon( ":/icons/256/mac.png" );
			}
			else if ( fileSystem.contains( "ntfs", Qt::CaseInsensitive ) || fileSystem.contains( "fat", Qt::CaseInsensitive ) ) {
				return QIcon( ":/icons/256/windows.png" );
			}
			else {
				return QIcon( ":/icons/256/hdd.png" );
			}
		}
		
		QString name;
		QString label;
		QString origin;
		QString model;
		qint64 free;
		qint64 used;
		qint64 total;
		QDateTime lastCheck;
		
		Type type;
		QString fileSystem;
		qint64 fileSystemMark;
		QMap<QString, QString> extendedAttributes;
	};
	
	enum Column {
		Icon = 0,
		Label,
		Origin,
		FileSystem,
		Free,
		Used,
		Total,
		LastCheck
	};
	
	typedef QList<Partition> Partitions;
	
	pPartitionModel( QObject* parent = 0 );
	virtual ~pPartitionModel();
	
	virtual int columnCount( const QModelIndex& parent = QModelIndex() ) const;
	virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
	virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;
	
	virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
	virtual bool insertRow( int row, const QModelIndex& parent = QModelIndex() );
	virtual bool insertRows( int row, int count, const QModelIndex& parent = QModelIndex() );
	virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
	//virtual void	sort ( int column, Qt::SortOrder order = Qt::AscendingOrder )
	
	pPartitionModel::Partition partition( const QModelIndex& index ) const;
	QStringList customPartitions() const;
	void updatePartition( const pPartitionModel::Partition& partition );
	void addPartitions( const QStringList& partitions );
	void addPartition( const QString& partition );
	
	void dump() const;

public slots:
	void update();

protected:
	pPartitionModel::Partitions mPartitions;
	
#if defined( Q_OS_WIN )
	pPartitionModel::Partitions windowsPartitions() const;
#elif defined( Q_OS_MAC )
	pPartitionModel::Partitions macPartitions() const;
#elif defined( HAVE_UDEV )
	pPartitionModel::Partitions udevPartitions() const;
#endif
	
	pPartitionModel::Partitions partitions() const;
};

#endif // PPARTITIONMODEL_H
