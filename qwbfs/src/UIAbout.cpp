/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : UIAbout.cpp
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
	lDomain->setText( QString( "<a href=\"http://%1\" style=\"text-decoration:none;\">http://%1</a>" ).arg( APPLICATION_DOMAIN ) );
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
