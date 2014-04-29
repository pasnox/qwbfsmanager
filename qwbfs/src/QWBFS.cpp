#include "QWBFS.h"
#include "main.h"
#include "filesystem/FileSystemEntry.h"

#include <FreshCore/pNetworkAccessManager>
#include <FreshCore/pSettings>
#include <FreshCore/pTranslationManager>

#include <QApplication>
#include <QPixmapCache>
#include <QDateTime>
#include <QDir>
#include <QDebug>

extern "C" {
	#include "lib-sf.h"
	#include "titles.h"
	#include "patch.h"
}

namespace QWBFS {
	//
};

void QWBFS::init( int argc, char** argv ) {
	SetupLib( argc, argv, "QWBFS", PROG_UNKNOWN );
	
	qsrand( QDateTime( QDate( 0, 0, 0 ) ).secsTo( QDateTime::currentDateTime() ) );
	
	QPixmapCache::setCacheLimit( QPixmapCache::cacheLimit() *4 );
	
	pNetworkAccessManager::instance()->setCacheDirectory( QDir::tempPath() );
	
	pSettings::setDefaultProperties( pSettings::Properties( APPLICATION_NAME, APPLICATION_VERSION, pSettings::Auto ) );
	
	pTranslationManager* translationManager = pTranslationManager::instance();
	translationManager->setFakeCLocaleEnabled( true );
	translationManager->addTranslationsMask( "qt*.qm" );
	translationManager->addTranslationsMask( "fresh*.qm" );
	translationManager->addTranslationsMask( "qwbfsmanager*.qm" );
	translationManager->addForbiddenTranslationsMask( "qt_help*.qm" );
}

void QWBFS::deInit()
{
	CloseAll();
}

pNetworkAccessManager* QWBFS::networkManager()
{
	return pNetworkAccessManager::instance();
}

QString QWBFS::entryTypeToString( QWBFS::EntryType type )
{
	return QString::fromUtf8( GetNameFT( enumFileType( type ), 0 ) );
}

QString QWBFS::entryStateToString( QWBFS::EntryState state )
{
	switch ( state ) {
		case QWBFS::EntryStateNone:
			return QObject::tr( "None" );
		case QWBFS::EntryStateSuccess:
			return QObject::tr( "Success" );
		case QWBFS::EntryStateFailed:
			return QObject::tr( "Failed" );
	}
	
	return QString::null;
}

QString QWBFS::entryRegionToString( char region, bool longFormat )
{
	const RegionInfo_t* rinfo = GetRegionInfo( region );
	return longFormat
		? QString::fromUtf8( rinfo->name )
		: QString( "%1 - %2" )
			.arg( QString::fromUtf8( rinfo->name4 ) )
			.arg( QString::fromUtf8( GetRegionName( rinfo->reg, "NaN" ) ) )
	;
}

QPixmap QWBFS::coverPixmap( QWBFS::WiiTDB::Scan scan, const QString& id, const QSize& size )
{
	if ( scan == QWBFS::WiiTDB::Cover ) {
		return QWBFS::WiiTDB::coverBoxPixmap( id, QWBFS::networkManager(), size );
	}
	
	return QWBFS::WiiTDB::coverDiscPixmap( id, QWBFS::networkManager(), size );
}

QPixmap QWBFS::statePixmap( QWBFS::EntryState state, const QSize& size )
{
	const QString url = state == QWBFS::EntryStateSuccess ? ":/icons/256/success.png" : ":/icons/256/error.png";
	const QString key = QString( "%1-%2-%3" ).arg( url ).arg( size.width() ).arg( size.height() );
	QPixmap pixmap;
	
	if ( !QPixmapCache::find( key, pixmap ) ) {
		if ( pixmap.load( url ) ) {
			pixmap = pixmap.scaled( size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
			QPixmapCache::insert( key, pixmap );
		}
	}
	
	return pixmap;
}

FileSystemEntry QWBFS::createEntry( const QString& filePath )
{
	SuperFile_t sf;
    InitializeSF( &sf );
	
	const enumError result = OpenSF( &sf, filePath.toLocal8Bit().constData(), true, false );
	
	if ( result != ERR_OK ) {
		CloseSF( &sf, 0 );
		qWarning( "%s: Error #%i - %s", Q_FUNC_INFO, result, GetErrorName( result ) );
		return FileSystemEntry();
	}
	
	wd_disc_t* disc = OpenDiscSF( &sf, false, false );
	
	if ( !sf.discs_loaded || !disc ) {
		CloseSF( &sf, 0 );
		qWarning( "%s: Can't open Wii disc: %s", Q_FUNC_INFO, sf.f.fname );
		return FileSystemEntry();
	}
	
	const QString title = QString::fromUtf8( GetTitle( sf.f.id6_src, QFileInfo( filePath ).baseName().toUtf8().constData() ) );
	const FileSystemEntry entry(
		filePath,
		QString::fromAscii( sf.f.id6_src, 6 ),
		sf.file_size,
		title,
		disc->dhead.region_code,
		QWBFS::EntryType( sf.f.ftype )
	);
	
	CloseSF( &sf, 0 );
	
	return entry;
}
