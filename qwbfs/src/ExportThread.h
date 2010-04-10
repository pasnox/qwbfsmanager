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
	ExportThread( QObject* parent = 0 );
	virtual ~ExportThread();
	
	bool exportDiscs( const DiscList& discs, const QString& path );
	void emitCurrentProgressChanged( int value, int maximum, const QTime& remaining );

public slots:
	void stop();

protected:
	DiscList mDiscs;
	QString mPath;
	bool mStop;
	QMutex mMutex;
	static ExportThread* mCurrentExportThread;
	
	virtual void run();
	
	static void progress_callback( int value, int maximum );

signals:
	void currentProgressChanged( int value, int maximum, const QTime& remaining );
	void globalProgressChanged( int value );
	void message( const QString& text );
	void error( const QString& text );
};

#endif // EXPORTTHREAD_H
