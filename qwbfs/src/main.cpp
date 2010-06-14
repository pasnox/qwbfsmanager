/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : main.cpp
** Date      : 2010-04-25T13:05:33
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a crossplatform WBFS Manager developed using Qt4/C++.
** It's currently working under Unix/Linux, Mac OS X, and build under windows (but not yet working).
** 
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR WII CONSOLE OR WII PARTITION
** BECAUSE OF IMPROPER USAGE OF THIS SOFTWARE.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <QApplication>#include <QSplashScreen>
#include <QWeakPointer>#include <QTimer>

#include "main.h"
#include "UIMain.h"#define TIMEOUT 3000

class SplashScreen : public QSplashScreen
{
	Q_OBJECT

public:
	SplashScreen( const QPixmap& pixmap )
		: QSplashScreen( pixmap )
	{
		QFont font = this->font();		font.setPixelSize( 9 );
		setFont( font );
				showMessage( tr( "Version %1" ).arg( APPLICATION_VERSION_STR ), Qt::AlignRight | Qt::AlignBottom, QColor( 0, 0, 0 ) );
		
		show();
		raise();
	}
	
	void handle( QWidget* widget )
	{
		mWidget = widget;		QTimer::singleShot( TIMEOUT, this, SLOT( close() ) );
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
	app.setWindowIcon( QIcon( ":/icons/qwbfs.png" ) );
	
	QObject::connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );		SplashScreen splash( QPixmap( ":/icons/splashscreen.png" ) );
	UIMain w;
	
	splash.handle( &w );	
	return app.exec();
}

#include "main.moc"
