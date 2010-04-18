#include "Driver.h"
#include "PartitionDiscHandle.h"
#include "models/Disc.h"

#include <libwbfs.h>

#include <QDir>
#include <QDebug>

using namespace QWBFS;

QMutex Driver::mMutex;
bool Driver::mForce = false;
QHash<QString, QWBFS::Partition::Handle> Driver::mHandles;
QWBFS::Driver* Driver::mCurrentDriver = 0;

Driver::Driver( QObject* parent, const QWBFS::Partition::Handle& partitionHandle )
	: QObject( parent )
{
	mProperties = partitionHandle.properties();
	mHandle = partitionHandle;
	mHasCreatedHandle = false;
}

Driver::~Driver()
{
	close();
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
	
	disc.origin = mHandle.partition();

	if ( wbfs_get_disc_info( mHandle.ptr(), index, header, headerSize, &disc.size ) != 0 ) {
		wbfs_iofree( header );
		return Driver::DiscReadFailed;
	}
	
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
	if ( !isOpen() ) {
		return Driver::PartitionNotOpened;
	}
	
	void* fileHandle = wbfs_open_file_for_read( fileName.toLocal8Bit().data() );

	if ( !fileHandle ) {
		return Driver::DiscReadFailed;
	}
	
	disc.origin = fileName;
	u8* header = (u8*)wbfs_ioalloc( 0x100 );
	disc.size = wbfs_estimate_disc( mHandle.ptr(), wbfs_read_wii_file, fileHandle, partitionSelection, header );
	
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
	void* fileHandle = wbfs_open_file_for_write( filePath.toLocal8Bit().data() );

	if ( fileHandle ) {
		// write a zero at the end of the iso to ensure the correct size
		// XXX should check if the game is DVD9..
		wbfs_file_reserve_space( fileHandle, ( discHandle.ptr()->p->n_wii_sec_per_disc /2 ) *0x8000ULL );
		const int result = wbfs_extract_disc( discHandle.ptr(), wbfs_write_wii_file, fileHandle, progressCallback );
		wbfs_close_file( fileHandle );
		
		if ( result == 0 ) {
			return Driver::Ok;
		}
		else {
			return Driver::DiscExtractFailed;
		}
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
	
	if ( wbfs_add_disc( mHandle.ptr(), discRead_callback/*wbfs_disc_read*/, discHandle.ptr(), progressCallback, partitionSelection, 0 ) == 0 ) {
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
	
	if ( ( sourcePartitionHandle.ptr()->wbfs_sec_sz /sourcePartitionHandle.ptr()->hd_sec_sz ) == ( mHandle.ptr()->wbfs_sec_sz /mHandle.ptr()->hd_sec_sz ) ) {
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

bool Driver::isWBFSPartition( const QString& fileName )
{
	if ( !QFile::exists( fileName ) ) {
		return false;
	}
	
	QFile file( fileName );
	
	if ( file.open( QIODevice::ReadOnly ) ) {
		return file.read( 4 ).toLower() == "wbfs";
	}
	
	return false;
}

QString Driver::errorToString( QWBFS::Driver::Error error )
{
	switch ( error )
	{
		case Driver::Ok:
			return tr( "No error." );
		case Driver::PartitionNotOpened:
			return tr( "Partition not opened." );
		case Driver::SourcePartitionNotOpened:
			return tr( "Source partition not opened." );
		case Driver::DiscReadFailed:
			return tr( "Disc read failed." );
		case Driver::DiscWriteFailed:
			return tr( "Disc write failed." );
		case Driver::DiscExtractFailed:
			return tr( "Disc extract failed." );
		case Driver::DiscAddFailed:
			return tr( "Disc add failed." );
		case Driver::DiscFound:
			return tr( "Disc found." );
		case Driver::DiscNotFound:
			return tr( "Disc not found (or not exists)." );
		case Driver::InvalidDiscIndex:
			return tr( "Invalid disc index." );
		case Driver::InvalidDiscID:
			return tr( "Invalid disc id." );
		case Driver::InvalidDisc:
			return tr( "Invalid disc." );
		case Driver::CantDrive2Drive:
			return tr( "Can't drive to drive copy." );
	}
	
	return QString::null;
}

QString Driver::regionToString( QWBFS::Driver::Region region )
{
	switch ( region )
	{
		case Driver::NTSC:
			return tr( "NTSC" );
		case Driver::NTSCJapan:
			return tr( "NTSC Japan" );
		case Driver::PAL:
			return tr( "PAL" );
		case Driver::Korean:
			return tr( "Korean" );
		case Driver::NoRegion:
			return tr( "World" );
	}
	
	return QString::null;
}

QString Driver::stateToString( QWBFS::Driver::State state )
{
	switch ( state )
	{
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
				*created = false;
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
	
	switch ( header[ 0x3 ] ) {
		case 'E':
			disc.region = Driver::NTSC;
			break;
		case 'P':
		case 'F':
		case 'L':
		case 'D':
			disc.region = Driver::PAL;
			break;
		case 'J':
			disc.region = Driver::NTSCJapan;
			break;
		case 'K':
		case 'Q':
		case 'T':
			disc.region = Driver::Korean;
			break;
		default:
			disc.region = Driver::NoRegion;
	}

	const int offset = 0x20;
	const int length = u8StrLength( header +offset );
	
	disc.id = QString::fromLocal8Bit( (const char*)header, 6 );
	disc.title = QString::fromLocal8Bit( (const char*)header +offset, length );
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

void Driver::progress_callback( int x, int max )
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
		
		if ( mCurrentDriver ) {
			emit mCurrentDriver->currentProgressChanged( x, max, QTime( h, m, s ) );
		}
		
		return;
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
	
	if ( mCurrentDriver ) {
		emit mCurrentDriver->currentProgressChanged( x, max, QTime( h, m, s ) );
	}
}
