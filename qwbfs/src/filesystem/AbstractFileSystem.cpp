#include "AbstractFileSystem.h"
#include "FileSystemManager.h"

#include <FreshCore/pCoreUtils>

#include <QTimer>
#include <QSize>
#include <QDebug>

AbstractFileSystem::AbstractFileSystem( FileSystemManager* manager )
    : QAbstractTableModel( manager ),
        mRefCount( 0 ),
        mFilePath( QString::null ),
        mChangeTimeOut( new QTimer( this ) )
{
    mChangeTimeOut->setSingleShot( true );
    mChangeTimeOut->setInterval( 250 );
    connect( mChangeTimeOut, SIGNAL( timeout() ), this, SLOT( dataChangedTimeOut() ) );
    
    qWarning() << Q_FUNC_INFO << this;
}

AbstractFileSystem::~AbstractFileSystem()
{
    mChangeTimeOut->stop();
    
    qWarning() << Q_FUNC_INFO << this;
}

int AbstractFileSystem::columnCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : AbstractFileSystemColumnCount;
}

int AbstractFileSystem::rowCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : entries().count();
}

QVariant AbstractFileSystem::data( const QModelIndex& index, int role ) const
{
    switch ( role ) {
		case AbstractFileSystem::ListModeSizeHintRole:
			return QSize( -1, 37 );
		case AbstractFileSystem::IconModeSizeHintRole:
			return QSize( 120, 120 );
		case AbstractFileSystem::CoverFlowModeSizeHintRole:
			return QSize( 180, 240 );
	}
	
	if ( !index.isValid() || index.row() < 0 || index.row() >= entries().count() || index.column() < 0 || index.column() >= AbstractFileSystemColumnCount ) {
		return QVariant();
	}
	
	const FileSystemEntry entry = this->entry( index.row() );
	
	switch ( role ) {
		case Qt::DecorationRole: {
			/*if ( index.column() == 0 ) {
				return coverPixmap( entry.id(), data( index, AbstractFileSystem::CoverFlowModeSizeHintRole ).toSize() );
			}*/
			
			break;
		}
		case Qt::DisplayRole: {
			switch ( index.column() ) {
				case 0:
					return QVariant();
				case 1:
					return entry.id();
				case 2:
					return pCoreUtils::toTitleCase( entry.title() );
				case 3:
					return pCoreUtils::fileSizeToString( entry.size() );
				case 4:
					//return QWBFS::Driver::regionToString( entry.region() );
                    return QVariant();
				case 5:
					return entry.filePath();
			}
			
			break;
		}
		case Qt::ToolTipRole: {
			/*QStringList values;
			
			if ( !disc.id.isEmpty() ) {
				values << tr( "Id: %1" ).arg( disc.id );
			}
			
			values << tr( "Title: %1" ).arg( pCoreUtils::toTitleCase( disc.title ) );
			values << tr( "Size: %1" ).arg( pCoreUtils::fileSizeToString( disc.size ) );
			values << tr( "Origin: %1" ).arg( disc.origin );
			values << tr( "Region: %1" ).arg( QWBFS::Driver::regionToString( disc.region ) );
			values << tr( "State: %1" ).arg( QWBFS::Driver::stateToString( QWBFS::Driver::State( disc.state ) ) );
			values << tr( "Error: %1" ).arg( QWBFS::Driver::errorToString( QWBFS::Driver::Error( disc.error ) ) );
			
			return values.join( "\n" );*/
            break;
		}
	}
	
	return QVariant();
}

QVariant AbstractFileSystem::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal && section >= 0 && section < AbstractFileSystemColumnCount ) {
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
				
				break;
			}
			case Qt::ToolTipRole: {
				switch ( section ) {
					case 0:
						break; // nothing to sort
					case 1:
						return tr( "Sort by Id" );
					case 2:
						return tr( "Sort by Title" );
					case 3:
						return tr( "Sort by Size" );
					case 4:
						return tr( "Sort by Region" );
					case 5:
						break; // nothing to sort
				}
				
				break;
			}
		}
	}
	
	return QAbstractTableModel::headerData( section, orientation, role );
}

int AbstractFileSystem::ref()
{
    return ++mRefCount;
}

int AbstractFileSystem::unref()
{
    return --mRefCount;
}

int AbstractFileSystem::refCount() const
{
    return mRefCount;
}

QString AbstractFileSystem::filePath() const
{
    return mFilePath;
}

void AbstractFileSystem::dataChanged()
{
    mChangeTimeOut->start();
}

void AbstractFileSystem::dataChangedTimeOut()
{
    emit changed();
}
