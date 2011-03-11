#include "pPartitionModel.h"

#if defined( __linux__ )

#include <QFile>
#include <QDebug>

#include <sys/vfs.h> /* or <sys/statfs.h> */
//#include <sys/statfs.h>

pPartitionModel::Partitions pPartitionModel::linuxPartitions() const
{
	pPartitionModel::Partitions partitions;
	QFile file( "/proc/partitions" );
	
	if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
		const QStringList lines = QString::fromAscii( file.readAll() ).split( "\n" );
		
		foreach ( QString line, lines ) {
			line = line
				.simplified()
				.replace( "\t", " " )
				;
			bool isNumeric = false;
			
			line.section( " ", 0, 0 ).toLongLong( &isNumeric );
			
			if ( !isNumeric ) {
				continue;
			}
			
			QString path = QString( "/dev/%1" ).arg( line.section( " ", -1, -1 ) );
			
			if ( path.isEmpty() || ( !path.isEmpty() && !path[ path.length() -1 ].isNumber() ) ) {
				continue;
			}
			
			struct statfs stats;
			int result = statfs( qPrintable( path ), &stats );
			
			switch ( result ) {
				case 0: {
					pPartitionModel::Partition partition;
					
					/*qWarning() << "----" << path;
					qWarning() << stats.f_type;
					qWarning() << stats.f_bsize;
					qWarning() << stats.f_blocks;
					qWarning() << stats.f_bfree;
					qWarning() << stats.f_bavail;
					qWarning() << stats.f_files;
					qWarning() << stats.f_ffree;
					//qWarning() << stats.f_fsid;
					qWarning() << stats.f_namelen;
					qWarning() << stats.f_frsize;
					qWarning() << stats.f_spare;*/
					
					partition.label = QString::null;
					partition.origin = path;
					partition.type = QString::null;
					partition.ctype = stats.f_type;
					partition.total = stats.f_blocks *stats.f_bsize;
					partition.free = stats.f_bfree *stats.f_bsize;
					partition.used = partition.total -partition.free;
					partition.lastCheck = QDateTime::currentDateTime();
					
					partitions << partition;
					break;
				}
				default:
					qWarning() << Q_FUNC_INFO << result;
					break;
			}
		}
	}
	else {
		qWarning() << Q_FUNC_INFO << file.errorString();
	}
	
	return partitions;
}

#endif
