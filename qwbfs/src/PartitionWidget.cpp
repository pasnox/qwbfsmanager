#include "PartitionWidget.h"
#include "qwbfsdriver/Driver.h"
#include "models/DiscModel.h"
#include "models/DiscDelegate.h"
#include "ProgressDialog.h"

#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

PartitionWidget::PartitionWidget( QWidget* parent )
	: QWidget( parent )
{
	mDriver = new QWBFS::Driver( this );
	mDiscModel = new QWBFS::Model::DiscModel( this );
	mImportModel = new QWBFS::Model::DiscModel( this );
	
	setupUi( this );
	lvDiscs->setModel( mDiscModel );
	lvDiscs->setItemDelegate( new QWBFS::Model::DiscDelegate( mDiscModel ) );
	lvImport->setModel( mImportModel );
	lvImport->setItemDelegate( new QWBFS::Model::DiscDelegate( mImportModel ) );
	
	connect( mDiscModel, SIGNAL( countChanged( int ) ), this, SLOT( models_countChanged() ) );
	connect( mImportModel, SIGNAL( countChanged( int ) ), this, SLOT( models_countChanged() ) );
	connect( cbPartitions->lineEdit(), SIGNAL( textChanged( const QString& ) ), this, SLOT( setCurrentPartition( const QString& ) ) );
	connect( cbPartitions->lineEdit(), SIGNAL( returnPressed() ), tbLoad, SLOT( click() ) );
}

PartitionWidget::~PartitionWidget()
{
	mDriver->close();
}

const QWBFS::Driver* PartitionWidget::driver() const
{
	return mDriver;
}

QWBFS::Model::DiscModel* PartitionWidget::discModel() const
{
	return mDiscModel;
}

QWBFS::Model::DiscModel* PartitionWidget::importModel() const
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
	mDriver->setPartition( partition );
	cbPartitions->setEditText( partition );
}

void PartitionWidget::showError( const QString& error )
{
	QMessageBox::information( this, QString::null, error );
}

void PartitionWidget::showError( int error )
{
	showError( QWBFS::Driver::errorToString( QWBFS::Driver::Error( error ) ) );
}
void PartitionWidget::models_countChanged()
{
	QWBFS::Partition::Status status;
	const int result = mDriver->status( status );
	
	gStatus->setSize( status.size );
	gStatus->setUsedSize( status.used );
	gStatus->setFreeSize( status.free );
	gStatus->setTemporarySize( mImportModel->size() );
	lInformations->setText( tr( "%1 disc(s) on the partition - %2 disc(s) to import." ).arg( mDiscModel->rowCount() ).arg( mImportModel->rowCount() ) );
	
	if ( result != QWBFS::Driver::Ok ) {
		showError( result );
	}
}

void PartitionWidget::on_cbPartitions_currentIndexChanged( int index )
{
	setCurrentPartition( cbPartitions->itemText( index ) );
}	

void PartitionWidget::on_tbLoad_clicked()
{
	mDiscModel->clear();
	
	if ( !mDriver->open() && !mDriver->partition().isEmpty() ) {
		showError( tr( "Can't open partition." ) );
	}
	
	QWBFS::Model::DiscList discs;
	const int result = mDriver->discList( discs );
	
	if ( result == QWBFS::Driver::Ok ) {
		mDiscModel->setDiscs( discs );
		
		if ( mDiscModel->rowCount() == 0 ) {
			models_countChanged();
		}
	}
	else {
		showError( result );
	}
}

void PartitionWidget::on_tbFormat_clicked()
{
	const QString text = tr( "The partition '%1' will be formatted,\nall data will be erased permanently, are you sure?" ).arg( mDriver->partition() );
	const QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No;
	const QMessageBox::StandardButton button = QMessageBox::No;
	
	if ( QMessageBox::question( this, QString::null, text, buttons, button ) == button ) {
		return;
	}
	
	if ( mDriver->format() ) {
		tbLoad->click();
	}
	else {
		showError( tr( "Can't format partition." ) );
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
	const QModelIndexList indexes = lvDiscs->selectionModel()->selectedIndexes();
	
	const QString text = tr( "You are about to permanently delete %1 disc(s) on partition '%2', are you sure?" ).arg( indexes.count() ).arg( mDriver->partition() );
	const QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No;
	const QMessageBox::StandardButton button = QMessageBox::No;
	
	if ( indexes.isEmpty() || QMessageBox::question( this, QString::null, text, buttons, button ) == button ) {
		return;
	}
	
	int errors = 0;
	
	for ( int i = indexes.count() -1; i >= 0; i-- ) {
		const QModelIndex& index = indexes[ i ];
		const QString discId = mDiscModel->discId( index );
		
		if ( mDriver->removeDisc( discId ) == QWBFS::Driver::Ok ) {
			mDiscModel->removeRow( index.row() );
		}
		else {
			errors++;
		}
	}
	
	if ( errors > 0 ) {
		showError( tr( "One or more discs have failed to be removed." ) );
	}
}

void PartitionWidget::on_tbRenameDisc_clicked()
{
	const QModelIndex index = lvDiscs->selectionModel()->selectedIndexes().value( 0 );
	const QWBFS::Model::Disc disc = mDiscModel->disc( index );
	
	if ( !index.isValid() ) {
		return;
	}
	
	const QString name = QInputDialog::getText( this, QString::null, tr( "Choose a new name for the disc" ), QLineEdit::Normal, disc.title );
	
	if ( mDriver->renameDisc( disc.id, name ) == QWBFS::Driver::Ok ) {
		mDiscModel->setData( index, name, Qt::DisplayRole );
	}
	else {
		showError( tr( "Can't rename disc id #%1 (%2) to '%3'" ).arg( disc.id ).arg( disc.title ).arg( name ) );
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
	dlg->importDiscs( mImportModel->discs(), mDriver->handle() );
}
