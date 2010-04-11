#include "ExportThread.h"
#include "qwbfsdriver/PartitionDiscHandle.h"
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
		qWarning() << "Waiting...";
		stop();
		wait();
	}
	
	qWarning() << Q_FUNC_INFO;
}

bool ExportThread::exportDiscs( const DiscList& discs, const QString& path )
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

bool ExportThread::importDiscs( const DiscList& discs, const QString& partition )
{
	if ( isRunning() ) {
		Q_ASSERT( 0 );
		return false;
	}
	
	mTask = ExportThread::Import;
	mDiscs = discs;
	mPartition = partition;
	mStop = false;
	
	start();
	
	return true;
}

void ExportThread::emitCurrentProgressChanged( int value, int maximum, const QTime& remaining )
{
	emit currentProgressChanged( value, maximum, remaining );
}

void ExportThread::emitMessage( const QString& text )
{
	emit message( text );
}

void ExportThread::emitError( const QString& text )
{
	emit error( text );
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
	
	switch ( mTask ) {
		case ExportThread::Export:
			exportWorker();
			break;
		case ExportThread::Import:
			importWorker();
			break;
	}
	
	mCurrentExportThread = 0;
}

void ExportThread::exportWorker()
{
	QWBFS::Partition::Handle* handle = 0;
	
	emit globalProgressChanged( 0 );
	
	for ( int i = 0; i < mDiscs.count(); i++ ) {
		const Disc& disc = mDiscs[ i ];
		
		emit message( tr( "Exporting '%1'..." ).arg( disc.title ) );
		
		// delete handle if different partition
		if ( handle && handle->partition() != disc.origin ) {
			delete handle;
			handle = 0;
		}
		
		// get partition handle
		if ( !handle ) {
			QWBFS::Partition::Properties properties;
			properties.partition = disc.origin;
			
			handle = new QWBFS::Partition::Handle( properties );
			
			if ( !handle->isValid() ) {
				emit error( tr( "Can't initialize partition '%1'." ).arg( properties.partition ) );
				break;
			}
		}
		
		// get disc handle
		QWBFS::Partition::DiscHandle discHandle( *handle, disc.id );
		
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
}

void ExportThread::importWorker()
{
	// target partition handle
	QWBFS::Partition::Properties tpp;
	tpp.partition = mPartition;
	QWBFS::Partition::Handle tph( tpp );
	
	if ( !tph.isValid() ) {
		emit error( tr( "Invalid target handle, can't open partition '%1'." ).arg( tpp.partition ) );
		return;
	}
	
	// source partition handle
	QWBFS::Partition::Handle* sph = 0;
	
	emit globalProgressChanged( 0 );
	
	for ( int i = 0; i < mDiscs.count(); i++ ) {
		const Disc& disc = mDiscs[ i ];
		
		emit message( tr( "Importing '%1'..." ).arg( disc.title ) );
		
		// delete handle if different partition
		if ( sph && sph->partition() != disc.origin ) {
			delete sph;
			sph = 0;
		}
		
		void* fileHandle = wbfs_open_file_for_read( disc.origin.toLocal8Bit().data() );
		
        if ( !fileHandle ) {
			emit error( tr( "Unable to open disc file '%1'." ).arg( disc.origin ) );
		}
        else {
			u8 discInfo[7];
			wbfs_read_file( fileHandle, 6, discInfo );
			
			const bool haveDisc = QWBFS::Partition::DiscHandle( tph, disc.id ).isValid();
			
			// source is WBFS partition
			if ( memcmp( discInfo, "WBFS", 4 ) == 0 ) {
				wbfs_close_file( fileHandle );
				
				if ( !haveDisc ) {
					// get source partition handle
					if ( !sph ) {
						QWBFS::Partition::Properties spp;
						spp.partition = disc.origin;
						sph = new QWBFS::Partition::Handle( spp );
						
						if ( !sph->isValid() ) {
							emit error( tr( "Invalid source handle, can't open partition '%1'." ).arg( spp.partition ) );
							break;
						}
					}
					
					// get source disc handle
					const QWBFS::Partition::DiscHandle sdh( *sph, disc.id );
					
					if ( sdh.isValid() ) {
						if ( wbfs_add_disc( tph.ptr(), discRead_callback, sdh.ptr(), progress_callback, ONLY_GAME_PARTITION, 0 ) != 0 ) {
							emit error( tr( "Can't add disc '%1' in partition '%2'." ).arg( disc.title ).arg( tpp.partition ) );
						}
					}
					else {
						emit error( tr( "Invalid source handle, can't open the disc '%1'." ).arg( disc.title ) );
					}
				}
				else {
					emit error( tr( "Skipping add of existing disc '%1'." ).arg( disc.title ) );
				}
			}
			// source is file
			else
			{
				if ( !haveDisc ) {
					if ( wbfs_add_disc( tph.ptr(), wbfs_read_wii_file, fileHandle, progress_callback, ONLY_GAME_PARTITION, 0 ) != 0 ) {
						emit error( tr( "Can't add disc '%1' in partition '%2'." ).arg( disc.title ).arg( tpp.partition ) );
					}
				}
				else {
					emit error( tr( "Skipping add of existing disc '%1'." ).arg( disc.title ) );
				}
			}
        }
		
		emit globalProgressChanged( i +1 );
		
		{
			QMutexLocker locker( &mMutex );
			if ( mStop ) {
				break;
			}
		}
	}
	
	delete sph;
}

int ExportThread::discRead_callback( void* fp, u32 lba, u32 count, void* iobuf )
{
	int ret = wbfs_disc_read( (wbfs_disc_t*)fp, lba, (u8*)iobuf, count );
	static int num_fail = 0;
	
	if ( ret ) {
		if ( num_fail == 0 ) {
			mCurrentExportThread->emitError( "error reading lba probably the two wbfs don't have the same granularity. Ignoring...\n" );
		}
		
		if ( num_fail++ > 0x100 ) {
			mCurrentExportThread->emitError( "too many error giving up...\n" );
			return 1;
		}
	}
	else {
		num_fail = 0;
	}
	
	return 0;
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
