#include "Driver.h"
#include "qwbfsdriver/PartitionDiscHandle.h"

#include <libwbfs.h>

#include <QDir>
#include <QDebug>

using namespace QWBFS;

QMutex Driver::mMutex;
bool Driver::mForce = false;
QHash<QString, QWBFS::Partition::Handle> Driver::mHandles;
QWBFS::Driver* Driver::mCurrentDriver = 0;

Driver::Driver( QObject* parent, const QWBFS::Partition::Handle& partitionHandle )
	: QObject( parent )
{
	mProperties = partitionHandle.properties();
	mHandle = partitionHandle;
}

Driver::~Driver()
{
}

void Driver::setPartition( const QString& partition )
{
	mProperties.partition = partition;
}

QString Driver::partition() const
{
	return mHandle.isValid() ? mHandle.partition() : mProperties.partition;
}

QStringList Driver::lastErrors() const
{
	return mLastErrors;
}

bool Driver::open()
{
	if ( isOpen() ) {
		close();
	}
	
	mHandle = getHandle( mProperties.partition );
	return isOpen();
}

void Driver::close()
{
	if ( !isOpen() ) {
		return;
	}
	
	closeHandle( mHandle );
	mHandle = QWBFS::Partition::Handle();
}

bool Driver::isOpen() const
{
	return mHandle.isValid();
}

bool Driver::isWBFSPartition( const QString& fileName ) const
{
	QFile file( fileName );
	
	if ( !file.exists() ) {
		return false;
	}
	
	if ( file.open( QIODevice::ReadOnly ) ) {
		return file.read( 4 ).toLower() == "wbfs";
	}
	
	return false;
}

QWBFS::Partition::Handle Driver::handle() const
{
	return mHandle;
}

QWBFS::Partition::Status Driver::partitionStatus() const
{
	mLastErrors.clear();
	return QWBFS::Partition::Status( mHandle );
}

QWBFS::Model::DiscList Driver::partitionDiscList() const
{
	mLastErrors.clear();
	QWBFS::Model::DiscList discs;
	
	if ( isOpen() ) {
		const int count = wbfs_count_discs( mHandle.ptr() );
		const int header_size = 0x100;
		u8* header = (u8*)wbfs_ioalloc( header_size );
		u32 size;
		
		for ( int index = 0; index < count; index++ ) {
			if ( wbfs_get_disc_info( mHandle.ptr(), index, header, header_size, &size ) == 0 ) {
				discs << QWBFS::Model::Disc( header, &size, mHandle.partition() );
			}
			else {
				mLastErrors << tr( "Can't get disc informations for disc #%1." ).arg( index );
			}
		}

		wbfs_iofree( header );
	}
	else {
		mLastErrors << tr( "Can't get discs, partition '%1' not opened." ).arg( partition() );
	}
	
	return discs;
}

bool Driver::format()
{
	mLastErrors.clear();
	
	if ( isOpen() ) {
		close();
	}
	
	bool ok = false;
	
	{
		QWBFS::Partition::Properties properties = mProperties;
		properties.reset = true;
		
		const QWBFS::Partition::Handle handle( properties );
		const QWBFS::Partition::Status status( handle );
		
		qWarning() << status.size << status.used << status.free;
		
		ok = handle.isValid() && status.size > 0;
		
		if ( !ok ) {
			mLastErrors << tr( "Can't format partition '%1'." ).arg( properties.partition );
		}
	}
	
	return ok;
}

bool Driver::haveDisc( const QString& id ) const
{
	mLastErrors.clear();
	return QWBFS::Partition::DiscHandle( mHandle, id ).isValid();
}

bool Driver::renameDisc( const QString& id, const QString& name ) const
{
	mLastErrors.clear();
	
	if ( !isOpen() ) {
		mLastErrors << tr( "Can't rename disc #%1 to '%2' on partition '%3', partition not opened." ).arg( id ).arg( name ).arg( partition() );
		return false;
	}
	
	const bool ok = wbfs_ren_disc( mHandle.ptr(), (u8*)id.toLocal8Bit().data(), (u8*)name.toLocal8Bit().data() ) == 0;
	
	if ( !ok ) {
		mLastErrors << tr( "Can't rename disc #%1 to '%2'." ).arg( id ).arg( name );
	}
	
	return ok;
}

bool Driver::removeDisc( const QString& id ) const
{
	mLastErrors.clear();
	
	if ( isOpen() ) {
		if ( wbfs_rm_disc( mHandle.ptr(), (u8*)id.toLocal8Bit().data() ) == 0 ) {
			return true;
		}
	}
	
	mLastErrors << tr( "Can't delete disc #%1" ).arg( id );
	return false;
}

