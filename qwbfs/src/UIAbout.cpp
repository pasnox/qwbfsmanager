/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : UIAbout.cpp
** Date      : 2010-05-29T13:13:25
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
#include "UIAbout.h"
#include "main.h"

UIAbout::UIAbout( QWidget* parent )
	: QDialog( parent )
{
	setAttribute( Qt::WA_DeleteOnClose );
	setupUi( this );
	lName->setText( APPLICATION_NAME );
	lVersion->setText( tr( "Version %1" ).arg( APPLICATION_VERSION_STR ) );
	lDescription->setText( APPLICATION_DESCRIPTION );
	lCopyrights->setText( APPLICATION_COPYRIGHTS );
	lDomain->setText( QString( "<a href=\"%1\">%1</a>" ).arg( APPLICATION_DOMAIN ) );
	adjustSize();
}

void UIAbout::on_dbbButtons_clicked( QAbstractButton* button )
{
	const QDialogButtonBox::StandardButton sb = dbbButtons->standardButton( button );
	
	switch ( sb )
	{
		case QDialogButtonBox::Ok:
			QDialog::accept();
			break;
		default:
			break;
	}
}
