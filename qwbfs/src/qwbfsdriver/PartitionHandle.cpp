/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : PartitionHandle.cpp
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
#include "PartitionHandle.h"

#include <QDebug>

using namespace QWBFS::Partition;
using namespace QWBFS::Partition::Internal;

// HandleData

HandleData::HandleData( const QWBFS::Partition::Properties& _properties )
{
	properties = _properties;
	
	QString partition = properties.partition;
#if defined( Q_OS_WIN )
	partition.remove( ":\\" ).remove( ":/" );
#endif
	handle = partition.isEmpty() ? 0 : wbfs_try_open_partition( partition.toLocal8Bit().data(), properties.reset ? 1 : 0 );
	
	if ( handle ) {
		qWarning() << QString( "*** Opened partition: %1" ).arg( properties.partition ).toLocal8Bit().constData();
	}
}

HandleData::HandleData( const HandleData& other )
	: QSharedData( other )
{
	properties = other.properties;
	handle = other.handle;
}

HandleData::~HandleData()
{
	if ( handle ) {
		wbfs_close( handle );
		qWarning() << QString( "*** Closed partition: %1" ).arg( properties.partition ).toLocal8Bit().constData();
	}
	
	//qWarning() << Q_FUNC_INFO;
}

// Handle

Handle::Handle( const QWBFS::Partition::Properties& properties )
{
	d = new HandleData( properties );
}

Handle::Handle( const QString& partition )
{
	d = new HandleData( QWBFS::Partition::Properties( partition ) );
}

Handle::~Handle()
{
	//qWarning() << Q_FUNC_INFO;
}

bool Handle::isValid() const
{
	return d->handle;
}

wbfs_t* Handle::ptr() const
{
	return d->handle;
}

QWBFS::Partition::Properties Handle::properties() const
{
	return d->properties;
}

bool Handle::reset() const
{
	return d->properties.reset;
}

QString Handle::partition() const
{
	return d->properties.partition;
}
