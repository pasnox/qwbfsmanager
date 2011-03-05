/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : PartitionDelegate.cpp
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
#include "PartitionDelegate.h"
#include "qwbfsdriver/Driver.h"

#include <FreshCore/pCoreUtils>

#include <QPainter>
#include <QStyleFactory>
#include <QFileInfo>
#include <QDebug>

#define COMBOBOX_ITEM_HEIGHT 30

PartitionDelegate::PartitionDelegate( pPartitionModel* parent )
	: QStyledItemDelegate( parent )
{
	Q_ASSERT( parent );
	mModel = parent;
	mStyle = QStyleFactory::create( "plastique" );
}

PartitionDelegate::~PartitionDelegate()
{
}

void PartitionDelegate::paint( QPainter* painter, const QStyleOptionViewItem& _option, const QModelIndex& index ) const
{
	const bool selected = _option.state & QStyle::State_Selected;
	const bool hovered = _option.state & QStyle::State_MouseOver;
	const int margin = 3;
	pPartitionModel::Partition partition = mModel->partition( index );
	
	if ( QWBFS::Driver::isWBFSPartitionOrFile( partition.origin ) ) {
		// update wbfs partitions informations
		if ( partition.lastCheck < QDateTime::currentDateTime() && ( partition.used == -1 || partition.free == -1 ) ) {
			bool created = false;
			QWBFS::Partition::Handle handle = QWBFS::Driver::getHandle( partition.origin, &created );
			QWBFS::Driver driver( handle );
			QWBFS::Partition::Status status;
			
			driver.status( status );
			
			partition.type = "WBFS";
			partition.total = status.size;
			partition.used = status.used;
			partition.free = status.free;
			
			mModel->updatePartition( partition );
			
			if ( created ) {
				QWBFS::Driver::closeHandle( handle );
			}
		}
	}
	
	int total = 100;
	int used = ( (qreal)partition.used /(qreal)partition.total ) *(qreal)100;
	
	QStyleOptionViewItemV4 option = _option;
    initStyleOption( &option, index );
	option.palette = mStyle->standardPalette();
	
	QStyleOptionProgressBarV2 pbOption;
	pbOption.initFrom( option.widget );
	pbOption.state = option.state;
	pbOption.palette = option.palette;
	pbOption.rect = option.rect.adjusted( margin, margin, -margin, -margin );
	pbOption.bottomToTop = false;
	pbOption.invertedAppearance = false;
	pbOption.orientation = Qt::Horizontal;
	pbOption.maximum = total;
	pbOption.minimum = 0;
	pbOption.progress = used;
	pbOption.text = QString::null;
	pbOption.textAlignment = Qt::AlignCenter;
	pbOption.textVisible = false;
	
	pbOption.palette.setColor( QPalette::Highlight, QColor( 0, 160, 0, 100 ) );
	
	QStyleOptionButton bOption;
	bOption.initFrom( option.widget );
	bOption.state = option.state;
	bOption.palette = option.palette;
	bOption.rect = option.rect.adjusted( margin, margin, -margin, -margin );
	bOption.icon = partition.type == "WBFS" ? QIcon( ":/icons/256/wii.png" ) : partition.icon();
	bOption.iconSize = QSize( bOption.rect.height() -5, bOption.rect.height() -5 );
	bOption.text = partition.label.isEmpty() ? QFileInfo( partition.origin ).fileName() : partition.label;
	bOption.text = QString( "%1 (%2) - %3 / %4 Used - %5 Free" )
		.arg( bOption.text )
		.arg( partition.type.isEmpty() ? tr( "Unknown" ) : partition.type )
		.arg( pCoreUtils::fileSizeToString( partition.used ) )
		.arg( pCoreUtils::fileSizeToString( partition.total ) )
		.arg( pCoreUtils::fileSizeToString( partition.free ) )
		;
	
	if ( selected || hovered ) {
		paintFrame( painter, &option, selected );
	}
	
	mStyle->drawControl( QStyle::CE_ProgressBar, &pbOption, painter, option.widget );
	mStyle->drawControl( QStyle::CE_PushButtonLabel, &bOption, painter, option.widget );
}

QSize PartitionDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	Q_UNUSED( option );
	Q_UNUSED( index );
	return QSize( -1, COMBOBOX_ITEM_HEIGHT );
}

void PartitionDelegate::paintFrame( QPainter* painter, const QStyleOption* option, bool pair ) const
{
	const int corner = 5;
	const QRect r = option->rect.adjusted( 0, 0, -1, -1 );
	const QPalette::ColorRole role = pair ? QPalette::Highlight : QPalette::Button;
	QColor strokeColor = option->palette.color( role ).darker();
	QColor fillColor1 = option->palette.color( role ).lighter();
	QColor fillColor2 = option->palette.color( role );
	
	if ( option->state & QStyle::State_MouseOver ) {
		const int factor = 110;
		strokeColor = strokeColor.darker( factor );
		fillColor1 = fillColor1.darker( factor );
		fillColor2 = fillColor2.darker( factor );
	}
	
	QLinearGradient gradient( option->rect.topLeft(), option->rect.bottomLeft() );
	gradient.setColorAt( 0, fillColor1 );
	gradient.setColorAt( 1, fillColor2 );
	
	painter->setPen( strokeColor );
	painter->setBrush( gradient );
	painter->drawRoundedRect( r, corner, corner );
}

