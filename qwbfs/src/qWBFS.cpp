#include "qWBFS.h"

#include <libwbfs.h>

#include <QDebug>
	
// qWBFS

qWBFS::qWBFS( QObject* parent )
	: QObject( parent )
{
}

qWBFS::~qWBFS()
{
}

void qWBFS::setForceMode( bool force )
{
	mProperties.force = force;
	
	if ( force ) {
		wbfs_set_force_mode( 1 );
	}
	else {
		wbfs_set_force_mode( 0 );
	}
}

bool qWBFS::forceMode() const
{
	return mProperties.force;
}

void qWBFS::setDisk( const QString& disk )
{
	mProperties.disk = disk;
}

QString qWBFS::disk() const
{
	return mProperties.disk;
}

void qWBFS::setPartition( const QString& partition )
{
	mProperties.partition = partition;
}

QString qWBFS::partition() const
{
	return mProperties.partition;
}

QString qWBFS::lastError() const
{
	return mLastError;
}

DiscList qWBFS::discs() const
{
	mLastError.clear();
	const QWBFS::Partition::Handle handle( mProperties );
	DiscList discs;
	
	if ( handle.isValid() ) {
		const int count = wbfs_count_discs( handle.ptr() );
		const int header_size = 0x100;
		u8* header = (u8*)wbfs_ioalloc( header_size );
		u32 size;
		
		for ( int index = 0; index < count; index++ ) {
			if ( wbfs_get_disc_info( handle.ptr(), index, header, header_size, &size ) == 0 ) {
				discs << Disc( header, &size, mProperties.partition );
			}
		}

		wbfs_iofree( header );
	}
	else {
		mLastError = tr( "Invalid partition handle, can't open '%1'." ).arg( mProperties.partition );
	}
	
	return discs;
}

QWBFS::Partition::Status qWBFS::partitionStatus() const
{
	return QWBFS::Partition::Status( QWBFS::Partition::Handle( mProperties ) );
}

bool qWBFS::renameDisc( const QString& id, const QString& name )
{
	const QWBFS::Partition::Handle handle( mProperties );
	mLastError.clear();
	
	if ( !handle.isValid() ) {
		mLastError = tr( "Invalid partition handle, can't rename the disc #%1 to '%2' on '%3'." ).arg( id ).arg( name ).arg( mProperties.partition );
		return false;
	}
	
	const bool ok = wbfs_ren_disc( handle.ptr(), (u8*)id.toLocal8Bit().data(), (u8*)name.toLocal8Bit().data() ) == 0;
	
	if ( !ok ) {
		mLastError = tr( "Can't rename disc #%1 to '%2' on '%3'." ).arg( id ).arg( name ).arg( mProperties.partition );
	}
	
	return ok;
}

bool qWBFS::format()
{
	mLastError.clear();
	mProperties.reset = true;
	{
		if ( !QWBFS::Partition::Handle( mProperties ).isValid() ) {
			mLastError = tr( "Invalid partition handle, can't open '%1'." ).arg( mProperties.partition );
		}
	}
	mProperties.reset = false;
	return partitionStatus().size > 0;
}
