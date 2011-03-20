#include "pPartition.h"

#include <QStringList>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

pPartition::pPartition( const QString& devicePath, bool checkValidity )
{
	if ( !checkValidity || ( checkValidity && isValidDevicePath( devicePath ) ) ) {
		mDevicePath = devicePath;
	}
}

bool pPartition::operator==( const pPartition& other ) const
{
	return devicePath() == other.devicePath();
}

void pPartition::setProperties( const QVariantMap& properties )
{
	mProperties = properties;
	mDevicePath.clear();
	
	if ( isWBFSPartition( devicePath() ) ) {
#if defined( Q_OS_MAC )
		mProperties[ PROPERTY_FILE_SYSTEM_ID ] = 0x25;
		mProperties[ "DAVolumeKind" ] = fileSystemIdToString( 0x25 );
#elif defined( Q_OS_LINUX )
#elif defined( Q_OS_WIN )
#endif
	}
	
	mProperties[ PROPERTY_DISPLAY_TEXT ] = generateDisplayText();
	updateLastChecked();
}

QVariantMap pPartition::properties() const
{
	return mProperties;
}

QVariant pPartition::property( pPartition::Property property ) const
{
#if defined( Q_OS_MAC )
	switch ( property ) {
		case Label:
			return mProperties.value( "DAVolumeName", value( "DAMediaName" ) ).toString().simplified();
		case DevicePath:
			return ( mDevicePath.isEmpty() ? QString( "/dev/%1" ).arg( value( "DAMediaBSDName" ) ) : mDevicePath ).simplified();
		case TotalSize:
			return mProperties.value( "DAMediaSize", -1 );
		case UsedSize:
			return mProperties.value( "DAMediaUsed", -1 );
		case FreeSize:
			return mProperties.value( "DAMediaFree", -1 );
		/*case DeviceType:
			return mProperties.value( PROPERTY_DEVICE_TYPE );*/
		case MountPoints:
			return mProperties.value( "DAVolumePath" ).toString().simplified();
		case FileSystem: {
			const QString fs = mProperties.value( "DAVolumeKind", value( "DAMediaContent" ) ).toString().toUpper().replace( "_", " " ).simplified();
			return fs.contains( "-" ) ? QObject::tr( QT_TRANSLATE_NOOP( "pPartition", "Unknown FS" ) ) : fs;
		}
		case FileSystemId:
			return mProperties.value( PROPERTY_FILE_SYSTEM_ID );
		case DeviceVendor:
			return mProperties.value( "DADeviceVendor" ).toString().simplified();
		case DeviceModel:
			return mProperties.value( "DADeviceModel" ).toString().simplified();
		case DisplayText:
			return mProperties.value( PROPERTY_DISPLAY_TEXT ).toString().simplified();
		case LastCheck:
			return mProperties.value( PROPERTY_LAST_CHECK );
	}
#elif defined( Q_OS_LINUX )
#elif defined( Q_OS_WIN )
#endif
	
	return QVariant();
}

QVariant pPartition::property( const QString& property ) const
{
	return mProperties.value( property );
}

void pPartition::updateSizes( qint64 total, qint64 free )
{
#if defined( Q_OS_MAC )
	mProperties[ "DAMediaSize" ] = total;
	mProperties[ "DAMediaUsed" ] = total -free;
	mProperties[ "DAMediaFree" ] = free;
#elif defined( Q_OS_LINUX )
#elif defined( Q_OS_WIN )
#endif
	updateLastChecked();
}

bool pPartition::isValid() const
{
	return isValidDevicePath( devicePath() );
}

bool pPartition::isCustom() const
{
	return devicePath() == mDevicePath;
}

QString pPartition::devicePath() const
{
	return property( pPartition::DevicePath ).toString();
}

void pPartition::updateLastChecked()
{
	mProperties[ PROPERTY_LAST_CHECK ] = QDateTime::currentDateTime();
}

QString pPartition::generateDisplayText() const
{
	const QString label = property( pPartition::Label ).toString().isEmpty() ? devicePath() : property( pPartition::Label ).toString();
	const QString vendorModel = QString( "%1 %2" )
		.arg( property( pPartition::DeviceVendor ).toString() )
		.arg( property( pPartition::DeviceModel ).toString() )
		.replace( "_", " " )
		.simplified()
		;
	
	return QString( "%1 (%2/%3)" )
		.arg( label )
		.arg( property( pPartition::FileSystem ).toString() )
		.arg( vendorModel.isEmpty() ? QObject::tr( QT_TRANSLATE_NOOP( "pPartition", "Unknown Vendor/Model" ) ) : vendorModel )
		.simplified()
		;
}

