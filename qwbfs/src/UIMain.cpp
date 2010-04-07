#include "UIMain.h"
#include "DiscModel.h"
#include "DiscDelegate.h"

#include <QFileSystemModel>
#include <QDebug>

UIMain::UIMain( QWidget* parent )
	: QMainWindow( parent )
{
	setupUi( this );
	
	mFoldersModel = new QFileSystemModel( this );
	mFoldersModel->setFilter( QDir::Dirs | QDir::NoDotAndDotDot );
	// OS X has buggy filesystemmodel - no mounted drives can be viewed, so let show hidden folders ( /Volumes ) so mount points are visible.
#ifdef Q_OS_MAC
	mFoldersModel->setFilter( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden );
#endif
	
	mFilesModel = new QFileSystemModel( this );
	mFilesModel->setFilter( QDir::Files );

	tvFolders->setModel( mFoldersModel );
	tvFolders->setColumnHidden( 1, true );
	tvFolders->setColumnHidden( 2, true );
	tvFolders->setColumnHidden( 3, true );
	
	lvFiles->setModel( mFilesModel );
	
	mExportModel = new DiscModel( this );
	lvExport->setModel( mExportModel );
	lvExport->setItemDelegate( new DiscDelegate( mExportModel ) );
	
	pwMainView->setMainView( true );
	pwMainView->importGroupBox()->setChecked( false );
	connectView( pwMainView );
	
#ifdef Q_OS_UNIX
	mFoldersModel->setRootPath( "/" );
#else
	mFoldersModel->setRootPath( "C:\\" );
#endif

	on_tvFolders_activated( mFoldersModel->index( 0, 0 ) );
	
	updatePartitions();
}

UIMain::~UIMain()
{
}

void UIMain::updatePartitions()
{
	mPartitions.clear();
	
	mPartitions << "/dev/sdg1";
	
	const QList<PartitionWidget*> widgets = sViews->findChildren<PartitionWidget*>();
	
	foreach ( PartitionWidget* widget, widgets ) {
		widget->setPartitions( mPartitions );
	}
}

void UIMain::connectView( PartitionWidget* widget )
{
	connect( widget, SIGNAL( openViewRequested() ), this, SLOT( openViewRequested() ) );
	connect( widget, SIGNAL( closeViewRequested() ), this, SLOT( closeViewRequested() ) );
}

void UIMain::openViewRequested()
{
	PartitionWidget* pw = new PartitionWidget( this );
	pw->setMainView( false );
	pw->setPartitions( mPartitions );
	pw->importGroupBox()->setChecked( false );
	connectView( pw );
	sViews->addWidget( pw );
}

void UIMain::closeViewRequested()
{
	sender()->deleteLater();
}

void UIMain::on_tvFolders_activated( const QModelIndex& index )
{
	const QString filePath = mFoldersModel->filePath( index );
	mFilesModel->setRootPath( filePath );
	lvFiles->setRootIndex( mFilesModel->index( filePath ) );
}

void UIMain::on_tbClearExport_clicked()
{
	mExportModel->clear();
}

void UIMain::on_tbRemoveExport_clicked()
{
	mExportModel->removeSelection( lvExport->selectionModel()->selection() );
}

void UIMain::on_tbExport_clicked()
{
	//
}
