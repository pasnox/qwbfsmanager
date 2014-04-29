#ifndef QWBFS_H
#define QWBFS_H

#include <QFlags>
#include <QString>

#include "wiitdb/Covers.h"

#define _GNU_SOURCE 1 // dunno if it's needed but...
extern "C" {
	#include "lib-std.h"
}

class QPixmap;
class QSize;

class pNetworkAccessManager;

class FileSystemEntry;

namespace QWBFS
{
	enum EntryType {
		// 1. file types
		EntryTypeUnknown = FT_UNKNOWN,  // not analyzed yet
		
		EntryTypeIdDir = FT_ID_DIR,  // is a directory
		EntryTypeIdFST = FT_ID_FST,  // is a directory with a FST
		EntryTypeIdWBFS = FT_ID_WBFS,  // file is a WBFS
		EntryTypeIdGCISO = FT_ID_GC_ISO,  // file is a GC ISO image
		EntryTypeIdWiiISO = FT_ID_WII_ISO,  // file is a WII ISO image

		// special files
		EntryTypeIdDol = FT_ID_DOL,  // file is a DOL file
		EntryTypeIdCertBin = FT_ID_CERT_BIN,  // 'cert.bin' like file
		EntryTypeIdTikBin = FT_ID_TIK_BIN,  // 'ticket.bin' like file
		EntryTypeIdTmdBin = FT_ID_TMD_BIN,  // 'tmd.bin' like file
		EntryTypeIdHeadBin = FT_ID_HEAD_BIN,  // 'header.bin' like file
		EntryTypeIdBootBin = FT_ID_BOOT_BIN,  // 'boot.bin' like file
		EntryTypeIdFSTBin = FT_ID_FST_BIN,  // 'fst.bin' like file
		EntryTypeIdPatch = FT_ID_PATCH,  // wit patch file
		EntryTypeIdOther = FT_ID_OTHER,  // unknown file

		EntryTypeSpcMask = FT__SPC_MASK,  // mask of all special files
		EntryTypeIdMask = FT__ID_MASK,  // mask of all 'FT_ID_' values

		// 2. attributes
		EntryTypeAISO = FT_A_ISO,  // file is some kind of an ISO image
		EntryTypeAGCIso = FT_A_GC_ISO,  // file is some kind of a GC ISO image
		EntryTypeAWiiISO = FT_A_WII_ISO,  // file is some kind of a WII ISO image

		EntryTypeAWBFSDisc = FT_A_WDISC,  // flag: specific disc of a WBFS file
		EntryTypeAWDF = FT_A_WDF,  // flag: file is a packed WDF
		EntryTypeAWIA = FT_A_WIA,  // flag: file is a packed WIA
		EntryTypeACISO = FT_A_CISO,  // flag: file is a packed CISO
		EntryTypeARegFile = FT_A_REGFILE,  // flag: file is a regular file
		EntryTypeABlockDev = FT_A_BLOCKDEV,  // flag: file is a block device
		EntryTypeACharDev = FT_A_CHARDEV,  // flag: file is a block device
		EntryTypeASeekable = FT_A_SEEKABLE,  // flag: using of seek() is possible
		EntryTypeAWriting = FT_A_WRITING,  // is opened for writing
		EntryTypeAPartDir = FT_A_PART_DIR,  // FST is a partition

		EntryTypeAMask = FT__A_MASK,  // mask of all 'FT_A_' values

		// 3. mask of all 'FT_ values
		EntryTypeMask = FT__MASK,
		
		// QWBFS Extensions
		EntryTypeWiiWBFS = EntryTypeIdWBFS | EntryTypeAWBFSDisc,
		EntryTypeWiiISO = EntryTypeIdWiiISO,
		EntryTypeWiiCISO = EntryTypeAWiiISO | EntryTypeACISO
	};

    Q_DECLARE_FLAGS( EntryTypes, EntryType )
    
    enum EntryState {
		EntryStateNone = 0x0,
		EntryStateSuccess = 0x1,
		EntryStateFailed = 0x2
	};
    
    enum FileSystemType {
        FileSystemTypeNone = 0x0,
        FileSystemTypeWBFS = 0x1,
        FileSystemTypeNative = 0x2,
		FileSystemTypeStock = 0x3
    };
    
	enum Error {
		Ok = 0,
		PartitionNotOpened = -1,
		SourcePartitionNotOpened = -2,
		DiscReadFailed = -3,
		DiscWriteFailed = -4,
		DiscExtractFailed = -5,
		DiscAddFailed = -6,
		DiscConvertFailed = -7,
		DiscRenameFailed = -8,
		DiscFound = -9,
		DiscNotFound = -10,
		InvalidDiscIndex = -11,
		InvalidDiscID = -12,
		InvalidDisc = -13,
		CantDrive2Drive = -14,
		UnknownError = -1000
	};
	
	void init( int argc = 0, char** argv = 0 );
	void deInit();
	
	pNetworkAccessManager* networkManager();
	
	QString entryTypeToString( QWBFS::EntryType type );
	QString entryStateToString( QWBFS::EntryState state );
	QString entryRegionToString( char region, bool longFormat );
	
	QPixmap coverPixmap( QWBFS::WiiTDB::Scan scan, const QString& id, const QSize& size );
	QPixmap statePixmap( QWBFS::EntryState state, const QSize& size );
	
	FileSystemEntry createEntry( const QString& filePath );
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QWBFS::EntryTypes )

#endif // QWBFS_H
