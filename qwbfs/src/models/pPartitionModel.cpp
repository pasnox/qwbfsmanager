#include "pPartitionModel.h"

#include <FreshCore/pCoreUtils>

#include <QIcon>
#include <QProcess>
#include <QDebug>

#define COLUMN_COUNT 8

pPartitionModel::pPartitionModel( QObject* parent )
	: QAbstractTableModel( parent )
{
	update();
}

pPartitionModel::~pPartitionModel()
{
}

int pPartitionModel::columnCount( const QModelIndex& parent ) const
{
	return parent.isValid() ? 0 : COLUMN_COUNT;
}

QVariant pPartitionModel::data( const QModelIndex& index, int role ) const
{
	if ( !index.isValid() ||
		index.row() < 0 || index.row() >= rowCount( index.parent() ) ||
		index.column() < 0 || index.column() >= columnCount( index.parent() ) ) {
		return QVariant();
	}
	
	const pPartitionModel::Partition& partition = mPartitions[ index.row() ];
	
	switch ( role ) {
		case Qt::DecorationRole: {
			return index.column() == pPartitionModel::Icon ? partition.icon() : QVariant();
		}
		case Qt::DisplayRole:
		case Qt::ToolTipRole: {
			switch ( index.column() ) {
				case pPartitionModel::Icon:
					return QVariant();
				case pPartitionModel::Label:
					return partition.label.isEmpty() ? partition.origin : partition.label;
				case pPartitionModel::Origin:
					return partition.origin;
				case pPartitionModel::FileSystem:
					return partition.fileSystem;
				case pPartitionModel::Free:
					return pCoreUtils::fileSizeToString( partition.free );
				case pPartitionModel::Used:
					return pCoreUtils::fileSizeToString( partition.used );
				case pPartitionModel::Total:
					return pCoreUtils::fileSizeToString( partition.total );
				case pPartitionModel::LastCheck:
					return partition.lastCheck.toString( Qt::SystemLocaleShortDate );
			}
		}
	}
	
	return QVariant();
}

int pPartitionModel::rowCount( const QModelIndex& parent ) const
{
	return parent.isValid() ? 0 : mPartitions.count();
}

QVariant pPartitionModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if ( section >= 0 && section < columnCount() && orientation == Qt::Horizontal ) {
		switch ( role ) {
			case Qt::DisplayRole:
			case Qt::ToolTipRole: {
				switch ( section ) {
					case pPartitionModel::Icon:
						return tr( "Icon" );
					case pPartitionModel::Label:
						return tr( "Label" );
					case pPartitionModel::Origin:
						return tr( "Origin" );
					case pPartitionModel::FileSystem:
						return tr( "File System" );
					case pPartitionModel::Free:
						return tr( "Free" );
					case pPartitionModel::Used:
						return tr( "Used" );
					case pPartitionModel::Total:
						return tr( "Total" );
					case pPartitionModel::LastCheck:
						return tr( "Last check" );
				}
			}
		}
	}
	
	return QAbstractTableModel::headerData( section, orientation, role );
}

bool pPartitionModel::insertRow( int row, const QModelIndex& parent )
{
	const pPartitionModel::Partition partition( QString::null );
	const int index = mPartitions.indexOf( partition );
	
	if ( index == -1 && !parent.isValid() ) {
		beginInsertRows( parent, row, row );
		mPartitions << partition;
		endInsertRows();
		
		return true;
	}
	
	return false;
}

bool pPartitionModel::insertRows( int row, int count, const QModelIndex& parent )
{
	if ( count > 1 || parent.isValid() ) {
		return false;
	}
	
	return insertRow( row, parent );
}

bool pPartitionModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
	if ( !index.isValid() ||
		index.row() < 0 || index.row() >= rowCount( index.parent() ) ||
		index.column() < 0 || index.column() >= columnCount( index.parent() ) ) {
		return false;
	}
	
	switch ( role ) {
		case Qt::DisplayRole:
		case Qt::EditRole: {
			pPartitionModel::Partition& partition = mPartitions[ index.row() ];
			
			switch ( index.column() ) {
				case pPartitionModel::Icon:
					break;
				case pPartitionModel::Label:
					partition.label = value.toString();
					break;
				case pPartitionModel::Origin:
					partition.origin = value.toString();
					break;
				case pPartitionModel::FileSystem:
					partition.fileSystem = value.toString();
					break;
				case pPartitionModel::Free:
					partition.free = value.toLongLong();
					break;
				case pPartitionModel::Used:
					partition.used = value.toLongLong();
					break;
				case pPartitionModel::Total:
					partition.total = value.toLongLong();
					break;
				case pPartitionModel::LastCheck:
					partition.lastCheck = value.toDateTime();
					break;
			}
			
			if ( index.column() != pPartitionModel::LastCheck ) {
				partition.lastCheck = QDateTime::currentDateTime();
			}
			
			emit dataChanged( index.sibling( index.row(), 0 ), index.sibling( index.row(), columnCount( index.parent() ) ) );
			return true;
		}
	}
	
	return false;
}

pPartitionModel::Partition pPartitionModel::partition( const QModelIndex& index ) const
{
	return mPartitions.value( index.row() );
}

