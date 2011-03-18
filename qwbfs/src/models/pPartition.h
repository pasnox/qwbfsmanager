#ifndef PPARTITION_H
#define PPARTITION_H

#include <QVariant>
#include <QDateTime>

class pPartition {
public:
	enum Type {
		Unknown = 0,
		NotMounted = 1,
		Removable = 2,
		Fixed = 3,
		Remote = 4,
		CdRom = 5,
		RamDisk = 6
	};
	
	enum Property {
		DisplayText = 0,
		Label,
		DevicePath,
		TotalSize,
		UsedSize,
		FreeSize,
		LastCheck,
		//DeviceType,
		MountPoints,
		FileSystem,
		FileSystemId,
		DeviceVendor,
		DeviceModel
	};
	
	pPartition( const QString& devicePath = QString::null );
	
	bool operator==( const pPartition& other ) const;
	
	void setProperty( pPartition::Property property, const QVariant& content );
	QVariant property( pPartition::Property property ) const;
	
	bool isValid() const;
	QVariantMap& properties();
	QString generateDisplayText() const;
	
	static bool isWBFSPartition( const QString& devicePath );
	static QString fileSystemIdToString( qint64 id, pPartition::Type type = pPartition::Fixed, bool full = false );
	
protected:
	QVariantMap mProperties;
};

typedef QList<pPartition> pPartitionList;

#endif // PPARTITION_H
