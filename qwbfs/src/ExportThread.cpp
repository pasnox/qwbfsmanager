#include "ExportThread.h"
#include "qWBFS.h"

#include <QTime>
#include <QDir>
#include <QDebug>

ExportThread* ExportThread::mCurrentExportThread = 0;

ExportThread::ExportThread( QObject* parent )
	: QThread( parent )
{
}

ExportThread::~ExportThread()
{
	if ( isRunning() ) {
		stop();
		wait();
	}
}

bool ExportThread::exportDiscs( const DiscList& discs, const QString& path )
{
	if ( isRunning() ) {
		Q_ASSERT( 0 );
		return false;
	}
	
	mDiscs = discs;
	mPath = path;
	mStop = false;
	
	start();
	
	return true;
}

void ExportThread::emitCurrentProgressChanged( int value, int maximum, const QTime& remaining )
{
	emit currentProgressChanged( value, maximum, remaining );
}

void ExportThread::stop()
{
	QMutexLocker locker( &mMutex );
	mStop = true;
	emit error( tr( "Cancel requested, the export will stop after the current operation." ) );
}

void ExportThread::run()
{
	mCurrentExportThread = this;
	qWBFS::Handle* handle = 0;
	
	emit globalProgressChanged( 0 );
	
	for ( int i = 0; i < mDiscs.count(); i++ ) {
		const Disc& disc = mDiscs[ i ];
		
		emit message( tr( "Exporting '%1'..." ).arg( disc.title ) );
		
		// delete handle if different partition
		if ( handle && handle->properties.partition != disc.origin ) {
			delete handle;
			handle = 0;
		}
		
		// get partition handle
		if ( !handle ) {
			qWBFS::Properties properties;
			properties.partition = disc.origin;
			
			handle = new qWBFS::Handle( properties );
			
			if ( !handle->isValid() ) {
				emit error( tr( "Can't initialize partition '%1'." ).arg( properties.partition ) );
				break;
			}
		}
		
		// get disc handle
		qWBFS::DiscHandle discHandle( *handle, disc.id );
		
		if ( !discHandle.isValid() ) {
			emit error( tr( "Invalid handle for '%1', can't export the disc." ).arg( disc.title ) );
			emit globalProgressChanged( i +1 );
			continue;
		}
		
		const QString filePath = QDir::toNativeSeparators( QString( "%1/%2" ).arg( mPath ).arg( discHandle.isoName() ) );
		void* fileHandle = wbfs_open_file_for_write( filePath.toLocal8Bit().data() );

		if ( fileHandle ) {
			// write a zero at the end of the iso to ensure the correct size
			// XXX should check if the game is DVD9..
			wbfs_file_reserve_space( fileHandle, ( discHandle.ptr()->p->n_wii_sec_per_disc /2 ) *0x8000ULL );
			if ( wbfs_extract_disc( discHandle.ptr(), wbfs_write_wii_sector_file, fileHandle, progress_callback ) != 0 ) {
				emit error( tr( "Disc export failed '%1'." ).arg( disc.title ) );
			}
			wbfs_close_file( fileHandle );
		}
		
		emit globalProgressChanged( i +1 );
		
		{
			QMutexLocker locker( &mMutex );
			if ( mStop ) {
				break;
			}
		}
	}
	
	delete handle;
	mCurrentExportThread = 0;
}

void ExportThread::progress_callback( int x, int max )
{
	static time_t start_time;
	static u32 expected_total;
	u32 d;
	u32 h, m, s;

	if ( x == 0 ) {
		start_time = time( 0 );
		expected_total = 300;
	}

	if ( x == max ) {
		d = (u32)(time( 0 ) -start_time );
		h = d /3600;
		m = ( d /60 ) %60;
		s = d %60;
		
		if ( mCurrentExportThread ) {
			mCurrentExportThread->emitCurrentProgressChanged( x, max, QTime( h, m, s ) );
		}
		
		return;
	}

	d = (u32)(time( 0 ) -start_time );

	if ( x != 0 ) {
		expected_total = (u32)((3 *expected_total +d *max /x ) /4 );
	}

	if ( expected_total > d ) {
		d = expected_total -d;
	}
	else {
		d = 0;
	}

	h = d /3600;
	m = ( d /60 ) %60;
	s = d %60;

	if ( mCurrentExportThread ) {
		mCurrentExportThread->emitCurrentProgressChanged( x, max, QTime( h, m, s ) );
	}
}
