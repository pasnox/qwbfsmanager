#ifndef QWBFS_H
#define QWBFS_H

#include <QFlags>

namespace QWBFS
{
    enum EntryType {
        EntryTypeNone = 0x0,
        EntryTypeISO = 0x1,
        EntryTypeCISO = 0x2,
        EntryTypeWBFS = 0x4
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
        FileSystemTypeNative = 0x2
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
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QWBFS::EntryTypes )

#endif // QWBFS_H
