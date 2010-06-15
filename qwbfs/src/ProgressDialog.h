/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : ProgressDialog.h
** Date      : 2010-06-15T23:21:10
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR HARDWARE OR YOUR DATA
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
