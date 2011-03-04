#include "CoverFlowView.h"
#include "models/DiscModel.h"

#include <QScrollBar>
#include <QTimer>
#include <QHelpEvent>
#include <QToolTip>
#include <QWhatsThis>
#include <QPainter>
#include <QDebug>

#define COVERFLOWVIEW_ITEM_SIZE QSize( 180, 240 )

CoverFlowView::CoverFlowView( QWidget* parent )
	: PictureFlow( parent )
{
	mScrollBar = new QScrollBar( Qt::Horizontal, this );
	mModel = 0;
	mColumn = -1;
	mDisplayTextColumn = -1;
	mTimer = new QTimer( this );
	mTimer->setSingleShot( true );
	
	mScrollBar->setSingleStep( 1 );
	mScrollBar->setPageStep( 1 );
	
	setBackgroundColor( palette().color( backgroundRole() ).darker() );
	
	connect( mTimer, SIGNAL( timeout() ), this, SLOT( delayedResizeEvent() ) );
	connect( mScrollBar, SIGNAL( valueChanged( int ) ), this, SLOT( showSlide( int ) ) );
	connect( this, SIGNAL( centerIndexChanged( int ) ), this, SLOT( updateScrollBarState() ) );
}

CoverFlowView::~CoverFlowView()
{
}

bool CoverFlowView::event( QEvent* event )
{
	switch ( event->type() ) {
		case QEvent::ToolTip:
		case QEvent::WhatsThis: {
			const QHelpEvent* he = static_cast<QHelpEvent*>( event );
			const QString text = modelIndex( centerIndex(), mColumn ).data( Qt::ToolTipRole ).toString();
			
			if ( !text.isEmpty() ) {
				if ( event->type() == QEvent::ToolTip ) {
					QToolTip::showText( he->globalPos(), text, this );
				}
				else if ( event->type() == QEvent::ToolTip ) {
					QWhatsThis::showText( he->globalPos(), text, this );
				}
			}
			
			event->accept();
			return true;
		}
		default:
			break;
	}
	
	return PictureFlow::event( event );
}

void CoverFlowView::resizeEvent( QResizeEvent* event )
{
	PictureFlow::resizeEvent( event );
	
	const int sbMargin = 10;
	const int sbHeight = 16;
	QRect sbRect = QRect( QPoint( sbMargin, height() -sbHeight -sbMargin ), QSize( width() -( sbMargin *2 ), sbHeight ) );
	mScrollBar->setGeometry( sbRect );
	
	mTimer->start( 500 );
}

void CoverFlowView::paintEvent( QPaintEvent* event )
{
	PictureFlow::paintEvent( event );
	
	QPainter painter( this );
	
	const QString text = modelIndex( centerIndex(), mDisplayTextColumn ).data( Qt::DisplayRole ).toString();
	const int flags = Qt::AlignCenter | Qt::TextWordWrap;
	QRect rect = QRect( QPoint(), QSize( width(), 40 ) );
	
	rect.moveBottom( mScrollBar->pos().y() -5 );
	
	painter.drawText( rect, flags, text );
}

void CoverFlowView::_q_dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
	if ( topLeft.parent() != mRootIndex || ( mColumn < topLeft.column() && mColumn > bottomRight.column() ) ) {
		return;
	}
	
	const int start = topLeft.row();
	const int end = bottomRight.row();
	
	for ( int i = start; i < end +1; i++ ) {
		setSlide( i, pixmap( i ) );
	}
}

void CoverFlowView::_q_layoutChanged()
{
	initialize();
}

void CoverFlowView::_q_modelReset()
{
	initialize();
}

void CoverFlowView::_q_rowsInserted( const QModelIndex& parent, int start, int end )
{
	if ( parent == mRootIndex ) {
		for ( int i = start; i < end +1; i++ ) {
			insertSlide( i, pixmap( i ) );
		}
		
		setCenterIndex( start );
		updateScrollBarState();
	}
}

void CoverFlowView::_q_rowsMoved( const QModelIndex& sourceParent, int sourceStart, int sourceEnd, const QModelIndex& destinationParent, int destinationRow )
{
	Q_UNUSED( sourceParent );
	Q_UNUSED( sourceStart );
	Q_UNUSED( sourceEnd );
	Q_UNUSED( destinationParent );
	Q_UNUSED( destinationRow );
	qWarning() << Q_FUNC_INFO;
}

void CoverFlowView::_q_rowsRemoved( const QModelIndex& parent, int start, int end )
{
	if ( parent == mRootIndex ) {
		for ( int i = end; i >= start; i-- ) {
			removeSlide( i );
		}
		
		updateScrollBarState();
	}
}

void CoverFlowView::delayedResizeEvent()
{
	const qreal height = (qreal)this->height() *(qreal)0.50;
	const qreal factor = 180.0 /240.0;
	QSize size = QSize( height *factor, height );
	
	setSlideSize( size );
	initialize( centerIndex() );
}

void CoverFlowView::initialize( int index )
{
	clear();
	
	if ( mModel ) {
		for ( int i = 0; i < mModel->rowCount(); i++ ) {
			addSlide( pixmap( i ) );
		}
	}
	
	setCenterIndex( index );
	updateScrollBarState();
}

