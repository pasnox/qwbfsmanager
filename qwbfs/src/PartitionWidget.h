/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : PartitionWidget.h
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
#ifndef PARTITIONWIDGET_H
#define PARTITIONWIDGET_H

#include "ui_PartitionWidget.h"

namespace QWBFS {
class Driver;
namespace Model {
struct Disc;
class DiscModel;
}; // Model
}; // QWBFS

class PartitionWidget : public QWidget, public Ui::PartitionWidget
{
	Q_OBJECT

public:
	PartitionWidget( QWidget* parent = 0 );
	virtual ~PartitionWidget();
	
	const QWBFS::Driver* driver() const;
	QWBFS::Model::DiscModel* discModel() const;
	QWBFS::Model::DiscModel* importModel() const;
	QToolButton* showHideImportViewButton() const;
	QString currentPartition() const;
	
	void setMainView( bool main );

public slots:
	void setPartitions( const QStringList& partitions );
	void setCurrentPartition( const QString& partition );
	void showError( const QString& error );
	void showError( int error );

protected:
	QWBFS::Driver* mDriver;
	QWBFS::Model::DiscModel* mDiscModel;
	QWBFS::Model::DiscModel* mImportModel;
	
	virtual void dragEnterEvent( QDragEnterEvent* event );
	virtual void dropEvent( QDropEvent* event );

protected slots:
	void models_countChanged();
	void views_selectionChanged();
	void progress_jobFinished( const QWBFS::Model::Disc& disc );
	void progress_finished();
	
	void on_cbPartitions_currentIndexChanged( int index );
	
	void on_tbLoad_clicked();
	void on_tbFormat_clicked();
	void on_tbOpen_clicked();
	void on_tbClose_clicked();
	
	void on_tbRemoveDiscs_clicked();
	void on_tbRenameDisc_clicked();
	
	void on_tbClearImport_clicked();
	void on_tbRemoveImport_clicked();
	void on_tbImport_clicked();

signals:
	void openViewRequested();
	void closeViewRequested();
	void coverRequested( const QString& id );
};

#endif // PARTITIONWIDGET_H
