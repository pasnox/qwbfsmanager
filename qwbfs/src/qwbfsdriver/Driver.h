/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : Driver.h
** Date      : 2010-06-16T14:19:29
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**
** In addition, as a special exception, the copyright holders give permission
** to link this program with the OpenSSL project's "OpenSSL" library (or with
** modified versions of it that use the same license as the "OpenSSL"
** library), and distribute the linked executables. You must obey the GNU
** General Public License in all respects for all of the code used other than
** "OpenSSL".  If you modify file(s), you may extend this exception to your
** version of the file(s), but you are not obligated to do so. If you do not
** wish to do so, delete this exception statement from your version.
**
****************************************************************************/
#ifndef DRIVER_H
#define DRIVER_H

#include <QObject>
#include <QStringList>
#include <QTime>
#include <QMutex>

#include "qwbfsdriver/PartitionProperties.h"
#include "qwbfsdriver/PartitionHandle.h"
#include "qwbfsdriver/PartitionStatus.h"

namespace QWBFS {
namespace Model {
struct Disc;
typedef QList<struct Disc> DiscList;
}; // Model

class Driver : public QObject
{
	Q_OBJECT
	
public:
	enum State
	{
		None = 0,
		Success,
		Failed
	};
	
	enum Error
	{
		Ok = 0,
		PartitionNotOpened = -1,
		SourcePartitionNotOpened = -2,
		DiscReadFailed = -3,
		DiscWriteFailed = -4,
		DiscExtractFailed = -5,
		DiscAddFailed = -6,
		DiscFound = -7,
		DiscNotFound = -8,
		InvalidDiscIndex = -9,
		InvalidDiscID = -10,
		InvalidDisc = -11,
		CantDrive2Drive = -12
	};
	
	Driver( QObject* parent = 0, const QWBFS::Partition::Handle& partitionHandle = QWBFS::Partition::Handle() );
	virtual ~Driver();
	
	void setPartition( const QString& partition );
	QString partition() const;
	
	QWBFS::Partition::Handle handle() const;
	
