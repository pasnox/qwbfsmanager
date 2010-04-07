#include "PartitionWidget.h"

#include "qWBFS.h"
#include "DiscModel.h"
#include "DiscDelegate.h"

#include <QLineEdit>
#include <QDebug>

PartitionWidget::PartitionWidget( QWidget* parent )
	: QWidget( parent )
{
	mWBFS = new qWBFS( this );
	mDiscModel = new DiscModel( this );
	mImportModel = new DiscModel( this );
	
	setupUi( this );
	lvDiscs->setModel( mDiscModel );
	lvDiscs->setItemDelegate( new DiscDelegate( mDiscModel ) );
	lvImport->setModel( mImportModel );
	lvImport->setItemDelegate( new DiscDelegate( mImportModel ) );
	
	connect( cbPartitions->lineEdit(), SIGNAL( textChanged( const QString& ) ), this, SLOT( setCurrentPartition( const QString& ) ) );
	connect( cbPartitions->lineEdit(), SIGNAL( returnPressed() ), tbLoad, SLOT( click() ) );
}

PartitionWidget::~PartitionWidget()
{
}

const qWBFS* PartitionWidget::handle() const
{
	return mWBFS;
}

DiscModel* PartitionWidget::discModel() const
{
	return mDiscModel;
}

DiscModel* PartitionWidget::importModel() const
{
	return mImportModel;
}

QGroupBox* PartitionWidget::importGroupBox() const
{
	return gbImport;
}

QString PartitionWidget::currentPartition() const
{
	return cbPartitions->currentText();
}

void PartitionWidget::setMainView( bool main )
{
	tbOpen->setVisible( main );
	tbClose->setVisible( !main );
}

void PartitionWidget::setPartitions( const QStringList& partitions )
{
	const QString current = currentPartition();

	const bool locked = cbPartitions->blockSignals( true );
	cbPartitions->clear();
	cbPartitions->addItems( partitions );
	cbPartitions->setEditText( current );
	cbPartitions->blockSignals( locked );
}

void PartitionWidget::setCurrentPartition( const QString& partition )
{
	mWBFS->setPartition( partition );
	cbPartitions->setEditText( partition );
}

void PartitionWidget::on_cbPartitions_currentIndexChanged( int index )
{
	setCurrentPartition( cbPartitions->itemText( index ) );
}

void PartitionWidget::on_tbLoad_clicked()
{
	const DiscList discs = mWBFS->discs();
	const qWBFS::PartitionStatus status = mWBFS->partitionStatus();
	
	mDiscModel->setDiscs( discs );
	gStatus->setSize( status.size );
	gStatus->setUsedSize( status.used );
	gStatus->setFreeSize( status.free );
	lInformations->setText( tr( "%1 games" ).arg( discs.count() ) );
}

void PartitionWidget::on_tbOpen_clicked()
{
	emit openViewRequested();
}

void PartitionWidget::on_tbClose_clicked()
{
	emit closeViewRequested();
}

void PartitionWidget::on_tbClear_clicked()
{
	mImportModel->clear();
}

void PartitionWidget::on_tbRemove_clicked()
{
	mImportModel->removeSelection( lvImport->selectionModel()->selection() );
}

void PartitionWidget::on_tbImport_clicked()
{
	//
}
