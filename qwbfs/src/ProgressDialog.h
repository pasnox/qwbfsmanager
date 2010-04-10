#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include "ui_ProgressDialog.h"
#include "Disc.h"

#include <QPointer>
#include <QTime>

class ExportThread;

class ProgressDialog : public QDialog, public Ui::ProgressDialog
{
	Q_OBJECT
	
public:
	ProgressDialog( QWidget* parent = 0 );
	virtual ~ProgressDialog();
	
	void exportDiscs( const DiscList& discs, const QString& path );

protected:
	QPointer<ExportThread> mThread;
	QTime mElapsed;
	
	void closeEvent( QCloseEvent* event );

protected slots:
	void thread_started();
	void thread_currentProgressChanged( int value, int maximum, const QTime& remaining );
	void thread_finished();
};

#endif // PROGRESSDIALOG_H
