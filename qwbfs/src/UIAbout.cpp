#include "UIAbout.h"
#include "main.h"

UIAbout::UIAbout( QWidget* parent )
	: QDialog( parent )
{
	setAttribute( Qt::WA_DeleteOnClose );
	setupUi( this );
	lName->setText( APPLICATION_NAME );
	lVersion->setText( tr( "Version %1" ).arg( APPLICATION_VERSION ) );
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
