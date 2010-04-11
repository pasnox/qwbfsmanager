#ifndef QWBFS_H
#define QWBFS_H

#include <QObject>
#include <QFileInfo>

#include "Disc.h"
#include "qwbfsdriver/PartitionProperties.h"
#include "qwbfsdriver/PartitionHandle.h"
#include "qwbfsdriver/PartitionStatus.h"

class qWBFS : public QObject
{
	Q_OBJECT
	
public:
	qWBFS( QObject* parent = 0 );
	virtual ~qWBFS();
	
	void setForceMode( bool force );
	bool forceMode() const;
	
	void setDisk( const QString& disk );
	QString disk() const;
	
	void setPartition( const QString& partition );
	QString partition() const;
	
	QString lastError() const;
	DiscList discs() const;
	QWBFS::Partition::Status partitionStatus() const;
	bool renameDisc( const QString& id, const QString& name );
	bool format();

protected:
	mutable QWBFS::Partition::Properties mProperties;
	mutable QString mLastError;
};

#endif // QWBFS_H
