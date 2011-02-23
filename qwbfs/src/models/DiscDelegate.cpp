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

#include <QPainter>
#include <QPixmapCache>
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
				return QSize( -1, 37 );
			}
			case QListView::IconMode: {
				return QSize( 120, 120 );
			}
		}
	}
	
	return QStyledItemDelegate::sizeHint( option, index );
}

QPixmap DiscDelegate::coverPixmap( const QString& id, const QSize& size ) const
{
	const QWBFS::WiiTDB::Covers cover( id );
	QString url;
	QPixmap pixmap;
	
	if ( mModel->view()->viewIconType() == QWBFS::WiiTDB::Covers::Cover ) {
		url = cover.url( QWBFS::WiiTDB::Covers::Cover ).toString();
	}
	else {
		if ( mCache->hasCacheData( cover.url( QWBFS::WiiTDB::Covers::Disc ) ) ) {
			url = cover.url( QWBFS::WiiTDB::Covers::Disc ).toString();
		}
		else if ( mCache->hasCacheData( cover.url( QWBFS::WiiTDB::Covers::DiscCustom ) ) ) {
			url = cover.url( QWBFS::WiiTDB::Covers::DiscCustom ).toString();
		}
		else {
			url = cover.url( QWBFS::WiiTDB::Covers::Disc ).toString();
		}
	}
	
	const QString key = QString( "%1-%2-%3" ).arg( url ).arg( size.width() ).arg( size.height() );
	
	if ( !QPixmapCache::find( key, pixmap ) ) {
		if ( !mCache->hasCacheData( url ) ) {
			mCache->get( QNetworkRequest( url ) );
			return pixmap;
		}
		
		QIODevice* data = mCache->cacheData( url );
		
		if ( data && pixmap.loadFromData( data->readAll() ) ) {
			pixmap = pixmap.scaled( size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
			QPixmapCache::insert( key, pixmap );
		}
		
		delete data;
	}
	
	return pixmap;
}

QPixmap DiscDelegate::statePixmap( int state, const QSize& size ) const
{
	const QString url = state == QWBFS::Driver::Success ? ":/icons/256/success.png" : ":/icons/256/error.png";
	const QString key = QString( "%1-%2-%3" ).arg( url ).arg( size.width() ).arg( size.height() );
	QPixmap pixmap;
	
	if ( !QPixmapCache::find( key, pixmap ) ) {
		if ( pixmap.load( url ) ) {
			pixmap = pixmap.scaled( size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
			QPixmapCache::insert( key, pixmap );
		}
	}
	
	return pixmap;
}

void DiscDelegate::paintList( QPainter* painter, const QStyleOptionViewItemV4& option, const QModelIndex& index ) const
{
	painter->setRenderHint( QPainter::Antialiasing );
	
	QPainterPath path;
	path.addRoundedRect( option.rect.adjusted( 2, 2, -2, -2 ), 8, 8 );
	
	const bool selected = option.state & QStyle::State_Selected;
	const QWBFS::Model::Disc disc = mModel->disc( index );
	
	// selection
	if ( selected ) {
		painter->setPen( QColor( 145, 147, 255, 130 ) );
		painter->setBrush( QColor( 184, 153, 255, 130 ) );
		painter->drawPath( path );
	}
	// background
	else {
		painter->setPen( Qt::NoPen );
		painter->setBrush( QColor( 200, 200, 200, index.row() %2 == 0 ? 100 : 60 ) );
		painter->drawPath( path );
	}
	
	QRect rect;
	QString text;
	
	// title/region
	{
		QFont font = painter->font();
		font.setPixelSize( 13 );
		font.setBold( true );
		
		QFontMetrics metrics( font );
		rect = option.rect.adjusted( 40, 2, -10, -( metrics.height() -2 ) );
		
		text = QString( "%1 - %2 (%3 - %4)" ).arg( disc.id ).arg( disc.title ).arg( QWBFS::Driver::regionToString( disc.region ) ).arg( QWBFS::Driver::regionToLanguageString( disc.region ) );
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
		
		text = tr( "Estimated size: %1 - Origin: %2" ).arg( Gauge::fileSizeToString( disc.size ) ).arg( disc.origin );
		text = metrics.elidedText( text, Qt::ElideRight, rect.width() );
		
		painter->setFont( font );
		painter->setPen( QColor( 0, 0, 0 ) );
		painter->setBrush( Qt::NoBrush );
		painter->drawText( rect, Qt::AlignLeft | Qt::AlignVCenter, text );
	}
	
	rect = option.rect;
	rect = option.rect.adjusted( 8, 5, -rect.width() +40 -5, -5 );
	QPixmap pixmap = disc.state == QWBFS::Driver::None ? coverPixmap( disc.id, rect.size() ) : statePixmap( disc.state, rect.size() );
	
	// icon
	if ( !pixmap.isNull() ) {
		painter->drawPixmap( rect.topLeft(), pixmap );
	}
}

void DiscDelegate::paintIcon( QPainter* painter, const QStyleOptionViewItemV4& option, const QModelIndex& index ) const
{
	painter->setRenderHint( QPainter::Antialiasing );
	
	const bool selected = option.state & QStyle::State_Selected;
	const QWBFS::Model::Disc disc = mModel->disc( index );
	const int margin = 9;
	const int fontHeight = 12;
	const int spacing = 0;
	QRect rect = option.rect.adjusted( margin, margin, -margin, -margin );
	QPixmap cover = coverPixmap( disc.id, rect.size() -QSize( 0, fontHeight +spacing ) );
	QPixmap state = statePixmap( disc.state, QSize( 24, 24 ) );
	const QString text = disc.title;
	
	// selection
	if ( selected ) {
		QStyleOptionViewItemV4 opt = option;
		opt.rect = opt.rect.adjusted( 1, 1, -1, -1 );
		opt.icon = QIcon();
		opt.index = QModelIndex();
		opt.text = QString::null;
		
		QStyledItemDelegate::paint( painter, opt, QModelIndex() );
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
		QStyleOptionViewItemV4 opt = option;
		
		QFont font = painter->font();
		font.setPixelSize( fontHeight );
		font.setBold( true );
		
		QFontMetrics metrics( font );
		opt.text = metrics.elidedText( text, Qt::ElideRight, option.rect.width() );
		
		QRect r = option.rect.adjusted( 2, option.rect.height() -( margin *2 ), -2, -1 );
		
		opt.rect = r;
		opt.icon = QIcon();
		opt.font = font;
		opt.displayAlignment = Qt::AlignHCenter | Qt::AlignVCenter;
		
		if ( !selected ) {
			opt.state |= QStyle::State_Selected;
		}
		else {
			opt.state &= ~QStyle::State_Selected;
			opt.state &= ~QStyle::State_MouseOver;
		}
		
		QStyledItemDelegate::paint( painter, opt, QModelIndex() );
	}
}
