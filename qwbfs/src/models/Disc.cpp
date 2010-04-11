#include "Disc.h"

#include <QFileInfo>

#define WBFS_DISC_XML_NAME "WBFS-Discs"

Disc::Disc( u8* header, u32* _size, const QString& _origin )
{
	if ( header && _size ) {
		id = QString::fromAscii( (char*)header, 6 );
		title = QString::fromAscii( (char*)header +0x20 );
		size = *_size;
		origin = _origin;
	}
}

Disc::Disc( const QString& _origin )
{
	QFileInfo file( _origin );
	
	if ( file.exists() ) {
		title = file.baseName();
		size = file.size();
		origin = _origin;
	}
}

Disc::Disc( const QDomElement& element )
{
	readFromElement( element );
}

bool Disc::operator==( const Disc& other ) const
{
	return id == other.id &&
		title == other.title &&
		size == other.size &&
		origin == other.origin;
}

void Disc::addToDocument( QDomDocument& document ) const
{
	QDomElement element = document.createElement( "disc" );
	element.setAttribute( "id", id );
	element.setAttribute( "title", title );
	element.setAttribute( "size", size );
	element.setAttribute( "origin", origin );
	document.documentElement().appendChild( element );
}

void Disc::readFromElement( const QDomElement& element )
{
	id = element.attribute( "id" );
	title = element.attribute( "title" );
	size = element.attribute( "size" ).toUInt();
	origin = element.attribute( "origin" );
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
