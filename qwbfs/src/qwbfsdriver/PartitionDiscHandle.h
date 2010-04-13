#ifndef PARTITIONDISCHANDLE_H
#define PARTITIONDISCHANDLE_H

#include <QSharedData>

#include "qwbfsdriver/PartitionHandle.h"

namespace QWBFS {
namespace Partition {

namespace Internal {

class DiscHandleData : public QSharedData
{
public:
	DiscHandleData( const QWBFS::Partition::Handle& handle = QWBFS::Partition::Handle(), const QString& id = QString::null );
	DiscHandleData( const DiscHandleData& other );
	~DiscHandleData();
	
	wbfs_disc_t* handle;
	QString id;
};

}; // Internal

struct DiscHandle
{
	DiscHandle( const QWBFS::Partition::Handle& handle, const QString& id );
	~DiscHandle();
	
	bool isValid() const;
	wbfs_disc_t* ptr() const;
	
	QString id() const;
	QString isoName() const;
	
protected:
	QSharedDataPointer<Internal::DiscHandleData> d;
};

}; // Partition
}; // QWBFS

#endif // PARTITIONDISCHANDLE_H
