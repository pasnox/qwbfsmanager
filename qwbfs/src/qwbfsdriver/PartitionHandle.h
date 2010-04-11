#ifndef PARTITIONHANDLE_H
#define PARTITIONHANDLE_H

#include <QSharedData>

#include <libwbfs.h>

#include "qwbfsdriver/PartitionProperties.h"

namespace QWBFS {
namespace Partition {

struct HandleData;

class Handle
{
public:
	Handle( const QWBFS::Partition::Properties& properties = QWBFS::Partition::Properties() );
	~Handle();
	
	bool isValid() const;
	wbfs_t* ptr() const;
	
	bool force() const;
	bool reset() const;
	QString disk() const;
	QString partition() const;
	
protected:
	QSharedDataPointer<HandleData> d;
};

}; // Partition
}; // QWBFS

#endif // PARTITIONHANDLE_H
