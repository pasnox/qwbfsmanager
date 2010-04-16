#include "DiscDelegate.h"
#include "DiscModel.h"
#include "qwbfsdriver/Driver.h"
#include "Gauge.h"

#include <QPainter>
#include <QTextDocument>

using namespace QWBFS::Model;

DiscDelegate::DiscDelegate( QWBFS::Model::DiscModel* parent )
	: QStyledItemDelegate( parent )
{
	Q_ASSERT( parent );
	mModel = parent;
}

DiscDelegate::~DiscDelegate()
{
}

void DiscDelegate::paint( QPainter* painter, const QStyleOptionViewItem& _option, const QModelIndex& index ) const
{
	QStyleOptionViewItemV4 option = _option;
    initStyleOption( &option, index );
	
	painter->setRenderHint( QPainter::Antialiasing, true );
	
	QPainterPath path;
	path.addRoundedRect( option.rect.adjusted( 2, 2, -2, -2 ), 8, 8 );
	
	const bool selected = option.state & QStyle::State_Selected;
	const QWBFS::Model::Disc disc = mModel->disc( index );
	
	// selection
	if ( selected ) {
		painter->save();
		painter->setPen( QColor( 145, 147, 255, 130 ) );
		painter->setBrush( QColor( 184, 153, 255, 130 ) );
		painter->drawPath( path );
		painter->restore();
	}
	// background
	else {
		painter->save();
		painter->setPen( Qt::NoPen );
		painter->setBrush( QColor( 200, 200, 200, index.row() %2 == 0 ? 100 : 60 ) );
		painter->drawPath( path );
		painter->restore();
	}
	
	// text
	QStringList texts;
	texts << QString( "<b>%1 (%2)</b>" ).arg( disc.title ).arg( QWBFS::Driver::regionToString( QWBFS::Driver::Region( disc.region ) ) );
	
	if ( disc.id.isEmpty() ) {
		texts << QString( "<b>%1</b>" ).arg( Gauge::fileSizeToString( disc.size ) );
	}
	else {
		texts << QString( "%1 - <b>%2</b>" ).arg( disc.id ).arg( Gauge::fileSizeToString( disc.size ) );
	}
	
	QTextDocument document;
	
	document.setHtml( texts.join( "<br />" ) );
	
	painter->save();
	painter->translate( path.boundingRect().topLeft() );
	document.drawContents( painter );
	painter->restore();
}

QSize DiscDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	Q_UNUSED( option );
	Q_UNUSED( index );
	return QSize( -1, 37 );
}
