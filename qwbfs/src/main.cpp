#include <QApplication>

#include "src/UIMain.h"

int main( int argc, char** argv )
{
	QApplication app( argc, argv );
	app.setApplicationName( "QWBFS Manager" );
	
	QObject::connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
	
	UIMain w;
	w.show();
	
	return app.exec();
}
