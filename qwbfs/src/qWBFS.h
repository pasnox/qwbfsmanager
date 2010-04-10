#ifndef QWBFS_H
#define QWBFS_H

#include <QObject>
#include <QFileInfo>

#include "Disc.h"

class qWBFS : public QObject
{
	Q_OBJECT
	
public:
	enum Error
	{
		NoError,
		UnknownError,
		InvalidPartition
	};
	
	struct Properties
	{
		Properties();
		
		bool force;
		bool reset;
		QString disk;
		QString partition;
		qWBFS::Error lastError;
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
	
	DiscList discs() const;
	qWBFS::PartitionStatus partitionStatus() const;

protected:
	mutable qWBFS::Properties mProperties;
};

#endif // QWBFS_H
