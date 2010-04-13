#ifndef DISC_H
#define DISC_H

#include <QList>
#include <QDomDocument>

#include <tools.h>

namespace QWBFS {
namespace Model {

typedef QList<struct Disc> DiscList;

struct Disc
{
	Disc( u8* header = 0, u32* size = 0, const QString& origin = QString::null );
	Disc( const QString& origin );
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

}; // Model
}; // QWBFS

#endif // DISC_H
