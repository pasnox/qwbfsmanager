#ifndef QWBFS_H
#define QWBFS_H

#include <QObject>
#include <QFileInfo>

#include "Disc.h"

class qWBFS : public QObject
{
	Q_OBJECT
	
public:
	struct Properties
	{
		Properties();
		
		bool force;
		bool reset;
		QString disk;
		QString partition;
	};
	
	struct Handle
	{
		Handle( const qWBFS::Properties& properties );
		~Handle();
		
		bool isValid() const;
		wbfs_t* ptr() const;
		
		wbfs_t* mHandle;
		qWBFS::Properties properties;
	};
	
	struct DiscHandle
	{
		DiscHandle( const qWBFS::Handle& handle, const QString& id );
		~DiscHandle();
		
		QString isoName() const;
		bool isValid() const;
		wbfs_disc_t* ptr() const;
		
		wbfs_disc_t* mHandle;
	};
	
	struct PartitionStatus
	{
		PartitionStatus( const qWBFS::Handle& handle );
		
		qint64 size;
		qint64 used;
		qint64 free;
	};
	
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
	qWBFS::PartitionStatus partitionStatus() const;
	bool renameDisc( const QString& id, const QString& name );
	bool format();

protected:
	mutable qWBFS::Properties mProperties;
	mutable QString mLastError;
};

#endif // QWBFS_H
