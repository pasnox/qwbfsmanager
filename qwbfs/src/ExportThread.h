#ifndef EXPORTTHREAD_H
#define EXPORTTHREAD_H

#include <QThread>
#include <QMutex>
#include <QTime>

#include "models/Disc.h"
#include "qwbfsdriver/PartitionHandle.h"
#include "qwbfsdriver/Driver.h"

class ExportThread : public QThread
{
	Q_OBJECT
	
public:
	enum Task
	{
		Export,
		Import
	};
	
	ExportThread( QObject* parent = 0 );
	virtual ~ExportThread();
	
	ExportThread::Task task() const;
	
	bool exportDiscs( const QWBFS::Model::DiscList& discs, const QString& path );
	bool importDiscs( const QWBFS::Model::DiscList& discs, const QWBFS::Partition::Handle& partitionHandle );
	
	static QString taskToString( ExportThread::Task task );

public slots:
	void stop();

protected:
	ExportThread::Task mTask;
	QWBFS::Model::DiscList mDiscs;
	QString mPath;
	QWBFS::Partition::Handle mImportPartitionHandle;
	bool mStop;
	QMutex mMutex;
	
	virtual void run();
	
	void connectDriver( const QWBFS::Driver& driver );
	void exportWorker();
	void importWorker();

signals:
	void currentProgressChanged( int value, int maximum, const QTime& remaining );
	void globalProgressChanged( int value );
	void jobFinished( const QWBFS::Model::Disc& disc );
	void message( const QString& text );
};

#endif // EXPORTTHREAD_H
