/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : DiscDelegate.cpp
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
#include "DiscDelegate.h"
#include "DiscModel.h"
#include "Gauge.h"
#include "qwbfsdriver/Driver.h"
#include "wiitdb/Covers.h"
#include "ListView.h"

#include <FreshCore/pNetworkAccessManager>
#include <FreshGui/pIconManager>
#include <FreshCore/pCoreUtils>

#include <QPainter>
#include <QDebug>

using namespace QWBFS::Model;

DiscDelegate::DiscDelegate( QWBFS::Model::DiscModel* parent, pNetworkAccessManager* cache )
	: QStyledItemDelegate( parent )
{
	Q_ASSERT( parent );
	Q_ASSERT( cache );
	mModel = parent;
	mCache = cache;
}

DiscDelegate::~DiscDelegate()
{
}

void DiscDelegate::paint( QPainter* painter, const QStyleOptionViewItem& _option, const QModelIndex& index ) const
{
	QStyleOptionViewItemV4 option = _option;
    initStyleOption( &option, index );
	
	// remove ugly focus rect
	option.state &= ~QStyle::State_HasFocus;
	
	if ( mModel ) {
		switch ( mModel->view()->viewMode() ) {
			case QListView::ListMode: {
				paintList( painter, option, index );
				break;
			}
			case QListView::IconMode: {
				paintIcon( painter, option, index );
				break;
			}
		}
	}
}

QSize DiscDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	if ( mModel ) {
		switch ( mModel->view()->viewMode() ) {
			case QListView::ListMode: {
				return mModel->data( index, DiscModel::ListModeSizeHintRole ).toSize();
			}
			case QListView::IconMode: {
				return mModel->data( index, DiscModel::IconModeSizeHintRole ).toSize();
			}
		}
	}
	
	return QStyledItemDelegate::sizeHint( option, index );
}

void DiscDelegate::paintFrame( QPainter* painter, const QStyleOptionViewItemV4& option, bool pair ) const
{
	const int corner = 5;
	const QRect r = option.rect.adjusted( 0, 0, -1, -1 );
	const QPalette::ColorRole role = pair ? QPalette::Highlight : QPalette::Button;
	QColor strokeColor = option.palette.color( role ).darker();
	QColor fillColor1 = option.palette.color( role ).lighter();
	QColor fillColor2 = option.palette.color( role );
	
	if ( option.state & QStyle::State_MouseOver ) {
		const int factor = 110;
		strokeColor = strokeColor.darker( factor );
		fillColor1 = fillColor1.darker( factor );
		fillColor2 = fillColor2.darker( factor );
	}
	
	QLinearGradient gradient( option.rect.topLeft(), option.rect.bottomLeft() );
	gradient.setColorAt( 0, fillColor1 );
	gradient.setColorAt( 1, fillColor2 );
	
	painter->setPen( strokeColor );
	painter->setBrush( gradient );
	painter->drawRoundedRect( r, corner, corner );
}

void DiscDelegate::paintList( QPainter* painter, const QStyleOptionViewItemV4& option, const QModelIndex& index ) const
{
	painter->setRenderHint( QPainter::Antialiasing );
	
	const bool selected = option.state & QStyle::State_Selected;
	const QWBFS::Model::Disc disc = mModel->disc( index );
	QRect rect;
	
	// background / selection
	{
		QStyleOptionViewItemV4 o = option;
		o.rect = option.rect.adjusted( 1, 1, 0, 0 );
		
		paintFrame( painter, o, selected );
	}
	
	// icon
	{
		rect = option.rect;
		rect = option.rect.adjusted( 8, 5, -rect.width() +40 -5, -5 );
		QPixmap pixmap = disc.state == QWBFS::Driver::None ? mModel->coverPixmap( disc.id, rect.size() ) : mModel->statePixmap( disc.state, rect.size() );
		
		if ( !pixmap.isNull() ) {
			painter->drawPixmap( rect.topLeft(), pixmap );
		}
	}
	
	// title/region
	{
		QFont font = painter->font();
		font.setPixelSize( 13 );
		font.setBold( true );
		
		QFontMetrics metrics( font );
		rect = option.rect.adjusted( 40, 2, -10, -( metrics.height() -2 ) );
		
		QString text = QString( "%1 - %2 (%3 - %4)" ).arg( disc.id ).arg( disc.title ).arg( QWBFS::Driver::regionToString( disc.region ) ).arg( QWBFS::Driver::regionToLanguageString( disc.region ) );
		text = metrics.elidedText( text, Qt::ElideRight, rect.width() );
		
		painter->setFont( font );
		painter->setPen( QColor( 0, 0, 0 ) );
		painter->setBrush( Qt::NoBrush );
		painter->drawText( rect, Qt::AlignLeft | Qt::AlignVCenter, text );
	}
	
	// size/origin
	{
		QFont font = painter->font();
		font.setBold( false );
		font.setPixelSize( 9 );
		
		QFontMetricsF metrics( font );
		rect = option.rect.adjusted( 40, rect.height(), -10, -2 );
		
		QString text = tr( "Estimated size: %1 - Origin: %2" ).arg( Gauge::fileSizeToString( disc.size ) ).arg( disc.origin );
		text = metrics.elidedText( text, Qt::ElideRight, rect.width() );
		
		painter->setFont( font );
		painter->setPen( QColor( 0, 0, 0 ) );
		painter->setBrush( Qt::NoBrush );
		painter->drawText( rect, Qt::AlignLeft | Qt::AlignVCenter, text );
	}
}

void DiscDelegate::paintIcon( QPainter* painter, const QStyleOptionViewItemV4& option, const QModelIndex& index ) const
{
	painter->setRenderHint( QPainter::Antialiasing );
	
	const bool selected = option.state & QStyle::State_Selected;
	const QWBFS::Model::Disc disc = mModel->disc( index );
	const int margin = 9;
	const int spacing = 5;
	const QString text = disc.title;
	QRect rect = option.rect.adjusted( margin, margin, -margin, -margin );
	QPixmap cover = mModel->coverPixmap( disc.id, rect.size() -QSize( 0, painter->font().pointSize() +spacing ) );
	QPixmap state = mModel->statePixmap( disc.state, QSize( 24, 24 ) );
	
	// selection
	if ( selected ) {
		QStyleOptionViewItemV4 o = option;
		o.rect = option.rect.adjusted( 1, 1, 0, 0 );
		
		paintFrame( painter, o );
	}
	
	// icon
	if ( !cover.isNull() ) {
		QRect r = QRect( QPoint(), cover.size() );
		r.moveCenter( rect.center() );
		r.moveTop( rect.top() );
		
		painter->drawPixmap( r.topLeft(), cover );
	}
	
	// state
	if ( !state.isNull() && disc.state != QWBFS::Driver::None ) {
		painter->drawPixmap( option.rect.topLeft() +QPoint( 2, 2 ), state );
	}
	
	// title
	{
		QStyleOptionViewItemV4 o = option;
		o.displayAlignment = Qt::AlignCenter;
		o.rect = option.rect.adjusted( 0, option.rect.height() -( margin *2 ), 0, 0 );
		o.icon = QIcon();
		o.text = painter->fontMetrics().elidedText( pCoreUtils::toTitleCase( text ), Qt::ElideRight, o.rect.width() -margin );
		
		if ( !selected ) {
			paintFrame( painter, o );
		}
		
		painter->setPen( selected ? o.palette.color( QPalette::HighlightedText ) : o.palette.color( QPalette::WindowText ) );
		painter->drawText( o.rect, o.displayAlignment, o.text );
	}
}
