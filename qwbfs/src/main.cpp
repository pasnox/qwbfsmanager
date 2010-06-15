/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : main.cpp
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
#include <QApplication>
#include <QSplashScreen>
#include <QWeakPointer>
#include <QTimer>

#include "main.h"
#include "UIMain.h"

#define TIMEOUT 3000

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
		QTimer::singleShot( TIMEOUT, this, SLOT( close() ) );
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
	
	QObject::connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
	
	SplashScreen splash( QPixmap( ":/icons/splashscreen.png" ) );
	UIMain w;
	
	splash.handle( &w );	
	return app.exec();
}

#include "main.moc"
