/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : WorkerThread.cpp
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
#include "WorkerThread.h"

#include <QTime>
#include <QWidget>
#include <QDir>
#include <QMetaType>
#include <QDebug>

WorkerThread::WorkerThread( QObject* parent )
	: QThread( parent )
{
	qRegisterMetaType<QWBFS::Model::Disc>( "QWBFS::Model::Disc" );
}

WorkerThread::~WorkerThread()
{
	if ( isRunning() ) {
		qWarning() << "Waiting thread to finish...";
		stop();
		wait();
	}
	
	//qWarning() << Q_FUNC_INFO;
}

WorkerThread::Task WorkerThread::task() const
{
	QMutexLocker locker( &const_cast<WorkerThread*>( this )->mMutex );
	return mWork.task;
}

bool WorkerThread::setWork( const WorkerThread::Work& work )
{
	if ( isRunning() ) {
		Q_ASSERT( 0 );
		return false;
	}
	
	mWork = work;
	mWork.window->setWindowTitle( taskToWindowTitle( mWork.task ) );
	
	start();
	
	return true;
}

QString WorkerThread::taskToWindowTitle( WorkerThread::Task task, bool indirect )
{
	return taskToLabel( task, indirect ).append( "..." );
}

QString WorkerThread::taskToLabel( WorkerThread::Task task, bool indirect )
{
	switch ( task ) {
		case WorkerThread::ExportISO:
			return indirect ? tr( "Indirect Export to ISO" ) : tr( "Export to ISO" );
		case WorkerThread::ExportWBFS:
			return indirect ? tr( "Indirect Export to WBFS" ) : tr( "Export to WBFS" );
		case WorkerThread::ImportISO:
			return indirect ? tr( "Indirect Import to ISO" ) : tr( "Import to ISO" );
		case WorkerThread::ImportWBFS:
			return indirect ? tr( "Indirect Import to WBFS" ) : tr( "Import to WBFS" );
		case WorkerThread::ConvertISO:
			return indirect ? tr( "Indirect Convert to ISO" ) : tr( "Convert to ISO" );
		case WorkerThread::ConvertWBFS:
			return indirect ? tr( "Indirect Convert to WBFS" ) : tr( "Convert to WBFS" );
	}
	
	return QString::null;
}

void WorkerThread::stop()
{
	QMutexLocker locker( &mMutex );
	mStop = true;
	emit canceled();
}

