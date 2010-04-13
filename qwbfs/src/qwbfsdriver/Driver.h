#ifndef DRIVER_H
#define DRIVER_H

#include <QObject>
#include <QStringList>
#include <QTime>
#include <QMutex>

#include "qwbfsdriver/PartitionProperties.h"
#include "qwbfsdriver/PartitionHandle.h"
#include "qwbfsdriver/PartitionStatus.h"
#include "models/Disc.h"

namespace QWBFS {

class Driver : public QObject
{
	Q_OBJECT
	
public:
	Driver( QObject* parent = 0, const QWBFS::Partition::Handle& partitionHandle = QWBFS::Partition::Handle() );
	virtual ~Driver();
	
	void setPartition( const QString& partition );
	QString partition() const;
	
	QStringList lastErrors() const;
	
	bool open();
	void close();
	bool isOpen() const;
	bool isWBFSPartition( const QString& fileName ) const;
	QWBFS::Partition::Handle handle() const;
	
	QWBFS::Partition::Status partitionStatus() const;
	QWBFS::Model::DiscList partitionDiscList() const;
	
	bool format();
	bool haveDisc( const QString& id ) const;
	bool renameDisc( const QString& id, const QString& name ) const;
	bool removeDisc( const QString& id ) const;
	bool addDisc( const QWBFS::Model::Disc& disc, const QWBFS::Partition::Handle& sourcePartitionHandle = QWBFS::Partition::Handle() ) const;
	bool extractDisc( const QString& id, const QString& path ) const;
	
	static void setForceMode( bool force );
	static bool forceMode();
	
	static void addHandle( const QWBFS::Partition::Handle& handle );
	static QWBFS::Partition::Handle getHandle( const QString& partition, bool* created = 0 );
	static void closeHandle( const QWBFS::Partition::Handle& handle );

protected:
	mutable QWBFS::Partition::Properties mProperties;
	mutable QStringList mLastErrors;
	QWBFS::Partition::Handle mHandle;
	
	static QMutex mMutex;
	static bool mForce;
	static QWBFS::Driver* mCurrentDriver;
	static QHash<QString, QWBFS::Partition::Handle> mHandles;
	
	static int discRead_callback( void* fp, u32 lba, u32 count, void* iobuf );
	static void progress_callback( int value, int maximum );

signals:
	void currentProgressChanged( int value, int maximum, const QTime& remaining );
	void globalProgressChanged( int value );
};

}; // QWBFS

#endif // DRIVER_H
