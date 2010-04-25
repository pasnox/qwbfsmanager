/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : Disc.cpp
** Date      : 2010-04-25T13:05:33
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a crossplatform WBFS Manager developed using Qt4/C++.
** It's currently working under Unix/Linux, Mac OS X, and build under windows (but not yet working).
** 
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR WII CONSOLE OR WII PARTITION
** BECAUSE OF IMPROPER USAGE OF THIS SOFTWARE.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "Disc.h"
#include "qwbfsdriver/Driver.h"

#define WBFS_DISC_XML_NAME "WBFS-Discs"

using namespace QWBFS::Model;

Disc::Disc( const QDomElement& element )
{
	readFromElement( element );
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
	region = element.attribute( "region", QString::number( QWBFS::Driver::NoRegion ) ).toInt();
	state = element.attribute( "state", QString::number( QWBFS::Driver::None ) ).toInt();
	error = element.attribute( "error", QString::number( QWBFS::Driver::Ok ) ).toInt();
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
