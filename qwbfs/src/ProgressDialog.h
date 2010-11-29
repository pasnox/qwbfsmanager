/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : ProgressDialog.h
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
	
	virtual bool event( QEvent* event );
	
	void exportDiscs( const QWBFS::Model::DiscList& discs, const QString& path );
	void importDiscs( const QWBFS::Model::DiscList& discs, const QWBFS::Partition::Handle& partitionHandle );
	void convertIsoToWBFS( const QString& isoFilePath, const QString& wbfsFilePath = QString::null );

protected:
	QPointer<ExportThread> mThread;
	QTime mElapsed;
	
	void closeEvent( QCloseEvent* event );
	void localeChanged();

protected slots:
	void thread_started();
	void thread_log( const QString& text );
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
