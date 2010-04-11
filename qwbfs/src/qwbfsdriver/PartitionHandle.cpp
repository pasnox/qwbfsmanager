#include "PartitionHandle.h"

#include <QDebug>

using namespace QWBFS::Partition;

// HandleData

namespace QWBFS {
namespace Partition {

struct HandleData : public QSharedData
{
	HandleData()
	{
		handle = 0;
	}
	
	HandleData( const HandleData& other )
		: QSharedData( other )
	{
		properties = other.properties;
		handle = other.handle;
	}
	
	~HandleData()
	{
		if ( handle ) {
			wbfs_close( handle );
		}
		
		qWarning() << Q_FUNC_INFO;
	}
	
	QWBFS::Partition::Properties properties;
	wbfs_t* handle;
};

}; // Partition
}; // QWBFS

// Handle

Handle::Handle( const QWBFS::Partition::Properties& properties )
{
	d = new HandleData;
	d->properties = properties;
	d->handle = wbfs_try_open( properties.disk.toLocal8Bit().data(), properties.partition.toLocal8Bit().data(), properties.reset ? 1 : 0 );
}

Handle::~Handle()
{
	qWarning() << Q_FUNC_INFO;
}

bool Handle::isValid() const
{
	return d->handle;
}

wbfs_t* Handle::ptr() const
{
	return d->handle;
}

bool Handle::force() const
{
	return d->properties.force;
}

bool Handle::reset() const
{
	return d->properties.reset;
}

QString Handle::disk() const
{
	return d->properties.disk;
}

QString Handle::partition() const
{
	return d->properties.partition;
}