void CoverFlowView::updateScrollBarState()
{
	mScrollBar->setMinimum( 0 );
	mScrollBar->setMaximum( mModel ? mModel->rowCount() -1 : 0 );
	
	if ( animatorStep() == 0 ) {
		mScrollBar->setValue( centerIndex() );
	}
	
	emit centerIndexChanged( modelIndex( centerIndex() ) );
}

void CoverFlowView::preload()
{
	if ( !mModel ) {
		return;
	}
	
	// try to load only few images on the left and right side 
	// i.e. all visible ones plus some extra
	#define COUNT 10  
	
	int indexes[ 2 *COUNT +1 ] ;
	int center = centerIndex();
	indexes[ 0 ] = center;
	
	for ( int j = 0; j < COUNT; j++ ) {
		indexes[ j *2 +1 ] = center +j +1;
		indexes[ j *2 +2 ] = center -j -1;
	}  

	for ( int c = 0; c < 2 *COUNT +1; c++ ) {
		int i = indexes[ c ];
		
		if ( ( i >= 0 ) && ( i < slideCount() ) ) {
			if ( slide( i ).isNull() ) {
				// schedule thumbnail loading
				Q_UNUSED( pixmap( i ) );
			}
		}
	}
}

void CoverFlowView::setModel( QAbstractItemModel* model )
{
	if ( mModel == model ) {
		return;
	}
	
	if ( mModel ) {
		disconnect( this, SIGNAL( centerIndexChanged( int ) ), this, SLOT( preload() ) );
		
		disconnect( mModel, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( _q_dataChanged( const QModelIndex&, const QModelIndex& ) ) );
		disconnect( mModel, SIGNAL( layoutChanged() ), this, SLOT( _q_layoutChanged() ) );
		disconnect( mModel, SIGNAL( modelReset() ), this, SLOT( _q_modelReset() ) );
		disconnect( mModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ), this, SLOT( _q_rowsInserted( const QModelIndex&, int, int ) ) );
		disconnect( mModel, SIGNAL( rowsMoved( const QModelIndex&, int, int, const QModelIndex&, int ) ), this, SLOT( _q_rowsMoved( const QModelIndex&, int, int, const QModelIndex&, int ) ) );
		disconnect( mModel, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ), this, SLOT( _q_rowsRemoved( const QModelIndex&, int, int ) ) );
		
		mModel = 0;
	}
	
	mModel = model;
	mColumn = -1;
	mDisplayTextColumn = -1;
	
	if ( mModel ) {
		connect( this, SIGNAL( centerIndexChanged( int ) ), this, SLOT( preload() ) );
		
		connect( mModel, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( _q_dataChanged( const QModelIndex&, const QModelIndex& ) ) );
		connect( mModel, SIGNAL( layoutChanged() ), this, SLOT( _q_layoutChanged() ) );
		connect( mModel, SIGNAL( modelReset() ), this, SLOT( _q_modelReset() ) );
		connect( mModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ), this, SLOT( _q_rowsInserted( const QModelIndex&, int, int ) ) );
		connect( mModel, SIGNAL( rowsMoved( const QModelIndex&, int, int, const QModelIndex&, int ) ), this, SLOT( _q_rowsMoved( const QModelIndex&, int, int, const QModelIndex&, int ) ) );
		connect( mModel, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ), this, SLOT( _q_rowsRemoved( const QModelIndex&, int, int ) ) );
	}
}

QAbstractItemModel* CoverFlowView::model() const
{
	return mModel;
}

void CoverFlowView::setColumn( int column )
{
	if ( !mModel ) {
		return;
	}
	
	const int min = mModel->columnCount() > 0 ? 0 : -1;
	const int max = mModel->columnCount() > 0 ? mModel->columnCount() : -1;
	
	mColumn = qBound( min, column, max );
	
	initialize();
}

int CoverFlowView::column() const
{
	return mColumn;
}

void CoverFlowView::setDisplayTextColumn( int column )
{
	if ( !mModel ) {
		return;
	}
	
	const int min = mModel->columnCount() > 0 ? 0 : -1;
	const int max = mModel->columnCount() > 0 ? mModel->columnCount() : -1;
	
	mDisplayTextColumn = qBound( min, column, max );
}

int CoverFlowView::displayTextColumn() const
{
	return mDisplayTextColumn;
}

QModelIndex CoverFlowView::modelIndex( int index, int column ) const
{
	return mModel ? mModel->index( index, column == -1 ? mColumn : column, mRootIndex ) : QModelIndex();
}

QPixmap CoverFlowView::pixmap( int _index ) const
{
	const QModelIndex index = modelIndex( _index );
	const QWBFS::Model::DiscModel* model = qobject_cast<QWBFS::Model::DiscModel*>( mModel );
	const QWBFS::Model::Disc disc = model ? model->disc( index ) : QWBFS::Model::Disc();
	return model ? model->coverPixmap( disc.id, slideSize(), true ) : QPixmap();
}
