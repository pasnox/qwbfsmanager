/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : UIMain.h
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
#ifndef UIMAIN_H
#define UIMAIN_H

#include "ui_UIMain.h"

class QFileSystemModel;

namespace QWBFS {
namespace Model {
	class DiscModel;
}; // Model
}; // QWBFS

class UIMain : public QMainWindow, public Ui::UIMain
{
	Q_OBJECT

public:
	UIMain( QWidget* parent = 0 );
	virtual ~UIMain();

protected:
	QStringList mPartitions;
	QFileSystemModel* mFoldersModel;
	QFileSystemModel* mFilesModel;
	QWBFS::Model::DiscModel* mExportModel;
	
	void connectView( PartitionWidget* widget );

protected slots:
	void openViewRequested();
	void closeViewRequested();
	void progress_jobFinished( const QWBFS::Model::Disc& disc );
	void on_aReloadPartitions_triggered();
	void on_aQuit_triggered();
	void on_aAbout_triggered();
	void on_tvFolders_activated( const QModelIndex& index );	void on_tbReloadDrives_clicked();	void on_cbDrives_currentIndexChanged( const QString& text );
	void on_tbClearExport_clicked();
	void on_tbRemoveExport_clicked();
	void on_tbExport_clicked();
};

#endif // UIMAIN_H
