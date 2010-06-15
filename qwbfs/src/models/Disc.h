/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : Disc.h
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
