#include "ListView.h"
#include "models/DiscModel.h"
#include "models/DiscDelegate.h"

#include <QMouseEvent>
#include <QHeaderView>
#include <QPainter>
#include <QDebug>

ListView::ListView( QWidget* parent )
	: QListView( parent )
{
	mIconType = QWBFS::WiiTDB::Cover;
	mDriver = 0;
	mModel = 0;
	mDelegate = 0;
	mHeader = new QHeaderView( Qt::Horizontal, this );
	mCacheManager = 0;
	
	QPalette pal = viewport()->palette();
	pal.setColor( viewport()->backgroundRole(), pal.color( viewport()->backgroundRole() ).darker() );
	
	setMouseTracking( true );
	setViewportMargins( 0, HEADER_HEIGHT, 0, 0 );
	viewport()->setPalette( pal );
	viewport()->setAutoFillBackground( true );
	
	connect( mHeader, SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), this, SLOT( header_sortIndicatorChanged( int, Qt::SortOrder ) ) );
}

ListView::~ListView()
{
}

void ListView::initialize( QWBFS::Driver* driver, pNetworkAccessManager* manager )
{
	delete mModel;
	delete mDelegate;
	mDriver = driver;
	mModel = new QWBFS::Model::DiscModel( this, mDriver );
	mDelegate = new QWBFS::Model::DiscDelegate( mModel, manager );
	mCacheManager = manager;
	
	setModel( mModel );
	setItemDelegate( mDelegate );
	mHeader->setModel( mModel );
	
	mHeader->setSectionHidden( 0, true );
	mHeader->setSectionHidden( 5, true );
	
	for ( int i = 0; i < mModel->columnCount(); i++ ) {
		mHeader->setResizeMode( i, QHeaderView::ResizeToContents );
	}
	
	mHeader->setResizeMode( 2, QHeaderView::Stretch );
	mHeader->setClickable( true );
	mHeader->setSortIndicatorShown( true );
}

void ListView::setViewMode( QListView::ViewMode mode )
{
	const bool wasDragEnabled = dragEnabled();
	const bool wasAcceptDrops = acceptDrops();
	const bool wasViewportAcceptDrops = viewport()->acceptDrops();
	
	QListView::setViewMode( mode );
	setMovement( QListView::Static );
	setResizeMode( QListView::Adjust );
	setAlternatingRowColors( false );
	
	switch ( mode ) {
		case QListView::ListMode: {
			setFlow( QListView::TopToBottom );
			break;
		}
		case QListView::IconMode: {
			setFlow( QListView::LeftToRight );
			break;
		}
	}
	
	setDragEnabled( wasDragEnabled );
    setAcceptDrops( wasAcceptDrops );
	viewport()->setAcceptDrops( wasViewportAcceptDrops );
}

void ListView::setViewIconType( QWBFS::WiiTDB::Scan scan )
{
	mIconType = scan;
	updateGeometries();
}

QWBFS::WiiTDB::Scan ListView::viewIconType() const
{
	return mIconType;
}

QWBFS::Driver* ListView::driver() const
{
	return mDriver;
}

QWBFS::Model::DiscModel* ListView::model() const
{
	return mModel;
}

pNetworkAccessManager* ListView::cacheManager() const
{
	return mCacheManager;
}

void ListView::header_sortIndicatorChanged( int logicalIndex, Qt::SortOrder order )
{
	mModel->sort( logicalIndex, order );
	
	if ( selectionModel()->hasSelection() ) {
		scrollTo( selectionModel()->selectedIndexes().last() );
	}
}

void ListView::resizeEvent( QResizeEvent* event )
{
	QListView::resizeEvent( event );
	
	QRect rect = QRect( QPoint( frameWidth(), frameWidth() ), QSize( viewport()->width(), HEADER_HEIGHT ) );
	mHeader->setGeometry( rect );
}

void ListView::mousePressEvent( QMouseEvent* event )
{
	const QModelIndex index = indexAt( event->pos() );
	const bool isSelected = selectionModel()->isSelected( index );
	const bool leftButtonNoModifier = event->buttons() == Qt::LeftButton && event->modifiers() == Qt::NoModifier;
	
	QListView::mousePressEvent( event );
	
	if ( isSelected && leftButtonNoModifier && dragEnabled() ) {
		setState( QAbstractItemView::DraggingState );
	}
	else {
		setState( QAbstractItemView::DragSelectingState );
	}
}

