#include "ListView.h"
#include "models/DiscModel.h"
#include "models/DiscDelegate.h"

ListView::ListView( QWidget* parent )
	: QListView( parent )
{
	mIconType = QWBFS::WiiTDB::Covers::Cover;
	mDriver = 0;
	mModel = 0;
	mDelegate = 0;
	
	QPalette pal = viewport()->palette();
	pal.setColor( viewport()->backgroundRole(), QColor( Qt::transparent ) );
	
	viewport()->setPalette( pal );
	viewport()->setAutoFillBackground( true );
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
	
	setModel( mModel );
	setItemDelegate( mDelegate );
}

void ListView::setViewMode( QListView::ViewMode mode )
{
	QListView::setViewMode( mode );
	setMovement( QListView::Static );
	setResizeMode( QListView::Adjust );
	
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
}

void ListView::setViewIconType( QWBFS::WiiTDB::Covers::Type type )
{
	mIconType = type;
	updateGeometries();
}

QWBFS::WiiTDB::Covers::Type ListView::viewIconType() const
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
