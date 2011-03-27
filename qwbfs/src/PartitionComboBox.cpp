/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : PartitionComboBox.cpp
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
#include "PartitionComboBox.h"
#include "models/PartitionDelegate.h"

#include "models/pPartitionModel.h"

#include <QPainter>
#include <QHelpEvent>
#include <QToolTip>
#include <QToolButton>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QDebug>

Q_GLOBAL_STATIC( pPartitionModel, _partitionModel );

pPartitionModel* PartitionComboBox::partitionModel()
{
	return _partitionModel();
}

PartitionComboBox::PartitionComboBox( QWidget* parent )
	: QComboBox( parent )
{
	mButton = new QToolButton( this );
	mButton->setAutoRaise( true );
	mButton->setIcon( QIcon( ":/icons/256/properties.png" ) );
	mButton->setToolTip( tr( "Add a partition" ) );
	
	QHBoxLayout* hl = new QHBoxLayout( this );
	hl->setMargin( 0 );
	hl->setSpacing( 5 );
	hl->addStretch();
	hl->addWidget( mButton );
	
	setAttribute( Qt::WA_Hover, true );
	setModel( partitionModel() );
	setModelColumn( pPartition::DevicePath );
	setItemDelegate( new PartitionDelegate( partitionModel() ) );
	
	connect( partitionModel(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ), this, SLOT( modelChanged() ) );
	connect( partitionModel(), SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ), this, SLOT( modelChanged() ) );
	connect( partitionModel(), SIGNAL( layoutChanged() ), this, SLOT( modelChanged() ) );
	connect( partitionModel(), SIGNAL( modelReset() ), this, SLOT( modelChanged() ) );
	connect( mButton, SIGNAL( clicked() ), this, SLOT( addPartition() ) );
	
	modelChanged();
}

PartitionComboBox::~PartitionComboBox()
{
}

void PartitionComboBox::modelChanged()
{
	bool custom = true;
	
	foreach ( const pPartition& partition, partitionModel()->partitions() ) {
		if ( !partition.isCustom() ) {
			custom = false;
			break;
		}
	}
	
	mButton->setVisible( custom );
}

void PartitionComboBox::addPartition()
{
	const QString partition = QInputDialog::getText( this, QString::null, tr( "Enter a partition path" ) );
	
	if ( !partition.isNull() && findText( partition ) == -1 ) {
		partitionModel()->addPartition( partition );
	}
	
	if ( !partition.isNull() ) {
		setCurrentIndex( findText( partition ) );
	}
}

bool PartitionComboBox::event( QEvent* event )
{
	if ( event->type() == QEvent::ToolTip ) {
		QHelpEvent* he = static_cast<QHelpEvent*>( event );
		QStringList tooltip( toolTip() );
		
		if ( currentIndex() != -1 ) {
			tooltip << QString( "<center><b>%1</b></center>" ).arg( tr( "Current Partition" ) );
			tooltip << itemData( currentIndex(), Qt::ToolTipRole ).toString();
		}
		
		QToolTip::showText( he->globalPos(), tooltip.join( "<br />" ), this );
		return true;
	}
	
	return QComboBox::event( event );
}

void PartitionComboBox::paintEvent( QPaintEvent* event )
{
	Q_UNUSED( event );
	QPainter painter( this );
	
	QStyleOptionComboBox option;
	initStyleOption( &option );
	option.rect.setWidth( layout()->itemAt( 0 )->geometry().width() -layout()->spacing() );
	
	if ( option.state & QStyle::State_MouseOver || currentIndex() == -1 ) {
		style()->drawComplexControl( QStyle::CC_ComboBox, &option, &painter, this );
		style()->drawControl( QStyle::CE_ComboBoxLabel, &option, &painter, this );
	}
	else {
		const QModelIndex index = partitionModel()->QAbstractTableModel::index( currentIndex(), modelColumn(), rootModelIndex() );
		
		QStyleOptionViewItemV4 o;
		o.initFrom( this );
		o.widget = this;
		o.rect = option.rect;
		
		itemDelegate()->paint( &painter, o, index );
	}
}