QStringList pPartitionModel::customPartitions() const
{
	const pPartitionModel::Partitions systemPartitions = partitions();
	QStringList custom;
	
	foreach ( const pPartitionModel::Partition& partition, mPartitions ) {
		if ( !systemPartitions.contains( partition ) ) {
			custom << partition.origin;
		}
	}
	
	return custom;
}

void pPartitionModel::updatePartition( const pPartitionModel::Partition& partition )
{
	const int id = mPartitions.indexOf( partition );
	
	if ( id != -1 ) {
		const QModelIndex index = this->index( id, 0, QModelIndex() );
		mPartitions[ id ] = partition;
		
		emit dataChanged( index.sibling( index.row(), 0 ), index.sibling( index.row(), columnCount( index.parent() ) ) );
	}
	else {
		beginInsertRows( QModelIndex(), mPartitions.count(), mPartitions.count() );
		mPartitions << partition;
		endInsertRows();
	}
}

void pPartitionModel::addPartitions( const QStringList& partitions )
{
	foreach ( const QString& partition, partitions ) {
		addPartition( partition );
	}
}

void pPartitionModel::addPartition( const QString& partition )
{
	updatePartition( pPartitionModel::Partition( partition ) );
}

void pPartitionModel::dump() const
{
	for ( int y = 0; y < rowCount(); y++ ) {
		QStringList entries;
		
		for ( int x = 0; x < columnCount(); x++ ) {
			entries << index( y, x ).data().toString();
		}
		
		qWarning() << entries.join( "\t" );
	}
}

void pPartitionModel::update()
{
	const QModelIndexList oldIndexes = persistentIndexList();
	pPartitionModel::Partitions partitions = this->partitions();
	QHash<int, int> mapping;
	QModelIndexList newIndexes;
	
	// manually added partitions
	for ( int i = 0; i < mPartitions.count(); i++ ) {
		const pPartitionModel::Partition& partition = mPartitions[ i ];
		
		if ( !partitions.contains( partition ) && QFile::exists( partition.origin ) ) {
			partitions << partition;
		}
	}
	
	// build mapping
	for ( int i = 0; i < mPartitions.count(); i++ ) {
		const pPartitionModel::Partition& partition = mPartitions[ i ];
		mapping[ i ] = partitions.indexOf( partition );
	}
	
	emit layoutAboutToBeChanged();
	
	foreach ( const QModelIndex& idx, oldIndexes ) {
		const int i = idx.row();
		
		newIndexes << index( mapping[ i ], idx.column(), idx.parent()  );
	}
	
	mPartitions = partitions;
	changePersistentIndexList( oldIndexes, newIndexes );
	
	emit layoutChanged();
	emit dataChanged( index( 0, 0 ), index( rowCount() -1, columnCount() -1 ) );
}

#if defined( Q_OS_MAC )
pPartitionModel::Partitions pPartitionModel::macPartitions() const
{
	pPartitionModel::Partitions partitions;
	QProcess process;
	
	process.start( "diskutil list" );
	process.waitForFinished();
	
	const QStringList entries = QString::fromLocal8Bit( process.readAll() ).split( "\n" );
	
	foreach ( QString entry, entries ) {
		entry = entry.trimmed();
		
		if ( entry.startsWith( "/" ) || entry.startsWith( "#" ) || entry.isEmpty() ) {
			continue;
		}
		
		entry = entry.simplified().section( ' ', -1 );
		
		// skip disks
		if ( entry[ entry.size() -2 ].toLower() != 's' ) {
			continue;
		}
		
		pPartitionModel::Partition partition;
		
		partition.label = QString::null;
		partition.origin = QString( "/dev/%1" ).arg( entry );
		partition.type = QString::null;
		partition.total = -1;
		partition.free = -1;
		partition.used = -1;
		partition.lastCheck = QDateTime::currentDateTime();
		
		partitions << partition;
	}
	
	return partitions;
}
#endif

pPartitionModel::Partitions pPartitionModel::partitions() const
{
#if defined( Q_OS_WIN )
	return windowsPartitions();
#elif defined( Q_OS_MAC )
	return macPartitions();
#elif defined( HAVE_UDEV )
	return udevPartitions();
#else
#endif
	
	pPartitionModel::Partitions partitions;
	
	// debug
	/*const QStringList types = QStringList( "EXT3" ) << "EXT4" << "FAT16" << "FAT32" << "ExFAT" << "NTFS" << "HFS" << "HFS+";
	
	for ( int i = 0; i < 10; i++ ) {
		pPartitionModel::Partition partition;
		
		partition.label = QString( "Partition %1" ).arg( i );
		partition.origin = QString( "/dev/sdz%1" ).arg( i );
		partition.type = types.at( qrand() %( types.count() -1 ) );
		partition.total = Q_INT64_C( 1024 *1024 *1024 *200 ); // 200gb
		partition.free = qrand() %( (qint64)partition.total -1 );
		partition.used = partition.total -partition.free;
		partition.lastCheck = QDateTime::currentDateTime();
		
		partitions << partition;
	}*/
	
	return partitions;
}
