#include "DiscModel.h"
#include "Gauge.h"

#include <QMimeData>
#include <QUrl>
#include <QDebug>

#define URLS_FORMAT "text/uri-list"
#define WBFS_DISCS_FORMAT "xml/wbfs-discs"

// Sorter
bool SelectionRangePairLessThanSorter::operator()( const DiscModel::PairIntInt& left, const DiscModel::PairIntInt& right ) const
{
	return left.first < right.first;
}

bool SelectionRangePairGreaterThanSorter::operator()( const DiscModel::PairIntInt& left, const DiscModel::PairIntInt& right ) const
{
	return left.first > right.first;
}

// DiscModel

DiscModel::DiscModel( QObject* parent )
	: QAbstractItemModel( parent )
{
	mMimeTypes << URLS_FORMAT << WBFS_DISCS_FORMAT;
}

DiscModel::~DiscModel()
{
}

int DiscModel::columnCount( const QModelIndex& parent ) const
{
	return parent.isValid() ? 0 : 1;
}

QVariant DiscModel::data( const QModelIndex& index, int role ) const
{
	if ( !index.isValid() || index.row() < 0 || index.row() >= mDiscs.count() || index.column() != 0 ) {
		return QVariant();
	}
	
	const Disc disc = mDiscs.value( index.row() );
	
	switch ( role )
	{
		case Qt::DisplayRole:
			return disc.title;
		case Qt::ToolTipRole:
		{
			QStringList values;
			
			if ( !disc.id.isEmpty() ) {
				values << tr( "Id: %1" ).arg( disc.id );
			}
			
			values << tr( "Title: %1" ).arg( disc.title );
			values << tr( "Size: %1" ).arg( Gauge::fileSizeToString( disc.size ) );
			values << tr( "Origin: %1" ).arg( disc.origin );
			
			return values.join( "\n" );
			
			break;
		}
		default:
			break;
	}
	
	return QVariant();
}

QModelIndex DiscModel::index( int row, int column, const QModelIndex& parent ) const
{
	if ( parent.isValid() || row < 0 || row >= mDiscs.count() || column != 0 ) {
		return QModelIndex();
	}
	
	return createIndex( row, column, row );
}

QModelIndex DiscModel::parent( const QModelIndex& index ) const
{
	Q_UNUSED( index );
	return QModelIndex();
}

int DiscModel::rowCount( const QModelIndex& parent ) const
{
	return parent.isValid() ? 0 : mDiscs.count();
}

bool DiscModel::hasChildren( const QModelIndex& parent ) const
{
	return parent.isValid() ? false : !mDiscs.isEmpty();
}

Qt::ItemFlags DiscModel::flags( const QModelIndex& index ) const
{
	Qt::ItemFlags f = QAbstractItemModel::flags( index );
	
	if ( index.isValid() ) {
		f |= Qt::ItemIsDragEnabled;
	}
	else {
		f |= Qt::ItemIsDropEnabled;
	}
	
	return f;
}

bool DiscModel::removeRows( int row, int count, const QModelIndex& parent )
{
	count = qBound( count, count, mDiscs.count() -row ); // -1
	
	if ( parent.isValid() || row >= mDiscs.count() || count <= 0 ) {
		return false;
	}
	
	beginRemoveRows( QModelIndex(), row, row +count -1 );
	for ( int i = 0; i < count; i++ ) {
		mDiscs.removeAt( row );
	}
	endRemoveRows();
	
	emit countChanged( mDiscs.count() );
	
	return true;
}

bool DiscModel::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
	Q_UNUSED( action );
	Q_UNUSED( row );
	Q_UNUSED( column );
	Q_UNUSED( parent );
	
	QSet<Disc> discs;
	
	if ( data->formats().contains( WBFS_DISCS_FORMAT ) ) {
		discs = Disc::fromByteArray( data->data( WBFS_DISCS_FORMAT ) ).toSet();
	}
	else if ( data->formats().contains( URLS_FORMAT ) ) {
		foreach ( const QUrl& url, data->urls() ) {
			const QFileInfo file( url.toLocalFile() );
			
			if ( !file.isDir() && file.exists() ) {
				QString fileName = file.isSymLink() ? file.symLinkTarget() : file.absoluteFilePath();
				discs << Disc( QFileInfo( fileName ).canonicalFilePath() );
			}
		}
	}
	
	if ( !discs.isEmpty() ) {
		// remove clones
		foreach ( const Disc& disc, mDiscs ) {
			if ( discs.contains( disc ) ) {
				discs.remove( disc );
			}
		}
		
		if ( discs.isEmpty() ) {
			return false;
		}
		
		addDiscs( discs.toList() );
		return true;
	}
	
	return false;
}

QMimeData* DiscModel::mimeData( const QModelIndexList& indexes ) const
{
	if ( indexes.isEmpty() )
	{
		return 0;
	}
	
	DiscList discs;
	
	foreach ( const QModelIndex& index, indexes ) {
		discs << disc( index );
	}
	
	QMimeData* data = new QMimeData;
	data->setData( WBFS_DISCS_FORMAT, Disc::toByteArray( discs ) );
	
	return data;
}

QStringList DiscModel::mimeTypes() const
{
	return mMimeTypes;
}

void DiscModel::insertDiscs( int index, const DiscList& discs )
{
	index = qBound( 0, index, rowCount() );
	
	if ( discs.isEmpty() ) {
		return;
	}
	
	beginInsertRows( QModelIndex(), index, discs.count() -1 );
	for ( int i = 0; i < discs.count(); i++ ) {
		mDiscs.insert( i +index, discs.at( i ) );
	}
	endInsertRows();
	
	emit countChanged( mDiscs.count() );
}

void DiscModel::addDiscs( const DiscList& discs )
{
	insertDiscs( rowCount(), discs );
}

void DiscModel::setDiscs( const DiscList& discs )
{
	clear();
	addDiscs( discs );
}

DiscList DiscModel::discs() const
{
	return mDiscs;
}

qint64 DiscModel::size() const
{
	qint64 size = 0;
	
	foreach ( const Disc& disc, mDiscs ) {
		size += disc.size;
	}
	
	return size;
}

Disc DiscModel::disc( const QModelIndex& index ) const
{
	return mDiscs.value( index.row() );
}

void DiscModel::removeSelection( const QItemSelection& _selection )
{
	QList<DiscModel::PairIntInt> selection;
	
	// get pair selection
	foreach ( const QItemSelectionRange& range, _selection ) {
		selection << qMakePair( range.top(), range.height() );
	}
	
	// reverse order to remove last items first
	SelectionRangePairGreaterThanSorter sorter;
	qSort( selection.begin(), selection.end(), sorter );
	
	// remove items
	foreach ( const DiscModel::PairIntInt& pair, selection ) {
		removeRows( pair.first, pair.second, QModelIndex() );
	}
}

void DiscModel::clear()
{
	if ( mDiscs.isEmpty() ) {
		return;
	}
	
	beginRemoveRows( QModelIndex(), 0, mDiscs.count() -1 );
	mDiscs.clear();
	endRemoveRows();
	
	emit countChanged( mDiscs.count() );
}
