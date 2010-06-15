/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : ProgressDialog.cpp
** Date      : 2010-06-15T23:21:10
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR HARDWARE OR YOUR DATA
** BECAUSE OF IMPROPER USAGE OF THIS SOFTWARE.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "ProgressDialog.h"
#include "ExportThread.h"

#include <QPushButton>
#include <QTimer>
#include <QDesktopWidget>
#include <QDebug>

ProgressDialog::ProgressDialog( QWidget* parent )
	: QDialog( parent )
{
	mThread = 0;
	
	setupUi( this );
	setAttribute( Qt::WA_DeleteOnClose );
	cbDetails->setChecked( false );
	dbbButtons->button( QDialogButtonBox::Ok )->setEnabled( false );
	dbbButtons->button( QDialogButtonBox::Cancel )->setEnabled( false );
}

ProgressDialog::~ProgressDialog()
{
	//qWarning() << Q_FUNC_INFO;
}

void ProgressDialog::closeEvent( QCloseEvent* event )
{
	if ( mThread && mThread->isRunning() ) {
		event->ignore();
		return;
	}
	
	QDialog::closeEvent( event );
}

void ProgressDialog::exportDiscs( const QWBFS::Model::DiscList& discs, const QString& path )
{
	mThread = new ExportThread( this );
	
	connect( dbbButtons->button( QDialogButtonBox::Ok ), SIGNAL( clicked() ), this, SLOT( close() ) );
	connect( dbbButtons->button( QDialogButtonBox::Cancel ), SIGNAL( clicked() ), mThread, SLOT( stop() ) );
	connect( mThread, SIGNAL( started() ), this, SLOT( thread_started() ) );
	connect( mThread, SIGNAL( message( const QString& ) ), lCurrentInformations, SLOT( setText( const QString& ) ) );
	connect( mThread, SIGNAL( jobFinished( const QWBFS::Model::Disc& ) ), this, SLOT( thread_jobFinished( const QWBFS::Model::Disc& ) ) );
	connect( mThread, SIGNAL( currentProgressChanged( int, int, const QTime& ) ), this, SLOT( thread_currentProgressChanged( int, int, const QTime& ) ) );
	connect( mThread, SIGNAL( globalProgressChanged( int ) ), pbGlobal, SLOT( setValue( int ) ) );
	connect( mThread, SIGNAL( finished() ), this, SLOT( thread_finished() ) );
	
	setWindowTitle( tr( "Exporting discs..." ) );
	pbGlobal->setMaximum( discs.count() );
	open();
	
	if ( !mThread->exportDiscs( discs, path ) ) {
		deleteLater();
	}
}

void ProgressDialog::importDiscs( const QWBFS::Model::DiscList& discs, const QWBFS::Partition::Handle& partitionHandle )
{
	mThread = new ExportThread( this );
	
	connect( dbbButtons->button( QDialogButtonBox::Ok ), SIGNAL( clicked() ), this, SLOT( close() ) );
	connect( dbbButtons->button( QDialogButtonBox::Cancel ), SIGNAL( clicked() ), mThread, SLOT( stop() ) );
	connect( mThread, SIGNAL( started() ), this, SLOT( thread_started() ) );
	connect( mThread, SIGNAL( message( const QString& ) ), lCurrentInformations, SLOT( setText( const QString& ) ) );
	connect( mThread, SIGNAL( jobFinished( const QWBFS::Model::Disc& ) ), this, SLOT( thread_jobFinished( const QWBFS::Model::Disc& ) ) );
	connect( mThread, SIGNAL( currentProgressChanged( int, int, const QTime& ) ), this, SLOT( thread_currentProgressChanged( int, int, const QTime& ) ) );
	connect( mThread, SIGNAL( globalProgressChanged( int ) ), pbGlobal, SLOT( setValue( int ) ) );
	connect( mThread, SIGNAL( finished() ), this, SLOT( thread_finished() ) );
	
	setWindowTitle( tr( "Importing discs..." ) );
	pbGlobal->setMaximum( discs.count() );
	open();
	
	if ( !mThread->importDiscs( discs, partitionHandle ) ) {
		deleteLater();
	}
}

void ProgressDialog::thread_started()
{
	mElapsed.restart();
	dbbButtons->button( QDialogButtonBox::Cancel )->setEnabled( true );
}

void ProgressDialog::thread_jobFinished( const QWBFS::Model::Disc& disc )
{
	
	const QString text = QString( "%1 '%2': %3 (%4)" )
		.arg( ExportThread::taskToString( mThread->task() ) )
		.arg( disc.title )
		.arg( QWBFS::Driver::stateToString( QWBFS::Driver::State( disc.state ) ) )
		.arg( QWBFS::Driver::errorToString( QWBFS::Driver::Error( disc.error ) ) );
	
	pteErrors->appendPlainText( text );
	
	if ( !cbDetails->isChecked() && disc.state == QWBFS::Driver::Failed ) {
		cbDetails->toggle();
	}
	
	emit jobFinished( disc );
}

void ProgressDialog::thread_currentProgressChanged( int value, int maximum, const QTime& remaining )
{
	pbCurrent->setMaximum( maximum );
	pbCurrent->setValue( value );
	lCurrentRemaining->setText( tr( "Time remaining: %1" ).arg( remaining.toString() ) );
}

void ProgressDialog::thread_finished()
{
	QTime time( 0, 0, 0, 0 );
	time = time.addMSecs( mElapsed.elapsed() );
	
	lCurrentInformations->setText( "Tasks finished." );
	lCurrentRemaining->clear();
	lGlobalInformations->setText( tr( "The thread finished in %1" ).arg( time.toString() ) );
	delete mThread;
	dbbButtons->button( QDialogButtonBox::Ok )->setEnabled( true );
	dbbButtons->button( QDialogButtonBox::Cancel )->setEnabled( false );
	
	emit finished();
}

void ProgressDialog::on_cbDetails_toggled()
{
	QTimer::singleShot( 0, this, SLOT( updateSpace() ) );
}

void ProgressDialog::updateSpace()
{
	QWidget* widget = parentWidget();
	QRect rect = geometry();
	
	if ( !widget ) {
		widget = QApplication::desktop();
	}
	
	widget = widget->window();
	
	if ( !cbDetails->isChecked() ) {
		rect.setHeight( minimumSizeHint().height() );
	}
	
	rect.moveCenter( widget->mapToGlobal( widget->rect().center() ) );
#if defined( Q_OS_MAC )
	rect.moveTop( widget->mapToGlobal( widget->rect().topLeft() ).y() );
#endif
	resize( rect.size() );
	move( rect.topLeft() );
}
