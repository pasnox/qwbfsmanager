#include "pPartitionModel.h"

#include <FreshCore/pWinHelpers>

#include <qt_windows.h>
#include <QStringList>
#include <QDebug>

#include <tchar.h>

void pPartitionModel::platformInit()
{
	emit layoutAboutToBeChanged();
	mData = 0;
	emit layoutChanged();
}

void pPartitionModel::platformDeInit()
{
}

void pPartitionModel::platformUpdate()
{
	// Code commented as the linux implementation has the concept of live auto update, thanks to UDev librarie :)
	/*const QStringList partitions = customPartitions();
	
	emit layoutAboutToBeChanged();
	delete (DisksSession*)mData;
	mPartitions.clear();
	foreach ( const QString& partition, partitions ) {
		mPartitions << pPartition( partition );
	}
	mData = new DisksSession( this );
	emit layoutChanged();*/
	
	const QStringList partitions = customPartitions();
	
	emit layoutAboutToBeChanged();
	
	mPartitions.clear();
	foreach ( const QString& partition, partitions ) {
		mPartitions << pPartition( partition );
	}
	
	mData = 0;
	
	TCHAR drives[ MAX_PATH +1 ];
	GetLogicalDriveStrings( MAX_PATH +1, drives );
	TCHAR* drive = drives;
	
	while ( *drive ) {
		/*HRESULT hr = GetDriveType( drive );
		
		switch ( hr ) {
			case DRIVE_UNKNOWN:
				partition.device = pPartition::Unknown;
				break;
			case DRIVE_NO_ROOT_DIR:
				partition.device = pPartition::NotMounted;
				continue;
			case DRIVE_REMOVABLE:
				partition.device = pPartition::Removable;
				break;
			case DRIVE_FIXED:
				partition.device = pPartition::Fixed;
				break;
			case DRIVE_REMOTE:
				partition.device = pPartition::Remote;
				break;
			case DRIVE_CDROM:
				partition.device = pPartition::CdRom;
				break;
			case DRIVE_RAMDISK:
				partition.device = pPartition::RamDisk;
				break;
		}*/
		
		TCHAR volume[ MAX_PATH +1 ] = {0}; // volumne name
		TCHAR fs[ MAX_PATH +1 ] = {0}; // file system type ( FAT/NTFS...)
		ulong serial; // partition serial
		ulong max; // max filename length
		ulong flags; // flags
		qint64 available = -1; // user quota
		qint64 total = -1; // system total
		qint64 free = -1; // system free
		
		if ( GetVolumeInformation(
			drive,
			volume,
			MAX_PATH +1,
			&serial,
			&max,
			&flags,
			fs,
			MAX_PATH +1 ) ) {
			if ( !GetDiskFreeSpaceEx(
				drive,
				(PULARGE_INTEGER)&available,
				(PULARGE_INTEGER)&total,
				(PULARGE_INTEGER)&free ) ) {
				qWarning( "GetDiskFreeSpaceEx Error: %s", qPrintable( TCHARToQString( drive ) ) );
			}
		}
		else {
			qWarning( "GetVolumeInformation Error: %s", qPrintable( TCHARToQString( drive ) ) );
		}
		
		pPartition partition;
		QVariantMap properties;
		
		properties[ "LABEL" ] = TCHARToQString( volume );
		properties[ "DEVICE" ] = TCHARToQString( drive );
		properties[ "MOUNT_POINTS" ] = TCHARToQString( drive );
		properties[ "FS_TYPE" ] = TCHARToQString( fs );
		properties[ "FS_TYPE_ID" ] = 0;
		
		if ( pPartition::isWBFSPartition( TCHARToQString( drive ) ) ) {
			properties[ "FS_TYPE_ID" ] = 0x25;
			properties[ "FS_TYPE" ] = pPartition::fileSystemIdToString( 0x25 );
		}
		
		partition.setProperties( properties );
		partition.updateSizes( total, free );
		
		mPartitions << partition;
		
		drive = &drive[ _tcslen( drive ) +1 ];
	}
	
	emit layoutChanged();
}
