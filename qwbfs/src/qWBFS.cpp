#include "qWBFS.h"

#include <libwbfs.h>

#include <QDebug>

// Properties

qWBFS::Properties::Properties()
{
	force = false;
	reset = false;
	disk = QString::null;
	partition = QString::null;
	lastError = qWBFS::NoError;
}

// qWBFSHandle

qWBFS::qWBFSHandle::qWBFSHandle( const qWBFS::Properties& properties )
{
#ifdef Q_OS_WIN
	if ( properties.partition.length() != 1 ) {
		properties.lastError = qWBFS::InvalidPartition;
	}
#endif
	mHandle = wbfs_try_open( properties.disk.toLocal8Bit().data(), properties.partition.toLocal8Bit().data(), properties.reset ? 1 : 0 );
}

qWBFS::qWBFSHandle::~qWBFSHandle()
{
	if ( mHandle ) {
		wbfs_close( mHandle );
	}
}

bool qWBFS::qWBFSHandle::isValid() const
{
	return mHandle;
}

wbfs_t* qWBFS::qWBFSHandle::ptr() const
{
	return mHandle;
}

// PartitionStatus

qWBFS::PartitionStatus::PartitionStatus( const qWBFS::qWBFSHandle& handle )
{
	const u32 blockCount = handle.isValid() ? wbfs_count_usedblocks( handle.ptr() ) : 0;
	size = handle.isValid() ? (double)handle.ptr()->n_wbfs_sec *handle.ptr()->wbfs_sec_sz : 0;
	used = handle.isValid() ? (double)( handle.ptr()->n_wbfs_sec -blockCount ) *handle.ptr()->wbfs_sec_sz : 0;
	free = handle.isValid() ? (double)(blockCount) *handle.ptr()->wbfs_sec_sz : 0;
}
	
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

DiscList qWBFS::discs() const
{
	const qWBFSHandle handle( mProperties );
	DiscList discs;
	
	if ( handle.isValid() ) {
		mProperties.lastError = qWBFS::NoError;
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
		mProperties.lastError = qWBFS::InvalidPartition;
	}
	
	return discs;
}

qWBFS::PartitionStatus qWBFS::partitionStatus() const
{
	return qWBFS::PartitionStatus( qWBFSHandle( mProperties ) );
}
