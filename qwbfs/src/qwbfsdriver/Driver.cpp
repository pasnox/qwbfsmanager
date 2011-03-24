/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : Driver.cpp
** Date      : 2010-06-16T14:19:29
** License   : GPL2
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
#include "Driver.h"
#include "PartitionDiscHandle.h"
#include "models/Disc.h"

#include <libwbfs.h>

#include <ctime>

#include <QDir>
#include <QApplication>
#include <QDebug>

#if defined( Q_OS_WIN )
#include <FreshCore/pWinHelpers>
#include <qt_windows.h>
#include <WinIoCtl.h>
#endif

using namespace QWBFS;

#define WBFS_FILE_MINIMUM_SIZE 1024 *1024 *16

QMutex Driver::mMutex;
bool Driver::mForce = false;
QHash<QString, QWBFS::Partition::Handle> Driver::mHandles;
QHash<int, QString> Driver::mLanguages;
QHash<int, QString> Driver::mRegions;
QWBFS::Driver* Driver::mCurrentDriver = 0;

Driver::Driver( QObject* parent, const QWBFS::Partition::Handle& partitionHandle )
	: QObject( parent )
{
	mProperties = partitionHandle.properties();
	mHandle = partitionHandle;
	mHasCreatedHandle = !partitionHandle.isValid();
	
	init();
}

Driver::Driver( const QWBFS::Partition::Handle& partitionHandle )
	: QObject( 0 )
{
	mProperties = partitionHandle.properties();
	mHandle = partitionHandle;
	mHasCreatedHandle = !partitionHandle.isValid();
	
	init();
}

Driver::~Driver()
{
	close();
}

