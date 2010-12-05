/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : Disc.h
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
	Disc( const QString& filePath );
	
	bool operator==( const Disc& other ) const;
	
	bool isValid() const;
	bool hasError() const;
	QString baseName() const;
	
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
