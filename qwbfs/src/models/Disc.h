#ifndef DISC_H
#define DISC_H

#include <QList>
#include <QDomDocument>
#include <QHash>

namespace QWBFS {
namespace Model {

typedef QList<struct Disc> DiscList;

struct Disc
{
	Disc( const QDomElement& element = QDomElement() );
	
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
	int region;
	int state;
	int error;
};

inline uint qHash( const Disc& disc )
{
	return qHash( QString( "%1 - %2" ).arg( disc.id ).arg( disc.origin ) );
}

}; // Model
}; // QWBFS

#endif // DISC_H
