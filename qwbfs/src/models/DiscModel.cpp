/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : DiscModel.cpp
** Date      : 2010-06-16T14:19:29
** License   : GPL2
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**
** In addition, as a special exception, the copyright holders give permission
** to link this program with the OpenSSL project's "OpenSSL" library (or with
** modified versions of it that use the same license as the "OpenSSL"
** library), and distribute the linked executables. You must obey the GNU
** General Public License in all respects for all of the code used other than
** "OpenSSL".  If you modify file(s), you may extend this exception to your
** version of the file(s), but you are not obligated to do so. If you do not
** wish to do so, delete this exception statement from your version.
**
****************************************************************************/
#include "DiscModel.h"
#include "qwbfsdriver/Driver.h"
#include "ListView.h"
#include "Gauge.h"

#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>

#define URLS_FORMAT "text/uri-list"
#define WBFS_DISCS_FORMAT "xml/wbfs-disc-list"

using namespace QWBFS::Model;

// Sorter

bool SelectionRangePairLessThanSorter::operator()( const DiscModel::PairIntInt& left, const DiscModel::PairIntInt& right ) const
{
	return left.first < right.first;
}

bool SelectionRangePairGreaterThanSorter::operator()( const DiscModel::PairIntInt& left, const DiscModel::PairIntInt& right ) const
{
	return left.first > right.first;
}

HashLessThanSorter::HashLessThanSorter( int column )
{
	c = column;
}

bool HashLessThanSorter::operator()( const QWBFS::Model::Disc& left, const QWBFS::Model::Disc& right ) const
{
	int compare = 0;
	
	switch ( c ) {
		case 0:
			compare = 0;
			break;
		case 1:
			compare = QString::compare( left.id, right.id, Qt::CaseInsensitive );
			break;
		case 2:
			compare = QString::compare( left.title, right.title, Qt::CaseInsensitive );
			break;
		case 3:
			compare = left.size == right.size ? 0 : ( left.size < right.size ? -1 : 1 );
			break;
		case 4:
			compare = QString::compare( QWBFS::Driver::regionToString( left.region ), QWBFS::Driver::regionToString( right.region ), Qt::CaseInsensitive );
			break;
		case 5:
			compare = QString::compare( left.origin, right.origin, Qt::CaseInsensitive );
			break;
	}
	
	return compare == 0 ? false : compare < 0;
}

HashGreaterThanSorter::HashGreaterThanSorter( int column )
{
	c = column;
}

bool HashGreaterThanSorter::operator()( const QWBFS::Model::Disc& left, const QWBFS::Model::Disc& right ) const
{
	int compare = 0;
	
	switch ( c ) {
		case 0:
			compare = 0;
			break;
		case 1:
			compare = QString::compare( left.id, right.id, Qt::CaseInsensitive );
			break;
		case 2:
			compare = QString::compare( left.title, right.title, Qt::CaseInsensitive );
			break;
		case 3:
			compare = left.size == right.size ? 0 : ( left.size < right.size ? -1 : 1 );
			break;
		case 4:
			compare = QString::compare( QWBFS::Driver::regionToString( left.region ), QWBFS::Driver::regionToString( right.region ), Qt::CaseInsensitive );
			break;
		case 5:
			compare = QString::compare( left.origin, right.origin, Qt::CaseInsensitive );
			break;
	}
	
	return compare == 0 ? false : compare > 0;
}

// DiscModel

QStringList DiscModel::mMimeTypes = QStringList() << URLS_FORMAT << WBFS_DISCS_FORMAT;

DiscModel::DiscModel( ListView* parent, QWBFS::Driver* driver )
	: QAbstractItemModel( parent )
{
	Q_ASSERT( parent );
	mListView = parent;
	mDriver = driver;
}

DiscModel::~DiscModel()
{
}

int DiscModel::columnCount( const QModelIndex& parent ) const
{
	return parent.isValid() ? 0 : DISC_MODEL_COLUMN_COUNT;
}

