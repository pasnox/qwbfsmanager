/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : Gauge.cpp
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
#include "Gauge.h"

#include <QStyleFactory>
#include <QStyleOptionProgressBarV2>
#include <QPainter>
#include <QDebug>

Gauge::Gauge( QWidget* parent )
	: QWidget( parent )
{
	setStyle( QStyleFactory::create( "plastique" ) );
	
	mSize = 0;
	mUsedSize = 0;
	mFreeSize = 0;
	mTemporarySize = 0;
}

Gauge::~Gauge()
{
}

QSize Gauge::sizeHint() const
{
	return QSize( 200, 21 );
}

QSize Gauge::minimumSizeHint() const
{
	return QSize( -1, sizeHint().height() );
}

void Gauge::setSize( qint64 value )
{
	mSize = qMax( Q_INT64_C( 0 ), value );
	update();
}

qint64 Gauge::size() const
{
	return mSize;
}

void Gauge::setUsedSize( qint64 value )
{
	mUsedSize = qMax( Q_INT64_C( 0 ), value );
	update();
}

qint64 Gauge::usedSize() const
{
	return mUsedSize;
}

void Gauge::setFreeSize( qint64 value )
{
	mFreeSize = qMax( Q_INT64_C( 0 ), value );
	update();
}

qint64 Gauge::freeSize() const
{
	return mFreeSize;
}

void Gauge::setTemporarySize( qint64 value )
{
	mTemporarySize = qMax( Q_INT64_C( 0 ), value );
	update();
}

qint64 Gauge::temporarySize() const
{
	return mTemporarySize;
}

void Gauge::paintEvent( QPaintEvent* event )
{
	Q_UNUSED( event );
	QPainter painter( this );	const qreal available = mSize;
	const qreal used = mUsedSize;
	const qreal free = mFreeSize;
	const qreal max = used +mTemporarySize;
	const qreal total = qMax( available, max );
	const qreal cent = 100;
	const int totalPercent = available != 0 ? total /available *cent : 0;
	const int usedPercent = available != 0 ? used /available *cent : 0;
	const int extraPercent = max != used && available != 0 ? qBound( (double)usedPercent, max /available *cent, cent ) : 0;
	const int overflowPercent = max > available ? ( available != 0 ? max /available *cent : 0 ) : 0;
	const int flags = Qt::AlignCenter | Qt::TextWordWrap;
	
	QStyleOptionProgressBarV2 option;
	option.bottomToTop = false;
	option.invertedAppearance = false;
	option.orientation = Qt::Horizontal;
	option.maximum = totalPercent;
	option.minimum = 0;
	option.progress = 0;
	option.text = option.text = tr( "Usage %1 (%2%) / %3 - Free %4 (%5%)" )
		.arg( fileSizeToString( used ) )
		.arg( fileSizeAdaptString( available != 0 ? used /available *cent : 0 ) )
		.arg( fileSizeToString( available ) )
		.arg( fileSizeToString( free ) )
		.arg( fileSizeAdaptString( available != 0 ? free /available *cent : 0 ) );
	option.textAlignment = Qt::AlignCenter;
	option.textVisible = true;
	option.direction = layoutDirection();
	option.fontMetrics = QFontMetrics( font() );
	option.palette = palette();
	option.rect = rect();
	option.state = QStyle::State_Horizontal;
	
	if ( isEnabled() ) {
		option.state |= QStyle::State_Active;
		option.state |= QStyle::State_Enabled;
	}
	
	if ( hasFocus() ) {
		option.state |= QStyle::State_HasFocus;
	}
	
	// groove
	style()->drawControl( QStyle::CE_ProgressBarGroove, &option, &painter, this );
	
	// overflow
	if ( overflowPercent != 0 ) {
		option.progress = overflowPercent;
		option.palette.setColor( QPalette::Highlight, QColor( 255, 181, 213, 255 ) );
		style()->drawControl( QStyle::CE_ProgressBarContents, &option, &painter, this );
	}
	
	// extra
	if ( extraPercent != 0 ) {
		option.progress = extraPercent;
		option.palette.setColor( QPalette::Highlight, QColor( 213, 255, 181, 255 ) );
		style()->drawControl( QStyle::CE_ProgressBarContents, &option, &painter, this );
	}
	
	// used
	if ( usedPercent != 0 ) {
		option.progress = usedPercent;
		option.palette.setColor( QPalette::Highlight, palette().color( QPalette::Highlight ) );
		style()->drawControl( QStyle::CE_ProgressBarContents, &option, &painter, this );
	}
	
	// text	painter.setPen( option.palette.color( QPalette::Text ) );	painter.setBrush( Qt::NoBrush );	painter.drawText( rect(), flags, option.text );
	//style()->drawControl( QStyle::CE_ProgressBarLabel, &option, &painter, this );
}

QString Gauge::fileSizeAdaptString( double nb )
{
	return nb >= 100 ? QString::number( nb, 'f', 0 ) : QString::number( nb, 'g', 3 );
}

QString Gauge::fileSizeToString( double nb )
{
	if ( nb < 1024 )
	{
		return QString::number( nb ) +" " +QObject::tr( "B"  );
	}
	
	if ( ( nb = nb / 1024 ) < 1024 )
	{
		return fileSizeAdaptString( nb ) +" " +QObject::tr( "KB" );
	}
	
	if ( ( nb = nb / 1024 ) < 1024 )
	{
		return fileSizeAdaptString( nb ) +" " +QObject::tr( "MB" );
	}
	
	if ( ( nb = nb / 1024 ) < 1024 )
	{
		return fileSizeAdaptString( nb ) +" " +QObject::tr( "GB" );
	}
	
	if ( ( nb = nb / 1024 ) < 1024 )
	{
		return fileSizeAdaptString( nb ) +" " +QObject::tr( "TB" );
	}
	
	if ( ( nb = nb / 1024 ) < 1024 )
	{
		return fileSizeAdaptString( nb ) +" " +QObject::tr( "PB" );
	}
	
	return QObject::tr( "Too big" );
}
