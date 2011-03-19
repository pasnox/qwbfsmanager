#include "pPartitionModel.h"

/*
http://www.datarecovery.com/hexcodes.asp
http://developer.apple.com/library/mac/#samplecode/VolumeToBSDNode/Listings/VolumeToBSDNode_c.html#//apple_ref/doc/uid/DTS10000434-VolumeToBSDNode_c-DontLinkElementID_4
http://stackoverflow.com/questions/1515068/list-all-drives-partitions-and-get-dev-rdisc-device-with-cocoa
*/

#if defined( Q_OS_MAC )
#import <Foundation/Foundation.h>
#import <DiskArbitration/DiskArbitration.h>
#import <CoreFoundation/CFDictionary.h>

#include <QDebug>

/*#include <sys/param.h>

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

QVariant toQVariant( CFTypeRef r );

void mergeMaps( const QVariantMap& in, QVariantMap& out )
{
	foreach ( const QString& key, in.keys() ) {
		if ( out.contains( key ) ) {
			continue;
		}
		
		out[ key ] = in[ key ];
	}
}*/

QVariant toQVariant( CFTypeRef r );

QVariantMap toQVariantMap( CFDictionaryRef dict )
{
	QVariantMap map;
	
	if ( dict ) {
		const CFIndex count = CFDictionaryGetCount( dict );
		const void* keys[ count ];
		const void* values[ count ];
		
		CFDictionaryGetKeysAndValues( dict, keys, values );
		
		for ( CFIndex i = 0; i < count; i++ ) {
			const QVariant key = toQVariant( (CFTypeRef)keys[ i ] );
			const QVariant value = toQVariant( (CFTypeRef)values[ i ] );
			
			map[ key.toString() ] = value;
		}
	}
	
	return map;
}

QVariantList toQVariantList( CFArrayRef array )
{
	QVariantList list;
	
	if ( array ) {
		const CFIndex count = CFArrayGetCount( array );
		
		for ( CFIndex i = 0; i < count; i++ ) {
			list << toQVariant( CFArrayGetValueAtIndex( array, i ) );
		}
	}
	
	return list;
}

QVariant toQVariant( CFStringRef string )
{
	if ( string ) {
		const CFIndex length = 2 *( CFStringGetLength( string ) +1 ); // Worst case for UTF8
		char buffer[ length ];
		
		if ( CFStringGetCString( string, buffer, length, kCFStringEncodingUTF8 ) ) {
			return QString::fromUtf8( buffer );
		}
		else {
			qWarning() << Q_FUNC_INFO << "CFStringRef conversion failed";
		}
	}
	
	return QVariant();
}

QVariant toQVariant( CFBooleanRef value )
{
	return value ? (bool)CFBooleanGetValue( value ) : QVariant();
}

QVariant toQVariant( CFNumberRef number )
{
	switch ( CFNumberGetType( number ) ) {
		case kCFNumberSInt8Type:
		case kCFNumberSInt16Type:
		case kCFNumberSInt32Type:
		case kCFNumberSInt64Type:
		case kCFNumberCharType:
		case kCFNumberShortType:
		case kCFNumberIntType:
		case kCFNumberLongType:
		case kCFNumberLongLongType:
		case kCFNumberCFIndexType:
		case kCFNumberNSIntegerType: {
			qint64 value = 0;
			if ( CFNumberGetValue( number, kCFNumberSInt64Type, &value ) ) {
				return value;
			}
			break;
		}
		case kCFNumberFloat32Type:
		case kCFNumberFloat64Type:
		case kCFNumberFloatType:
		case kCFNumberDoubleType:
		case kCFNumberCGFloatType: {
			qreal value = 0;
			if ( CFNumberGetValue( number, kCFNumberFloat64Type, &value ) ) {
				return value;
			}
			break;
		}
	}
	
	return QVariant();
}