void Driver::init()
{
	if ( mRegions.isEmpty() && mLanguages.isEmpty() )
	{
		/*
			ASCII Hex Region
			A 41 All regions. System channels like the Mii channel use it.
			D 44 German-speaking regions. Only if separate versions exist, e.g. Zelda: A Link to the Past
			E 45 USA and other NTSC regions except Japan
			F 46 French-speaking regions. Only if separate versions exist, e.g. Zelda: A Link to the Past.
			J 4A Japan
			K 4B Korea
			L 4C PAL/World?
			P 50 Europe, Australia and other PAL regions
			Q 51 Korea with Japanese language.
			T 54 Korea with English language.
			X 58 Not a real region code. Homebrew Channel uses it, though. 
		*/
		
		/*
			WiiTDB Folder Game ID:
			
			E = US
			J = JA
			K = KO
			R = RU
			W = ZH
			
			ça c'est fixe, ça ne bouge pas, c'est toujours en fonction du game ID
			
			Pour les ID en P,D,F,I,S,H,X,Y,Z, on fait en fonction de la langue de l'utilisateur
			(donc celle de l'interface ou avec une option) et on utilise EN, FR, DE, ES, IT, NL, PT
			
			Si l'image n'est pas trouvée dans la langue de préférence, on essaye EN (sauf si la langue préférée est l'anglais bien sur)
			
			
			Si l'ID ne contient aucune de ces lettres c'est que c'est un jeu custom avec un ID qui n'est pas standard.
			
			Dans ce cas on essaye la langue préférée, puis EN, puis other en tout dernier.
		*/
		
		// Regions
		mRegions.insert( 'A', "World" ); // All regions. System channels like the Mii channel use it.
		mRegions.insert( 'E', "NTSC-U" ); // USA and other NTSC regions except Japan.
		mRegions.insert( 'J', "NTSC-J" ); // Japan
		mRegions.insert( 'K', "NTSC-K" ); // Korea
		mRegions.insert( 'P', "PAL" ); // Europe, Australia and other PAL regions.
		mRegions.insert( 'W', "NTSC-T" ); // Taiwan
		mRegions.insert( 'R', "PAL-R" ); // Russia
		mRegions.insert( 'Q', "NTSC-K" ); // Korea with Japanese language.
		mRegions.insert( 'T', "NTSC-K" ); // Korea with English language.
		mRegions.insert( 'F', "PAL" ); // French-speaking regions. Only if separate versions exist, e.g. Zelda: A Link to the Past.
		mRegions.insert( 'D', "PAL" ); // German-speaking regions. Only if separate versions exist, e.g. Zelda: A Link to the Past.
		mRegions.insert( 'I', "PAL" ); // Italien-speaking regions. Only if separate versions exist.
		mRegions.insert( 'H', "PAL" ); // Dutch-speaking regions. Only if separate versions exist.
		mRegions.insert( 'S', "PAL" ); // Spanish-speaking regions. Only if separate versions exist.
		mRegions.insert( 'X', "PAL" ); // ??
		mRegions.insert( 'Y', "PAL" ); // ??
		mRegions.insert( 'Z', "PAL" ); // ??
		// Custom disc / Channels
		mRegions.insert( 'L', "CUSTOM" );
		mRegions.insert( 'M', "CUSTOM" );
		mRegions.insert( 'N', "CUSTOM" );
		
		// Languages
		/*mLanguages[ 'L' ] = QLatin1String( "EN" ); // Japanese Import to Europe, Australia and other PAL regions
		mLanguages[ 'M' ] = QLatin1String( "EN" ); // American Import to Europe, Australia and other PAL regions
		mLanguages[ 'N' ] = QLatin1String( "JA" ); // Japanese Import to USA and other NTSC regions*/
		
		mLanguages.insert( 'A', "EN" ); // All regions. System channels like the Mii channel use it.
		mLanguages.insert( 'E', "US" ); // USA and other NTSC regions except Japan.
		mLanguages.insert( 'J', "JA" ); // Japan
		mLanguages.insert( 'K', "KO" ); // Korea
		mLanguages.insert( 'R', "RU" ); // Russia
		mLanguages.insert( 'W', "ZH" ); // Taiwan
		/*mLanguages.insert( 'Q', "JA" ); // Korea with Japanese language.
		mLanguages.insert( 'T', "EN" ); // Korea with English language.
		mLanguages.insert( 'F', "FR" ); // French-speaking regions. Only if separate versions exist, e.g. Zelda: A Link to the Past.
		mLanguages.insert( 'D', "DE" ); // German-speaking regions. Only if separate versions exist, e.g. Zelda: A Link to the Past.
		mLanguages.insert( 'I', "IT" ); // Italian-speaking regions. Only if separate versions exist.
		mLanguages.insert( 'H', "NL" ); // Dutch-speaking regions. Only if separate versions exist.
		mLanguages.insert( 'S', "ES" ); // Spanish-speaking regions. Only if separate versions exist.*/
		
		//insert( 'P', "" ); // Europe, Australia and other PAL regions.
	}
}

void Driver::setPartition( const QString& partition )
{
	mProperties.partition = partition;
}

QString Driver::partition() const
{
	return mHandle.isValid() ? mHandle.partition() : mProperties.partition;
}

QWBFS::Partition::Handle Driver::handle() const
{
	return mHandle;
}

bool Driver::open()
{
	if ( isOpen() ) {
		close();
	}
	
	mHandle = getHandle( mProperties.partition, &mHasCreatedHandle );
	return isOpen();
}

void Driver::close()
{
	if ( !isOpen() ) {
		return;
	}
	
	if ( mHasCreatedHandle ) {
		mHasCreatedHandle = false;
		closeHandle( mHandle );
	}
	
	mHandle = QWBFS::Partition::Handle();
}

bool Driver::isOpen() const
{
	return mHandle.isValid();
}

bool Driver::format()
{
	if ( isOpen() ) {
		close();
	}
	
	QWBFS::Partition::Properties properties = mProperties;
	properties.reset = true;
	
	const QWBFS::Partition::Handle handle( properties );
	const QWBFS::Partition::Status status( handle );
	
	return handle.isValid() && status.size > 0;
}

int Driver::discCount() const
{
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	return wbfs_count_discs( mHandle.ptr() );
}

