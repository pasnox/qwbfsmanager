/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : UIMain.cpp
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
#include "UIMain.h"
#include "UIAbout.h"
#include "PartitionComboBox.h"
#include "wiitdb/Covers.h"
#include "PropertiesDialog.h"
#include "Properties.h"
#include "main.h"

#include <FreshCore/pNetworkAccessManager>
#include <FreshCore/pTranslationManager>
#include <FreshGui/pTranslationDialog>
#include <FreshGui/pUpdateChecker>
#include <FreshGui/pPaypalButton>
#include "models/pPartitionModel.h" // will be part of fresh library in a next version

#include <QMenuBar>
#include <QMenu>
#include <QFileSystemModel>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QInputDialog>
#include <QPainter>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QDebug>

#define COVER_DISC_SIZE QSize( 160, 160 )
#define COVER_SIZE QSize( 160, 224 )

UIMain::UIMain( QWidget* parent )
	: QMainWindow( parent )
{
#if defined( Q_OS_MAC )
	mMenuBar = new QMenuBar;
#endif
	mCache = pNetworkAccessManager::instance();
	mUpdateChecker = new pUpdateChecker( this );
	mUpdateChecker->setDownloadsFeedUrl( QUrl( APPLICATION_DOWNLOADS_FEED ) );
	mUpdateChecker->setVersion( APPLICATION_VERSION );
	mUpdateChecker->setVersionString( APPLICATION_VERSION_STR );
	mUpdateChecker->setVersionDiscoveryPattern( ".*qwbfsmanager-([0-9\\.]+).*" );
	
	setWindowTitle( QString( "%1 v%2" ).arg( APPLICATION_NAME ).arg( APPLICATION_VERSION_STR ) );
	setUnifiedTitleAndToolBarOnMac( true );
	setupUi( this );

#if defined( Q_OS_MAC )
	mMenuBar->addMenu( "Fake" )->addAction( aProperties );
#endif
	
	centralVerticalLayout->setMenuBar( qmtbInfos );
	qmtbInfos->layout()->setMargin( 5 );
	qmtbInfos->queuedMessageWidget()->setContentsMargins( 5, 0, 5, 0 );
	qmtbInfos->setVisible( false );
	
	dwTools->toggleViewAction()->setIcon( QIcon( ":/icons/256/tools.png" ) );
	dwCovers->toggleViewAction()->setIcon( QIcon( ":/icons/256/covers.png" ) );
	
	mDonationWidget = new pPaypalButton( this );
	mDonationWidget->setBusinessId( "5R924WYXJ6BAW" );
	mDonationWidget->setItemName( "QWBFS Manager" );
	mDonationWidget->setItemId( "QWBFS-DONATION" );
	mDonationWidget->setCurrencyCode( "EUR" );
	
	mActions = new QMenu( this );
	mActions->setIcon( aConvertToWBFSFiles->icon() );
	mActions->addAction( aConvertToWBFSFiles );
	mActions->addAction( aConvertToISOFiles );
	mActions->addAction( aRenameDiscsInFolder );
	
	toolBar->insertAction( aAbout, mUpdateChecker->menuAction() );
	toolBar->addAction( mActions->menuAction() );
	toolBar->addSeparator();
	toolBar->addAction( dwTools->toggleViewAction() );
	toolBar->addAction( dwCovers->toggleViewAction() );
	QWidget* spacerWidget = new QWidget( toolBar );
	spacerWidget->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum ) );
	toolBar->addWidget( spacerWidget );
	toolBar->addWidget( mDonationWidget );
	
	mFoldersModel = new QFileSystemModel( this );
	mFoldersModel->setFilter( QDir::Dirs | QDir::NoDotAndDotDot );
	
	mFilesModel = new QFileSystemModel( this );
	mFilesModel->setFilter( QDir::Files );

	tvFolders->setModel( mFoldersModel );
	tvFolders->setColumnHidden( 1, true );
	tvFolders->setColumnHidden( 2, true );
	tvFolders->setColumnHidden( 3, true );
	
	lvFiles->setModel( mFilesModel );
	
	lvExport->setModel( 0 );
	
	mLastDiscId = -1;
	
	pwMainView->setMainView( true );
	//pwMainView->showHideImportViewButton()->setChecked( false );
	connectView( pwMainView );
	
	qmtbInfos->installEventFilter( this );
	lWiiTDB->installEventFilter( this );
	
	localeChanged();
	
	connect( mCache, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( networkAccessManager_finished( QNetworkReply* ) ) );
	connect( mCache, SIGNAL( cached( const QUrl& ) ), this, SLOT( networkAccessManager_cached( const QUrl& ) ) );
	connect( mCache, SIGNAL( error( const QUrl&, const QString& ) ), this, SLOT( networkAccessManager_error( const QUrl&, const QString& ) ) );
	connect( mCache, SIGNAL( cacheCleared() ), this, SLOT( networkAccessManager_cacheCleared() ) );
}

