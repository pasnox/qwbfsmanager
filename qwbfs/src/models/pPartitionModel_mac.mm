#include "pPartitionModel.h"

/*
http://www.datarecovery.com/hexcodes.asp
http://stackoverflow.com/questions/1515068/list-all-drives-partitions-and-get-dev-rdisc-device-with-cocoa
*/

#if defined( Q_OS_MAC )
#import <DiskArbitration/DiskArbitration.h>
#import <CoreServices/CoreServices.h>

#include <FreshCore/pMacHelpers>

#include <QStringList>
#include <QDebug>

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
		const QVariantMap properties = pMacHelpers::toQVariantMap( dict );
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
				
				/*qint64 major = properties.value( "DAMediaBSDMajor" ).toLongLong();
				qint64 minor = properties.value( "DAMediaBSDMinor" ).toLongLong();
				
				qWarning() << "-----";
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
