/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : PartitionDiscHandle.cpp
** Date      : 2010-06-15T23:21:10
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR HARDWARE OR YOUR DATA
** BECAUSE OF IMPROPER USAGE OF THIS SOFTWARE.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