int Driver::discInfo( int index, QWBFS::Model::Disc& disc ) const
{
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	if ( index < 0 || index >= discCount() ) {
		return Driver::InvalidDiscIndex;
	}
	
	const int headerSize = 0x100;
	u8* header = (u8*)wbfs_ioalloc( headerSize );
	quint32 size;
	
	disc.origin = mHandle.partition();

	if ( wbfs_get_disc_info( mHandle.ptr(), index, header, headerSize, &size ) != 0 ) {
		wbfs_iofree( header );
		return Driver::DiscReadFailed;
	}
	
	disc.size = size *4ULL;
	discInfo( header, disc );
	wbfs_iofree( header );
	
	return Driver::Ok;
}

int Driver::discInfo( const QString& discId, QWBFS::Model::Disc& disc ) const
{
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	const QWBFS::Partition::DiscHandle discHandle( mHandle, discId );
	
	if ( discHandle.isValid() ) {
		return discInfo( discHandle.index(), disc );
	}
	
	return Driver::InvalidDiscID;
}

int Driver::usedBlocksCount() const
{
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	return wbfs_count_usedblocks( mHandle.ptr() );
}

int Driver::discImageInfo( const QString& fileName, QWBFS::Model::Disc& disc, partition_selector_t partitionSelection ) const
{
	if ( isWBFSPartitionOrFile( fileName ) ) {
		return wbfsFileInfo( fileName, disc );
	}
	
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	void* fileHandle = wbfs_open_file_for_read( fileName.toLocal8Bit().data() );

	if ( !fileHandle ) {
		return Driver::InvalidDisc;
	}
	
	u8* header = (u8*)wbfs_ioalloc( 0x100 );
	
	disc.size = wbfs_estimate_disc( mHandle.ptr(), wbfs_read_wii_file, fileHandle, partitionSelection, header );
	disc.origin = fileName;
	discInfo( header, disc );
	
	wbfs_iofree( header );
	wbfs_close_file( fileHandle );
	
	return disc.size != 0 ? Driver::Ok : Driver::InvalidDisc;
}

int Driver::addDiscImage( const QString& fileName, progress_callback_t progressCallback, partition_selector_t partitionSelection, bool copy1to1, const QString& newName ) const
{
	mCurrentDriver = const_cast<Driver*>( this );
#ifndef Q_OS_WIN
	Q_UNUSED( newName );
#endif

	if ( !QFile::exists( fileName ) ) {
		return Driver::DiscNotFound;
	}

	QWBFS::Model::Disc disc;
	const int result1 = discImageInfo( fileName, disc );
	
	if ( result1 != Driver::Ok ) {
		return result1;
	}
	
	if ( hasDisc( disc.id ) == Driver::DiscFound ) {
		return Driver::DiscFound;
	}
	
	void* fileHandle = wbfs_open_file_for_read( fileName.toLocal8Bit().data() );

	if ( !fileHandle ) {
		return Driver::DiscReadFailed;
	}
	
	const u32 result2 = wbfs_add_disc( mHandle.ptr(), wbfs_read_wii_file, fileHandle, progressCallback, partitionSelection, copy1to1 ? 1 : 0
#ifdef Q_OS_WIN
		, newName.isEmpty() ? 0 : newName.toLocal8Bit().data()
#endif
	);

	wbfs_close_file( fileHandle );
	return result2 == 0 ? Driver::Ok : Driver::DiscAddFailed;
}

int Driver::removeDisc( const QString& discId ) const
{
	if ( isOpen() ) {
		if ( wbfs_rm_disc( mHandle.ptr(), (u8*)discId.toLocal8Bit().data() ) == 0 ) {
			return Driver::Ok;
		}
		
		return Driver::DiscNotFound;
	}
	
	return Driver::PartitionNotOpened;
}

int Driver::extractDisc( const QString& discId, const QString& path, const QString& _name, progress_callback_t progressCallback ) const
{
	mCurrentDriver = const_cast<Driver*>( this );
	
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	// get disc handle
	const QWBFS::Partition::DiscHandle discHandle( mHandle, discId );
	
	if ( !discHandle.isValid() ) {
		return Driver::DiscNotFound;
	}
	
	const QString name = _name.isEmpty() ? discHandle.isoName() : _name;
	const QString filePath = QDir::toNativeSeparators( QString( "%1/%2" ).arg( path ).arg( name ) );
	int result = Driver::allocateFile( filePath, ( discHandle.ptr()->p->n_wii_sec_per_disc /2 ) *0x8000ULL );
	void* fileHandle = result == Driver::Ok ? wbfs_open_file_for_write( filePath.toLocal8Bit().data() ) : 0;

	if ( fileHandle ) {
		result = wbfs_extract_disc( discHandle.ptr(), wbfs_write_wii_file, fileHandle, progressCallback );
		wbfs_close_file( fileHandle );
		return result == 0 ? Driver::Ok : Driver::DiscExtractFailed;
	}
	else {
		QFile::remove( filePath );
	}
	
	return Driver::DiscWriteFailed;
}

