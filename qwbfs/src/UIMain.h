/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : UIMain.h
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
#ifndef UIMAIN_H
#define UIMAIN_H

#include "ui_UIMain.h"

class QFileSystemModel;
class DataNetworkCache;

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
	
	DataNetworkCache* cache() const;
	QPixmap cachedPixmap( const QUrl& url ) const;

protected:
	QStringList mPartitions;
	QFileSystemModel* mFoldersModel;
	QFileSystemModel* mFilesModel;
	QWBFS::Model::DiscModel* mExportModel;
	DataNetworkCache* mCache;
	QString mLastDiscId;
	
	virtual void showEvent( QShowEvent* event );
	virtual void closeEvent( QCloseEvent* event );
	
	void connectView( PartitionWidget* widget );

protected slots:
	void propertiesChanged();
	void openViewRequested();
	void closeViewRequested();
	void coverRequested( const QString& id );
	void progress_jobFinished( const QWBFS::Model::Disc& disc );
	void dataNetworkCache_dataCached( const QUrl& url );
	void dataNetworkCache_error( const QString& message, const QUrl& url );
	void dataNetworkCache_invalidated();
	void on_aReloadPartitions_triggered();
	void on_aQuit_triggered();
	void on_aAbout_triggered();
	void on_aProperties_triggered();
	void on_tvFolders_activated( const QModelIndex& index );
	void on_tbReloadDrives_clicked();
	void on_cbDrives_currentIndexChanged( const QString& text );
	void on_tbClearExport_clicked();
	void on_tbRemoveExport_clicked();
	void on_tbExport_clicked();
};

#endif // UIMAIN_H
