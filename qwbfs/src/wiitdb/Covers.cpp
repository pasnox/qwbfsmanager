/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : Covers.cpp
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
#include "Covers.h"
#include "../qwbfsdriver/Driver.h"

#include <QFileInfo>
#include <QDebug>

using namespace QWBFS::WiiTDB;

Covers::Covers( const QString& id )
{
	mId = id;
	Q_ASSERT( !id.isEmpty() );
}

Covers::Covers( const QUrl& url )
{
	Q_ASSERT( !url.isEmpty() );
	Q_ASSERT( url.isValid() );
	mId = QFileInfo( url.path() ).baseName();
	Q_ASSERT( !mId.isEmpty() );
}

Covers::~Covers()
{
}

Covers::Covers( const Covers& other )
{
	operator=( other );
}

Covers& Covers::operator=( const Covers& other )
{
	if( *this != other ) {
		mId = other.mId;
	}

	return *this;
}

bool Covers::operator==( const Covers& other ) const
{
	return mId == other.mId;
}

bool Covers::operator!=( const Covers& other ) const
{
	return !operator==( other );
}

QUrl Covers::url( Covers::Type type ) const
{
	return url( type, mId );
}

QUrl Covers::url( Covers::Type type, const QString& id )
{
	Q_ASSERT( !id.isEmpty() );
	
	const QString language = QWBFS::Driver::regionToLanguageString( id.at( 3 ).unicode() );
	
	switch ( type ) {
		case Covers::HQ:
			return QUrl( QString( "%3/wiitdb/artwork/coverfullHQ/%1/%2.png" ).arg( language ).arg( id ).arg( WIITDB_DOMAIN ) );
		case Covers::Cover:
			return QUrl( QString( "%3/wiitdb/artwork/cover/%1/%2.png" ).arg( language ).arg( id ).arg( WIITDB_DOMAIN ) );
		case Covers::_3D:
			return QUrl( QString( "%3/wiitdb/artwork/coverfull3D/%1/%2.png" ).arg( language ).arg( id ).arg( WIITDB_DOMAIN ) );
		case Covers::Disc:
			return QUrl( QString( "%3/wiitdb/artwork/disc/%1/%2.png" ).arg( language ).arg( id ).arg( WIITDB_DOMAIN ) );
		case Covers::DiscCustom:
			return QUrl( QString( "%3/wiitdb/artwork/disccustom/%1/%2.png" ).arg( language ).arg( id ).arg( WIITDB_DOMAIN ) );
		case Covers::Full:
			return QUrl( QString( "%3/wiitdb/artwork/coverfull/%1/%2.png" ).arg( language ).arg( id ).arg( WIITDB_DOMAIN ) );
		case Covers::Invalid:
			break;
	}
	
	return QUrl();
}

Covers::Type Covers::type( const QUrl& url )
{
	if ( Covers( url ).url( Covers::HQ ) == url ) {
		return Covers::HQ;
	}
	else if ( Covers( url ).url( Covers::Cover ) == url ) {
		return Covers::Cover;
	}
	else if ( Covers( url ).url( Covers::_3D ) == url ) {
		return Covers::_3D;
	}
	else if ( Covers( url ).url( Covers::Disc ) == url ) {
		return Covers::Disc;
	}
	else if ( Covers( url ).url( Covers::DiscCustom ) == url ) {
		return Covers::DiscCustom;
	}
	else if ( Covers( url ).url( Covers::Full ) == url ) {
		return Covers::Full;
	}
	
	return Covers::Invalid;
}