UIMain::~UIMain()
{
#if defined( Q_OS_MAC )
	delete mMenuBar;
#endif
	//qWarning() << Q_FUNC_INFO;
}

pNetworkAccessManager* UIMain::cache() const
{
	return mCache;
}

pQueuedMessageToolBar* UIMain::messageToolBar() const
{
	return qmtbInfos;
}

bool UIMain::event( QEvent* event )
{
	switch ( event->type() ) {
		case QEvent::LocaleChange:
			localeChanged();
			break;
		default:
			break;
	}
	
	return QMainWindow::event( event );
}

void UIMain::showEvent( QShowEvent* event )
{
	QMainWindow::showEvent( event );
	
	static bool shown = false;
	
	if ( !shown ) {
		shown = true;
		loadProperties();
		mUpdateChecker->silentCheck();
		qmtbInfos->appendMessage( tr(
				"Welcome to %1, the cross-platform WBFS Manager. Report bugs <a href=\"%2\">here</a>, discuss <a href=\"%3\">here</a>."
			).arg( APPLICATION_NAME ).arg( APPLICATION_REPORT_BUG_URL ).arg( APPLICATION_DISCUSS_URL ) );
	}
}

void UIMain::closeEvent( QCloseEvent* event )
{
	saveProperties();
	
	QMainWindow::closeEvent( event );
}

bool UIMain::eventFilter( QObject* object, QEvent* event )
{
	if ( object == qmtbInfos ) {
		if ( event->type() == QEvent::Paint ) {
			if ( qmtbInfos->queuedMessageWidget()->pendingMessageCount() > 0 ) {
				QPainter painter( qmtbInfos );
				QBrush brush;
				
				qmtbInfos->queuedMessageWidget()->currentMessageInformations( 0, &brush, 0 );
				
				painter.setRenderHint( QPainter::Antialiasing );
				painter.setPen( QPen( brush.color().darker( 150 ), 0.5 ) );
				painter.setBrush( brush );
				painter.drawRoundedRect( qmtbInfos->rect().adjusted( 10 -4, -9, -10 +4, -1 ), 9, 9 );
				
				return true;
			}
		}
	}
	else if ( object == lWiiTDB ) {
		if ( event->type() == QEvent::MouseButtonPress ) {
			QDesktopServices::openUrl( lWiiTDB->toolTip() );
		}
	}
	
	return QMainWindow::eventFilter( object, event );
}

void UIMain::connectView( PartitionWidget* widget )
{
	connect( widget, SIGNAL( openViewRequested() ), this, SLOT( openViewRequested() ) );
	connect( widget, SIGNAL( closeViewRequested() ), this, SLOT( closeViewRequested() ) );
	connect( widget, SIGNAL( coverRequested( const QString& ) ), this, SLOT( coverRequested( const QString& ) ) );
}

void UIMain::localeChanged()
{
	retranslateUi( this );
	mActions->setTitle( tr( "Actions" ) );
}

void UIMain::loadProperties( bool firstInit )
{
	Properties properties( this );
	
	PartitionComboBox::partitionModel()->addPartitions( properties.customPartitions() );
	
	tbReloadDrives->click();
	aReloadPartitions->trigger();
	
	mCache->setMaximumCacheSize( properties.cacheDiskSize() );
	mCache->setCacheDirectory( properties.cacheUseTemporaryPath() ? properties.temporaryPath() : properties.cacheWorkingPath() );
	
	QNetworkProxy proxy( properties.proxyType() );
	proxy.setHostName( properties.proxyServer() );
	proxy.setPort( properties.proxyPort() );
	proxy.setUser( properties.proxyLogin() );
	proxy.setPassword( properties.proxyPassword() );
	
	QNetworkProxy::setApplicationProxy( proxy );
	
	if ( firstInit ) {
		mUpdateChecker->setLastUpdated( properties.updateLastUpdated() );
		mUpdateChecker->setLastChecked( properties.updateLastChecked() );
		
		const QModelIndex index = mFoldersModel->index( properties.selectedPath() );
		
		properties.restoreState( this );
		tvFolders->setCurrentIndex( index );
		tvFolders->scrollTo( index );
		on_tvFolders_activated( index );
		pwMainView->setCurrentPartition( properties.selectedPartition() );
	}
	
	pTranslationManager* translationManager = pTranslationManager::instance();
	translationManager->setTranslationsPaths( properties.translationsPaths() );
	translationManager->setCurrentLocale( properties.locale().name() );
	
	if ( !properties.localeAccepted() ) {
		changeLocaleRequested();
	}
	
	translationManager->reloadTranslations();
	
	foreach ( QWidget* widget, QApplication::topLevelWidgets() ) {
		widget->setLocale( translationManager->currentLocale() );
	}
	
	foreach ( ListView* view, findChildren<ListView*>() ) {
		view->setViewMode( properties.viewMode() );
		view->setViewIconType( properties.viewIconType() );
	}
}

