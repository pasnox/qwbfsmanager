#include "pPartitionModel.h"

#include <FreshCore/pCoreUtils>

#include <QDebug>

pPartitionModel::pPartitionModel( QObject* parent )
	: QAbstractTableModel( parent )
{
	mData = 0;
	platformInit();
}

pPartitionModel::~pPartitionModel()
{
	platformDeInit();
}

int pPartitionModel::columnCount( const QModelIndex& parent ) const
{
	return parent.isValid() ? 0 : pPartition::LastProperty +1;
}

QVariant pPartitionModel::data( const QModelIndex& index, int role ) const
{
	if ( !index.isValid() ||
		index.row() < 0 || index.row() >= rowCount( index.parent() ) ||
		index.column() < 0 || index.column() >= columnCount( index.parent() ) ) {
		return QVariant();
	}
	
	const pPartition& partition = mPartitions[ index.row() ];
	
	switch ( role ) {
		case Qt::DecorationRole: {
			if ( index.column() == pPartition::Label ) {
			}
			
			break;
		}
		case Qt::DisplayRole: {
			switch ( index.column() ) {
				case pPartition::Label:
				case pPartition::DevicePath:
				case pPartition::FileSystem:
				case pPartition::MountPoints:
				case pPartition::DeviceVendor:
				case pPartition::DeviceModel:
				case pPartition::DisplayText:
					return partition.property( pPartition::Property( index.column() ) );
				case pPartition::FileSystemId:
					return QString( "%1" ).arg( partition.property( pPartition::Property( index.column() ) ).toLongLong(), 0, 16 ).toUpper().prepend( "0x" );
				case pPartition::TotalSize:
				case pPartition::UsedSize:
				case pPartition::FreeSize:
					return pCoreUtils::fileSizeToString( partition.property( pPartition::Property( index.column() ) ).toLongLong() );
				case pPartition::LastCheck:
					return partition.property( pPartition::LastCheck ).toDateTime().toString( Qt::SystemLocaleShortDate );
			}
			
			break;
		}
		case Qt::ToolTipRole: {
			QStringList tooltip;
			
			for ( int i = 0; i < columnCount(); i++ ) {
				tooltip << QString( "<b>%1:</b> %2" )
					.arg( headerData( i, Qt::Horizontal, Qt::DisplayRole ).toString() )
					.arg( data( index.sibling( index.row(), i ), Qt::DisplayRole ).toString() )
					;
			}
			
			return tooltip.join( "<br />" );
			break;
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
					case pPartition::Label:
						return tr( "Label" );
					case pPartition::DevicePath:
						return tr( "Path" );
					case pPartition::FileSystem:
						return tr( "File System" );
					case pPartition::MountPoints:
						return tr( "Mount Points" );
					case pPartition::DeviceVendor:
						return tr( "Vendor" );
					case pPartition::DeviceModel:
						return tr( "Model" );
					case pPartition::FileSystemId:
						return tr( "File System Id" );
					case pPartition::DisplayText:
						return tr( "Display Text" );
					case pPartition::TotalSize:
						return tr( "Size" );
					case pPartition::UsedSize:
						return tr( "Used" );
					case pPartition::FreeSize:
						return tr( "Free" );
					case pPartition::LastCheck:
						return tr( "Last Check" );
				}
			}
		}
	}
	
	return QAbstractTableModel::headerData( section, orientation, role );
}

bool pPartitionModel::insertRow( int row, const QModelIndex& parent )
{
	const pPartition partition( QString::null );
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

pPartition pPartitionModel::partition( const QModelIndex& index ) const
{
	return mPartitions.value( index.row() );
}

QModelIndex pPartitionModel::index( const pPartition& partition, int column ) const
{
	const int id = mPartitions.indexOf( partition );
	return QAbstractTableModel::index( id, column, QModelIndex() );
}

void pPartitionModel::addPartition( const QString& partition )
{
	updatePartition( pPartition( partition ) );
}

void pPartitionModel::addPartitions( const QStringList& partitions )
{
	foreach ( const QString& partition, partitions ) {
		addPartition( partition );
	}
}

void pPartitionModel::updatePartition( const pPartition& partition )
{
	if ( !partition.isValid() ) {
		return;
	}
	
	const int id = mPartitions.indexOf( partition );
	
	if ( id != -1 ) {
		const QModelIndex index = QAbstractTableModel::index( id, 0, QModelIndex() );
		mPartitions[ id ] = partition;
		
		emit dataChanged( index.sibling( index.row(), 0 ), index.sibling( index.row(), columnCount( index.parent() ) ) );
	}
	else {
		beginInsertRows( QModelIndex(), mPartitions.count(), mPartitions.count() );
		mPartitions << partition;
		endInsertRows();
	}
}

void pPartitionModel::removePartition( const QString& partition )
{
	const int id = mPartitions.indexOf( pPartition( partition, false ) );
	
	if ( id != -1 ) {
		beginRemoveRows( QModelIndex(), id, id );
		mPartitions.removeAt( id );
		endRemoveRows();
	}
}

pPartitionList pPartitionModel::partitions() const
{
	return mPartitions;
}

QStringList pPartitionModel::customPartitions() const
{
	QStringList custom;
	
	foreach ( const pPartition& partition, mPartitions ) {
		if ( partition.isCustom() ) {
			custom << partition.devicePath();
		}
	}
	
	return custom;
}

void pPartitionModel::dump() const
{
	for ( int y = 0; y < rowCount(); y++ ) {
		QStringList entries;
		
		for ( int x = 0; x < columnCount(); x++ ) {
			entries << QAbstractTableModel::index( y, x ).data().toString();
		}
		
		qWarning() << entries.join( "\t" );
	}
}

void pPartitionModel::update()
{
#ifdef Q_OS_MAC
	platformUpdate();
#else
	const QModelIndexList oldIndexes = persistentIndexList();
	pPartitionList partitions = this->partitions();
	QHash<int, int> mapping;
	QModelIndexList newIndexes;
	
	// manually added partitions
	for ( int i = 0; i < mPartitions.count(); i++ ) {
		const pPartition& partition = mPartitions[ i ];
		
		if ( !partitions.contains( partition ) && partition.isValid() ) {
			partitions << partition;
		}
	}
	
	// build mapping
	for ( int i = 0; i < mPartitions.count(); i++ ) {
		const pPartition& partition = mPartitions[ i ];
		mapping[ i ] = partitions.indexOf( partition );
	}
	
	emit layoutAboutToBeChanged();
	
	foreach ( const QModelIndex& idx, oldIndexes ) {
		const int i = idx.row();
		
		newIndexes << QAbstractTableModel::index( mapping[ i ], idx.column(), idx.parent()  );
	}
	
	mPartitions = partitions;
	changePersistentIndexList( oldIndexes, newIndexes );
	
	emit layoutChanged();
#endif
}
