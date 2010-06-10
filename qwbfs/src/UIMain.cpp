/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : UIMain.cpp
** Date      : 2010-04-25T13:05:33
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a crossplatform WBFS Manager developed using Qt4/C++.
** It's currently working under Unix/Linux, Mac OS X, and build under windows (but not yet working).
** 
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR WII CONSOLE OR WII PARTITION
** BECAUSE OF IMPROPER USAGE OF THIS SOFTWARE.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "UIMain.h"
#include "UIAbout.h"
#include "models/DiscModel.h"
#include "models/DiscDelegate.h"
#include "wiitdb/Covers.h"
#include "datacache/DataNetworkCache.h"
#include "ProgressDialog.h"
#include "PropertiesDialog.h"
#include "Properties.h"

#include <QFileSystemModel>
#include <QFileDialog>
#include <QProcess>
#include <QDebug>

UIMain::UIMain( QWidget* parent )
	: QMainWindow( parent )
{
	setUnifiedTitleAndToolBarOnMac( true );
	setupUi( this );
	
	mFoldersModel = new QFileSystemModel( this );
	mFoldersModel->setFilter( QDir::Dirs | QDir::NoDotAndDotDot );
	
	mFilesModel = new QFileSystemModel( this );
	mFilesModel->setFilter( QDir::Files );

	tvFolders->setModel( mFoldersModel );
	tvFolders->setColumnHidden( 1, true );
	tvFolders->setColumnHidden( 2, true );
	tvFolders->setColumnHidden( 3, true );
	
	lvFiles->setModel( mFilesModel );
	
	mExportModel = new QWBFS::Model::DiscModel( this );
	lvExport->setModel( mExportModel );
	lvExport->setItemDelegate( new QWBFS::Model::DiscDelegate( mExportModel ) );
	
	mCache = new DataNetworkCache( this );
	
	mLastDiscId = -1;
	
	pwMainView->setMainView( true );
	pwMainView->showHideImportViewButton()->setChecked( false );
	connectView( pwMainView );
		tbReloadDrives->click();
	aReloadPartitions->trigger();
	
	connect( mCache, SIGNAL( dataCached( const QUrl& ) ), this, SLOT( dataNetworkCache_dataCached( const QUrl& ) ) );
	connect( mCache, SIGNAL( error( const QString& ) ), this, SLOT( dataNetworkCache_error( const QString& ) ) );
	connect( mCache, SIGNAL( invalidated() ), this, SLOT( dataNetworkCache_invalidated() ) );
}

UIMain::~UIMain()
{
	//qWarning() << Q_FUNC_INFO;
}

void UIMain::connectView( PartitionWidget* widget )
{
	connect( widget, SIGNAL( openViewRequested() ), this, SLOT( openViewRequested() ) );
	connect( widget, SIGNAL( closeViewRequested() ), this, SLOT( closeViewRequested() ) );
	connect( widget, SIGNAL( coverRequested( const QString& ) ), this, SLOT( coverRequested( const QString& ) ) );
}

QPixmap UIMain::cachedPixmap( const QUrl& url ) const
{
	const QByteArray* data = mCache->cachedData( url );
	QPixmap pixmap;
	
	if ( !data ) {
		return pixmap;
	}
	
	pixmap.loadFromData( *data );
	
	return pixmap;
}

void UIMain::propertiesChanged()
{
	Properties properties( this );
	
	mCache->setDiskCacheSize( properties.cacheDiskSize() );
	mCache->setMemoryCacheSize( properties.cacheMemorySize() );
	mCache->setWorkingPath( properties.cacheUseTemporaryPath() ? properties.temporaryPath() : properties.cacheWorkingPath() );
}

void UIMain::openViewRequested()
{
	PartitionWidget* pw = new PartitionWidget( this );
	pw->setMainView( false );
	pw->setPartitions( mPartitions );
	pw->showHideImportViewButton()->setChecked( false );
	connectView( pw );
	sViews->addWidget( pw );
}

void UIMain::closeViewRequested()
{
	sender()->deleteLater();
}

void UIMain::coverRequested( const QString& id )
{
	mLastDiscId = id;
	
	const QUrl urlCD = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::Disc, id );
	const QUrl urlCover = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::Cover, id );
	
	lCDCover->clear();
	lCover->clear();
	
	if ( mCache->hasCachedData( urlCD ) || mCache->hasCachedData( urlCover ) ) {
		dataNetworkCache_dataCached( QUrl() );
	}
	
	if ( !lCDCover->pixmap() ) {
		mCache->cacheData( urlCD );
	}
	
	if ( !lCover->pixmap() ) {
		mCache->cacheData( urlCover );
	}
	
	/*
	static QNetworkAccessManager* manager = new QNetworkAccessManager( this );
	QNetworkReply* reply = manager->get( QNetworkRequest( url ) );
	QPixmap pixmap;
	
	while ( !reply->isFinished() ) {
		QApplication::processEvents();
	}
	
	pixmap.loadFromData( reply->readAll() );
	
	if ( pixmap.isNull() ) {
		url = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::DiscCustom, id );
		reply = manager->get( QNetworkRequest( url ) );
		
		while ( !reply->isFinished() ) {
			QApplication::processEvents();
		}
	}
	
	pixmap.loadFromData( reply->readAll() );
	
	lCover->setPixmap( pixmap );*/
}

