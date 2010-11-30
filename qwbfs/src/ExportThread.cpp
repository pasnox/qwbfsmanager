/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : ExportThread.cpp
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
#include "ExportThread.h"
#include "qwbfsdriver/PartitionDiscHandle.h"

#include <QTime>
#include <QDir>
#include <QMetaType>
#include <QDebug>

ExportThread::ExportThread( QObject* parent )
	: QThread( parent )
{
	qRegisterMetaType<QWBFS::Model::Disc>( "QWBFS::Model::Disc" );
}

ExportThread::~ExportThread()
{
	if ( isRunning() ) {
		qWarning() << "Waiting thread to finish...";
		stop();
		wait();
	}
	
	//qWarning() << Q_FUNC_INFO;
}

ExportThread::Task ExportThread::task() const
{
	QMutexLocker locker( &const_cast<ExportThread*>( this )->mMutex );
	return mTask;
}

bool ExportThread::exportDiscs( const QWBFS::Model::DiscList& discs, const QString& path )
{
	if ( isRunning() ) {
		Q_ASSERT( 0 );
		return false;
	}
	
	mTask = ExportThread::Export;
	mDiscs = discs;
	mPath = path;
	mStop = false;
	
	start();
	
	return true;
}

bool ExportThread::importDiscs( const QWBFS::Model::DiscList& discs, const QWBFS::Partition::Handle& partitionHandle )
{
	if ( isRunning() ) {
		Q_ASSERT( 0 );
		return false;
	}
	
	mTask = ExportThread::Import;
	mDiscs = discs;
	mImportPartitionHandle = partitionHandle;
	mStop = false;
	
	start();
	
	return true;
}

bool ExportThread::convertIsoToWBFS( const QString& isoFilePath, const QString& wbfsFilePath )
{
	if ( isRunning() ) {
		Q_ASSERT( 0 );
		return false;
	}
	
	mTask = ExportThread::ConvertISO;
	mConvertFile.first = isoFilePath;
	mConvertFile.second = wbfsFilePath;
	mStop = false;
	
	start();
	
	return true;
}

bool ExportThread::convertWBFSToIso( const QString& wbfsFilePath, const QString& isoFilePath )
{
	if ( isRunning() ) {
		Q_ASSERT( 0 );
		return false;
	}
	
	mTask = ExportThread::ConvertWBFS;
	mConvertFile.first = wbfsFilePath;
	mConvertFile.second = isoFilePath;
	mStop = false;
	
	start();
	
	return true;
}

void ExportThread::stop()
{
	QMutexLocker locker( &mMutex );
	mStop = true;
	emit message( tr( "Cancel requested, the process will stop after the current operation." ) );
}

void ExportThread::run()
{
	switch ( mTask ) {
		case ExportThread::Export:
			exportWorker();
			break;
		case ExportThread::Import:
			importWorker();
			break;
		case ExportThread::ConvertISO:
			convertISOWorker();
			break;
		case ExportThread::ConvertWBFS:
			convertWBFSWorker();
			break;
	}
}

void ExportThread::connectDriver( const QWBFS::Driver& driver )
{
	connect( &driver, SIGNAL( currentProgressChanged( int, int, const QTime& ) ), this, SIGNAL( currentProgressChanged( int, int, const QTime& ) ) );
	connect( &driver, SIGNAL( globalProgressChanged( int ) ), this, SIGNAL( globalProgressChanged( int ) ) );
}

void ExportThread::exportWorker()
{
	QWBFS::Partition::Handle handle;
	bool hc = false;
	
	emit globalProgressChanged( 0 );
	
	for ( int i = 0; i < mDiscs.count(); i++ ) {
		QWBFS::Model::Disc disc = mDiscs.at( i );
		
		emit message( tr( "Exporting '%1'..." ).arg( disc.title ) );
		
		// close handle if different partition
		if ( handle.partition() != disc.origin ) {
			if ( hc ) {
				QWBFS::Driver::closeHandle( handle );
			}
			
			handle = QWBFS::Partition::Handle();
		}
		
		// create handle if needed
		if ( !handle.isValid() ) {
			handle = QWBFS::Driver::getHandle( disc.origin, &hc );
		}
		
		// create driver
		QWBFS::Driver driver( 0, handle );
		connectDriver( driver );
		const int result = driver.extractDisc( disc.id, mPath );
		
		disc.state = result == QWBFS::Driver::Ok ? QWBFS::Driver::Success : QWBFS::Driver::Failed;
		disc.error = result;
		
		emit globalProgressChanged( i +1 );
		emit jobFinished( disc );
		
		{
			QMutexLocker locker( &mMutex );
			if ( mStop ) {
				break;
			}
		}
	}
	
	if ( hc ) {
		QWBFS::Driver::closeHandle( handle );
	}
}

