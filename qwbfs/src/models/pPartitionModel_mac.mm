#include "pPartitionModel.h"

/*
http://www.datarecovery.com/hexcodes.asp
http://developer.apple.com/library/mac/#samplecode/VolumeToBSDNode/Listings/VolumeToBSDNode_c.html#//apple_ref/doc/uid/DTS10000434-VolumeToBSDNode_c-DontLinkElementID_4
*/

#if defined( Q_OS_MAC )
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IODVDMedia.h>

#include <QDebug>

// The macro is defined only since 10.4 so add it if needed
#ifndef IO_OBJECT_NULL
#define IO_OBJECT_NULL  ((io_object_t)0)
#endif
 
Boolean IsWholeMedia( io_service_t service )
{
	// Check that service is a IOMedia checking recursively the parents if needed and get its informations.
    Boolean isWholeMedia = false;
    io_name_t className;
    kern_return_t kernResult;
 
    if ( IOObjectConformsTo( service, kIOMediaClass ) ) {
        CFTypeRef wholeMedia = IORegistryEntryCreateCFProperty( service, CFSTR( kIOMediaWholeKey ), kCFAllocatorDefault, 0 );
		
        if ( !wholeMedia ) {
            qWarning() << "Could not retrieve Whole property.";
        }
        else {                                        
            isWholeMedia = CFBooleanGetValue( (const CFBooleanRef)wholeMedia );
            CFRelease( wholeMedia );
        }
    }
	
    if ( isWholeMedia ) {
        if ( IOObjectConformsTo( service, kIOCDMediaClass ) ) {
            fprintf(stdout, "is a CD\n");
        }
        else if ( IOObjectConformsTo( service, kIODVDMediaClass ) ) {
            fprintf(stdout, "is a DVD\n");
        }
        else {
            kernResult = IOObjectGetClass(service, className);
            fprintf(stdout, "is of class %s\n", className);
        }            
    }
 
    return isWholeMedia;
}

pPartitionModel::Partitions pPartitionModel::partitions() const
{
	pPartitionModel::Partitions partitions;
    OSStatus result = noErr;
    ItemCount volumeIndex;
 
    // Iterate across all mounted volumes using FSGetVolumeInfo. This will return nsvErr
    // (no such volume) when volumeIndex becomes greater than the number of mounted volumes.
    for ( volumeIndex = 1; result == noErr || result != nsvErr; volumeIndex++ ) {
        FSVolumeRefNum actualVolume;
        HFSUniStr255 volumeName;
        FSVolumeInfo volumeInfo;
		FSRef rootDirectory;
        
        bzero( (void*)&volumeInfo, sizeof(volumeInfo)) ;
        
		result = FSGetVolumeInfo( kFSInvalidVolumeRefNum, volumeIndex, &actualVolume, kFSVolInfoGettableInfo, &volumeInfo, &volumeName, &rootDirectory ); 
        
        if ( result == noErr ) {
            GetVolParmsInfoBuffer volumeParms;
            HParamBlockRec pb;
            
            pb.ioParam.ioNamePtr = NULL;
            pb.ioParam.ioVRefNum = actualVolume;
            pb.ioParam.ioBuffer = (Ptr)&volumeParms;
            pb.ioParam.ioReqCount = sizeof(volumeParms);
            
            // A version 4 GetVolParmsInfoBuffer contains the BSD node name in the vMDeviceID field.
            // It is actually a char * value. This is mentioned in the header CoreServices/CarbonCore/Files.h.
            result = PBHGetVolParmsSync( &pb );
			
            if ( result != noErr ) {
                qWarning() << QString( "PBHGetVolParmsSync returned %1." ).arg( result );
            }
            else {
				if ( volumeParms.vMServerAdr != 0 ) { // currently i don't know how check network attached drives from mapping etc so let only use native volumes
					continue;
				}
				
				if ( volumeParms.vMServerAdr == 0 ) { // The volume is local if vMServerAdr is 0. Network volumes won't have a BSD node name.
					CFMutableDictionaryRef matchingDict = IOBSDNameMatching( kIOMasterPortDefault, 0, (char*)volumeParms.vMDeviceID );
					
					if ( !matchingDict ) {
						qWarning() << "IOBSDNameMatching returned a NULL dictionary.";
					}
					else {
						io_service_t service = IOServiceGetMatchingService( kIOMasterPortDefault, matchingDict );    
					
						if ( service == IO_OBJECT_NULL ) {
							qWarning() << "IOServiceGetMatchingService returned IO_OBJECT_NULL.";
						}
						else {
							io_iterator_t iter;
							kern_return_t kernResult = IORegistryEntryCreateIterator( service, kIOServicePlane, kIORegistryIterateRecursively | kIORegistryIterateParents, &iter );
							
							if ( kernResult != KERN_SUCCESS ) {
								qWarning() << QString( "IORegistryEntryCreateIterator returned 0x%18x" ).arg( kernResult );
							}
							else if ( iter == IO_OBJECT_NULL ) {
								qWarning() << "IORegistryEntryCreateIterator returned a NULL iterator.";
							}
							else {
								Boolean isWholeMedia;
								
								// A reference on the initial service object is released in the do-while loop below, so add a reference to balance
								IOObjectRetain( service );
								
								do {
									isWholeMedia = IsWholeMedia( service );
									IOObjectRelease( service );
								} while ( ( service = IOIteratorNext( iter ) ) && !isWholeMedia );
								
								IOObjectRelease( iter );
							}
							
							IOObjectRelease( service );
						}
					}
				}
				
				/*HFSUniStr255 path;
				result = FSGetCatalogInfo( &rootDirectory, kFSCatInfoNone, 0, &path, 0, 0 );
				
				qWarning() << "path" << QString::fromUtf16( path.unicode, path.length );*/
				
				pPartitionModel::Partition partition;
		
				partition.label = QString::fromUtf16( volumeName.unicode, volumeName.length );
				partition.origin = QString( "/dev/%1" ).arg( (char*)volumeParms.vMDeviceID );
				partition.fileSystem = QString::null;
				partition.total = volumeInfo.totalBytes;
				partition.free = volumeInfo.freeBytes;
				partition.used = partition.total -partition.free;
				partition.fileSystem = volumeInfo.filesystemID == 0 ? "HFS/HFS+" : "";
				partition.fileSystemMark = volumeInfo.filesystemID; // seem to be bad :(
				partition.name = pPartitionModel::Partition::displayText(
					partition.origin, 
					partition.label, 
					partition.fileSystemMark, 
					QString::null, 
					QString::null );
				partition.extendedAttributes[ "REMOVABLE" ] = volumeParms.vMExtendedAttributes & bIsRemovable ? "1" : "0";
				partition.lastCheck = QDateTime::currentDateTime();
				
				partitions << partition;
            }
        }
    }
	
	return partitions;
}

#endif
