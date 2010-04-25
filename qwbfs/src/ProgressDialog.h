/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : ProgressDialog.h
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
#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include "ui_ProgressDialog.h"
#include "models/Disc.h"
#include "qwbfsdriver/PartitionHandle.h"

#include <QPointer>
#include <QTime>

class ExportThread;

class ProgressDialog : public QDialog, public Ui::ProgressDialog
{
	Q_OBJECT
	
public:
	ProgressDialog( QWidget* parent = 0 );
	virtual ~ProgressDialog();
	
	void exportDiscs( const QWBFS::Model::DiscList& discs, const QString& path );
	void importDiscs( const QWBFS::Model::DiscList& discs, const QWBFS::Partition::Handle& partitionHandle );

protected:
	QPointer<ExportThread> mThread;
	QTime mElapsed;
	
	void closeEvent( QCloseEvent* event );

protected slots:
	void thread_started();
	void thread_jobFinished( const QWBFS::Model::Disc& disc );
	void thread_currentProgressChanged( int value, int maximum, const QTime& remaining );
	void thread_finished();
	void on_cbDetails_toggled();
	void updateSpace();

signals:
	void jobFinished( const QWBFS::Model::Disc& disc );
	void finished();
};

#endif // PROGRESSDIALOG_H
