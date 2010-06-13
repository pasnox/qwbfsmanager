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
	if( *this != other )
	{
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

QUrl Covers::url( Type type ) const
{
	return url( type, mId );
}

QUrl Covers::url( Type type, const QString& id )
{
	Q_ASSERT( !id.isEmpty() );
	
	const QString language = QWBFS::Driver::regionToLanguageString( id.at( 3 ).unicode() );
	
	switch ( type )
	{
		case HQ:
			return QUrl( QString( "http://wiitdb.com/wiitdb/artwork/coverfullHQ/%1/%2.png" ).arg( language ).arg( id ) );
		case Cover:
			return QUrl( QString( "http://wiitdb.com/wiitdb/artwork/cover/%1/%2.png" ).arg( language ).arg( id ) );
		case _3D:
			return QUrl( QString( "http://wiitdb.com/wiitdb/artwork/coverfull3D/%1/%2.png" ).arg( language ).arg( id ) );
		case Disc:
			return QUrl( QString( "http://wiitdb.com/wiitdb/artwork/disc/%1/%2.png" ).arg( language ).arg( id ) );
		case DiscCustom:
			return QUrl( QString( "http://wiitdb.com/wiitdb/artwork/disccustom/%1/%2.png" ).arg( language ).arg( id ) );
		case Full:
			return QUrl( QString( "http://wiitdb.com/wiitdb/artwork/coverfull/%1/%2.png" ).arg( language ).arg( id ) );
		case Invalid:
			break;
	}
	
	return QUrl();
}

Covers::Type Covers::type( const QUrl& url )
{
	if ( Covers( url ).url( HQ ) == url )
	{
		return HQ;
	}
	else if ( Covers( url ).url( Cover ) == url )
	{
		return Cover;
	}
	else if ( Covers( url ).url( _3D ) == url )
	{
		return _3D;
	}
	else if ( Covers( url ).url( Disc ) == url )
	{
		return Disc;
	}
	else if ( Covers( url ).url( DiscCustom ) == url )
	{
		return DiscCustom;
	}
	else if ( Covers( url ).url( Full ) == url )
	{
		return Full;
	}
	
	return Invalid;
}