int Driver::renameDisc( const QString& discId, const QString& name ) const
{
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	if ( wbfs_ren_disc( mHandle.ptr(), (u8*)discId.toLocal8Bit().data(), (u8*)name.toLocal8Bit().data() ) == 0 ) {
		return Driver::Ok;
	}
	
	return Driver::DiscNotFound;
}

int Driver::status( QWBFS::Partition::Status& status ) const
{
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	status = QWBFS::Partition::Status( mHandle );
	return Driver::Ok;
}

int Driver::addDisc( const QString& discId, const QWBFS::Partition::Handle& sourcePartitionHandle, progress_callback_t progressCallback, partition_selector_t partitionSelection ) const
{
	mCurrentDriver = const_cast<Driver*>( this );
	
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	if ( !sourcePartitionHandle.isValid() ) {
		return Driver::SourcePartitionNotOpened;
	}
	
	const QWBFS::Partition::DiscHandle discHandle( sourcePartitionHandle, discId );
	
	if ( !discHandle.isValid() ) {
		return Driver::DiscNotFound;
	}
	
	if ( hasDisc( discId ) == Driver::DiscFound ) {
		return Driver::DiscFound;
	}
	
	if ( wbfs_add_disc( mHandle.ptr(), discRead_callback/*wbfs_disc_read*/, discHandle.ptr(), progressCallback, partitionSelection, 0
#ifdef Q_OS_WIN
		, 0
#endif
		) == 0 ) {
		return Driver::Ok;
	}
	
	return Driver::DiscAddFailed;
}

int Driver::canDrive2Drive( const QWBFS::Partition::Handle& sourcePartitionHandle ) const
{
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	if ( !sourcePartitionHandle.isValid() ) {
		return Driver::SourcePartitionNotOpened;
	}
	
	const double left = sourcePartitionHandle.ptr()->wbfs_sec_sz /sourcePartitionHandle.ptr()->hd_sec_sz;
	const double right = mHandle.ptr()->wbfs_sec_sz /mHandle.ptr()->hd_sec_sz;
	
	if ( qFuzzyCompare( left, right ) ) {
		return Driver::Ok;
	}
	
	return Driver::CantDrive2Drive;
}

int Driver::hasDisc( const QString& discId ) const
{
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	const QWBFS::Partition::DiscHandle discHandle( mHandle, discId );
	
	if ( discHandle.isValid() ) {
		return Driver::DiscFound;
	}
	
	return Driver::DiscNotFound;
}

int Driver::discList( QWBFS::Model::DiscList& list ) const
{
	const int count = discCount();
	
	if ( count < 0 ) {
		return count; // error
	}
	
	for ( int index = 0; index < count; index++ ) {
		QWBFS::Model::Disc disc;
		const int result = discInfo( index, disc );
		
		if ( result != Driver::Ok ) {
			return result;
		}
		
		list << disc;
	}
	
	return Driver::Ok;
}

int Driver::trim() const
{
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	wbfs_trim( mHandle.ptr() );
	wbfs_file_truncate( mHandle.ptr()->callback_data, mHandle.ptr()->n_hd_sec *512ULL );
	
	return Driver::Ok;
}

