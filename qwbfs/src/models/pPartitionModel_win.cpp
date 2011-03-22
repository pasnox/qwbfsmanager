#include "pPartitionModel.h"

#define ASCII_CHAR_A 65
#define ASCII_CHAR_Z 90

#ifdef UNICODE
#define WCHAR_T               wchar_t
#define QStringToTCHAR(x)     (wchar_t*) x.utf16()
#define PQStringToTCHAR(x)    (wchar_t*) x->utf16()
#define TCHARToQString(x)     QString::fromUtf16((ushort*)(x))
#define TCHARToQStringN(x,y)  QString::fromUtf16((ushort*)(x),(y))
#else
#define WCHAR_T               char
#define QStringToTCHAR(x)     x.local8Bit().constData()
#define PQStringToTCHAR(x)    x->local8Bit().constData()
#define TCHARToQString(x)     QString::fromLocal8Bit((x))
#define TCHARToQStringN(x,y)  QString::fromLocal8Bit((x),(y))
#endif /*UNICODE*/

// in case of need
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <qt_windows.h>
#include <QStringList>
#include <QDebug>

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
	
	// may need a better way to list available partitions...
	for ( char i = ASCII_CHAR_A; i <= ASCII_CHAR_Z; i++ ) {
		QString drive = QString( "%1:" ).arg( QChar( i ) );
		HRESULT hr = GetDriveType( QStringToTCHAR( drive ) );
		
		drive.append( "\\" );
		
		/*switch ( hr ) {
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
		
		WCHAR_T volume[ MAX_PATH +1 ]; // volumne name
		WCHAR_T fs[ MAX_PATH +1 ]; // file system type ( FAT/NTFS...)
		ulong serial; // partition serial
		ulong max; // max filename length
		ulong flags; // flags
		qint64 available; // user quota
		qint64 total; // system total
		qint64 free; // system free
		
		if ( !GetVolumeInformation(
			QStringToTCHAR( drive ),
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
			QStringToTCHAR( drive ),
			(PULARGE_INTEGER)&available,
			(PULARGE_INTEGER)&total,
			(PULARGE_INTEGER)&free ) ) {
			qWarning( "GetDiskFreeSpaceEx Error: %i", i );
			continue;
		}
		
		pPartition partition;
		QVariantMap properties;
		
		properties[ "LABEL" ] = TCHARToQString( volume );
		properties[ "DEVICE" ] = drive;
		properties[ "MOUNT_POINTS" ] = drive;
		properties[ "FS_TYPE" ] = TCHARToQString( fs );
		properties[ "FS_TYPE_ID" ] = 0;
		
		partition.setProperties( properties );
		partition.updateSizes( total, free );
		
		mPartitions << partition;
	}
	
	emit layoutChanged();
}
