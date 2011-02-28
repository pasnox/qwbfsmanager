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

#include <FreshCore/pNetworkAccessManager>
#include <FreshGui/pGuiUtils>

#include <QFileInfo>
#include <QPixmapCache>
#include <QDebug>

using namespace QWBFS;

QUrl WiiTDB::checkPixmapCache( WiiTDB::Scan scan, const QString& id, pNetworkAccessManager* cache )
{
	QUrl url = coverUrl( scan, id );
	
	if ( !cache->hasCacheData( url ) ) {
		QUrl u = coverUrl( scan, id, "EN" ).toString();
		
		if ( !cache->hasCacheData( u ) ) {
			u = coverUrl( scan, id, "other" ).toString();
			
			if ( cache->hasCacheData( u ) ) {
				url = u;
			}
		}
		else {
			url = u;
		}
	}
	
	return url;
}

QUrl WiiTDB::coverUrl( WiiTDB::Scan scan, const QString& id, const QString& local )
{
	Q_ASSERT( !id.isEmpty() );
	
	const QString language = local.isNull() ? QWBFS::Driver::regionToLanguageString( id.at( 3 ).unicode() ) : local;
	
	switch ( scan ) {
		case WiiTDB::CoverHQ:
			return QUrl( QString( "%3/wiitdb/artwork/coverfullHQ/%1/%2.png" ).arg( language ).arg( id ).arg( WIITDB_DOMAIN ) );
		case WiiTDB::Cover:
			return QUrl( QString( "%3/wiitdb/artwork/cover/%1/%2.png" ).arg( language ).arg( id ).arg( WIITDB_DOMAIN ) );
		case WiiTDB::Cover3D:
			return QUrl( QString( "%3/wiitdb/artwork/coverfull3D/%1/%2.png" ).arg( language ).arg( id ).arg( WIITDB_DOMAIN ) );
		case WiiTDB::CoverDisc:
			return QUrl( QString( "%3/wiitdb/artwork/disc/%1/%2.png" ).arg( language ).arg( id ).arg( WIITDB_DOMAIN ) );
		case WiiTDB::CoverDiscCustom:
			return QUrl( QString( "%3/wiitdb/artwork/disccustom/%1/%2.png" ).arg( language ).arg( id ).arg( WIITDB_DOMAIN ) );
		case WiiTDB::CoverFull:
			return QUrl( QString( "%3/wiitdb/artwork/coverfull/%1/%2.png" ).arg( language ).arg( id ).arg( WIITDB_DOMAIN ) );
		case WiiTDB::CoverInvalid:
			break;
	}
	
	return QUrl();
}

QPixmap WiiTDB::coverBoxPixmap( const QString& id, pNetworkAccessManager* cache, const QSize& size )
{
	const QString url = checkPixmapCache( QWBFS::WiiTDB::Cover, id, cache ).toString();
	const QString key = pGuiUtils::cacheKey( url, size );
	QPixmap pixmap;
	
	if ( !QPixmapCache::find( key, pixmap ) ) {
		if ( !cache->hasCacheData( url ) ) {
			cache->get( QNetworkRequest( url ) );
			return pGuiUtils::scaledPixmap( ":/wii/cover.png", size );
		}
		
		QIODevice* data = cache->cacheData( url );
		
		if ( data && pixmap.loadFromData( data->readAll() ) ) {
			pixmap = pGuiUtils::scaledPixmap( pixmap, url, size );
		}
		
		delete data;
	}
	
	return pixmap;
}

QPixmap WiiTDB::coverDiscPixmap( const QString& id, pNetworkAccessManager* cache, const QSize& size )
{
	QString url = checkPixmapCache( QWBFS::WiiTDB::CoverDisc, id, cache ).toString();
	
	if ( !cache->hasCacheData( url ) ) {
		const QString u = checkPixmapCache( QWBFS::WiiTDB::CoverDiscCustom, id, cache ).toString();
		
		if ( cache->hasCacheData( u ) ) {
			url = u;
		}
	}
	
	const QString key = pGuiUtils::cacheKey( url, size );
	QPixmap pixmap;
	
	if ( !QPixmapCache::find( key, pixmap ) ) {
		if ( !cache->hasCacheData( url ) ) {
			cache->get( QNetworkRequest( url ) );
			return pGuiUtils::scaledPixmap( ":/wii/disc.png", size );
		}
		
		QIODevice* data = cache->cacheData( url );
		
		if ( data && pixmap.loadFromData( data->readAll() ) ) {
			pixmap = pGuiUtils::scaledPixmap( pixmap, url, size );
		}
		
		delete data;
	}
	
	return pixmap;
}

WiiTDB::Scan WiiTDB::urlCover( const QUrl& url )
{
	const QString path = url.path();
	
	if ( path.contains( "coverfullHQ", Qt::CaseInsensitive ) ) {
		return WiiTDB::CoverHQ;
	}
	else if ( path.contains( "coverfull3D", Qt::CaseInsensitive ) ) {
		return WiiTDB::Cover3D;
	}
	else if ( path.contains( "coverfull", Qt::CaseInsensitive ) ) {
		return WiiTDB::CoverFull;
	}
	else if ( path.contains( "cover", Qt::CaseInsensitive ) ) {
		return WiiTDB::Cover;
	}
	else if ( path.contains( "disccustom", Qt::CaseInsensitive ) ) {
		return WiiTDB::CoverDiscCustom;
	}
	else if ( path.contains( "disc", Qt::CaseInsensitive ) ) {
		return WiiTDB::CoverDisc;
	}
	
	return WiiTDB::CoverInvalid;
}