	/*!
		\details open the partition.
		\param none.
		\return return true on success, else false.
	*/
	bool open();
	/*!
		\details close the partition.
		\param none.
		\return none.
	*/
	void close();
	/*!
		\details return the partition open state.
		\param none.
		\return return true if open, else false.
	*/
	bool isOpen() const;
	/*!
		\details format the partition, closing it first if needed. /!\ Be sure to close all opened handle of the same partition first.
		\param none.
		\return return true on success, else false.
	*/
	bool format();
	/*!
		\details return the disc count.
		\param none.
		\return PartitionNotOpened or the disc count.
	*/
	int discCount() const;
	/*!
		\details return a filled disc structure.
		\param index the disc index position in the wbfs partition.
		\param discId the disc ID of the disc.
		\param disc the structure to be filled.
		\return PartitionNotOpened, InvalidDiscIndex, InvalidDiscID or Ok.
	*/
	int discInfo( int index, QWBFS::Model::Disc& disc ) const;
	int discInfo( const QString& discId, QWBFS::Model::Disc& disc ) const;
	/*!
		\details return the used blocks count.
		\param none.
		\return PartitionNotOpened or the used blocks count.
	*/
	int usedBlocksCount() const;
	/*!
		\details return a filled disc structure.
		\param fileName the file to get informations from.
		\param partitionSelector the partition selector.
		\param disc the disc to fill informations.
		\return PartitionNotOpened or the used blocks count.
	*/
	int discImageInfo( const QString& fileName, QWBFS::Model::Disc& disc, partition_selector_t partitionSelection = ONLY_GAME_PARTITION ) const;
	/*!
		\details add a disc image in the partition.
		\param fileName the disc image file path to add.
		\param progressCallback the progress callback to use.
		\param partitionSelection the partition to add.
		\param copy1to1 do a 1to1 copy or not.
		\param newName a new name to use for the added disc. /!\ windows only.
		\return PartitionNotOpened, DiscReadFailed, DiscAlreadyInPartition, DiscAddFailed or Ok
	*/
	int addDiscImage( const QString& fileName, progress_callback_t progressCallback = Driver::progress_callback, partition_selector_t partitionSelection = ONLY_GAME_PARTITION, bool copy1to1 = false, const QString& newName = QString::null ) const;
	/*!
		\details remove a disc from the partition.
		\param discId the disc id to remove.
		\return remove a disc from the partition.
	*/
	int removeDisc( const QString& discId ) const;
	/*!
		\details extract a disc from the wbfs partition to a path.
		\param discId the disc id to extract.
		\param path the path where to extract the disc.
		\param progressCallback the progress callback to use.
		\param name the name of the iso if not null, else the name is based on the disc title.
		\return PartitionNotOpened, DiscNotFound, DiscExtractFailed, DiscWriteFailed or Ok.
	*/
	int extractDisc( const QString& discId, const QString& path, const QString& name = QString::null, progress_callback_t progressCallback = Driver::progress_callback ) const;
	/*!
		\details rename the disc id to another name.
		\param discId the disc id to rename.
		\param name the new name to set for the disc.
		\return PartitionNotOpened, DiscNotFound, or Ok.
	*/
	int renameDisc( const QString& discId, const QString& name ) const;
	/*!
		\details fill a status structure with partition infos.
		\param status the structure to be filled with partition status.
		\return PartitionNotOpened or Ok.
	*/
	int status( QWBFS::Partition::Status& status ) const;
	/*!
		\details add a disc from another partition aka drive2drive copy.
		\param discId the disc id to copy.
		\param sourcePartitionHandle the partition to read disc from.
		\return PartitionNotOpened, SourcePartitionNotOpened, DiscNotFound, DiscAlreadyInPartition, DiscAddFailed or Ok.
	*/
	int addDisc( const QString& discId, const QWBFS::Partition::Handle& sourcePartitionHandle, progress_callback_t progressCallback = Driver::progress_callback, partition_selector_t partitionSelection = ONLY_GAME_PARTITION ) const;
	/*!
		\details check if drive2drive copy is possible.
		\param handle the source partition handle.
		\return PartitionNotOpened, SourcePartitionNotOpened, CantDrive2Drive or Ok
	*/
	int canDrive2Drive( const QWBFS::Partition::Handle& sourcePartitionHandle ) const;
	/*!
		\details check if a disc is in the partition.
		\param discId the disc id to check.
		\return PartitionNotOpened, DiscNotFound or DiscFound
	*/
	int hasDisc( const QString& discId ) const;
	/*!
		\details 
		\param paramName paramDescription
		\return returnDescription
	*/
	int discList( QWBFS::Model::DiscList& list ) const;
	/*!
		\details return if a file is a wbfs partition.
		\param fileName the fileName to check.
		\return return true if fileName is a wbfs partition, else false.
	*/
	static bool isWBFSPartition( const QString& fileName );
	/*!
		\details return a string representation of the error.
		\param error the error to represent.
		\return return a QString representing the textual error message.
	*/
	static QString errorToString( QWBFS::Driver::Error error );
	/*!
		\details return a string representation of the region.
		\param region the region to represent.
		\return return a QString representing the textual region.
	*/
	static QString regionToString( int region );
	/*!
		\details return a string representation of the region language.
		\param region the region language to represent.
		\return return a QString representing the textual region language.
	*/
	static QString regionToLanguageString( int region );
	/*!
		\details return a string representation of the state.
		\param state the state to represent.
		\return return a QString representing the textual state.
	*/
	static QString stateToString( QWBFS::Driver::State state );
	/*!
		\details set the force mode of the libwbfs.
		\param force true to force, else false.
		\return none.
	*/
	static void setForceMode( bool force );
	/*!
		\details return the force mode of the libwbfs.
		\param none.
		\return return true if force is active, else false.
	*/
	static bool forceMode();
	
	static void addHandle( const QWBFS::Partition::Handle& handle );
	static QWBFS::Partition::Handle getHandle( const QString& partition, bool* created = 0 );
	static void closeHandle( const QWBFS::Partition::Handle& handle );

protected:
	mutable QWBFS::Partition::Properties mProperties;
	QWBFS::Partition::Handle mHandle;
	bool mHasCreatedHandle;
	
	static QMutex mMutex;
	static bool mForce;
	static QWBFS::Driver* mCurrentDriver;
	static QHash<QString, QWBFS::Partition::Handle> mHandles;
	static QHash<int, QString> mLanguages;
	static QHash<int, QString> mRegions;
	
	static int u8StrLength( u8* str );
	static void discInfo( u8* header, QWBFS::Model::Disc& disc );
	
	static int discRead_callback( void* fp, u32 lba, u32 count, void* iobuf );
	static void progress_callback( int value, int maximum );

signals:
	void currentProgressChanged( int value, int maximum, const QTime& remaining );
	void globalProgressChanged( int value );
};

}; // QWBFS

#endif // DRIVER_H
