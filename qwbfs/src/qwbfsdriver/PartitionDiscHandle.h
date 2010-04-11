#ifndef PARTITIONDISCHANDLE_H
#define PARTITIONDISCHANDLE_H

#include <QSharedData>

#include "qwbfsdriver/PartitionHandle.h"

namespace QWBFS {
namespace Partition {

struct DiscHandleData;

struct DiscHandle
{
	DiscHandle( const QWBFS::Partition::Handle& handle, const QString& id );
	~DiscHandle();
	
	bool isValid() const;
	wbfs_disc_t* ptr() const;
	
	QString id() const;
	QString isoName() const;
	
protected:
	QSharedDataPointer<DiscHandleData> d;
};

}; // Partition
}; // QWBFS

#endif // PARTITIONDISCHANDLE_H
