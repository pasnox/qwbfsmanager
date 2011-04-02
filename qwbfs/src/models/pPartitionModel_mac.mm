/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : Fresh Library
** FileName  : pPartitionModel_mac.cpp
** Date      : 2011-02-20T00:41:35
** License   : LGPL v3
** Home Page : http://bettercodes.org/projects/fresh
** Comment   : Fresh Library is a Qt 4 extension library providing set of new core & gui classes.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Leser General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/
#include "pPartitionModel.h"

/*
http://www.datarecovery.com/hexcodes.asp
http://stackoverflow.com/questions/1515068/list-all-drives-partitions-and-get-dev-rdisc-device-with-cocoa
*/

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
	
	virtual ~DADisksSession()
	{
		// deinit session
		DASessionUnscheduleFromRunLoop( mSession, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
		CFRelease( mSession );
	}
	
	static pPartition createPartition( DADiskRef disk )
	{
		const CFDictionaryRef dict = DADiskCopyDescription( disk );
		QVariantMap properties = pMacHelpers::toQVariantMap( dict );
		pPartition partition;
		
		CFRelease( dict );
		
		// set properties on partitions only
		if ( !properties.value( "DAMediaWhole" ).toBool() ) {
			const QString devicePath = QString( "/dev/%1" ).arg( properties.value( "DAMediaBSDName" ).toString() );
			
			if ( pPartition::isWBFSPartition( devicePath ) ) {
				properties[ "DAVolumeKindId" ] = 0x25;
				properties[ "DAVolumeKind" ] = pPartition::fileSystemIdToString( 0x25 );
			}
			
			FSVolumeRefNum volume;
			qint64 total = properties.value( "DAMediaSize", -1 ).toLongLong();
			qint64 free = -1;
			
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
				
				if ( properties[ "DAVolumeKindId" ] != 0x25 && FSGetVolumeInfo( volume, 0, 0, kFSVolInfoSizes | kFSVolInfoFSInfo, &volumeInfo, 0, 0 ) == noErr ) {
					properties[ "DAVolumeKindId" ] = volumeInfo.filesystemID == 0 ? 0xAF : volumeInfo.filesystemID;
					total = volumeInfo.totalBytes;
					free = volumeInfo.freeBytes;
				}
			}
			
			partition.setProperties( properties );
			partition.updateSizes( total, free );
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
