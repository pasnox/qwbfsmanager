#include "PartitionWidget.h"
#include "qWBFS.h"
#include "DiscModel.h"
#include "DiscDelegate.h"
#include "ProgressDialog.h"

#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>
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
	
	connect( mDiscModel, SIGNAL( countChanged( int ) ), this, SLOT( models_countChanged() ) );
	connect( mImportModel, SIGNAL( countChanged( int ) ), this, SLOT( models_countChanged() ) );
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

QToolButton* PartitionWidget::showHideImportViewButton() const
{
	return tbShowHideImportView;
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

void PartitionWidget::models_countChanged()
{
	const QWBFS::Partition::Status status = mWBFS->partitionStatus();
	
	gStatus->setSize( status.size );
	gStatus->setUsedSize( status.used );
	gStatus->setFreeSize( status.free );
	gStatus->setTemporarySize( mImportModel->size() );
	lInformations->setText( tr( "%1 disc(s) on the partition - %2 disc(s) to import." ).arg( mDiscModel->rowCount() ).arg( mImportModel->rowCount() ) );
}

void PartitionWidget::on_cbPartitions_currentIndexChanged( int index )
{
	setCurrentPartition( cbPartitions->itemText( index ) );
}	

void PartitionWidget::on_tbLoad_clicked()
{
	mDiscModel->setDiscs( mWBFS->discs() );
	
	if ( mWBFS->lastError().isEmpty() ) {
		if ( mDiscModel->rowCount() == 0 ) {
			models_countChanged();
		}
	}
	else {
		QMessageBox::information( this, QString::null, mWBFS->lastError() );
	}
}

void PartitionWidget::on_tbFormat_clicked()
{
	const QString text = tr( "The partition '%1' will be formatted,\nall data will be erased permanently, are you sure?" ).arg( mWBFS->partition() );
	const QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No;
	const QMessageBox::StandardButton button = QMessageBox::No;
	
	if ( QMessageBox::question( this, QString::null, text, buttons, button ) == button ) {
		return;
	}
	
	if ( mWBFS->format() ) {
		models_countChanged();
	}
	else {
		QMessageBox::information( this, QString::null, tr( "Can't format the partition:\n%1" ).arg( mWBFS->lastError() ) );
	}
}

void PartitionWidget::on_tbOpen_clicked()
{
	emit openViewRequested();
}

void PartitionWidget::on_tbClose_clicked()
{
	emit closeViewRequested();
}

void PartitionWidget::on_tbRemoveDiscs_clicked()
{
}

void PartitionWidget::on_tbRenameDisc_clicked()
{
	const QModelIndex index = lvDiscs->selectionModel()->selectedIndexes().value( 0 );
	const Disc disc = mDiscModel->disc( index );
	
	if ( !index.isValid() ) {
		return;
	}
	
	const QString name = QInputDialog::getText( this, QString::null, tr( "Choose a new name for the disc" ), QLineEdit::Normal, disc.title );
	
	if ( mWBFS->renameDisc( disc.id, name ) ) {
		mDiscModel->setData( index, name, Qt::DisplayRole );
	}
	else {
		QMessageBox::information( this, QString::null, tr( "Can't rename disc id #%1 (%2) to '%3'" ).arg( disc.id ).arg( disc.title ).arg( name ) );
	}
}

void PartitionWidget::on_tbClearImport_clicked()
{
	mImportModel->clear();
}

void PartitionWidget::on_tbRemoveImport_clicked()
{
	mImportModel->removeSelection( lvImport->selectionModel()->selection() );
}

void PartitionWidget::on_tbImport_clicked()
{
	ProgressDialog* dlg = new ProgressDialog( this );
	dlg->importDiscs( mImportModel->discs(), mWBFS->partition() );
}