void WorkerThread::run()
{
	WorkerThread::Work work;
	int count = 0;
	
	{
		QMutexLocker locker( &mMutex );
		mStop = false;
		work = mWork;
	}
	
	emit globalProgressChanged( 0 );
	
	foreach ( QWBFS::Model::Disc disc, work.discs ) {
		switch ( QWBFS::Driver::fileType( disc.origin ) ) {
			case QWBFS::Driver::WBFSFile:
			case QWBFS::Driver::WBFSPartitionFile: {
				switch ( QWBFS::Driver::fileType( work.target ) ) {
					case QWBFS::Driver::WBFSPartitionFile: {
						wbfsToWBFS( work.task, disc, work.target, false );
						break;
					}
					case QWBFS::Driver::WBFSFile:
					case QWBFS::Driver::ISOFile:
					case QWBFS::Driver::UnknownFile: {
						if ( work.task & WorkerThread::WBFS ) {
							wbfsToWBFS( work.task, disc, work.target, true );
						}
						else if ( work.task & WorkerThread::ISO ) {
							wbfsToISO( work.task, disc, work.target );
						}
						else {
							if ( work.task & WorkerThread::Import ) {
								disc.error = QWBFS::Driver::DiscAddFailed;
							}
							else if ( work.task & WorkerThread::Export ) {
								disc.error = QWBFS::Driver::DiscExtractFailed;
							}
							else if ( work.task & WorkerThread::Convert ) {
								disc.error = QWBFS::Driver::DiscConvertFailed;
							}
							else {
								disc.error = QWBFS::Driver::UnknownError;
							}
						}
						
						break;
					}
				}
				
				break;
			}
			case QWBFS::Driver::ISOFile: {
				switch ( QWBFS::Driver::fileType( work.target ) ) {
					case QWBFS::Driver::WBFSPartitionFile: {
						isoToWBFS( work.task, disc, work.target, false );
						break;
					}
					case QWBFS::Driver::WBFSFile:
					case QWBFS::Driver::ISOFile:
					case QWBFS::Driver::UnknownFile: {
						if ( work.task & WorkerThread::WBFS ) {
							isoToWBFS( work.task, disc, work.target, true );
						}
						else if ( work.task & WorkerThread::ISO ) {
							isoToISO( work.task, disc, work.target );
						}
						else {
							if ( work.task & WorkerThread::Import ) {
								disc.error = QWBFS::Driver::DiscAddFailed;
							}
							else if ( work.task & WorkerThread::Export ) {
								disc.error = QWBFS::Driver::DiscExtractFailed;
							}
							else if ( work.task & WorkerThread::Convert ) {
								disc.error = QWBFS::Driver::DiscConvertFailed;
							}
							else {
								disc.error = QWBFS::Driver::UnknownError;
							}
						}
						
						break;
					}
				}
				
				break;
			}
			case QWBFS::Driver::UnknownFile:
				disc.error = QWBFS::Driver::UnknownError;
				break;
		}
		
		disc.state = disc.error == QWBFS::Driver::Ok ? QWBFS::Driver::Success : QWBFS::Driver::Failed;
		
		emit globalProgressChanged( ++count );
		emit jobFinished( disc );
		
		{
			QMutexLocker locker( &mMutex );
			
			if ( mStop ) {
				break;
			}
		}
	}
	
	emit globalProgressChanged( work.discs.count() );
}

void WorkerThread::connectDriver( QWBFS::Driver* driver )
{
	connect( driver, SIGNAL( currentProgressChanged( int, int, const QTime& ) ), this, SIGNAL( currentProgressChanged( int, int, const QTime& ) ) );
	connect( driver, SIGNAL( globalProgressChanged( int ) ), this, SIGNAL( globalProgressChanged( int ) ) );
}

void WorkerThread::isoToWBFS( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& _target, bool trimWBFS )
{
	if ( !source.isValid() ) {
		source.error = QWBFS::Driver::InvalidDisc;
		return;
	}
	
	const QFileInfo file( _target );
	QString target = _target;
	bool created = false;
	QWBFS::Partition::Handle handle;
	
	if ( trimWBFS && file.isDir() ) {
		target = QDir::cleanPath( target.append( QString( "/%1.wbfs" ).arg( source.baseName() ) ) );
	}
	
	if ( trimWBFS ) {
		
		if ( QFile::exists( target ) ) {
			source.error = QWBFS::Driver::DiscFound;
			return;
		}
		
		emit message( tr( "Initializing WBFS disc '%1'..." ).arg( source.baseName() ) );
		source.error = QWBFS::Driver::initializeWBFSFile( target );
		
		if ( source.hasError() ) {
			QFile::remove( target );
			return;
		}
		
		QWBFS::Partition::Properties properties( target );
		properties.reset = true;
		
		emit message( tr( "Formating WBFS disc '%1'..." ).arg( source.baseName() ) );
		
		handle = QWBFS::Partition::Handle( properties );
	}
	else {
		handle = QWBFS::Driver::getHandle( target, &created );
	}
	
	if ( !handle.isValid() ) {
		if ( created ) {
			QWBFS::Driver::closeHandle( handle );
		}
		
		if ( trimWBFS ) {
			QFile::remove( target );
		}
		
		source.error = QWBFS::Driver::PartitionNotOpened;
		return;
	}
	
	QWBFS::Driver driver( 0, handle );
	connectDriver( &driver );
	
	emit message( QString( "%1 '%2'..." ).arg( taskToLabel( task ) ).arg( source.baseName() ) );
	source.error = driver.addDiscImage( source.origin );
	
	if ( source.hasError() ) {
		if ( created ) {
			QWBFS::Driver::closeHandle( handle );
		}
		
		if ( trimWBFS ) {
			QFile::remove( target );
		}
		
		return;
	}
	
	if ( trimWBFS ) {
		source.error = driver.trim();
	}
	
	if ( created ) {
		QWBFS::Driver::closeHandle( handle );
	}
}

