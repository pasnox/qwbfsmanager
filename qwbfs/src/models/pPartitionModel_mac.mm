#include "pPartitionModel.h"

/*
http://www.datarecovery.com/hexcodes.asp
http://stackoverflow.com/questions/1515068/list-all-drives-partitions-and-get-dev-rdisc-device-with-cocoa
*/

#if defined( Q_OS_MAC )
#import <Foundation/Foundation.h>
#import <DiskArbitration/DiskArbitration.h>

#include <QStringList>
#include <QDebug>

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
			
			FSVolumeRefNum volume;
			
			// get volume infos, like total bytes, free bytes...
			if ( FSGetVolumeForDADisk( disk, &volume ) == noErr ) {
				FSVolumeInfo volumeInfo;
				
				bzero( (void*)&volumeInfo, sizeof(volumeInfo));
				
				qint64 major = properties.value( "DAMediaBSDMajor" ).toLongLong();
				qint64 minor = properties.value( "DAMediaBSDMinor" ).toLongLong();
				
				/*qWarning() << "-----";
				QStringList test;
				test << QString( "%1%2" ).arg( major, 0, 16 ).arg( minor, 0, 16 ).toUpper().prepend( "0x" );
				test << QString( "%1" ).arg( major +minor, 0, 16 ).toUpper().prepend( "0x" );
				test << QString( "%1" ).arg( major, 0, 16 ).toUpper().prepend( "0x" );
				
				qWarning()
					<< properties.value( "DAVolumeKind" ).toString().toUpper()
					<< properties.value( "DAMediaContent" ).toString().toUpper()
					<< test;*/
				
				if ( FSGetVolumeInfo( volume, 0, 0, kFSVolInfoSizes | kFSVolInfoFSInfo, &volumeInfo, 0, 0 ) == noErr ) {
					partition.mProperties[ PROPERTY_FILE_SYSTEM_ID ] = volumeInfo.filesystemID == 0 ? 0xAF : volumeInfo.filesystemID;
					partition.updateSizes( volumeInfo.totalBytes, volumeInfo.freeBytes );
				}
			}
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
		model->removePartition( QString( "/dev/%1" ).arg( DADiskGetBSDName( disk ) ) );
	}

protected:
	pPartitionModel* mModel;
	DASessionRef mSession;
};

void pPartitionModel::platformInit()
{
	emit layoutAboutToBeChanged();
	mData = new DADisksSession( this );
	emit layoutChanged();
}

void pPartitionModel::platformDeInit()
{
	delete (DADisksSession*)mData;
}

void pPartitionModel::platformUpdate()
{
	// Code commented as the mac implementation has the concept of live auto update, thanks Disk Arbitration framework :)
	/*const QStringList partitions = customPartitions();
	
	emit layoutAboutToBeChanged();
	delete (DADisksSession*)mData;
	mPartitions.clear();
	foreach ( const QString& partition, partitions ) {
		mPartitions << pPartition( partition );
	}
	mData = new DADisksSession( this );
	emit layoutChanged();*/
}

#endif
