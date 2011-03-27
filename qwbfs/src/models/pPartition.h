/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : Fresh Library
** FileName  : pPartition.h
** Date      : 2011-02-20T00:41:35
** License   : LGPL v3
** Home Page : http://bettercodes.org/projects/fresh
** Comment   : Fresh Library is a Qt 4 extension library providing set of new core & gui classes.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Leser General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/
#ifndef PPARTITION_H
#define PPARTITION_H

/*!
	\file pPartition.h
	\brief An object that represent a partition on the system.
	\author Filipe Azevedo aka Nox P\@sNox <pasnox@gmail.com>
*/

#include <QVariant>
#include <QDateTime>

/*!
	\ingroup Core
	\class pPartition
	\brief An object that represent a partition on the system.
*/
class pPartition {
public:
	/*! This enum defines the differents types of device. */
	enum Type {
		Device, /*!< Generic device. */ 
		Disc /*!< CdRom / DVDRom / BluRay device. */ 
	};
	
	/*! This enum defines the differents types of property. */
	enum Property {
		Label, /*!< The partition label. */ 
		DevicePath, /*!< The device path like /dev/sda1, C:\... */ 
		FileSystem, /*!< The file system name. */ 
		TotalSize, /*!< The total size. */ 
		UsedSize, /*!< The used size. */ 
		FreeSize, /*!< The free size. */ 
		MountPoints, /*!< The mount points. */ 
		//DeviceType,
		DeviceVendor, /*!< The device vendor */ 
		DeviceModel, /*!< The device model. */ 
		FileSystemId, /*!< The file system id. */ 
		LastCheck, /*!< The last date time the partition was checked. */ 
		DisplayText, /*!< The display text. */ 
		LastProperty = DisplayText /*!< The last available property. */ 
	};
	
	/*!
		Create a partition for device at \a devicePath. The device validity is checked according to \a checkValidity beefore setting the internal property.
	*/
	pPartition( const QString& devicePath = QString::null, bool checkValidity = true );
	/*!
		Check if \a other is same partition as this. The test is only done against the device path.
	*/
	bool operator==( const pPartition& other ) const;
	/*!
		Set the properties map.
	*/
	void setProperties( const QVariantMap& properties );
	/*!
		Return the properties map.
	*/
	QVariantMap properties() const;
	/*!
		Return the content of the property \a property.
	*/
	QVariant property( pPartition::Property property ) const;
	/*!
		Return the content of the property \a property. This allow to query non standard properties.
	*/
	QVariant property( const QString& property ) const;
	/*!
		Update the partitions sizes. The used spaces is computed by the formula: total -free.
	*/
	void updateSizes( qint64 total, qint64 free );
	/*!
		Return true if the partition is valid else false.
	*/
	bool isValid() const;
	/*!
		Return true if the partition is a custom one else a false one.
		A partition is custom if it's internal device path is equal to the device path property.
		A non custom partition has its internal device path to null which is differnt than the device path property..
	*/
	bool isCustom() const;
	/*!
		Return the device path, the property is queried first and if it's empty it return the internal device path.
	*/
	QString devicePath() const;
	/*!
		Check is the partition \a devicePath is a WBFS partition.
	*/
	static bool isWBFSPartition( const QString& devicePath );
	/*!
		Return true if the partition \a devicePath is a valid partition.
	*/
	static bool isValidDevicePath( const QString& devicePath );
	/*!
		Return the file system name from the partition \a id.
		The device \a type is checked.
		Some Ids can represent differents kinds of file system, in this case \a full will return the concatened of all possible choice, else only the first one.
	*/
	static QString fileSystemIdToString( qint64 id, pPartition::Type type = pPartition::Device, bool full = false );
	
protected:
	QVariantMap mProperties;
	QString mDevicePath;
	
	void updateLastChecked();
	QString generateDisplayText() const;
	QString value( const QString& key ) const;
};

typedef QList<pPartition> pPartitionList;

#endif // PPARTITION_H