bool Driver::isWBFSPartitionOrFile( const QString& _fileName )
{
	QString filePath = _fileName;
	
#if defined( Q_OS_WIN )
	if ( !filePath.isEmpty() && filePath.length() <= 3 ) {
		filePath = QString( "\\\\?\\%1:" ).arg( filePath[ 0 ] );
		DISK_GEOMETRY diskGeometry;
		HANDLE handle = CreateFile( QStringToTCHAR( filePath ), GENERIC_READ | FILE_SHARE_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL );
		
		if ( handle != INVALID_HANDLE_VALUE ) {
			DWORD bytes;
			
			if ( DeviceIoControl( handle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &diskGeometry, sizeof(DISK_GEOMETRY), &bytes, NULL ) ) {
				const DWORD sectorSize = diskGeometry.BytesPerSector;
				char buffer[ sectorSize  ];
				DWORD read;
				
				if ( ReadFile( handle, buffer, sectorSize, &read, NULL ) ) {
					CloseHandle( handle );
					return QByteArray( buffer ).left( 4 ).toLower() == "wbfs";
				}
			}
		}
		
		CloseHandle( handle );
		return false;
	}
#endif
	
	QFile file( filePath );
	
	if ( file.open( QIODevice::ReadOnly ) ) {
		return file.read( 4 ).toLower() == "wbfs";
	}
	
	return false;
}

int Driver::allocateFile( const QString& filePath, qint64 size )
{
	const int go = 1024 *1024 *1024;
	QFile file( filePath );
	
	if ( file.exists() ) {
		return Driver::DiscFound;
	}
	
	if ( file.open( QIODevice::WriteOnly ) ) {
		if ( size == -1 ) {
			if ( file.resize( go *8.8 ) ) { // 8.8 go - all FS except FAT32 // 143432 *2 *0x8000ULL
				return Driver::Ok;
			}
			else if ( file.resize( ( go *4.0 ) -1 ) ) { // 4go - fat32 FS
				return Driver::Ok;
			}
		}
		else if ( file.resize( size ) ) {
			return Driver::Ok;
		}
	}
	
	file.remove();
	
	return Driver::DiscWriteFailed;
}

int Driver::wbfsFileInfo( const QString& wbfsFileName, QWBFS::Model::Disc& disc )
{
	/*if ( !isWBFSPartitionOrFile( wbfsFileName ) ) {
		return Driver::InvalidDisc;
	}*/
	
	wbfs_t* partition = wbfs_try_open_partition( wbfsFileName.toLocal8Bit().data(), 0 );
	
	if ( !partition ) {
		return Driver::InvalidDisc;
	}
	
	u8* header = (u8*)wbfs_ioalloc( 0x100 );
	int count = wbfs_count_discs( partition );
	
	if ( count != 1 ) {
		wbfs_iofree( header );
		wbfs_close( partition );
		return Driver::InvalidDisc;
	}
	
	wbfs_disc_t* cd;
	quint32 size;
	
	if( !wbfs_get_disc_info( partition, 0, header, 0x100, &size ) ) {
		cd = wbfs_open_disc( partition, header );
		
		if ( !cd ) {
			wbfs_iofree( header );
			wbfs_close( partition );
			return Driver::InvalidDisc;
		}
		
		wbfs_close_disc( cd );
	}
	
	disc.size = size *4ULL;
	disc.origin = wbfsFileName;
	discInfo( header, disc );
	
	wbfs_iofree( header );
	wbfs_close( partition );
	
	return disc.size != 0 ? Driver::Ok : Driver::InvalidDisc;
}