void UIMain::saveProperties()
{
	const QModelIndex index = tvFolders->selectionModel()->selectedIndexes().value( 0 );
	const QString selectedPath = mFoldersModel->filePath( index );
	Properties properties;
	
	properties.setUpdateLastUpdated( mUpdateChecker->lastUpdated() );
	properties.setUpdateLastChecked( mUpdateChecker->lastChecked() );
	properties.setSelectedPath( selectedPath );
	properties.setSelectedPartition( pwMainView->currentPartition() );
	properties.setCustomPartitions( PartitionComboBox::partitionModel()->customPartitions() );
	properties.saveState( this );
}

void UIMain::changeLocaleRequested()
{
	pTranslationManager* translationManager = pTranslationManager::instance();
	const QString locale = pTranslationDialog::getLocale( translationManager, this );
	
	if ( !locale.isEmpty() ) {
		Properties properties;
		properties.setTranslationsPaths( translationManager->translationsPaths() );
		properties.setLocaleAccepted( true );
		properties.setLocale( QLocale( locale ) );
		
		translationManager->setCurrentLocale( locale );
	}
}

void UIMain::propertiesChanged()
{
	loadProperties( false );
}

void UIMain::openViewRequested()
{
	PartitionWidget* pw = new PartitionWidget( this );
	pw->setMainView( false );
	//pw->showHideImportViewButton()->setChecked( false );
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
	
	networkAccessManager_cached( QUrl( WIITDB_DOMAIN ) );
	QWBFS::WiiTDB::coverDiscPixmap( id, mCache, COVER_DISC_SIZE );
	QWBFS::WiiTDB::coverBoxPixmap( id, mCache, COVER_SIZE );
}

/*void UIMain::progress_jobFinished( const QWBFS::Model::Disc& disc )
{
	lvExport->model()->updateDisc( disc );
}*/

void UIMain::networkAccessManager_finished( QNetworkReply* reply )
{
	reply->deleteLater();
}

void UIMain::networkAccessManager_cached( const QUrl& url )
{
	/*if ( !url.toString().startsWith( WIITDB_DOMAIN, Qt::CaseInsensitive ) && url != QUrl() ) {
		return;
	}
	
	// update all models
	const QString id = QFileInfo( url.path() ).baseName();
	const QList<QWBFS::Model::DiscModel*> models = findChildren<QWBFS::Model::DiscModel*>();
	
	foreach ( QWBFS::Model::DiscModel* model, models ) {
		const QModelIndex index = model->index( id );
		
		model->setData( index, QVariant(), Qt::DecorationRole );
	}
	
	lCDCover->setPixmap( QWBFS::WiiTDB::coverDiscPixmap( mLastDiscId, mCache, COVER_DISC_SIZE ) );
	lCover->setPixmap( QWBFS::WiiTDB::coverBoxPixmap( mLastDiscId, mCache, COVER_SIZE ) );*/
}

