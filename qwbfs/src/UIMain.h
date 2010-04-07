#ifndef UIMAIN_H
#define UIMAIN_H

#include "ui_UIMain.h"

class QFileSystemModel;
class DiscModel;

class UIMain : public QMainWindow, public Ui::UIMain
{
	Q_OBJECT

public:
	UIMain( QWidget* parent = 0 );
	virtual ~UIMain();
	
public slots:
	void updatePartitions();

protected:
	QStringList mPartitions;
	QFileSystemModel* mFoldersModel;
	QFileSystemModel* mFilesModel;
	DiscModel* mExportModel;
	
	void connectView( PartitionWidget* widget );

protected slots:
	void openViewRequested();
	void closeViewRequested();
	void on_tvFolders_activated( const QModelIndex& index );
	void on_tbClearExport_clicked();
	void on_tbRemoveExport_clicked();
	void on_tbExport_clicked();
};

#endif // UIMAIN_H