QVariant toQVariant( CFDataRef _data )
{
	if ( _data ) {
		const CFIndex size = CFDataGetLength( _data );
		return QByteArray( (const char*)CFDataGetBytePtr( _data ), size );
	}
	
	return QVariant();
}

QVariant toQVariant( CFURLRef url )
{
	if ( url ) {
		CFStringRef string = CFURLCopyFileSystemPath( url, kCFURLPOSIXPathStyle );
		QVariant variant = toQVariant( string );
		CFRelease( string );
		return variant;
	}
	
	return QVariant();
}

QVariant toQVariant( CFUUIDRef uuid )
{
	if ( uuid ) {
		CFStringRef string = CFUUIDCreateString( kCFAllocatorDefault, uuid );
		QVariant variant = toQVariant( string );
		CFRelease( string );
		return variant;
	}
	
	return QVariant();
}

QVariant toQVariant( CFTypeRef ref )
{
	const CFTypeID id = CFGetTypeID( ref );
	
	if ( id == CFStringGetTypeID() ) {
		return toQVariant( (CFStringRef)ref );
	}
	else if ( id == CFBooleanGetTypeID() ) {
		return toQVariant( (CFBooleanRef)ref );
	}
	else if ( id == CFBundleGetTypeID() ) {
		return toQVariant( CFBundleGetIdentifier( (CFBundleRef)ref ) );
	}
	else if ( id == CFNumberGetTypeID() ) {
		return toQVariant( (CFNumberRef)ref );
	}
	else if ( id == CFDictionaryGetTypeID() ) {
		return toQVariantMap( (CFDictionaryRef)ref );
	}
	else if ( id == CFArrayGetTypeID() ) {
		return toQVariantList( (CFArrayRef)ref );
	}
	else if ( id == CFDataGetTypeID() ) {
		return toQVariant( (CFDataRef)ref );
	}
	else if ( id == CFURLGetTypeID() ) {
		return toQVariant( (CFURLRef)ref );
	}
	else if ( id == CFUUIDGetTypeID() ) {
		return toQVariant( (CFUUIDRef)ref );
	}
	
	qWarning() << Q_FUNC_INFO << "Unknow ID" << id;
	CFShow( ref );
	
	return QVariant();
}