void UIMain::networkAccessManager_error( const QUrl& url, const QString& message )
{
	if ( !url.toString().startsWith( WIITDB_DOMAIN, Qt::CaseInsensitive ) ) {
		return;
	}
	
	const QString id = QFileInfo( url.path() ).baseName();
	
	switch ( QWBFS::WiiTDB::urlCover( url ) )
	{
		case QWBFS::WiiTDB::CoverDisc: {
			const QUrl enUrl = QWBFS::WiiTDB::coverUrl( QWBFS::WiiTDB::CoverDisc, id, "EN" );
			
			// request english
			if ( url != enUrl ) {
				mCache->get( QNetworkRequest( enUrl ) );
			}
			// or custom disk
			else {
				mCache->get( QNetworkRequest( QWBFS::WiiTDB::coverUrl( QWBFS::WiiTDB::CoverDiscCustom, id ) ) );
			}
			
			return;
		}
		case QWBFS::WiiTDB::CoverDiscCustom: {
			const QUrl enUrl = QWBFS::WiiTDB::coverUrl( QWBFS::WiiTDB::CoverDiscCustom, id, "EN" );
			
			// request english
			if ( url != enUrl ) {
				mCache->get( QNetworkRequest( enUrl ) );
				return;
			}
			
			break;
		}
		case QWBFS::WiiTDB::Cover: {
			const QUrl enUrl = QWBFS::WiiTDB::coverUrl( QWBFS::WiiTDB::Cover, id, "EN" );
			
			// request english
			if ( url != enUrl ) {
				mCache->get( QNetworkRequest( enUrl ) );
				return;
			}
			
			break;
		}
		case QWBFS::WiiTDB::CoverHQ:
		case QWBFS::WiiTDB::Cover3D:
		case QWBFS::WiiTDB::CoverFull:
		case QWBFS::WiiTDB::CoverInvalid:
			break;
	}
	
	qmtbInfos->appendMessage( message );
}

void UIMain::networkAccessManager_cacheCleared()
{
	networkAccessManager_cached( QUrl() );
}

void UIMain::on_aReloadPartitions_triggered()
{
	PartitionComboBox::partitionModel()->update();
	
	if ( PartitionComboBox::partitionModel()->rowCount() == 0 ) {
		qmtbInfos->appendMessage( tr(
				"I don't know how to list partition for this platform.\n"
				"You will have to set the correct partition path yourself for mounting partitions." ) );
	}
}

void UIMain::on_aQuit_triggered()
{
	close();
}

void UIMain::on_aAbout_triggered()
{
	UIAbout* about = new UIAbout( this );
	about->open();
}

void UIMain::on_aProperties_triggered()
{
	saveProperties();
	
	PropertiesDialog* dlg = new PropertiesDialog( this );
	connect( dlg, SIGNAL( propertiesChanged() ), this, SLOT( propertiesChanged() ) );
	
	dlg->open();
}

void UIMain::on_aConvertToWBFSFiles_triggered()
{
	/*const QStringList filePaths = QFileDialog::getOpenFileNames( this, tr( "Choose ISO files to convert" ), QString::null, tr( "ISO Files (*.iso)" ) );
	
	if ( filePaths.isEmpty() ) {
		return;
	}
	
	ProgressDialog* dlg = new ProgressDialog( this );
	
	WorkerThread::Work work;
	work.task = WorkerThread::ConvertWBFS;
	
	foreach ( const QString& filePath, filePaths ) {
		work.discs << QWBFS::Model::Disc( filePath );
	}
	
	work.target = QFileInfo( filePaths.first() ).absolutePath();
	work.window = dlg;
	
	dlg->setWork( work );*/
}

void UIMain::on_aConvertToISOFiles_triggered()
{
	/*const QStringList filePaths = QFileDialog::getOpenFileNames( this, tr( "Choose WBFS files to convert" ), QString::null, tr( "WBFS Files (*.wbfs)" ) );
	
	if ( filePaths.isEmpty() ) {
		return;
	}
	
	ProgressDialog* dlg = new ProgressDialog( this );
	
	WorkerThread::Work work;
	work.task = WorkerThread::ConvertISO;
	
	foreach ( const QString& filePath, filePaths ) {
		work.discs << QWBFS::Model::Disc( filePath );
	}
	
	work.target = QFileInfo( filePaths.first() ).absolutePath();
	work.window = dlg;
	
	dlg->setWork( work );*/
}