bool pPartition::isWBFSPartition( const QString& devicePath )
{
	QFile file( devicePath );
	
	if ( file.open( QIODevice::ReadOnly ) ) {
		return file.read( 4 ).toLower() == "wbfs";
	}
	
	return false;
}

bool pPartition::isValidDevicePath( const QString& devicePath )
{
	const QFileInfo file( devicePath );
	return !devicePath.trimmed().isEmpty() && file.exists() && !file.isDir();
}

QString pPartition::value( const QString& key ) const
{
	return mProperties.value( key ).toString();
}

QString pPartition::fileSystemIdToString( qint64 id, pPartition::Type type, bool full )
{
	typedef QPair<QString, QString> Info;
	typedef QList<Info> InfoList;
	QHash<qint64, InfoList> hash;
	
	if ( type == pPartition::CdRom ) {
		hash[ 0x00 ] << Info( "Unformatted", "NO-Partition empty partition-table entry" );
		hash[ 0x01 ] << Info( "CD_DA", "CD_DA" );
		hash[ 0x02 ] << Info( "CD_ROM_Mode_1", "CD_ROM_Mode_1" );
		hash[ 0x03 ] << Info( "CD_ROM_Mode_2", "CD_ROM_Mode_2" );
		hash[ 0x04 ] << Info( "CD_ROM_Mode_2_Form_1", "CD_ROM_Mode_2_Form_1" );
		hash[ 0x05 ] << Info( "CD_ROM_Mode_2_Form_2", "CD_ROM_Mode_2_Form_2" );
	}
	else {
		hash[ 0x00 ] << Info( "Unformatted", "NO-Partition empty partition-table entry" );
		hash[ 0x01 ] << Info( "FAT12", "DOS DOS 12-bit FAT" );
		hash[ 0x02 ] << Info( "Xenix", "XENIX root file system" );
		hash[ 0x03 ] << Info( "Xenix", "XENIX /usr file system (obsolete)" );
		hash[ 0x04 ] << Info( "FAT16", "DOS 16-bit FAT (up to 32M)" );
		hash[ 0x05 ] << Info( "Extended", "Extended DOS 3.3+ extended partition" );
		hash[ 0x06 ] << Info( "FAT16", "DOS 3.31+ Large File System (16-bit FAT, over 32M)" );
		hash[ 0x07 ] << Info( "NTFS", "WindowsNT NTFS" );
		hash[ 0x07 ] << Info( "HPFS", "OS/2 HPFS" );
		hash[ 0x07 ] << Info( "Unix", "Advanced Unix" );
		hash[ 0x07 ] << Info( "QNX", "QNX QNX" );
		hash[ 0x08 ] << Info( "OS/2", "OS/2 (v1.0-1.3 only)" );
		hash[ 0x08 ] << Info( "AIX", "AIX bootable partition, SplitDrive" );
		hash[ 0x08 ] << Info( "Commodore", "Commodore Commodore DOS" );
		hash[ 0x08 ] << Info( "DELL", "DELL DELL partition spanning multiple drives" );
		hash[ 0x09 ] << Info( "Coherent", "Coherent Coherent filesystem" );
		hash[ 0x09 ] << Info( "AIX", "AIX AIX data partition" );
		hash[ 0x0A ] << Info( "OPUS", "OPUS OPUS" );
		hash[ 0x0A ] << Info( "Coherent", "Coherent Coherent swap partition" );
		hash[ 0x0A ] << Info( "Boot Manager", "OS/2 OS/2 Boot Manager" );
		hash[ 0x0B ] << Info( "FAT32", "Windows95 with 32-bit FAT" );
		hash[ 0x0C ] << Info( "FAT32 (LBA)", "Windows95 with 32-bit FAT (using LBA-mode INT 13 extensions)" );
		hash[ 0x0E ] << Info( "FAT16 (LBA)", "VFAT logical-block-addressable VFAT (same as 06h but using LBA)" );
		hash[ 0x0F ] << Info( "Extended (LBA)", "Extended LBA Extended partition (same as 05h but using LBA)" );
		hash[ 0x10 ] << Info( "OPUS", "OPUS OPUS" );
		hash[ 0x11 ] << Info( "FAT12 Boot Manager", "FAT12 OS/2 Boot Manager hidden 12-bit FAT partition" );
		hash[ 0x12 ] << Info( "Compaq", "Compaq Compaq Diagnostics partition" );
		hash[ 0x14 ] << Info( "FAT16 Boot Manager", "FAT16 OS/2 Boot Manager hidden sub-32M 16-bit FAT partition" );
		hash[ 0x16 ] << Info( "FAT16 Boot Manager", "FAT16 OS/2 Boot Manager hidden over-32M 16-bit FAT partition" );
		hash[ 0x17 ] << Info( "HPFS Boot Manager", "OS/2 OS/2 Boot Manager hidden HPFS partition" );
		hash[ 0x17 ] << Info( "NTFS", "NTFS hidden NTFS partition" );
		hash[ 0x18 ] << Info( "", "ASTSuspend AST special Windows swap file (\"Zero-Volt Suspend\" partition)" );
		hash[ 0x19 ] << Info( "", "Willowtech Willowtech Photon coS" );
		hash[ 0x1B ] << Info( "FAT32", "Windows hidden Windows95 FAT32 partition" );
		hash[ 0x1C ] << Info( "FAT32 (LBA)", "Windows hidden Windows95 FAT32 partition (LBA-mode)" );
		hash[ 0x1E ] << Info( "VFAT (BLA)", "Windows hidden LBA VFAT partition" );
		hash[ 0x20 ] << Info( "", "Willowsoft Willowsoft Overture File System (OFS1)" );
		hash[ 0x21 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0x21 ] << Info( "", "FSo2" );
		hash[ 0x23 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0x24 ] << Info( "NEC", "NEC MS-DOS 3.x" );
		hash[ 0x25 ] << Info( "WBFS", "Wii Backup File System" ); // Not official, just took a free place to set it.
		hash[ 0x26 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0x31 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0x33 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0x34 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0x36 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0x38 ] << Info( "", "Theos" );
		hash[ 0x3C ] << Info( "PM", "PowerQuest PartitionMagic recovery partition" );
		hash[ 0x40 ] << Info( "", "VENIX 80286" );
		hash[ 0x41 ] << Info( "", "Personal RISC Boot" );
		hash[ 0x41 ] << Info( "", "PowerPC boot partition" );
		hash[ 0x42 ] << Info( "LDM", "Windows Logical Disk Manager" );
		hash[ 0x42 ] << Info( "SFS", "SFS(Secure File System) by Peter Gutmann" );
		hash[ 0x45 ] << Info( "", "EUMEL/Elan" );
		hash[ 0x46 ] << Info( "", "EUMEL/Elan" );
		hash[ 0x47 ] << Info( "", "EUMEL/Elan" );
		hash[ 0x48 ] << Info( "", "EUMEL/Elan" );
		hash[ 0x4F ] << Info( "", "Oberon Oberon boot/data partition" );
		hash[ 0x50 ] << Info( "", "OnTrack Disk Manager, read-only partition" );
		hash[ 0x51 ] << Info( "", "OnTrack Disk Manager, read/write partition" );
		hash[ 0x51 ] << Info( "Novell", "NOVELL" );
		hash[ 0x52 ] << Info( "CP/M", "CP/M" );
		hash[ 0x52 ] << Info( "", "Microport System V/386" );
		hash[ 0x53 ] << Info( "", "OnTrack Disk Manager, write-only partition???" );
		hash[ 0x54 ] << Info( "", "OnTrack Disk Manager (DDO)" );
		hash[ 0x55 ] << Info( "", "EZ-Drive EZ-Drive (see also INT 13/AH=FFh\"EZ-Drive\")" );
		hash[ 0x56 ] << Info( "", "GoldenBow GoldenBow VFeature" );
		hash[ 0x5C ] << Info( "", "Priam Priam EDISK" );
		hash[ 0x61 ] << Info( "", "SpeedStor" );
		hash[ 0x63 ] << Info( "Unix", "UnixSysV Unix SysV/386, 386/ix" );
		hash[ 0x63 ] << Info( "BSD", "Mach Mach, MtXinu BSD 4.3 on Mach" );
		hash[ 0x63 ] << Info( "HURD", "GNU-HURD GNU HURD" );
		hash[ 0x64 ] << Info( "Novell", "Novell Novell NetWare 286" );
		hash[ 0x64 ] << Info( "", "SpeedStore SpeedStore" );
		hash[ 0x65 ] << Info( "Novell", "Novell NetWare (3.11)" );
		hash[ 0x67 ] << Info( "Novell", "Novell" );
		hash[ 0x68 ] << Info( "Novell", "Novell" );
		hash[ 0x69 ] << Info( "NSS", "Novell NSS Volume" );
		hash[ 0x70 ] << Info( "DS", "DiskSecure DiskSecure Multi-Boot" );
		hash[ 0x71 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0x73 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0x74 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0x75 ] << Info( "PC/IX", "PC/IX PC/IX" );
		hash[ 0x76 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0x7E ] << Info( "F.I.X.", "F.I.X. F.I.X." );
		hash[ 0x80 ] << Info( "Minix", "Minix Minix v1.1 - 1.4a" );
		hash[ 0x81 ] << Info( "Minix", "Minix Minix v1.4b+" );
		hash[ 0x81 ] << Info( "Linux", "Linux Linux" );
		hash[ 0x81 ] << Info( "", "Mitac Mitac Advanced Disk Manager" );
		hash[ 0x82 ] << Info( "Linux", "Linux/Swap Linux Swap partition" );
		hash[ 0x82 ] << Info( "", "Prime Prime" );
		hash[ 0x82 ] << Info( "Solaris", "Solaris Solaris (Unix)" );
		hash[ 0x83 ] << Info( "Linux", "Linux Linux native file system (ext2fs/xiafs)" );
		hash[ 0x84 ] << Info( "", "DOS OS/2-renumbered type 04h partition (hiding DOS C: drive)" );
		hash[ 0x85 ] << Info( "Linux", "Linux Linux EXT" );
		hash[ 0x86 ] << Info( "FAT16", "FAT16 FAT16 volume/stripe set (Windows NT)" );
		hash[ 0x87 ] << Info( "HPFS", "HPFS HPFS Fault-Tolerant mirrored partition" );
		hash[ 0x87 ] << Info( "NTFS", "NTFS NTFS volume/stripe set" );
		hash[ 0x8E ] << Info( "LVM", "Linux Logical Volume Manager" );
		hash[ 0x93 ] << Info( "", "Amoeba Amoeba file system" );
		hash[ 0x94 ] << Info( "", "Amoeba Amoeba bad block table" );
		hash[ 0x98 ] << Info( "", "Datalight Datalight ROM-DOS SuperBoot" );
		hash[ 0x99 ] << Info( "", "Mylex Mylex EISA SCSI" );
		hash[ 0xA0 ] << Info( "", "Phoenix Phoenix NoteBIOS Power Management \"Save-to-Disk\" partition" );
		hash[ 0xA1 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0xA3 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0xA4 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0xA5 ] << Info( "FreeBSD", "FreeBSD FreeBSD, BSD/386" );
		hash[ 0xA6 ] << Info( "OpenBSD", "OpenBSD OpenBSD" );
		hash[ 0xA7 ] << Info( "USF", "Apple Rhapsody UFS" );
		hash[ 0xA8 ] << Info( "UFS", "Apple UFS" );
		hash[ 0xA9 ] << Info( "NetBSD", "NetBSD NetBSD (http://www.netbsd.org/)" );
		hash[ 0xAB ] << Info( "Boot", "Apple Boot" );
		hash[ 0xAE ] << Info( "Encryped", "Apple Encrypted" );
		hash[ 0xAF ] << Info( "HFS", "Apple HFS" );
		hash[ 0xB1 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0xB3 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0xB4 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0xB6 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0xB6 ] << Info( "FAT16", "Windows Windows NT mirror set (master), FAT16 file system" );
		hash[ 0xB7 ] << Info( "", "BSDI BSDI file system (secondarily swap)" );
		hash[ 0xB7 ] << Info( "NTFS", "Windows Windows NT mirror set (master), NTFS file system" );
		hash[ 0xB8 ] << Info( "", "BSDI BSDI swap partition (secondarily file system)" );
		hash[ 0xBE ] << Info( "", "Solaris Solaris boot partition" );
		hash[ 0xC0 ] << Info( "", "CTOS CTOS" );
		hash[ 0xC0 ] << Info( "", "DR-DOS DR DOS/DR-DOS/Novell DOS secured partition" );
		hash[ 0xC1 ] << Info( "", "DR-DOS6.0 DR DOS 6.0 LOGIN.EXE-secured 12-bit FAT partition" );
		hash[ 0xC4 ] << Info( "", "DR-DOS6.0 DR DOS 6.0 LOGIN.EXE-secured 16-bit FAT partition" );
		hash[ 0xC6 ] << Info( "", "DR-DOS6.0 DR DOS 6.0 LOGIN.EXE-secured Huge partition" );
		hash[ 0xC6 ] << Info( "", "corrupted corrupted FAT16 volume/stripe set (Windows NT)" );
		hash[ 0xC6 ] << Info( "FAT16", "Windows Windows NT mirror set (slave), FAT16 file system" );
		hash[ 0xC7 ] << Info( "Boot", "Syrinx Syrinx Boot" );
		hash[ 0xC7 ] << Info( "NTFS", "corrupted corrupted NTFS volume/stripe set" );
		hash[ 0xC7 ] << Info( "NTFS", "Windows Windows NT mirror set (slave), NTFS file system" );
		hash[ 0xCB ] << Info( "FAT32", "DR-DOS Reserved for DR DOS/DR-DOS/OpenDOS secured FAT32" );
		hash[ 0xCC ] << Info( "FAT32 (LBA)", "DR-DOS Reserved for DR DOS/DR-DOS secured FAT32 (LBA)" );
		hash[ 0xCE ] << Info( "FAT16 (LBA)", "DR-DOS Reserved for DR DOS/DR-DOS secured FAT16 (LBA)" );
		hash[ 0xD0 ] << Info( "FAT12", "Multiuser Multiuser DOS secured FAT12" );
		hash[ 0xD1 ] << Info( "FAT12", "Old-FAT12 Old Multiuser DOS secured FAT12" );
		hash[ 0xD4 ] << Info( "FAT16", "Old-FAT16 Old Multiuser DOS secured FAT16 (<= 32M)" );
		hash[ 0xD5 ] << Info( "Extended", "Old-Ext Old Multiuser DOS secured extended partition" );
		hash[ 0xD6 ] << Info( "FAT16", "Old-FAT16 Old Multiuser DOS secured FAT16 (> 32M)" );
		hash[ 0xD8 ] << Info( "CP/M-86", "CP/M-86 CP/M-86" );
		hash[ 0xDB ] << Info( "CP/M", "CP/M CP/M, Concurrent CP/M, Concurrent DOS" );
		hash[ 0xDB ] << Info( "CTOS", "CTOS CTOS (Convergent Technologies OS)" );
		hash[ 0xE1 ] << Info( "Extended", "SpeedStor SpeedStor 12-bit FAT extended partition" );
		hash[ 0xE2 ] << Info( "", "DOS DOS read-only (Florian Painke's XFDISK 1.0.4)" );
		hash[ 0xE3 ] << Info( "", "DOS DOS read-only" );
		hash[ 0xE3 ] << Info( "", "Storage Storage Dimensions" );
		hash[ 0xE4 ] << Info( "Extended", "SpeedStor SpeedStor 16-bit FAT extended partition" );
		hash[ 0xE5 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0xE6 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0xEB ] << Info( "BFS", "BeOS BeOS BFS (BFS1)" );
		hash[ 0xF1 ] << Info( "", "Storage Storage Dimensions" );
		hash[ 0xF2 ] << Info( "", "DOS DOS 3.3+ secondary partition" );
		hash[ 0xF3 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0xF4 ] << Info( "", "SpeedStor SpeedStor" );
		hash[ 0xF4 ] << Info( "", "Storage Storage Dimensions" );
		hash[ 0xF5 ] << Info( "", "Prologue Prologue" );
		hash[ 0xF6 ] << Info( "", "[reserved] officially listed as reserved" );
		hash[ 0xFB ] << Info( "VMware", "VMWARE vmware partition" );
		hash[ 0xFD ] << Info( "Linux", "Linux RAID" );
		hash[ 0xFE ] << Info( "", "LANstep LANstep" );
		hash[ 0xFE ] << Info( "PS/2-IML", "PS/2-IML IBM PS/2 IML (Initial Microcode Load) partition" );
		hash[ 0xFF ] << Info( "Xenix", "Xenix Xenix bad block table" );
		//hash[ 0xFM ] << Info( "VMware", "VMware VMware raw partition" );
	}
	
	QStringList values;
	
	foreach ( const Info& info, hash.value( id ) ) {
		values << ( full ? info.second : info.first );
	}
	
	return values.isEmpty() ? QObject::tr( QT_TRANSLATE_NOOP( "pPartition", "Unknown FS" ) ) : values.join( ", " ).replace( "_", " " );
}
