/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : Covers.h
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
#ifndef COVERS_H
#define COVERS_H

#include <QString>
#include <QUrl>

namespace QWBFS {
namespace WiiTDB {

class Covers
{
public:
	enum Type
	{
		Invalid,
		HQ,
		Cover,
		_3D,
		Disc,
		DiscCustom,
		Full
	};
	
	Covers( const QString& id );
	Covers( const QUrl& url );
	virtual ~Covers();

	Covers( const Covers& other );
	Covers& operator=( const Covers& other );
	bool operator==( const Covers& other ) const;
	bool operator!=( const Covers& other ) const;
	
	QUrl url( Type type ) const;
	static QUrl url( Type type, const QString& id );
	static Type type( const QUrl& url );

protected:
	QString mId;
};

}; // WiiTDB
}; // QWBDFS

#endif // COVERS_H