/*Boolean IsWholeMedia( io_service_t service )
{
	// Check that service is a IOMedia checking recursively the parents if needed and get its informations.
    Boolean isWholeMedia = false;
	
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
	
    return isWholeMedia;
}

void wholeMedia( io_service_t service, io_iterator_t iter, pPartition& partition )
{
	Boolean isWhole;
	
	// A reference on the initial service object is released in the do-while loop below, so add a reference to balance
	IOObjectRetain( service );
	
	do {
		isWhole = IsWholeMedia( service );
		
		CFMutableDictionaryRef properties;
		if ( IORegistryEntryCreateCFProperties( service, &properties, kCFAllocatorDefault, 0 ) == noErr ) {
			mergeMaps( toQVariantMap( (CFDictionaryRef)properties ), partition.properties() );
			CFRelease( properties );
		}
		
		IOObjectRelease( service );
	} while ( ( service = IOIteratorNext( iter ) ) && !isWhole );
}

void fillPartitionInformations( FSVolumeRefNum volume, pPartition& partition )
{
	OSStatus result = noErr;
	HFSUniStr255 volumeName;
	FSVolumeInfo volumeInfo;
	FSRef rootDirectory;
	
	bzero( (void*)&volumeInfo, sizeof(volumeInfo));
	
	if ( FSGetVolumeInfo( volume, 0, 0, kFSVolInfoGettableInfo, &volumeInfo, &volumeName, &rootDirectory ) != noErr ) {
		return;
	}
	
	GetVolParmsInfoBuffer volumeParms;
	HParamBlockRec pb;
	
	pb.ioParam.ioNamePtr = 0;
	pb.ioParam.ioVRefNum = volume;
	pb.ioParam.ioBuffer = (Ptr)&volumeParms;
	pb.ioParam.ioReqCount = sizeof(volumeParms);
	
	// A version 4 GetVolParmsInfoBuffer contains the BSD node name in the vMDeviceID field.
	// It is actually a char* value. This is mentioned in the header CoreServices/CarbonCore/Files.h.
	result = PBHGetVolParmsSync( &pb );
	
	if ( result != noErr ) {
		qWarning() << QString( "PBHGetVolParmsSync returned %1." ).arg( result );
		return;
	}
	else {
		// currently i don't know how check network attached drives from mapping etc so let only use native volumes
		if ( volumeParms.vMServerAdr != 0 ) {
			return;
		}
		
		// The volume is local if vMServerAdr is 0. Network volumes won't have a BSD node name.
		if ( volumeParms.vMServerAdr == 0 ) {
			CFMutableDictionaryRef matchingDict = IOBSDNameMatching( kIOMasterPortDefault, 0, (char*)volumeParms.vMDeviceID );
			
			if ( !matchingDict ) {
				qWarning() << "IOBSDNameMatching returned a NULL dictionary.";
			}
			else {
				io_service_t service = IOServiceGetMatchingService( kIOMasterPortDefault, matchingDict );    
			
				if ( service == IO_OBJECT_NULL ) {
					qWarning() << "IOServiceGetMatchingService returned IO_OBJECT_NULL.";
					return;
				}
				
				CFMutableDictionaryRef properties;
				if ( IORegistryEntryCreateCFProperties( service, &properties, kCFAllocatorDefault, 0 ) == noErr ) {
					mergeMaps( toQVariantMap( (CFDictionaryRef)properties ), partition.properties() );
					CFRelease( properties );
				}

				io_iterator_t iter;
				result = IORegistryEntryCreateIterator( service, kIOServicePlane, kIORegistryIterateRecursively | kIORegistryIterateParents, &iter );
				
				if ( result != KERN_SUCCESS ) {
					qWarning() << QString( "IORegistryEntryCreateIterator returned 0x%18x" ).arg( result );
				}
				else if ( iter == IO_OBJECT_NULL ) {
					qWarning() << "IORegistryEntryCreateIterator returned a NULL iterator.";
				}
				else {
					wholeMedia( service, iter, partition );
					IOObjectRelease( iter );
				}
				
				IOObjectRelease( service );
			}
		}
		
		QString mountPoint;
		
		{
			char path[ MAXPATHLEN ];
			
			if ( FSRefMakePath( &rootDirectory, (UInt8*)path, MAXPATHLEN ) != noErr ) {
				return;
			}
			
			mountPoint = QString::fromUtf8( path );
		}
		
		const QString ch = partition.properties().value( "Content Hint" ).toString();
		const qint64 ct = partition.properties().value( "Content Table" ).toMap().key( ch ).toLongLong( 0, 16 );
		const qint64 fsId = volumeInfo.filesystemID == 0 ? 0xAF : ct;
		pPartition::Type type = pPartition::Fixed;
		
		if ( partition.properties().value( "IOProviderClass" ) == "IOMedia" ) {
			type = pPartition::Fixed;
		}
		else if ( partition.properties().value( "IOProviderClass" ) == "IODVDMedia" ) {
			type = pPartition::CdRom;
		}
		else if ( partition.properties().value( "IOProviderClass" ) == "IOCDMedia" ) {
			type = pPartition::CdRom;
		}
		
		partition.setProperty( pPartition::Label, QString::fromUtf16( volumeName.unicode, volumeName.length ) );
		partition.setProperty( pPartition::DevicePath,  QString( "/dev/%1" ).arg( (char*)volumeParms.vMDeviceID ) );
		partition.setProperty( pPartition::TotalSize, volumeInfo.totalBytes );
		partition.setProperty( pPartition::FreeSize, volumeInfo.freeBytes );
		partition.setProperty( pPartition::UsedSize, volumeInfo.totalBytes -volumeInfo.freeBytes );
		partition.setProperty( pPartition::MountPoints, mountPoint );
		partition.setProperty( pPartition::FileSystemId, fsId );
		partition.setProperty( pPartition::FileSystem, pPartition::fileSystemIdToString( fsId, type ) );
		partition.setProperty( pPartition::DisplayText, partition.generateDisplayText() );
		partition.setProperty( pPartition::LastCheck, QDateTime::currentDateTime() );
		
		/qWarning() << "********" << partition.property( pPartition::DevicePath ) << partition.property( pPartition::Label );
		foreach ( const QString& key, partition.properties().keys() ) {
			qWarning() << key << " -- " << partition.properties()[ key ];
		}/
	}
}

pPartitionList pPartitionModel::partitions() const
{
	pPartitionList partitions;
    OSStatus result = noErr;
    ItemCount volumeIndex;
	
    for ( volumeIndex = 1; result == noErr || result != nsvErr; volumeIndex++ ) {
        FSVolumeRefNum volume;
		
		result = FSGetVolumeInfo( kFSInvalidVolumeRefNum, volumeIndex, &volume, kFSVolInfoNone, 0, 0, 0 ); 
        
        if ( result == noErr ) {
			pPartition partition;
			fillPartitionInformations( volume, partition );
			
			if ( partition.isValid() ) {
				partitions << partition;
			}
        }
		
		//exit( 0 );
    }
	
	//exit( 0 );
	
	return partitions;
}*/