int Driver::convertIsoFileToWBFSFile( const QString& isoFilePath, const QString& _wbfsFilePath )
{
	int result;
	QString wbfsFilePath = _wbfsFilePath;
	
	if ( wbfsFilePath.isEmpty() ) {
		wbfsFilePath = QString( "%1.wbfs" ).arg( isoFilePath );
	}
	
	result = Driver::allocateFile( wbfsFilePath );
	
	if ( result != QWBFS::Driver::Ok ) {
		return result;
	}
	
	QWBFS::Partition::Properties properties( wbfsFilePath );
	properties.reset = true;
	
	QWBFS::Partition::Handle handle( properties );
	QWBFS::Driver td( 0, handle );
	
	if ( !td.isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	QWBFS::Model::Disc disc;
	result = td.discImageInfo( isoFilePath, disc );
	
	if ( result != QWBFS::Driver::Ok ) {
		return result;
	}
	
	result = td.addDiscImage( disc.origin );
	
	if ( result != QWBFS::Driver::Ok ) {
		return result;
	}
	
	return td.trim();
}

QString Driver::errorToString( QWBFS::Driver::Error error )
{
	switch ( error ) {
		case Driver::Ok:
			return tr( "No error" );
		case Driver::PartitionNotOpened:
			return tr( "Partition not opened" );
		case Driver::SourcePartitionNotOpened:
			return tr( "Source partition not opened" );
		case Driver::DiscReadFailed:
			return tr( "Disc read failed" );
		case Driver::DiscWriteFailed:
			return tr( "Disc write failed" );
		case Driver::DiscExtractFailed:
			return tr( "Disc extract failed" );
		case Driver::DiscAddFailed:
			return tr( "Disc add failed" );
		case Driver::DiscConvertFailed:
			return tr( "Disc convert failed" );
		case Driver::DiscRenameFailed:
			return tr( "Disc rename failed" );
		case Driver::DiscFound:
			return tr( "Disc found" );
		case Driver::DiscNotFound:
			return tr( "Disc not found (or not exists)" );
		case Driver::InvalidDiscIndex:
			return tr( "Invalid disc index" );
		case Driver::InvalidDiscID:
			return tr( "Invalid disc id" );
		case Driver::InvalidDisc:
			return tr( "Invalid disc" );
		case Driver::CantDrive2Drive:
			return tr( "Can't drive to drive copy" );
		case Driver::UnknownError:
			return tr( "Unknown error" );
	}
	
	return QString::null;
}

QString Driver::regionToString( int region )
{
	return mRegions.value( region, "Unknow Region" );
}

QString Driver::regionToLanguageString( int region )
{
	const QString systemLanguage = QLocale::system().name().section( '_', 0, 0 ).toUpper();
	return mLanguages.value( region, systemLanguage.isEmpty() ? "EN" : systemLanguage );
}

QString Driver::stateToString( QWBFS::Driver::State state )
{
	switch ( state ) {
		case Driver::None:
			return tr( "None" );
		case Driver::Success:
			return tr( "Success" );
		case Driver::Failed:
			return tr( "Failed" );
	}
	
	return QString::null;
}

void Driver::setForceMode( bool force )
{
	QMutexLocker locker( &mMutex );
	mForce = force;
	wbfs_set_force_mode( mForce ? 1 : 0 );
}

bool Driver::forceMode()
{
	QMutexLocker locker( &mMutex );
	return mForce;
}

void Driver::addHandle( const QWBFS::Partition::Handle& handle )
{
	if ( !handle.isValid() ) {
		return;
	}
	
	QMutexLocker locker( &mMutex );
	mHandles[ handle.partition() ] = handle;
}

QWBFS::Partition::Handle Driver::getHandle( const QString& partition, bool* created )
{
	QMutexLocker locker( &mMutex );
	QWBFS::Partition::Handle handle = mHandles.value( partition );
	
	if ( created ) {
		*created = false;
	}
	
	if ( !handle.isValid() ) {
		const QWBFS::Partition::Handle _handle( partition );
		
		if ( _handle.isValid() ) {
			mHandles[ partition ] = _handle;
			handle = _handle;
			
			if ( created ) {
				*created = true;
			}
		}
	}
	
	return handle;
}

void Driver::closeHandle( const QWBFS::Partition::Handle& handle )
{
	QMutexLocker locker( &mMutex );
	mHandles.remove( handle.partition() );
}

int Driver::u8StrLength( u8* str )
{
	int counter = 0;
	int length = 0;
	
	while ( str[ counter++ ] != '\0' ) {
		length++;
	}
	
	return length;
}

void Driver::discInfo( u8* header, QWBFS::Model::Disc& disc )
{
	const int offset = 0x20;
	const int length = u8StrLength( header +offset );
	
	disc.id = QString::fromLocal8Bit( (const char*)header, 6 );
	disc.title = QString::fromLocal8Bit( (const char*)header +offset, length );
	disc.region = QChar( header[ 0x3 ] ).unicode();
}

qint64 Driver::minimumWBFSFileSize()
{
	return WBFS_FILE_MINIMUM_SIZE;
}

QWBFS::Model::Disc Driver::isoDiscInfo( const QString& filePath )
{
	const QString tmpFile = QString( "%1/%2.tmp%3" )
		.arg( QDir::tempPath() )
		.arg( QApplication::applicationName() )
		.arg( minimumWBFSFileSize() );
	QWBFS::Model::Disc disc;
	
	// create temporary wbfs container for estimating the size
	if ( !QFile::exists( tmpFile ) ) {
		if ( Driver::allocateFile( tmpFile, minimumWBFSFileSize() ) != QWBFS::Driver::Ok ) {
			qWarning() << Q_FUNC_INFO << "Can't create tmp file" << tmpFile.toLocal8Bit().constData();
			return disc;
		}
	}
	
	QWBFS::Partition::Properties properties( tmpFile );
	properties.reset = true;
	
	QWBFS::Partition::Handle handle( properties );
	
	if ( handle.isValid() ) {
		void* fileHandle = wbfs_open_file_for_read( filePath.toLocal8Bit().data() );

		if ( fileHandle ) {
			u8* header = (u8*)wbfs_ioalloc( 0x100 );
			
			disc.size = wbfs_estimate_disc( handle.ptr(), wbfs_read_wii_file, fileHandle, ONLY_GAME_PARTITION, header );
			disc.origin = filePath;
			discInfo( header, disc );
			
			wbfs_iofree( header );
			wbfs_close_file( fileHandle );
		}
		else {
			qWarning() << Q_FUNC_INFO << "Invalid file handle";
		}
	}
	else {
		qWarning() << Q_FUNC_INFO << "Invalid wbfs handle";
	}
	
	//QFile::remove( tmpFile ); // should delete it but may be not performant for multiple requests of iso estimated size
	
	return disc;
}

Driver::FileType Driver::fileType( const QString& filePath )
{
	const QFileInfo file( filePath );
	const QString suffix = file.isDir() ? QString::null : file.suffix().toLower();
	
	if ( suffix == "iso" ) {
		return QWBFS::Driver::ISOFile;
	}
	else if ( suffix == "wbfs" ) {
		if ( isWBFSPartitionOrFile( filePath ) ) {
			return QWBFS::Driver::WBFSFile;
		}
	}
	else if ( suffix.isEmpty() ) {
		if ( isWBFSPartitionOrFile( filePath ) ) {
			return QWBFS::Driver::WBFSPartitionFile;
		}
	}
	
	return QWBFS::Driver::UnknownFile;
}

int Driver::discRead_callback( void* fp, u32 lba, u32 count, void* iobuf )
{
	int ret = wbfs_disc_read( (wbfs_disc_t*)fp, lba, (u8*)iobuf, count );
	static int num_fail = 0;
	
	if ( ret ) {
		if ( num_fail == 0 ) {
			qWarning() << "Error reading lba probably the two wbfs don't have the same granularity. Ignoring...";
		}
		
		if ( num_fail++ > 0x100 ) {
			qWarning() << "Too many error giving up...";
			return 1;
		}
	}
	else {
		num_fail = 0;
	}
	
	return 0;
}

QTime Driver::estimatedTimeForTask( int x, int max )
{
	static time_t start_time;
	static u32 expected_total;
	u32 d;
	u32 h, m, s;

	if ( x == 0 ) {
		start_time = time( 0 );
		expected_total = 300;
	}

	if ( x == max ) {
		d = (u32)( time( 0 ) -start_time );
		h = d /3600;
		m = ( d /60 ) %60;
		s = d %60;
		
		return QTime( h, m, s );
	}

	d = (u32)( time( 0 ) -start_time );

	if ( x != 0 ) {
		expected_total = (u32)( ( 3 *expected_total +d *max /x ) /4 );
	}

	if ( expected_total > d ) {
		d = expected_total -d;
	}
	else {
		d = 0;
	}

	h = d /3600;
	m = ( d /60 ) %60;
	s = d %60;
	
	return QTime( h, m, s );
}

void Driver::progress_callback( int x, int max )
{
	if ( mCurrentDriver ) {
		emit mCurrentDriver->currentProgressChanged( x, max, estimatedTimeForTask( x, max ) );
	}
}
