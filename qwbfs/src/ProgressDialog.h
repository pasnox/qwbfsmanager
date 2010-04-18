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
