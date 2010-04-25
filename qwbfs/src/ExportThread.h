/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : ExportThread.h
** Date      : 2010-04-25T13:05:33
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a crossplatform WBFS Manager developed using Qt4/C++.
** It's currently working under Unix/Linux, Mac OS X, and build under windows (but not yet working).
** 
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR WII CONSOLE OR WII PARTITION
** BECAUSE OF IMPROPER USAGE OF THIS SOFTWARE.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
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