QVariant DiscModel::data( const QModelIndex& index, int role ) const
{
	if ( !index.isValid() || index.row() < 0 || index.row() >= mDiscs.count() || index.column() < 0 || index.column() >= DISC_MODEL_COLUMN_COUNT ) {
		return QVariant();
	}
	
	const QWBFS::Model::Disc disc = mDiscs.value( index.row() );
	
	switch ( role )
	{
		case Qt::DisplayRole: {
			switch ( index.column() ) {
				case 0:
					return QVariant();
				case 1:
					return disc.id;
				case 2:
					return disc.title;
				case 3:
					return Gauge::fileSizeToString( disc.size );
				case 4:
					return QWBFS::Driver::regionToString( disc.region );
				case 5:
					return disc.origin;
			}
		}
		case Qt::ToolTipRole:
		{
			QStringList values;
			
			if ( !disc.id.isEmpty() ) {
				values << tr( "Id: %1" ).arg( disc.id );
			}
			
			values << tr( "Title: %1" ).arg( disc.title );
			values << tr( "Size: %1" ).arg( Gauge::fileSizeToString( disc.size ) );
			values << tr( "Origin: %1" ).arg( disc.origin );
			values << tr( "Region: %1" ).arg( QWBFS::Driver::regionToString( disc.region ) );
			values << tr( "State: %1" ).arg( QWBFS::Driver::stateToString( QWBFS::Driver::State( disc.state ) ) );
			values << tr( "Error: %1" ).arg( QWBFS::Driver::errorToString( QWBFS::Driver::Error( disc.error ) ) );
			
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
	if ( parent.isValid() || row < 0 || row >= mDiscs.count() || column < 0 || column >= DISC_MODEL_COLUMN_COUNT ) {
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

bool DiscModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
	if ( !index.isValid() || index.row() < 0 || index.row() >= mDiscs.count() || index.column() < 0 || index.column() >= DISC_MODEL_COLUMN_COUNT ) {
		return false;
	}
	
	switch ( role )
	{
		case Qt::DisplayRole:
			switch ( index.column() ) {
				case 0:
					break;
				case 1:
					mDiscs[ index.row() ].id = value.toString();
					break;
				case 2:
					mDiscs[ index.row() ].title = value.toString();
					break;
				case 3:
					mDiscs[ index.row() ].size = value.toUInt();
					break;
				case 4:
					mDiscs[ index.row() ].region = value.toInt();
					break;
				case 5:
					mDiscs[ index.row() ].origin = value.toString();
					break;
			}
			
			break;
		default:
			return false;
	}
	
	emit dataChanged( index.sibling( index.row(), 0 ), index.sibling( index.row(), DISC_MODEL_COLUMN_COUNT -1 ) );
	return true;
}

QVariant DiscModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if ( orientation == Qt::Horizontal && section >= 0 && section < DISC_MODEL_COLUMN_COUNT ) {
		switch ( role ) {
			case Qt::DisplayRole: {
				switch ( section ) {
					case 0:
						return tr( "Image" );
					case 1:
						return tr( "Id" );
					case 2:
						return tr( "Title" );
					case 3:
						return tr( "Size" );
					case 4:
						return tr( "Region" );
					case 5:
						return tr( "Origin" );
				}
			}
		}
	}
	
	return QAbstractItemModel::headerData( section, orientation, role );
}

void DiscModel::sort( int column, Qt::SortOrder order )
{
	/*
	Call beginLayoutChanged()
	Remember the QModelIndex that will change
	Update your internal data
	Call changePersistentIndex()
	Call endLayoutChanged()
	*/
	
	emit layoutAboutToBeChanged();
	
	QModelIndexList oldIndexes = persistentIndexList();
	QWBFS::Model::DiscList newDiscs = mDiscs;
	QModelIndexList newIndexes;
	QHash<int, int> mapping; // old row, new row
	
	for ( int i = 0; i < mDiscs.count(); i++ ) {
		mapping[ i ] = i;
	}
	
	if ( order == Qt::AscendingOrder ) {
		HashLessThanSorter lesser( column );
		qSort( newDiscs.begin(), newDiscs.end(), lesser );
	}
	else {
		HashGreaterThanSorter greater( column );
		qSort( newDiscs.begin(), newDiscs.end(), greater );
	}
	
	for ( int i = 0; i < newDiscs.count(); i++ ) {
		mapping[ index( newDiscs[ i ] ).row() ] = i;
	}
	
	foreach ( const QModelIndex& index, oldIndexes ) {
		newIndexes << createIndex( mapping[ index.row() ], index.column(), mapping[ index.row() ] );
	}
	
	mDiscs = newDiscs;
	changePersistentIndexList( oldIndexes, newIndexes );
	
	emit layoutChanged();
}

Qt::DropActions DiscModel::supportedDropActions() const
{
	return Qt::CopyAction;
}

bool DiscModel::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
	Q_UNUSED( action );
	Q_UNUSED( row );
	Q_UNUSED( column );
	Q_UNUSED( parent );
	
	QSet<QWBFS::Model::Disc> discs;
	
	if ( data->formats().contains( WBFS_DISCS_FORMAT ) ) {
		discs = QWBFS::Model::Disc::fromByteArray( data->data( WBFS_DISCS_FORMAT ) ).toSet();
	}
	else if ( data->formats().contains( URLS_FORMAT ) ) {
		foreach ( const QUrl& url, data->urls() ) {
			const QFileInfo file( url.toLocalFile() );
			
			if ( !file.isDir() && file.exists() ) {
				const QString fileName = QFileInfo( file.isSymLink() ? file.symLinkTarget() : file.absoluteFilePath() ).canonicalFilePath();
				QWBFS::Model::Disc disc;
				const int result = mDriver->discImageInfo( fileName, disc );
				
				if ( result != Driver::Ok ) {
					continue;
				}
				
				discs << disc;
			}
		}
	}
	
	// remove clones
	foreach ( const QWBFS::Model::Disc& disc, mDiscs ) {
		if ( discs.contains( disc ) ) {
			discs.remove( disc );
		}
	}
	
	// remove drop from same view
	if ( mDriver ) {
		foreach ( const QWBFS::Model::Disc& disc, discs ) {
			if ( disc.origin == mDriver->partition() ) {
				discs.remove( disc );
			}
		}
	}
	
	if ( discs.isEmpty() ) {
		return false;
	}
	
	addDiscs( discs.toList() );
	return true;
}

QMimeData* DiscModel::mimeData( const QModelIndexList& indexes ) const
{
	if ( indexes.isEmpty() || !mDriver || ( mDriver && !mDriver->isOpen() ) ) {
		return 0;
	}
	
	QWBFS::Model::DiscList discs;
	
	foreach ( const QModelIndex& index, indexes ) {
		discs << disc( index );
	}
	
	QMimeData* data = new QMimeData;
	data->setData( WBFS_DISCS_FORMAT, QWBFS::Model::Disc::toByteArray( discs ) );
	
	return data;
}

QStringList DiscModel::mimeTypes() const
{
	return mMimeTypes;
}

ListView* DiscModel::view() const
{
	return mListView;
}

void DiscModel::insertDiscs( int index, const QWBFS::Model::DiscList& discs )
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

void DiscModel::addDiscs( const QWBFS::Model::DiscList& discs )
{
	insertDiscs( rowCount(), discs );
}

void DiscModel::setDiscs( const QWBFS::Model::DiscList& discs )
{
	clear();
	addDiscs( discs );
}

void DiscModel::setDisc( const QModelIndex& index, const QWBFS::Model::Disc& disc )
{
	if ( !index.isValid() || index.row() < 0 || index.row() >= mDiscs.count() || index.column() < 0 || index.column() >= DISC_MODEL_COLUMN_COUNT ) {
		return;
	}
	
	mDiscs[ index.row() ] = disc;
	emit dataChanged( index.sibling( index.row(), 0 ), index.sibling( index.row(), DISC_MODEL_COLUMN_COUNT -1 ) );
}

QWBFS::Model::DiscList DiscModel::discs() const
{
	return mDiscs;
}

QWBFS::Model::DiscList DiscModel::discs( const QModelIndexList& indexes )
{
	QWBFS::Model::DiscList discs;
	QSet<int> rows;
	
	foreach ( const QModelIndex& index, indexes ) {
		if ( !rows.contains( index.row() ) ) {
			rows << index.row();
			discs << disc( index );
		}
	}
	
	return discs;
}

QWBFS::Model::DiscList DiscModel::discs( const QItemSelection& selection )
{
	return discs( selection.indexes() );
}

QWBFS::Model::Disc DiscModel::disc( const QModelIndex& index ) const
{
	return mDiscs.value( index.row() );
}

QModelIndex DiscModel::index( const QWBFS::Model::Disc& disc, int column ) const
{
	const int index = mDiscs.indexOf( disc );
	return index == -1 ? QModelIndex() : createIndex( index, column, index );
}

QString DiscModel::discId( const QModelIndex& index ) const
{
	return disc( index ).id;
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

void DiscModel::updateDisc( const QWBFS::Model::Disc& disc )
{
	const int row = mDiscs.indexOf( disc );
	
	if ( row != -1 ) {
		const QModelIndex index = this->index( disc );
		mDiscs[ row ] = disc;
		emit dataChanged( index.sibling( index.row(), 0 ), index.sibling( index.row(), DISC_MODEL_COLUMN_COUNT -1 ) );
	}
}

qint64 DiscModel::size() const
{
	qint64 size = 0;
	
	foreach ( const QWBFS::Model::Disc& disc, mDiscs ) {
		size += disc.size;
	}
	
	return size;
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
