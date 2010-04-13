#include "PartitionHandle.h"

#include <QDebug>

using namespace QWBFS::Partition;
using namespace QWBFS::Partition::Internal;

// HandleData

HandleData::HandleData( const QWBFS::Partition::Properties& _properties )
{
	properties = _properties;
	handle = wbfs_try_open_partition( properties.partition.toLocal8Bit().data(), properties.reset ? 1 : 0 );
	
	if ( handle ) {
		qWarning() << QString( "*** Opened partition: %1" ).arg( properties.partition ).toLocal8Bit().constData();
	}
}

HandleData::HandleData( const HandleData& other )
	: QSharedData( other )
{
	properties = other.properties;
	handle = other.handle;
}

HandleData::~HandleData()
{	
	if ( handle ) {
		wbfs_close( handle );
		qWarning() << QString( "*** Closed partition: %1" ).arg( properties.partition ).toLocal8Bit().constData();
	}
	
	//qWarning() << Q_FUNC_INFO;
}

// Handle

Handle::Handle( const QWBFS::Partition::Properties& properties )
{
	d = new HandleData( properties );
}

Handle::Handle( const QString& partition )
{
	d = new HandleData( QWBFS::Partition::Properties( partition ) );
}

Handle::~Handle()
{
	//qWarning() << Q_FUNC_INFO;
}

bool Handle::isValid() const
{
	return d->handle;
}

wbfs_t* Handle::ptr() const
{
	return d->handle;
}

QWBFS::Partition::Properties Handle::properties() const
{
	return d->properties;
}

bool Handle::reset() const
{
	return d->properties.reset;
}

QString Handle::partition() const
{
	return d->properties.partition;
}
