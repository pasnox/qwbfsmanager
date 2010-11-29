/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : ExportThread.h
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
	enum Task {
		Export,
		Import,
		Convert
	};
	
	ExportThread( QObject* parent = 0 );
	virtual ~ExportThread();
	
	ExportThread::Task task() const;
	
	bool exportDiscs( const QWBFS::Model::DiscList& discs, const QString& path );
	bool importDiscs( const QWBFS::Model::DiscList& discs, const QWBFS::Partition::Handle& partitionHandle );
	bool convertIsoToWBFS( const QString& isoFilePath, const QString& wbfsFilePath = QString::null );
	
	static QString taskToString( ExportThread::Task task );

public slots:
	void stop();

protected:
	ExportThread::Task mTask;
	QWBFS::Model::DiscList mDiscs;
	QString mPath;
	QWBFS::Partition::Handle mImportPartitionHandle;
	QPair<QString, QString> mConvertFile;
	bool mStop;
	QMutex mMutex;
	
	virtual void run();
	
	void connectDriver( const QWBFS::Driver& driver );
	void exportWorker();
	void importWorker();
	void convertWorker();

signals:
	void currentProgressChanged( int value, int maximum, const QTime& remaining );
	void globalProgressChanged( int value );
	void jobFinished( const QWBFS::Model::Disc& disc );
	void message( const QString& text );
	void log( const QString& text );
};

#endif // EXPORTTHREAD_H
