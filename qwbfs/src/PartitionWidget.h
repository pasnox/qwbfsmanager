/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : PartitionWidget.h
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
	
	virtual bool event( QEvent* event );
	
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
	
	virtual void dragEnterEvent( QDragEnterEvent* event );
	virtual void dropEvent( QDropEvent* event );
	
	void localeChanged();

protected slots:
	void models_countChanged();
	void views_selectionChanged();
	void coverFlow_centerIndexChanged( const QModelIndex& index );
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
