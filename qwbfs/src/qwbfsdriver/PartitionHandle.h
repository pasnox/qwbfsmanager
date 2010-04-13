#ifndef PARTITIONHANDLE_H
#define PARTITIONHANDLE_H

#include <QSharedData>

#include <libwbfs.h>

#include "qwbfsdriver/PartitionProperties.h"

namespace QWBFS {
namespace Partition {

namespace Internal {

class HandleData : public QSharedData
{
public:
	HandleData( const QWBFS::Partition::Properties& properties = QWBFS::Partition::Properties() );
	HandleData( const HandleData& other );
	~HandleData();
	
	QWBFS::Partition::Properties properties;
	wbfs_t* handle;
};

}; // Internal

class Handle
{
public:
	Handle( const QWBFS::Partition::Properties& properties = QWBFS::Partition::Properties() );
	Handle( const QString& partition );
	~Handle();
	
	bool isValid() const;
	wbfs_t* ptr() const;
	
	QWBFS::Partition::Properties properties() const;
	
	bool reset() const;
	QString partition() const;
	
protected:
	QSharedDataPointer<Internal::HandleData> d;
};

}; // Partition
}; // QWBFS

#endif // PARTITIONHANDLE_H
