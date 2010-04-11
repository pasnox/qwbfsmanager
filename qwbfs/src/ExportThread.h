#ifndef EXPORTTHREAD_H
#define EXPORTTHREAD_H

#include <QThread>
#include <QMutex>
#include <QTime>

#include "Disc.h"

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
	
	bool exportDiscs( const DiscList& discs, const QString& path );
	bool importDiscs( const DiscList& discs, const QString& partition );
	
	void emitCurrentProgressChanged( int value, int maximum, const QTime& remaining );
	void emitMessage( const QString& text );
	void emitError( const QString& text );

public slots:
	void stop();

protected:
	ExportThread::Task mTask;
	DiscList mDiscs;
	QString mPath;
	QString mPartition;
	bool mStop;
	QMutex mMutex;
	static ExportThread* mCurrentExportThread;
	
	virtual void run();
	
	void exportWorker();
	void importWorker();
	
	static int discRead_callback( void* fp, u32 lba, u32 count, void* iobuf );
	static void progress_callback( int value, int maximum );

signals:
	void currentProgressChanged( int value, int maximum, const QTime& remaining );
	void globalProgressChanged( int value );
	void message( const QString& text );
	void error( const QString& text );
};

#endif // EXPORTTHREAD_H
