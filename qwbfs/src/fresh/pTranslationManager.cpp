#include "pTranslationManager.h"

#include <QDir>
#include <QCoreApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>

Q_GLOBAL_STATIC( pTranslationManager, translationManagerInstance );

pTranslationManager* pTranslationManager::instance()
{
	return translationManagerInstance();
}

pTranslationManager::pTranslationManager( QObject* parent )
	: QObject( parent )
{
	mCurrentLocale = systemLocale();
	mSystemTranslationsPaths << QLibraryInfo::location( QLibraryInfo::TranslationsPath );
	mFakeCLocaleEnabled = false;
}

pTranslationManager::~pTranslationManager()
{
}

bool pTranslationManager::addTranslator( const QString& filePath, const QLocale& locale )
{
	QTranslator* translator = new QTranslator( QCoreApplication::instance() );
	
	if ( translator->load( filePath ) ) {
		QCoreApplication::installTranslator( translator );
		mTranslators[ locale ] << translator;
		//qWarning() << "added" << filePath << locale.name();
		return true;
	}
	
	delete translator;
	return false;
}

void pTranslationManager::clearTranslators()
{
	foreach ( const QLocale& locale, mTranslators.keys() ) {
		foreach ( QTranslator* translator, mTranslators[ locale ] ) {
			QCoreApplication::removeTranslator( translator );
		}
		
		qDeleteAll( mTranslators[ locale ] );
	}
	
	mAvailableLocales.clear();
	mTranslators.clear();
}

QStringList pTranslationManager::availableLocales() const
{
	return mAvailableLocales.toList();
}

QList<QLocale> pTranslationManager::availableQLocales() const
{
	QList<QLocale> locales;
	
	foreach ( const QString& locale, availableLocales() ) {
		locales << QLocale( locale );
	}
	
	return locales;
}

void pTranslationManager::reloadTranslations()
{
	clearTranslators();
	
	const QStringList paths = QStringList() << mTranslationsPaths << mSystemTranslationsPaths;
	const QString appDirPath = qApp->applicationDirPath();
	QList<QFileInfo> files;
	QSet<QString> translations;

	foreach ( QString path, paths ) {
		if ( QDir::isRelativePath( path ) ) {
			path = QString( "%1/%2" ).arg( appDirPath ).arg( path );
		}
		
		files << QDir( path ).entryInfoList( mTranslationsMasks.toList() );
	}
	
	foreach ( const QFileInfo& file, files ) {
		const QString cfp = file.canonicalFilePath();
		QString fileName = file.fileName();
		
		if ( translations.contains( cfp )
			|| QDir::match( mForbiddenTranslationsMasks.toList(), fileName ) ) {
			continue;
		}
		
		fileName.remove( ".qm", Qt::CaseInsensitive ).replace( ".", "_" ).replace( "-", "_" );
		const int count = fileName.count( "_" );
		bool added = false;
		bool foundValidLocale = false;
		QLocale locale;
		
		for ( int i = 1; i < count +1; i++ ) {
			QString part = fileName.section( '_', i );
			
			if ( part.toLower() == "iw" || part.toLower().endsWith( "_iw" ) )
			{
				part.replace( "iw", "he" );
			}
			
			locale = QLocale( part );
			
			//qWarning() << fileName << part << locale.name() << mCurrentLocale.name();
			
			if ( locale != QLocale::c() ) {
				foundValidLocale = true;
				mAvailableLocales << locale.name();
				
				if ( locale == mCurrentLocale
					|| locale.language() == mCurrentLocale.language() ) {
					if ( addTranslator( cfp, locale ) ) {
						translations << cfp;
						added = true;
					}
				}
				
				break;
			}
		}
		
		if ( foundValidLocale || added /*|| mCurrentLocale != QLocale::c()*/ ) {
			continue;
		}
		
		mAvailableLocales << QLocale::c().name();
		
		if ( mCurrentLocale.language() == QLocale::C
			|| mCurrentLocale.language() == QLocale::English ) {
			if ( addTranslator( cfp, QLocale::c() ) ) {
				translations << cfp;
			}
		}
	}
	
	if ( !mAvailableLocales.contains( QLocale::c().name() ) && mFakeCLocaleEnabled ) {
		mAvailableLocales << QLocale::c().name();
	}
}

QStringList pTranslationManager::translationsMasks() const
{
	return mTranslationsMasks.toList();
}

void pTranslationManager::setTranslationsMasks( const QStringList& masks )
{
	mTranslationsMasks = masks.toSet();
}

void pTranslationManager::addTranslationsMask( const QString& mask )
{
	mTranslationsMasks << mask;
}

void pTranslationManager::removeTranslationsMask( const QString& mask )
{
	mTranslationsMasks.remove( mask );
}

QStringList pTranslationManager::forbiddenTranslationsMasks() const
{
	return mForbiddenTranslationsMasks.toList();
}

void pTranslationManager::setForbiddenTranslationsMasks( const QStringList& masks )
{
	mForbiddenTranslationsMasks = masks.toSet();
}

void pTranslationManager::addForbiddenTranslationsMask( const QString& mask )
{
	mForbiddenTranslationsMasks << mask;
}

void pTranslationManager::removeForbiddenTranslationsMask( const QString& mask )
{
	mForbiddenTranslationsMasks.remove( mask );
}

QLocale pTranslationManager::currentLocale() const
{
	return mCurrentLocale;
}

QLocale pTranslationManager::systemLocale() const
{
	return QLocale::system();
}

void pTranslationManager::setCurrentLocale( const QLocale& locale )
{
	mCurrentLocale = locale;
	QLocale::setDefault( locale );
}

QStringList pTranslationManager::translationsPaths() const
{
	return mTranslationsPaths;
}

void pTranslationManager::setTranslationsPaths( const QStringList& paths )
{
	mTranslationsPaths = paths;
}

QStringList pTranslationManager::systemTranslationsPaths() const
{
	return mSystemTranslationsPaths;
}

void pTranslationManager::setSystemTranslationsPaths( const QStringList& paths )
{
	mSystemTranslationsPaths = paths;
}

void pTranslationManager::setFakeCLocaleEnabled( bool enabled )
{
	mFakeCLocaleEnabled = enabled;
}

bool pTranslationManager::isFakeCLocaleEnabled() const
{
	return mFakeCLocaleEnabled;
}
