#ifndef PARTITIONWIDGET_H
#define PARTITIONWIDGET_H

#include "ui_PartitionWidget.h"

class qWBFS;

namespace QWBFS {
	class Driver;
namespace Model {
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
	void showDriverErrors();

protected:
	QWBFS::Driver* mDriver;
	QWBFS::Model::DiscModel* mDiscModel;
	QWBFS::Model::DiscModel* mImportModel;

protected slots:
	void models_countChanged();
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
};

#endif // PARTITIONWIDGET_H
