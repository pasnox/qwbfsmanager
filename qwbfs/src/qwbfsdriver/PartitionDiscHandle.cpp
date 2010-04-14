#include "PartitionDiscHandle.h"

#include <QDebug>

using namespace QWBFS::Partition;
using namespace QWBFS::Partition::Internal;

// DiscHandleData

DiscHandleData::DiscHandleData( const QWBFS::Partition::Handle& _handle, const QString& _discId )
{
	handle = wbfs_open_disc( _handle.ptr(), (u8*)( _discId.toLocal8Bit().data() ) );
	discId = _discId;
	index = handle ? handle->i : -1;
	
	if ( handle ) {
		QString( "*** Opened disc: %1" ).arg( discId ).toLocal8Bit().constData();
	}
}

DiscHandleData::DiscHandleData( const DiscHandleData& other )
	: QSharedData( other )
{
	handle = other.handle;
	discId = other.discId;
	index = other.index;
}

DiscHandleData::~DiscHandleData()
{
	if ( handle ) {
		wbfs_close_disc( handle );
		QString( "*** Closed disc: %1" ).arg( discId ).toLocal8Bit().constData();
	}
	
	//qWarning() << Q_FUNC_INFO;
}

// DiscHandleData

DiscHandle::DiscHandle( const QWBFS::Partition::Handle& handle, const QString& discId )
{
	d = new DiscHandleData( handle, discId );
}

DiscHandle::~DiscHandle()
{
	//qWarning() << Q_FUNC_INFO;
}

QString DiscHandle::discId() const
{
	return d->discId;
}

int DiscHandle::index() const
{
	return d->index;
}

QString DiscHandle::isoName() const
{
	if ( !isValid() ) {
		return QString::null;
	}
	
	QString isoName = QString::fromLocal8Bit( QString::fromLocal8Bit( (char*)d->handle->header->disc_header_copy +0x20, 0x100 )
		.replace( ' ', '_' )
		.replace( '/', '_' )
		.replace( ':', '_' ).trimmed().toLocal8Bit().constData() );
	
	while ( isoName.contains( "__" ) ) {
		isoName.replace( "__", "_" );
	}
	
	if ( isoName.length() >= 0x100 ) {
		isoName.chop( ( isoName.length() -0x100 ) +4 );
	}
	
	isoName.append( ".iso" );
	
	return isoName;
}

bool DiscHandle::isValid() const
{
	return d->handle;
}

wbfs_disc_t* DiscHandle::ptr() const
{
	return d->handle;
}
