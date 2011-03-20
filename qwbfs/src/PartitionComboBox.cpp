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
	bool custom = false;
	
	foreach ( const pPartition& partition, partitionModel()->partitions() ) {
		if ( partition.isCustom() ) {
			custom = true;
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
