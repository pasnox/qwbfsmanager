/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : PartitionDiscHandle.cpp
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
#include "PartitionDiscHandle.h"

#include <QDebug>

using namespace QWBFS::Partition;
using namespace QWBFS::Partition::Internal;

// DiscHandleData

DiscHandleData::DiscHandleData( const QWBFS::Partition::Handle& _handle, const QString& _discId )
{
	handle = wbfs_open_disc( _handle.ptr(), (u8*)( _discId.toLocal8Bit().data() ) );
	discId = _discId;
	index = handle ? handle->i : -1;
	
	if ( handle ) {
		QString( "*** Opened disc: %1" ).arg( discId ).toLocal8Bit().constData();
	}
}

DiscHandleData::DiscHandleData( const DiscHandleData& other )
	: QSharedData( other )
{
	handle = other.handle;
	discId = other.discId;
	index = other.index;
}

DiscHandleData::~DiscHandleData()
{
	if ( handle ) {
		wbfs_close_disc( handle );
		QString( "*** Closed disc: %1" ).arg( discId ).toLocal8Bit().constData();
	}
	
	//qWarning() << Q_FUNC_INFO;
}

// DiscHandleData

DiscHandle::DiscHandle( const QWBFS::Partition::Handle& handle, const QString& discId )
{
	d = new DiscHandleData( handle, discId );
}

DiscHandle::~DiscHandle()
{
	//qWarning() << Q_FUNC_INFO;
}

QString DiscHandle::discId() const
{
	return d->discId;
}

int DiscHandle::index() const
{
	return d->index;
}

QString DiscHandle::isoName() const
{
	if ( !isValid() ) {
		return QString::null;
	}
	
	QString isoName = QString::fromLocal8Bit( QString::fromLocal8Bit( (char*)d->handle->header->disc_header_copy +0x20, 0x100 )
		.replace( ' ', '_' )
		.replace( '/', '_' )
		.replace( ':', '_' ).trimmed().toLocal8Bit().constData() );
	
	while ( isoName.contains( "__" ) ) {
		isoName.replace( "__", "_" );
	}
	
	if ( isoName.length() >= 0x100 ) {
		isoName.chop( ( isoName.length() -0x100 ) +4 );
	}
	
	isoName.append( ".iso" );
	
	return isoName;
}

bool DiscHandle::isValid() const
{
	return d->handle;
}

wbfs_disc_t* DiscHandle::ptr() const
{
	return d->handle;
}
