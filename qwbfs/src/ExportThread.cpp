#include "ExportThread.h"
#include "qwbfsdriver/PartitionDiscHandle.h"

#include <QTime>
#include <QDir>
#include <QDebug>

ExportThread::ExportThread( QObject* parent )
	: QThread( parent )
{
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

void ExportThread::stop()
{
	QMutexLocker locker( &mMutex );
	mStop = true;
	emit error( tr( "Cancel requested, the process will stop after the current operation." ) );
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
		const QWBFS::Model::Disc& disc = mDiscs[ i ];
		
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
		
		if ( driver.isOpen() ) {
			connectDriver( driver );
			
			if ( !driver.extractDisc( disc.id, mPath ) ) {
				emit error( driver.lastErrors().join( "\n" ) );
			}
		}
		else {
			emit error( tr( "Invalid handle, can't open partition '%1' for copying game '%2'." ).arg( driver.partition() ).arg( disc.title ) );
			return;
		}
		
		emit globalProgressChanged( i +1 );
		
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
		emit error( tr( "Invalid target handle, can't open partition '%1'." ).arg( td.partition() ) );
		return;
	}
	
	connectDriver( td );
	
	// source partition handle
	QWBFS::Partition::Handle sph;
	bool hc = false;
	
	emit globalProgressChanged( 0 );
	
	for ( int i = 0; i < mDiscs.count(); i++ ) {
		const QWBFS::Model::Disc& disc = mDiscs[ i ];
		
		emit message( tr( "Importing '%1'..." ).arg( disc.title ) );
		
		// close handle if different partition
		if ( sph.partition() != disc.origin ) {
			if ( hc ) {
				QWBFS::Driver::closeHandle( sph );
			}
			
			sph = QWBFS::Partition::Handle();
		}
		
		// create handle if needed
		if ( td.isWBFSPartition( disc.origin ) ) {
			sph = QWBFS::Driver::getHandle( disc.origin, &hc );
			
			if ( !sph.isValid() ) {
				emit error( tr( "Invalid source partition handle, can't open partition '%1'." ).arg( sph.partition() ) );
				break;
			}
		}
		
		if ( !td.addDisc( disc, sph ) ) {
			emit error( td.lastErrors().join( "\n" ) );
		}
		
		emit globalProgressChanged( i +1 );
		
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
