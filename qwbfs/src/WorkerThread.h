/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : WorkerThread.h
** Date      : 2010-06-16T14:19:29
** License   : GPL2
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**
** In addition, as a special exception, the copyright holders give permission
** to link this program with the OpenSSL project's "OpenSSL" library (or with
** modified versions of it that use the same license as the "OpenSSL"
** library), and distribute the linked executables. You must obey the GNU
** General Public License in all respects for all of the code used other than
** "OpenSSL".  If you modify file(s), you may extend this exception to your
** version of the file(s), but you are not obligated to do so. If you do not
** wish to do so, delete this exception statement from your version.
**
****************************************************************************/
#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QTime>

#include "models/Disc.h"
#include "qwbfsdriver/PartitionHandle.h"
#include "qwbfsdriver/Driver.h"

class QWidget;

class WorkerThread : public QThread
{
	Q_OBJECT
	
	enum BasicTask {
		Export = 0x1,
		Import = 0x2,
		Convert = 0x4,
		Rename = 0x8,
		ISO = 0x10,
		WBFS = 0x20,
		Indirect = 0x40
	};
	
public:
	enum Task {
		ExportISO = Export | ISO,
		ExportWBFS = Export | WBFS,
		ImportISO = Import | ISO,
		ImportWBFS = Import | WBFS,
		ConvertISO = Convert | ISO,
		ConvertWBFS = Convert | WBFS,
		RenameAll = Rename | WBFS | ISO
	};
	
	struct Work
	{
		WorkerThread::Task task;
		QWBFS::Model::DiscList discs;
		QString target;
		QString pattern;
		QWidget* window;
	};
	
	WorkerThread( QObject* parent = 0 );
	virtual ~WorkerThread();
	
	WorkerThread::Task task() const;
	
	bool setWork( const WorkerThread::Work& work );
	
	static QString taskToWindowTitle( WorkerThread::Task task, bool indirect = false );
	static QString taskToLabel( WorkerThread::Task task, bool indirect = false );

public slots:
	void stop();

protected:
	QMutex mMutex;
	bool mStop;
	WorkerThread::Work mWork;
	
	virtual void run();
	
	void connectDriver( QWBFS::Driver* driver );
	void renameDisc( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& target, const QString& pattern );
	void isoToWBFS( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& target, bool trimWBFS );
	void wbfsToISO( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& target );
	void isoToISO( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& target );
	void wbfsToWBFS( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& target, bool trimWBFS );

signals:
	void currentProgressChanged( int value, int maximum, const QTime& remaining );
	void globalProgressChanged( int value, int maximum );
	void jobFinished( const QWBFS::Model::Disc& disc );
	void message( const QString& text );
	void log( const QString& text );
	void canceled();
};

#endif // WORKERTHREAD_H
