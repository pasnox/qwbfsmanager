#include "PropertiesDialog.h"
#include "Properties.h"

PropertiesDialog::PropertiesDialog( QWidget* parent )
	: QDialog( parent )
{
	setupUi( this );
	setAttribute( Qt::WA_DeleteOnClose );
	mProperties = new Properties( this );
}

PropertiesDialog::~PropertiesDialog()
{
}
