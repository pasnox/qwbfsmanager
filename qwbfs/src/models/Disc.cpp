/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : Disc.cpp
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
#include "Disc.h"
#include "qwbfsdriver/Driver.h"

#include <FreshCore/pCoreUtils>

#include <QFileInfo>

#define WBFS_DISC_XML_NAME "WBFS-Discs"

using namespace QWBFS::Model;

Disc::Disc( const QDomElement& element )
{
	readFromElement( element );
}

Disc::Disc( const QString& filePath )
{
	readFromElement( QDomElement() );
	
	switch ( QWBFS::Driver::fileType( filePath ) ) {
		case QWBFS::Driver::WBFSFile:
			QWBFS::Driver::wbfsFileInfo( filePath, *this );
			break;
		case QWBFS::Driver::ISOFile:
			operator=( Driver::isoDiscInfo( filePath ) );
			break;
		case QWBFS::Driver::WBFSPartitionFile:
		case QWBFS::Driver::UnknownFile:
			Q_ASSERT( 0 );
			break;
	}
}

bool Disc::operator==( const Disc& other ) const
{
	return id == other.id &&
		title == other.title &&
		size == other.size &&
		origin == other.origin &&
		region == other.region /*&&
		state == other.state &&
		error == other.error*/;
}

bool Disc::isValid() const
{
	return !id.isEmpty() && !title.isEmpty();
}

bool Disc::hasError() const
{
	return error != QWBFS::Driver::Ok;
}

QString Disc::baseName( const QString& invalidChars ) const
{
	return isValid() ? QString( "%1 [%2]" ).arg( cleanupGameTitle( title, invalidChars ) ).arg( id ) : QString::null;
}

void Disc::addToDocument( QDomDocument& document ) const
{
	QDomElement element = document.createElement( "disc" );
	element.setAttribute( "id", id );
	element.setAttribute( "title", title );
	element.setAttribute( "size", size );
	element.setAttribute( "origin", origin );
	element.setAttribute( "region", region );
	element.setAttribute( "state", state );
	element.setAttribute( "error", error );
	document.documentElement().appendChild( element );
}

void Disc::readFromElement( const QDomElement& element )
{
	id = element.attribute( "id", QString::null );
	title = element.attribute( "title", QString::null );
	size = element.attribute( "size", QString::number( 0 ) ).toUInt();
	origin = element.attribute( "origin", QString::null );
	region = element.attribute( "region", 0 ).toInt();
	state = element.attribute( "state", QString::number( QWBFS::Driver::None ) ).toInt();
	error = element.attribute( "error", QString::number( QWBFS::Driver::Ok ) ).toInt();
}

QString Disc::cleanupGameTitle( const QString& _title, const QString& invalidChars )
{
	QString title = pCoreUtils::toTitleCase( _title );
	
	foreach ( const QChar& c, invalidChars ) {
		QString r;
		
		/*switch ( c.toAscii() ) {
			case '\'':
				r = " ";
				break;
			case ':':
				r = "-";
		}*/
		
		title.replace( c, r );
	}
	
	return title;
}

QDomDocument Disc::toDocument( const DiscList& discs )
{	
	QDomDocument document( WBFS_DISC_XML_NAME );
	document.appendChild( document.createElement( "discs" ) );
	
	foreach ( const Disc& disc, discs ) {
		disc.addToDocument( document );
	}
	
	return document;
}

DiscList Disc::fromDocument( const QDomDocument& document )
{
	DiscList discs;
	
	const QDomNodeList nodes = document.documentElement().childNodes();
	
	for ( int i = 0; i < nodes.count(); i++ ) {
		discs << Disc( nodes.at( i ).toElement() );
	}
	
	return discs;
}

QByteArray Disc::toByteArray( const DiscList& discs )
{
	return toDocument( discs ).toString().toUtf8();
}

DiscList Disc::fromByteArray( const QByteArray& data )
{
	QDomDocument document( WBFS_DISC_XML_NAME );
	document.setContent( data );
	return fromDocument( document );
}
