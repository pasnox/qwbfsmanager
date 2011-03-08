#include "pPartitionModel.h"

#if defined( Q_OS_WIN )

// in case of need
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

// force use of xxxxA version of functon instead of xxxxxxW to avoid to deal with boring wchar_t
#undef UNICODE

#include <qt_windows.h>

#define ASCII_CHAR_A	65
#define ASCII_CHAR_Z	90

pPartitionModel::Partitions pPartitionModel::windowsPartitions() const
{
	pPartitionModel::Partitions partitions;
	
	// may need a better way to list available partitions...
	for ( char i = ASCII_CHAR_A; i <= ASCII_CHAR_Z; i++ ) {
		QString drive = QString( "%1:" ).arg( QChar( i ) );
		HRESULT hr = GetDriveType( qPrintable( drive ) );
		pPartitionModel::Partition partition;
		
		drive.append( "\\" );
		
		switch ( hr ) {
			case DRIVE_UNKNOWN:
				partition.device = pPartitionModel::Partition::Unknown;
				break;
			case DRIVE_NO_ROOT_DIR:
				partition.device = pPartitionModel::Partition::NotMounted;
				continue;
			case DRIVE_REMOVABLE:
				partition.device = pPartitionModel::Partition::Removable;
				break;
			case DRIVE_FIXED:
				partition.device = pPartitionModel::Partition::Fixed;
				break;
			case DRIVE_REMOTE:
				partition.device = pPartitionModel::Partition::Remote;
				break;
			case DRIVE_CDROM:
				partition.device = pPartitionModel::Partition::CdRom;
				break;
			case DRIVE_RAMDISK:
				partition.device = pPartitionModel::Partition::RamDisk;
				break;
		}
		
		char volume[ MAX_PATH +1 ]; // volumne name
		char fs[ MAX_PATH +1 ]; // file system type ( FAT/NTFS...)
		ulong serial; // partition serial
		ulong max; // max filename length
		ulong flags; // flags
		qint64 available; // user quota
		qint64 total; // system total
		qint64 free; // system free
		
		if ( !GetVolumeInformation(
			qPrintable( drive ),
			volume,
			MAX_PATH +1,
			&serial,
			&max,
			&flags,
			fs,
			MAX_PATH +1 ) ) {
			qWarning( "GetVolumeInformation Error: %i", i );
			continue;
		}
		
		if ( !GetDiskFreeSpaceEx(
			qPrintable( drive ),
			(PULARGE_INTEGER)&available,
			(PULARGE_INTEGER)&total,
			(PULARGE_INTEGER)&free ) ) {
			qWarning( "GetDiskFreeSpaceEx Error: %i", i );
			continue;
		}
		
		partition.label = QString::fromLocal8Bit( volume );
		partition.origin = drive;
		partition.type = fs;
		partition.ctype = 0;
		partition.total = total;
		partition.free = free;
		partition.used = total -free;
		partition.lastCheck = QDateTime::currentDateTime();
		
		partitions << partition;
	}
	
	return partitions;
}

#endif