void WorkerThread::wbfsToISO( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& _target )
{
	if ( !source.isValid() ) {
		source.error = QWBFS::Driver::InvalidDisc;
		return;
	}
	
	QFileInfo file( _target );
	QString target = _target;
	
	if ( file.isDir() ) {
		target = QDir::cleanPath( target.append( QString( "/%1.iso" ).arg( source.baseName() ) ) );
	}
	
	if ( QFile::exists( target ) ) {
		source.error = QWBFS::Driver::DiscFound;
		return;
	}
	
	file.setFile( target );
	
	bool created = false;
	QWBFS::Partition::Handle handle = QWBFS::Driver::getHandle( source.origin, &created );
	
	// check handle validity
	if ( !handle.isValid() ) {
		if ( created ) {
			QWBFS::Driver::closeHandle( handle );
		}
		
		source.error = QWBFS::Driver::PartitionNotOpened;
		return;
	}
	
	QWBFS::Driver driver( 0, handle );
	connectDriver( &driver );
	
	emit message( QString( "%1 '%2'..." ).arg( taskToLabel( task ) ).arg( source.baseName() ) );
	source.error = driver.extractDisc( source.id, file.absolutePath(), file.fileName() );
	
	if ( source.hasError() ) {
		QFile::remove( target );
	}
	
	if ( created ) {
		QWBFS::Driver::closeHandle( handle );
	}
}

void WorkerThread::isoToISO( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& _target )
{
	if ( !source.isValid() ) {
		source.error = QWBFS::Driver::InvalidDisc;
		return;
	}
	
	const QFileInfo file( _target );
	QString target = _target;
	
	if ( file.isDir() ) {
		target = QDir::cleanPath( target.append( QString( "/%1.iso" ).arg( source.baseName() ) ) );
	}
	
	if ( QFile::exists( target ) ) {
		source.error = QWBFS::Driver::DiscFound;
		return;
	}
	
	emit message( QString( "%1 '%2'..." ).arg( taskToLabel( task ) ).arg( source.baseName() ) );
	
	// copying the file chunk by chunk instead of using QFile::copy() to be able to see progression
	QFile in( source.origin );
	QFile out( target );
	
	if ( !in.open( QIODevice::ReadOnly ) ) {
		source.error = QWBFS::Driver::DiscReadFailed;
		return;
	}
	
	if ( !out.open( QIODevice::WriteOnly ) ) {
		source.error = QWBFS::Driver::DiscWriteFailed;
		return;
	}
	
	const uint bufferSize = 1024 *1024 *5; // 5 MB buffer size
	char buffer[ bufferSize ];
	int totalRead = 0;
	
	QTime estimatedTime = QWBFS::Driver::estimatedTimeForTask( totalRead, in.size() );
	emit currentProgressChanged( totalRead, in.size(), estimatedTime );
	
	while ( !in.atEnd() ) {
		const qint64 read = in.read( buffer, bufferSize );
		
		if ( read == -1 ) {
			out.close();
			out.remove();
			
			QFile::remove( target );
			source.error = QWBFS::Driver::DiscReadFailed;
			return;
		}
		
		const qint64 write = out.write( buffer, read );
		
		if ( write == -1 ) {
			out.close();
			out.remove();
			
			QFile::remove( target );
			source.error = QWBFS::Driver::DiscWriteFailed;
			return;
		}
		
		totalRead += read;
		
		estimatedTime = QWBFS::Driver::estimatedTimeForTask( totalRead, in.size() );
		emit currentProgressChanged( totalRead, in.size(), estimatedTime );
	}
	
	in.close();
	out.close();
}

