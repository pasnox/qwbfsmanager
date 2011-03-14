#include "pPartitionModel.h"

#if defined( __linux__ )
#include <libudev.h>
#include <sys/statfs.h>

#include <QtDBus>

void fillPartitionInformations( struct udev_device* device, pPartitionModel::Partition& partition )
{
    struct udev_list_entry* entries = udev_device_get_properties_list_entry( device );
    struct udev_list_entry* entry = 0;
	
	partition.extendedAttributes.clear();
	
    udev_list_entry_foreach( entry, entries ) {
        const QString name = udev_list_entry_get_name( entry );
        const QString value = QString::fromLocal8Bit( udev_device_get_property_value( device, name.toLocal8Bit().constData() ) );

        partition.extendedAttributes[ name ] = value;
    }
	
	const QString devName = QFileInfo( partition.extendedAttributes[ "DEVNAME" ] ).fileName();
	QDBusMessage question = QDBusMessage::createMethodCall( "org.freedesktop.UDisks", QString( "/org/freedesktop/UDisks/devices/%1" ).arg( devName ), "org.freedesktop.DBus.Properties", "Get" );
	question << "org.freedesktop.UDisks.Device" << "DeviceMountPaths";
	QDBusMessage answer = QDBusConnection::systemBus().call( question, QDBus::Block, 1 );
	QString mount;
	
	foreach ( const QVariant& variant, answer.arguments() ) {
		const QDBusVariant v = variant.value<QDBusVariant>();
		mount = v.variant().toStringList().value( 0 );
		
		if ( !mount.isEmpty() ) {
			break;
		}
	}
	
	partition.label = partition.extendedAttributes[ "ID_FS_LABEL" ];
    partition.origin = partition.extendedAttributes[ "DEVNAME" ];
    partition.model = partition.extendedAttributes[ "ID_MODEL" ];
    partition.total = partition.extendedAttributes[ "UDISKS_PARTITION_SIZE" ].toLongLong();
	partition.fileSystem = partition.extendedAttributes[ "ID_FS_TYPE" ];
	partition.fileSystemMark = partition.extendedAttributes[ "UDISKS_PARTITION_TYPE" ].toLongLong( 0, 0 );
	
	if ( !mount.isEmpty() ) {
		struct statfs stats;
		
		if ( statfs( qPrintable( mount ), &stats ) == 0 ) {
			const qint64 total = stats.f_blocks *stats.f_bsize;
			partition.used = total -( stats.f_bfree *stats.f_bsize );
			partition.free = partition.total -partition.used;
		}
	}
	
	if ( partition.label.isEmpty() ) {
		partition.label = QString( partition.origin ).replace( "\\", "/" ).section( '/', -1, -1 );
	}
	
	const QString vendorModel = QString( "%1 %2" )
		.arg( partition.extendedAttributes[ "ID_VENDOR" ] )
		.arg( partition.extendedAttributes[ "ID_MODEL" ] )
		.replace( "_", " " )
		.simplified()
		;
	
	partition.name = QString( "%1 %2 (%3)" )
		.arg( partition.label )
		.arg( partition.fileSystem.isEmpty() ? pPartitionModel::tr( "Unknown FS" ) : partition.fileSystem.toUpper() )
		.arg( vendorModel )
		;
	
    partition.extendedAttributes[ "REMOVABLE" ] = "0";

    // Check removable attr
    struct udev_device* parent = 0;
	
    if ( partition.extendedAttributes[ "DEVTYPE" ] == "partition" ) {
        parent = udev_device_get_parent( device );
    }
    else if ( partition.extendedAttributes[ "DEVTYPE" ] == "disk" ) {
        parent = device; // disk without partitions
    }

    if ( parent != 0 ) {
        const char* value = udev_device_get_sysattr_value( parent, "removable" );

        if ( value ) {
            partition.extendedAttributes[ "REMOVABLE" ] = QString::fromLocal8Bit( value );
        }
    }
	
	partition.lastCheck = QDateTime::currentDateTime();
}

pPartitionModel::Partitions pPartitionModel::partitions() const
{
	pPartitionModel::Partitions partitions;
	udev* _udev = udev_new();
	
	if ( !_udev ) {
		qWarning( "%s: UDev not valid.", Q_FUNC_INFO );
		return partitions;
	}
	
    struct udev_enumerate* enumerate  = udev_enumerate_new( _udev );
	
    if ( !enumerate ) {
		qWarning( "%s: Can't enumerate.", Q_FUNC_INFO );
        return partitions;
    }
	
    udev_enumerate_add_match_subsystem( enumerate, "block" );
    udev_enumerate_scan_devices( enumerate );
	
    struct udev_device* device = 0;
    struct udev_list_entry* entries = udev_enumerate_get_list_entry( enumerate );
    struct udev_list_entry* entry = 0;
	
    udev_list_entry_foreach( entry, entries ) {
        device = udev_device_new_from_syspath( _udev, udev_list_entry_get_name( entry ) );
		
        if ( !device ) {
            continue;
        }
		
        pPartitionModel::Partition partition;
        fillPartitionInformations( device, partition );
		
		if ( partition.extendedAttributes[ "DEVTYPE" ] == "partition"
			|| ( partition.extendedAttributes[ "ID_TYPE" ] == "cd" 
				&& partition.extendedAttributes[ "ID_FS_USAGE" ] == "filesystem" ) ) {
			partitions << partition;
		}
		
        udev_device_unref( device );
    }

    udev_enumerate_unref( enumerate );
	udev_unref( _udev );
	
	return partitions;
}

#endif
