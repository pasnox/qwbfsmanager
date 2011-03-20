#include "PartitionComboBox.h"
#include "models/PartitionDelegate.h"

#include "models/pPartitionModel.h"

#include <QPainter>
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
	QToolButton* button = new QToolButton( this );
	button->setAutoRaise( true );
	button->setIcon( QIcon( ":/icons/256/properties.png" ) );
	button->setToolTip( tr( "Add a partition" ) );
	
	QHBoxLayout* hl = new QHBoxLayout( this );
	hl->setMargin( 0 );
	hl->setSpacing( 5 );
	hl->addStretch();
	hl->addWidget( button );
	
	setModel( partitionModel() );
	setModelColumn( pPartition::DevicePath );
	setItemDelegate( new PartitionDelegate( partitionModel() ) );
	
	connect( button, SIGNAL( clicked() ), this, SLOT( addPartition() ) );
	
	button->setVisible( partitionModel()->partitions().isEmpty() );
}

PartitionComboBox::~PartitionComboBox()
{
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
