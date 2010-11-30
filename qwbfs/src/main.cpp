/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : main.cpp
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
#include <QApplication>
#include <QSplashScreen>
#include <QWeakPointer>
#include <QTimer>
#include <QDebug>

#include <Core/pSettings>
#include <Core/pTranslationManager>
#include <Gui/pIconManager>

#include "main.h"
#include "UIMain.h"

#define SPLASHSCREEN_TIMEOUT 3000

class SplashScreen : public QSplashScreen
{
	Q_OBJECT

public:
	SplashScreen( const QPixmap& pixmap )
		: QSplashScreen( pixmap )
	{
		QFont font = this->font();
		font.setPixelSize( 9 );
		setFont( font );
		showMessage( tr( "Version %1" ).arg( APPLICATION_VERSION_STR ), Qt::AlignRight | Qt::AlignBottom, QColor( 0, 0, 0 ) );
		
		show();
		raise();
	}
	
	void handle( QWidget* widget )
	{
		mWidget = widget;
		QTimer::singleShot( SPLASHSCREEN_TIMEOUT, this, SLOT( close() ) );
	}

protected:
	QWeakPointer<QWidget> mWidget;
	
	virtual void hideEvent( QHideEvent* event )
	{
		QSplashScreen::hideEvent( event );
		
		if ( mWidget && !mWidget.data()->isVisible() )
		{
			mWidget.data()->show();
			mWidget.data()->raise();
		}
	}
};

int main( int argc, char** argv )
{
	QApplication app( argc, argv );
	app.setApplicationName( APPLICATION_NAME );
	app.setOrganizationName( APPLICATION_ORGANIZATION );
	app.setOrganizationDomain( APPLICATION_DOMAIN );
	app.setWindowIcon( QIcon( ":/icons/qwbfsmanager.png" ) );
	
	Q_INIT_RESOURCE( fresh );
	Q_UNUSED( QT_TRANSLATE_NOOP( "QObject", "The Free, Fast and Powerful cross platform Wii Backup File System manager" ) );
	
	pSettings::setDefaultProperties( pSettings::Properties( APPLICATION_NAME, APPLICATION_VERSION, pSettings::Portable ) );
	
	pTranslationManager* translationManager = pTranslationManager::instance();
	translationManager->setFakeCLocaleEnabled( true );
	translationManager->addTranslationsMask( "qt*.qm" );
	translationManager->addTranslationsMask( "qwbfsmanager*.qm" );
	translationManager->addForbiddenTranslationsMask( "qt_help*.qm" );
	
	QObject::connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
	
	SplashScreen splash( pIconManager::pixmap( "splashscreen.png", ":/icons" ) );
	UIMain w;
	
	splash.handle( &w );	
	return app.exec();
}

#include "main.moc"
