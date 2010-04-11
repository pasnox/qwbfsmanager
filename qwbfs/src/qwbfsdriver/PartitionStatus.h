#ifndef PARTITIONSTATUS_H
#define PARTITIONSTATUS_H

#include "qwbfsdriver/PartitionHandle.h"

namespace QWBFS {
namespace Partition {

struct Status
{
	Status( const QWBFS::Partition::Handle& handle );
	
	qint64 size;
	qint64 used;
	qint64 free;
};

}; // Partition
}; // QWBFS

#endif // PARTITIONSTATUS_H