void ExportThread::importWorker()
{
	// target partition handle
	QWBFS::Driver td( 0, mImportPartitionHandle );
	
	if ( !td.isOpen() ) {
		emit log( tr( "Can't open partition '%1'." ).arg( td.partition() ) );
		return;
	}
	
	connectDriver( td );
	
	// source partition handle
	QWBFS::Partition::Handle sph;
	bool hc = false;
	
	emit globalProgressChanged( 0 );
	
	for ( int i = 0; i < mDiscs.count(); i++ ) {
		QWBFS::Model::Disc disc = mDiscs.at( i );
		int result;
		
		emit message( tr( "Importing '%1'..." ).arg( disc.title ) );
		
		// close handle if different partition
		if ( sph.partition() != disc.origin ) {
			if ( hc ) {
				QWBFS::Driver::closeHandle( sph );
			}
			
			sph = QWBFS::Partition::Handle();
		}
		
		// create handle if needed
		if ( QWBFS::Driver::isWBFSPartitionOrFile( disc.origin ) ) {
			sph = QWBFS::Driver::getHandle( disc.origin, &hc );
			result = td.addDisc( disc.id, sph );
		}
		else {
			result = td.addDiscImage( disc.origin );
		}
		
		disc.state = result == QWBFS::Driver::Ok ? QWBFS::Driver::Success : QWBFS::Driver::Failed;
		disc.error = result;
		
		emit globalProgressChanged( i +1 );
		emit jobFinished( disc );
		
		{
			QMutexLocker locker( &mMutex );
			if ( mStop ) {
				break;
			}
		}
	}
	
	if ( hc ) {
		QWBFS::Driver::closeHandle( sph );
	}
}

void ExportThread::convertISOWorker()
{
	int result;
	
	if ( mConvertFile.second.isEmpty() ) {
		mConvertFile.second = QString( "%1.wbfs" ).arg( mConvertFile.first );
	}
	
	emit globalProgressChanged( 0 );
	
	result = QWBFS::Driver::initializeWBFSFile( mConvertFile.second );
	emit message( tr( "Initializing wbfs file '%1'." ).arg( mConvertFile.second ) );
	
	if ( result != QWBFS::Driver::Ok ) {
		emit log(
			tr( "Can't create wbfs file '%1' (%2)." )
				.arg( mConvertFile.second )
				.arg( QWBFS::Driver::errorToString( QWBFS::Driver::Error( result ) ) )
		);
		return;
	}
	
	QWBFS::Partition::Properties properties( mConvertFile.second );
	properties.reset = true;
	
	QWBFS::Partition::Handle handle( properties );
	QWBFS::Driver td( 0, handle );
	
	if ( !td.isOpen() ) {
		emit log( tr( "Can't open wbfs file '%1'." ).arg( td.partition() ) );
		return;
	}
	
	connectDriver( td );
	
	QWBFS::Model::Disc disc;
	result = td.discImageInfo( mConvertFile.first, disc );
	
	if ( result != QWBFS::Driver::Ok ) {
		emit log(
			tr( "Can't get disc informations '%1' (%2)." )
				.arg( mConvertFile.first )
				.arg( QWBFS::Driver::errorToString( QWBFS::Driver::Error( result ) ) )
		);
		return;
	}
	
	emit message( tr( "Converting '%1' to WBFS..." ).arg( disc.title ) );
	result = td.addDiscImage( disc.origin );
	
	if ( result != QWBFS::Driver::Ok ) {
		emit log(
			tr( "Can't add disc '%1' (%2)." )
				.arg( mConvertFile.first )
				.arg( QWBFS::Driver::errorToString( QWBFS::Driver::Error( result ) ) )
		);
		return;
	}
	
	emit message( tr( "Triming '%1'..." ).arg( disc.title ) );
	result = td.trim();
	disc.state = result == QWBFS::Driver::Ok ? QWBFS::Driver::Success : QWBFS::Driver::Failed;
	disc.error = result;
	
	emit globalProgressChanged( 1 );
	emit jobFinished( disc );
}

void ExportThread::convertWBFSWorker()
{
	if ( mConvertFile.second.isEmpty() ) {
		mConvertFile.second = QString( "%1.iso" ).arg( mConvertFile.first );
	}
	
	emit globalProgressChanged( 0 );
	
	const QFileInfo file( mConvertFile.second );
	QWBFS::Partition::Handle handle = QWBFS::Driver::getHandle( mConvertFile.first );
	QWBFS::Model::Disc disc;
	int result;
	
	// create driver
	QWBFS::Driver driver( 0, handle );
	connectDriver( driver );
	result = driver.discInfo( 0, disc );
	
	if ( result != QWBFS::Driver::Ok ) {
		emit log(
			tr( "Can't get disc informations '%1' (%2)." )
				.arg( mConvertFile.first )
				.arg( QWBFS::Driver::errorToString( QWBFS::Driver::Error( result ) ) )
		);
		return;
	}
	
	emit message( tr( "Converting '%1' to ISO..." ).arg( disc.title ) );
	
	result = driver.extractDisc( disc.id, file.absolutePath(), file.fileName() );
	
	disc.state = result == QWBFS::Driver::Ok ? QWBFS::Driver::Success : QWBFS::Driver::Failed;
	disc.error = result;
	
	emit globalProgressChanged( 1 );
	emit jobFinished( disc );
	
	QWBFS::Driver::closeHandle( handle );
}

QString ExportThread::taskToString( ExportThread::Task task )
{
	switch ( task )
	{
		case ExportThread::Export:
			return tr( "Extracting" );
		case ExportThread::Import:
			return tr( "Adding" );
		case ExportThread::ConvertISO:
			return tr( "Converting ISO" );
		case ExportThread::ConvertWBFS:
			return tr( "Converting WBFS" );
	}
	
	return QString::null;
}
