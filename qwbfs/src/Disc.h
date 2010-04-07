#ifndef DISC_H
#define DISC_H

#include <QString>
#include <QList>
#include <QDomDocument>

#include <libwbfs.h>

typedef QList<struct Disc> DiscList;

struct Disc
{
	Disc( u8* header = 0, u32* _size = 0, const QString& _origin = QString::null );
	Disc( const QString& _origin );
	Disc( const QDomElement& element );
	
	bool operator==( const Disc& other ) const;
	
	void addToDocument( QDomDocument& document ) const;
	void readFromElement( const QDomElement& element );
	
	static QDomDocument toDocument( const DiscList& discs );
	static DiscList fromDocument( const QDomDocument& document );
	
	static QByteArray toByteArray( const DiscList& discs );
	static DiscList fromByteArray( const QByteArray& data );
	
	QString id;
	QString title;
	quint32 size;
	QString origin;
};

inline uint qHash( const Disc& disc )
{
	return qHash( QString( "%1 - %2" ).arg( disc.id ).arg( disc.origin ) );
}

#endif // DISC_H
