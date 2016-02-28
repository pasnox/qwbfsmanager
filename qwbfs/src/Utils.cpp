#include "Utils.h"

#include <QApplication>
#include <QStyleFactory>

QStyle* Utils::neutralStyle()
{
    const QStringList keys = QStyleFactory::keys();

    if ( keys.contains( QLatin1String( "plastique" ), Qt::CaseInsensitive ) ) {
        return QStyleFactory::create( QLatin1String( "plastique" ) );
    }
    else if ( keys.contains( QLatin1String( "fusion" ), Qt::CaseInsensitive ) ) {
        return QStyleFactory::create( QLatin1String( "fusion" ) );
    }

    // Just use native style as fallback
    return QApplication::style();
}
