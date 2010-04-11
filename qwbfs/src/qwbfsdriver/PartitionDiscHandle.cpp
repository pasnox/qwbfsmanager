#include "PartitionDiscHandle.h"

#include <QDebug>

using namespace QWBFS::Partition;

// DiscHandleData

namespace QWBFS {
namespace Partition {

struct DiscHandleData : public QSharedData
{
	DiscHandleData()
	{
		handle = 0;
		id.clear();
	}
	
	DiscHandleData( const DiscHandleData& other )
		: QSharedData( other )
	{
		handle = other.handle;
		id = other.id;
	}
	
	~DiscHandleData()
	{
		if ( handle ) {
			wbfs_close_disc( handle );
		}
		
		qWarning() << Q_FUNC_INFO;
	}
	
	wbfs_disc_t* handle;
	QString id;
};

}; // Partition
}; // QWBFS

// DiscHandleData

DiscHandle::DiscHandle( const QWBFS::Partition::Handle& handle, const QString& id )
{
	d = new DiscHandleData;
	d->handle = wbfs_open_disc( handle.ptr(), (u8*)( id.toLocal8Bit().data() ) );
	d->id = id;
}

DiscHandle::~DiscHandle()
{
	qWarning() << Q_FUNC_INFO;
}

QString DiscHandle::id() const
{
	return d->id;
}

QString DiscHandle::isoName() const
{
	if ( !isValid() ) {
		return QString::null;
	}
	
	QString isoname = QString::fromLocal8Bit( QString::fromLocal8Bit( (char*)d->handle->header->disc_header_copy +0x20, 0x100 )
		.replace( ' ', '_' )
		.replace( '/', '_' )
		.replace( ':', '_' ).trimmed().toLocal8Bit().constData() );
	
	if ( isoname.length() >= 0x100 ) {
		isoname.chop( ( isoname.length() -0x100 ) +4 );
	}
	
	isoname.append( ".iso" );
	
	return isoname;
}

bool DiscHandle::isValid() const
{
	return d->handle;
}

wbfs_disc_t* DiscHandle::ptr() const
{
	return d->handle;
}
