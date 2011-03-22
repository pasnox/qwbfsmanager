#ifndef PPARTITION_H
#define PPARTITION_H

#include <QVariant>
#include <QDateTime>

#define PROPERTY_DISPLAY_TEXT "_DISPLAY_TEXT"
#define PROPERTY_LAST_CHECK "_LAST_CHECK"

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
		Label,
		DevicePath,
		FileSystem,
		TotalSize,
		UsedSize,
		FreeSize,
		MountPoints,
		//DeviceType,
		DeviceVendor,
		DeviceModel,
		FileSystemId,
		LastCheck,
		DisplayText,
		LastProperty = DisplayText
	};
	
	pPartition( const QString& devicePath = QString::null, bool checkValidity = true );
	
	bool operator==( const pPartition& other ) const;
	
	void setProperties( const QVariantMap& properties );
	QVariantMap properties() const;
	
	QVariant property( pPartition::Property property ) const;
	QVariant property( const QString& property ) const;
	
	void updateSizes( qint64 total, qint64 free );
	
	bool isValid() const;
	bool isCustom() const;
	QString devicePath() const;
	
	static bool isWBFSPartition( const QString& devicePath );
	static bool isValidDevicePath( const QString& devicePath );
	static QString fileSystemIdToString( qint64 id, pPartition::Type type = pPartition::Fixed, bool full = false );
	
protected:
	QVariantMap mProperties;
	QString mDevicePath;
	
	void updateLastChecked();
	QString generateDisplayText() const;
	QString value( const QString& key ) const;
};

typedef QList<pPartition> pPartitionList;

#endif // PPARTITION_H
