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
	DiscHandleData( const QWBFS::Partition::Handle& handle = QWBFS::Partition::Handle(), const QString& discId = QString::null );
	DiscHandleData( const DiscHandleData& other );
	~DiscHandleData();
	
	wbfs_disc_t* handle;
	QString discId;
	int index;
};

}; // Internal

struct DiscHandle
{
	DiscHandle( const QWBFS::Partition::Handle& handle, const QString& discId );
	~DiscHandle();
	
	bool isValid() const;
	wbfs_disc_t* ptr() const;
	
	QString discId() const;
	int index() const;
	QString isoName() const;
	
protected:
	QSharedDataPointer<Internal::DiscHandleData> d;
};

}; // Partition
}; // QWBFS

#endif // PARTITIONDISCHANDLE_H
