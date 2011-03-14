#include "pPartitionModel.h"

#if defined( Q_OS_MAC )
#include <QProcess>

pPartitionModel::Partitions pPartitionModel::partitions() const
{
	pPartitionModel::Partitions partitions;
	QProcess process;
	
	process.start( "diskutil list" );
	process.waitForFinished();
	
	const QStringList entries = QString::fromLocal8Bit( process.readAll() ).split( "\n" );
	
	foreach ( QString entry, entries ) {
		entry = entry.trimmed();
		
		if ( entry.startsWith( "/" ) || entry.startsWith( "#" ) || entry.isEmpty() ) {
			continue;
		}
		
		entry = entry.simplified().section( ' ', -1 );
		
		// skip disks
		if ( entry[ entry.size() -2 ].toLower() != 's' ) {
			continue;
		}
		
		pPartitionModel::Partition partition;
		
		partition.label = QString::null;
		partition.origin = QString( "/dev/%1" ).arg( entry );
		partition.fileSystem = QString::null;
		partition.total = -1;
		partition.free = -1;
		partition.used = -1;
		partition.lastCheck = QDateTime::currentDateTime();
		
		partitions << partition;
	}
	
	return partitions;
}

#endif