void UIMain::progress_jobFinished( const QWBFS::Model::Disc& disc )
{
	mExportModel->updateDisc( disc );
}

void UIMain::dataNetworkCache_dataCached( const QUrl& url )
{
	Q_UNUSED( url );
	
	if ( mLastDiscId.isEmpty() ) {
		return;
	}
	
	const QUrl urlCD = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::Disc, mLastDiscId );
	const QUrl urlCover = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::Cover, mLastDiscId );
	
	if ( mCache->hasCachedData( urlCD ) ) {
		lCDCover->setPixmap( cachedPixmap( urlCD ) );
	}
	
	if ( mCache->hasCachedData( urlCover ) ) {
		lCover->setPixmap( cachedPixmap( urlCover ) );
	}
}

void UIMain::dataNetworkCache_error( const QString& message )
{
	qWarning() << message;
}

void UIMain::dataNetworkCache_invalidated()
{
	dataNetworkCache_dataCached( QUrl() );
}

void UIMain::on_aReloadPartitions_triggered()
{
	mPartitions.clear();

#if defined( Q_OS_WIN )
	foreach ( const QFileInfo& drive, QDir::drives() ) {
		mPartitions << drive.absoluteFilePath().remove( ":" ).remove( "/" ).remove( "\\" );
	}#elif defined( Q_OS_MAC )	QProcess process;
	process.start( "diskutil list" );
	process.waitForFinished();
	
	const QStringList partitions = QString::fromLocal8Bit( process.readAll() ).split( "\n" );		foreach ( QString partition, partitions ) {		partition = partition.trimmed();		
		if ( partition.startsWith( "/" ) || partition.startsWith( "#" ) || partition.isEmpty() ) {
			continue;
		}
		
		partition = partition.simplified().section( ' ', -1 );
		
		// skip disks
		if ( partition[ partition.size() -2 ].toLower() != 's' ) {
			continue;
		}
		
		mPartitions << QString( "/dev/%1" ).arg( partition );
	}
#else
	QProcess process;
	process.start( "cat /proc/partitions" );
	process.waitForFinished();
	
	const QStringList partitions = QString::fromLocal8Bit( process.readAll() ).split( "\n" );
	
	foreach ( QString partition, partitions ) {
		if ( partition.startsWith( "major" ) || partition.isEmpty() ) {
			continue;
		}
		
		partition = partition.simplified().section( ' ', -1 );
		
		// skip disks
		if ( !partition[ partition.size() -1 ].isDigit() ) {
			continue;
		}
		
		mPartitions << QString( "/dev/%1" ).arg( partition );
	}
#endif
	
	const QList<PartitionWidget*> widgets = sViews->findChildren<PartitionWidget*>();
	
	foreach ( PartitionWidget* widget, widgets ) {
		widget->setPartitions( mPartitions );
	}
}void UIMain::on_aQuit_triggered(){	close();}

void UIMain::on_aAbout_triggered()
{
	UIAbout* about = new UIAbout( this );
	about->open();
}

void UIMain::on_aProperties_triggered()
{
	PropertiesDialog* dlg = new PropertiesDialog( this );
	connect( dlg, SIGNAL( propertiesChanged() ), this, SLOT( propertiesChanged() ) );
	dlg->open();
}

void UIMain::on_tvFolders_activated( const QModelIndex& index )
{
	const QString filePath = mFoldersModel->filePath( index );
	mFilesModel->setRootPath( filePath );
	lvFiles->setRootIndex( mFilesModel->index( filePath ) );
}void UIMain::on_tbReloadDrives_clicked(){	const QString drive = cbDrives->currentText();	QFileInfoList drives = QDir::drives();		cbDrives->clear();	#if defined( Q_OS_WIN )#elif defined( Q_OS_MAC )	foreach ( const QFileInfo& fi, QDir( "/Volumes" ).entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot ) ) {		if ( !drives.contains( fi ) ) {			drives << fi;		}	}#else#endif	foreach ( const QFileInfo& fi, drives ) {		cbDrives->addItem( fi.absoluteFilePath() );	}		if ( !drive.isEmpty() ) {		cbDrives->setCurrentIndex( cbDrives->findText( drive ) );	}}void UIMain::on_cbDrives_currentIndexChanged( const QString& text ){
	mFoldersModel->setRootPath( text );	tvFolders->setRootIndex( mFoldersModel->index( text ) );
	on_tvFolders_activated( tvFolders->rootIndex() );}

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
	if ( mExportModel->rowCount() == 0 ) {
		return;
	}
	
	const QString path = QFileDialog::getExistingDirectory( this, tr( "Choose a folder to export the discs" ), QString::null );
	
	if ( path.isEmpty() ) {
		return;
	}
	
	ProgressDialog* dlg = new ProgressDialog( this );
	
	connect( dlg, SIGNAL( jobFinished( const QWBFS::Model::Disc& ) ), this, SLOT( progress_jobFinished( const QWBFS::Model::Disc& ) ) );
	
	dlg->exportDiscs( mExportModel->discs(), path );
}