bool Driver::addDisc( const QWBFS::Model::Disc& disc, const QWBFS::Partition::Handle& sourcePartitionHandle ) const
{
	mLastErrors.clear();
	mCurrentDriver = const_cast<QWBFS::Driver*>( this );
	
	if ( !isOpen() ) {
		mLastErrors << tr( "Partition '%1' not opened." ).arg( partition() );
		return false;
	}
	
	// check if disc already exists
	if ( !disc.id.isEmpty() && haveDisc( disc.id ) ) {
		mLastErrors << tr( "The disc '%1' is already on partition '%1'." ).arg( disc.title ).arg( partition() );
		return false;
	}
	
	// source is WBFS partition
	if ( isWBFSPartition( disc.origin ) ) {
		if ( !sourcePartitionHandle.isValid() ) {
			mLastErrors << tr( "Invalid source handle, can't open partition '%1'." ).arg( sourcePartitionHandle.partition() );
			return false;
		}
		
		// get source disc handle
		const QWBFS::Partition::DiscHandle sdh( sourcePartitionHandle, disc.id );
		
		if ( sdh.isValid() ) {
			if ( wbfs_add_disc( mHandle.ptr(), discRead_callback, sdh.ptr(), progress_callback, ONLY_GAME_PARTITION, 0 ) == 0 ) {
				return true;
			}
			else {
				mLastErrors << tr( "Can't add disc '%1' in partition '%2'." ).arg( disc.title ).arg( sourcePartitionHandle.partition() );
			}
		}
		else {
			mLastErrors << tr( "Invalid source disc handle, can't open the disc '%1'." ).arg( disc.title );
		}
	}
	// source if a local file
	else {
		void* file = wbfs_open_file_for_read( disc.origin.toLocal8Bit().data() );
	
		if ( file ) {
			const bool ok = wbfs_add_disc( mHandle.ptr(), wbfs_read_wii_file, file, progress_callback, ONLY_GAME_PARTITION, 0 ) == 0;
			wbfs_close_file( file );
			
			if ( ok ) {
				return true;
			}
			else {
				mLastErrors << tr( "Can't add disc '%1'." ).arg( disc.origin );
			}
		}
		else {
			mLastErrors << tr( "Can't open disc '%1'." ).arg( disc.origin );
		}
	}
	
	return false;
}

bool Driver::extractDisc( const QString& id, const QString& path ) const
{
	mLastErrors.clear();
	mCurrentDriver = const_cast<QWBFS::Driver*>( this );
	
	if ( !isOpen() ) {
		mLastErrors << tr( "Partition '%1' not opened." ).arg( partition() );
		return false;
	}
	
	// get disc handle
	const QWBFS::Partition::DiscHandle discHandle( mHandle, id );
	
	if ( !discHandle.isValid() ) {
		mLastErrors << tr( "Can't extract the disc #%1, invalid handle." ).arg( id );
		return false;
	}
	
	const QString filePath = QDir::toNativeSeparators( QString( "%1/%2" ).arg( path ).arg( discHandle.isoName() ) );
	void* fileHandle = wbfs_open_file_for_write( filePath.toLocal8Bit().data() );
	bool ok = false;

	if ( fileHandle ) {
		// write a zero at the end of the iso to ensure the correct size
		// XXX should check if the game is DVD9..
		wbfs_file_reserve_space( fileHandle, ( discHandle.ptr()->p->n_wii_sec_per_disc /2 ) *0x8000ULL );
		const int ok = wbfs_extract_disc( discHandle.ptr(), wbfs_write_wii_file, fileHandle, progress_callback ) == 0;
		wbfs_close_file( fileHandle );
		
		if ( !ok ) {
			mLastErrors << tr( "Disc export failed '%1'." ).arg( id );
		}
	}
	else {
		mLastErrors << tr( "Can't open file for writing disc '%1'." ).arg( id );
	}
	
	return ok;
}

void Driver::setForceMode( bool force )
{
	QMutexLocker locker( &mMutex );
	mForce = force;
	wbfs_set_force_mode( mForce ? 1 : 0 );
}

bool Driver::forceMode()
{
	QMutexLocker locker( &mMutex );
	return mForce;
}

void Driver::addHandle( const QWBFS::Partition::Handle& handle )
{
	if ( !handle.isValid() ) {
		return;
	}
	
	QMutexLocker locker( &mMutex );
	mHandles[ handle.partition() ] = handle;
}

QWBFS::Partition::Handle Driver::getHandle( const QString& partition, bool* created )
{
	QMutexLocker locker( &mMutex );
	QWBFS::Partition::Handle handle = mHandles.value( partition );
	
	if ( created ) {
		*created = false;
	}
	
	if ( !handle.isValid() ) {
		const QWBFS::Partition::Handle _handle( partition );
		
		if ( _handle.isValid() ) {
			mHandles[ partition ] = _handle;
			handle = _handle;
			
			if ( created ) {
				*created = false;
			}
		}
	}
	
	return handle;
}

void Driver::closeHandle( const QWBFS::Partition::Handle& handle )
{
	QMutexLocker locker( &mMutex );
	mHandles.remove( handle.partition() );
}

int Driver::discRead_callback( void* fp, u32 lba, u32 count, void* iobuf )
{
	int ret = wbfs_disc_read( (wbfs_disc_t*)fp, lba, (u8*)iobuf, count );
	static int num_fail = 0;
	
	if ( ret ) {
		if ( num_fail == 0 ) {
			mCurrentDriver->mLastErrors << tr( "error reading lba probably the two wbfs don't have the same granularity. Ignoring..." );
		}
		
		if ( num_fail++ > 0x100 ) {
			mCurrentDriver->mLastErrors << tr( "too many error giving up..." );
			return 1;
		}
	}
	else {
		num_fail = 0;
	}
	
	return 0;
}

void Driver::progress_callback( int x, int max )
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
		d = (u32)( time( 0 ) -start_time );
		h = d /3600;
		m = ( d /60 ) %60;
		s = d %60;
		
		if ( mCurrentDriver ) {
			emit mCurrentDriver->currentProgressChanged( x, max, QTime( h, m, s ) );
		}
		
		return;
	}

	d = (u32)( time( 0 ) -start_time );

	if ( x != 0 ) {
		expected_total = (u32)( ( 3 *expected_total +d *max /x ) /4 );
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
	
	if ( mCurrentDriver ) {
		emit mCurrentDriver->currentProgressChanged( x, max, QTime( h, m, s ) );
	}
}