void ListView::startDrag( Qt::DropActions supportedActions )
{
	QModelIndexList indexes = selectionModel()->selectedIndexes();
	
	if ( indexes.count() > 0 ) {
		QMimeData *data = model()->mimeData( indexes );
		
		if ( !data ) {
			return;
		}
		
		QRect rect;
		QPixmap pixmap = renderToPixmap( indexes, &rect );
		//rect.adjust( horizontalOffset(), verticalOffset(), 0, 0 ); // P@sNox: This cause a decal in the preview that cause the QDrag pixmap to not be visible.
		
		QDrag* drag = new QDrag( this );
		drag->setPixmap( pixmap );
		drag->setMimeData( data );
		drag->setHotSpot( viewport()->mapFromGlobal( QCursor::pos() ) -rect.topLeft() );
		
		Qt::DropAction defaultDropAction = Qt::IgnoreAction;
		
		if ( this->defaultDropAction() != Qt::IgnoreAction && (supportedActions & this->defaultDropAction() ) ) {
			defaultDropAction = this->defaultDropAction();
		}
		else if ( supportedActions & Qt::CopyAction && dragDropMode() != QAbstractItemView::InternalMove ) {
			defaultDropAction = Qt::CopyAction;
		}
		
		if ( drag->exec(supportedActions, defaultDropAction ) == Qt::MoveAction ) {
			clearOrRemove();
		}
	}
}

// dirty code get from QAbstractItemView for avoid drag & drop bug (no preview when dragging)

QItemViewPaintPairs ListView::draggablePaintPairs( const QModelIndexList& indexes, QRect* r ) const
{
	Q_ASSERT( r );
	QRect& rect = *r;
	const QRect viewportRect = viewport()->rect();
	QItemViewPaintPairs ret;
	
	for ( int i = 0; i < indexes.count(); ++i ) {
		const QModelIndex& index = indexes.at( i );
		const QRect current = visualRect( index );
		
		if ( current.intersects( viewportRect ) ) {
			ret += qMakePair( current, index );
			rect |= current;
		}
	}
	
	rect &= viewportRect;
	return ret;
}

QStyleOptionViewItemV4 ListView::viewOptionsV4() const
{
	QStyleOptionViewItemV4 option = viewOptions();
	
	if ( wordWrap() ) {
		option.features = QStyleOptionViewItemV2::WrapText;
	}
	
	option.locale = locale();
	option.locale.setNumberOptions( QLocale::OmitGroupSeparator );
	option.widget = this;
	
	return option;
}

QPixmap ListView::renderToPixmap( const QModelIndexList& indexes, QRect* r ) const
{
	Q_ASSERT( r );
	QItemViewPaintPairs paintPairs = draggablePaintPairs( indexes, r );
	
	if ( paintPairs.isEmpty() ) {
		return QPixmap();
	}
	
	QPixmap pixmap( r->size() );
	pixmap.fill( Qt::transparent );
	
	QPainter painter( &pixmap );
	QStyleOptionViewItemV4 option = viewOptionsV4();
	option.state |= QStyle::State_Selected;
	
	for ( int j = 0; j < paintPairs.count(); ++j ) {
		option.rect = paintPairs.at( j ).first.translated( -r->topLeft() );
		const QModelIndex& current = paintPairs.at( j ).second;
		
		itemDelegate( current )->paint( &painter, option, current );
	}
	
	return pixmap;
}

void ListView::clearOrRemove()
{
	const QItemSelection selection = selectionModel()->selection();
	QList<QItemSelectionRange>::const_iterator it = selection.constBegin();
	
	if ( !dragDropOverwriteMode() ) {
		for ( ; it != selection.constEnd(); ++it ) {
			QModelIndex parent = (*it).parent();
			
			if ( (*it).left() != 0 ) {
				continue;
			}
			
			if ( (*it).right() != ( model()->columnCount( parent ) -1 ) ) {
				continue;
			}
			
			int count = (*it).bottom() -(*it).top() +1;
			model()->removeRows( (*it).top(), count, parent );
		}
	} else {
		// we can't remove the rows so reset the items (i.e. the view is like a table)
		QModelIndexList list = selection.indexes();
		
		for ( int i = 0; i < list.size(); ++i ) {
			QModelIndex index = list.at( i );
			QMap<int, QVariant> roles = model()->itemData( index );
			
			for ( QMap<int, QVariant>::Iterator it = roles.begin(); it != roles.end(); ++it ) {
				it.value() = QVariant();
			}
			
			model()->setItemData( index, roles );
		}
	}
}