void WorkerThread::wbfsToWBFS( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& _target, bool trimWBFS )
{
	if ( !source.isValid() ) {
		source.error = QWBFS::Driver::InvalidDisc;
		return;
	}
	
	const QFileInfo file( _target );
	QString target = _target;
	bool sourceCreated = false;
	QWBFS::Partition::Handle sourceHandle = QWBFS::Driver::getHandle( source.origin, &sourceCreated );
	bool targetCreated = false;
	QWBFS::Partition::Handle targetHandle;
	
	if ( !sourceHandle.isValid() ) {
		if ( sourceCreated ) {
			QWBFS::Driver::closeHandle( sourceHandle );
		}
		
		source.error = QWBFS::Driver::PartitionNotOpened;
		return;
	}
	
	if ( trimWBFS && file.isDir() ) {
		target = QDir::cleanPath( target.append( QString( "/%1.wbfs" ).arg( source.baseName() ) ) );
	}
	
	if ( trimWBFS ) {
		
		if ( QFile::exists( target ) ) {
			if ( sourceCreated ) {
				QWBFS::Driver::closeHandle( sourceHandle );
			}
			
			source.error = QWBFS::Driver::DiscFound;
			return;
		}
		
		emit message( tr( "Initializing WBFS disc '%1'..." ).arg( source.baseName() ) );
		source.error = QWBFS::Driver::initializeWBFSFile( target );
		
		if ( source.hasError() ) {
			if ( sourceCreated ) {
				QWBFS::Driver::closeHandle( sourceHandle );
			}
			
			QFile::remove( target );
			return;
		}
		
		QWBFS::Partition::Properties properties( target );
		properties.reset = true;
		
		emit message( tr( "Formating WBFS disc '%1'..." ).arg( source.baseName() ) );
		
		targetHandle = QWBFS::Partition::Handle( properties );
	}
	else {
		targetHandle = QWBFS::Driver::getHandle( target, &targetCreated );
	}
	
	if ( !targetHandle.isValid() ) {
		if ( sourceCreated ) {
			QWBFS::Driver::closeHandle( sourceHandle );
		}
		
		if ( targetCreated ) {
			QWBFS::Driver::closeHandle( targetHandle );
		}
		
		if ( trimWBFS ) {
			QFile::remove( target );
		}
		
		source.error = QWBFS::Driver::PartitionNotOpened;
		return;
	}
	
	QWBFS::Driver targetDriver( 0, targetHandle );
	connectDriver( &targetDriver );
	
	// direct drive2drive
	if ( targetDriver.canDrive2Drive( sourceHandle ) == QWBFS::Driver::Ok ) {
		emit message( QString( "%1 '%2'..." ).arg( taskToLabel( task ) ).arg( source.baseName() ) );
		
		source.error = targetDriver.addDisc( source.id, sourceHandle );
	}
	// indirect drive2drive
	else {
		emit message( QString( "%1 '%2'..." ).arg( taskToLabel( task, true ) ).arg( source.baseName() ) );
		
		const QFileInfo tmpFile( QString( "%1/%2.iso" ).arg( QDir::tempPath() ).arg( source.baseName() ) );
		QWBFS::Driver sourceDriver( 0, sourceHandle );
		connectDriver( &sourceDriver );
		
		if ( targetDriver.hasDisc( source.id ) == QWBFS::Driver::DiscNotFound ) {
			source.error = sourceDriver.extractDisc( source.id, tmpFile.absolutePath(), tmpFile.fileName() );
			
			if ( !source.hasError() ) {
				source.error = targetDriver.addDiscImage( tmpFile.absoluteFilePath() );
			}
			
			QFile::remove( tmpFile.absoluteFilePath() );
		}
		else {
			source.error = QWBFS::Driver::DiscFound;
		}
	}
	
	if ( !source.hasError() && trimWBFS ) {
		source.error = targetDriver.trim();
	}
	
	if ( sourceCreated ) {
		QWBFS::Driver::closeHandle( sourceHandle );
	}
	
	if ( targetCreated ) {
		QWBFS::Driver::closeHandle( targetHandle );
	}
	
	if ( source.hasError() && trimWBFS ) {
		QFile::remove( target );
	}
}