class DADisksSession
{
public:
	DADisksSession( pPartitionModel* model )
	{
		mModel = model;
		
		// init session
		mSession = DASessionCreate( kCFAllocatorDefault );
		DARegisterDiskAppearedCallback( mSession, 0/*all disks*/, diskAppeared, mModel );
		DARegisterDiskDescriptionChangedCallback( mSession, 0/*all disks*/, 0/*all keys*/, diskChanged, mModel );
		DARegisterDiskDisappearedCallback( mSession, 0/*all disks*/, diskDisappeared, mModel );
		DASessionScheduleWithRunLoop( mSession, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
	}
	
	~DADisksSession()
	{
		// deinit session
		DASessionUnscheduleFromRunLoop( mSession, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
		CFRelease( mSession );
	}
	
	static pPartition createPartition( DADiskRef disk )
	{
		const CFDictionaryRef dict = DADiskCopyDescription( disk );
		const QVariantMap properties = toQVariantMap( dict );
		pPartition partition;
		
		CFRelease( dict );
		
		// set properties on partitions only
		if ( !properties.value( "DAMediaWhole" ).toBool() ) {
			partition.setProperties( properties );
		}
		
		return partition;
	}
	
	static void diskAppeared( DADiskRef disk, void* context )
	{
		pPartitionModel* model = static_cast<pPartitionModel*>( context );
		model->updatePartition( createPartition( disk ) );
	}
	
	static void diskChanged( DADiskRef disk, CFArrayRef keys, void* context )
	{
		Q_UNUSED( keys );
		pPartitionModel* model = static_cast<pPartitionModel*>( context );
		model->updatePartition( createPartition( disk ) );
	}
	
	static void diskDisappeared( DADiskRef disk, void* context )
	{
		pPartitionModel* model = static_cast<pPartitionModel*>( context );
		model->removePartition( createPartition( disk ).devicePath() );
	}

protected:
	pPartitionModel* mModel;
	DASessionRef mSession;
};

void pPartitionModel::platformInit()
{
	mData = 0;
}

void pPartitionModel::platformDeInit()
{
	delete (DADisksSession*)mData;
}

pPartitionList pPartitionModel::partitions() const
{
	delete (DADisksSession*)mData;
	mData = new DADisksSession( const_cast<pPartitionModel*>( this ) );
	return pPartitionList();
}

#endif