void UIMain::on_aRenameDiscsInFolder_triggered()
{
	/*const QString path = QFileDialog::getExistingDirectory( this, tr( "Choose the folder to scan for ISOs/WBFSs files" ) );
	
	if ( path.isEmpty() ) {
		return;
	}
	
	/
		%title = Game Title
		%id = Game ID
		%suffix = File Suffix
	/
	const QStringList patterns = QStringList()
		<< "%id.%suffix" // GAMEID.wbfs
		<< "%title [%id].%suffix" // Game Title [GAMEID].wbfs
		<< "%id_%title/%id.%suffix" // GAMEID_Game Title/GAMEID.wbfs
		<< "%title/%id.%suffix" // Game Title/GAMEID.wbfs
		<< "%title[%id]/%id.%suffix" // Game Title[GAMEID]/GAMEID.wbfs
		;
	const QString text = tr( "Choose the pattern to apply:\n%1\n%2\n%3\n" )
		.arg( tr( "%1 = Game Title" ).arg( "%title" ) )
		.arg( tr( "%1 = Game Id" ).arg( "%id" ) )
		.arg( tr( "%1 = File Suffix" ).arg( "%suffix" ) )
		;
	bool ok;
	const QString pattern = QInputDialog::getItem( this, QString::null, text, patterns, 0, true, &ok );
	
	if ( !ok || pattern.isEmpty() ) {
		return;
	}
	
	ProgressDialog* dlg = new ProgressDialog( this );
	
	WorkerThread::Work work;
	work.task = WorkerThread::RenameAll;
	work.pattern = pattern;
	work.target = path;
	work.window = dlg;
	
	dlg->setWork( work );*/
}

void UIMain::on_tvFolders_activated( const QModelIndex& index )
{
	const QString filePath = mFoldersModel->filePath( index );
	mFilesModel->setRootPath( filePath );
	lvFiles->setRootIndex( mFilesModel->index( filePath ) );
}

void UIMain::on_tbReloadDrives_clicked()
{
	const QString drive = cbDrives->currentText();
	QFileInfoList drives = QDir::drives();
	QStringList pathsToScan;
	
	cbDrives->clear();
#if defined( Q_OS_WIN )
#elif defined( Q_OS_MAC )
	pathsToScan << "/Volumes";
#else
	pathsToScan  << "/media" << "/mnt";
#endif

	foreach ( const QString& path, pathsToScan ) {
		foreach ( const QFileInfo& fi, QDir( path ).entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot ) ) {
			if ( !drives.contains( fi ) ) {
				drives << fi;
			}
		}
	}
	
	foreach ( const QFileInfo& fi, drives ) {
		cbDrives->addItem( fi.absoluteFilePath() );
	}
	
	if ( !drive.isEmpty() ) {
		cbDrives->setCurrentIndex( cbDrives->findText( drive ) );
	}
}

void UIMain::on_cbDrives_currentIndexChanged( const QString& text )
{
	mFoldersModel->setRootPath( text );
	tvFolders->setRootIndex( mFoldersModel->index( text ) );
	on_tvFolders_activated( tvFolders->rootIndex() );
}

void UIMain::on_tbClearExport_clicked()
{
	//lvExport->model()->clear();
}

void UIMain::on_tbRemoveExport_clicked()
{
	//lvExport->model()->removeSelection( lvExport->selectionModel()->selection() );
}

void UIMain::on_tbExport_clicked()
{
	/*if ( lvExport->model()->rowCount() == 0 ) {
		return;
	}
	
	const QString path = QFileDialog::getExistingDirectory( this, tr( "Choose a folder to export the discs" ), QString::null );
	
	if ( path.isEmpty() ) {
		return;
	}
	
	WorkerThread::Work work;
	work.discs = lvExport->model()->discs();
	work.target = path;
	
	const QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::Ok;
	const QMessageBox::StandardButton defaultButton = QMessageBox::Ok;
	
	QMessageBox msgBox( this );
	msgBox.setIcon( QMessageBox::Question );
	msgBox.setText( tr( "Which format do you want to use to export the discs ?" ) );
	msgBox.setStandardButtons( buttons );
	msgBox.button( QMessageBox::Yes )->setText( "ISO" );
	msgBox.button( QMessageBox::Ok )->setText( "WBFS" );
	msgBox.setEscapeButton( 0 );
	msgBox.setDefaultButton( defaultButton );
	
	switch ( msgBox.exec() ) {
		case QMessageBox::Yes:
			work.task = WorkerThread::ExportISO;
			break;
		case QMessageBox::Ok:
			work.task = WorkerThread::ExportWBFS;
			break;
		default:
			Q_ASSERT( 0 );
			return;
	}
	
	ProgressDialog* dlg = new ProgressDialog( this );
	
	connect( dlg, SIGNAL( jobFinished( const QWBFS::Model::Disc& ) ), this, SLOT( progress_jobFinished( const QWBFS::Model::Disc& ) ) );
	
	work.window = dlg;
	
	dlg->setWork( work );*/
}
