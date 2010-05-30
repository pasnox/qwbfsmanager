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
	
	mSize = 100;
	mUsedSize = 40;
	mFreeSize = 60;
	mTemporarySize = 62;
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
	mSize = value;
	update();
}

qint64 Gauge::size() const
{
	return mSize;
}

void Gauge::setUsedSize( qint64 value )
{
	mUsedSize = value;
	update();
}

qint64 Gauge::usedSize() const
{
	return mUsedSize;
}

void Gauge::setFreeSize( qint64 value )
{
	mFreeSize = value;
	update();
}

qint64 Gauge::freeSize() const
{
	return mFreeSize;
}

void Gauge::setTemporarySize( qint64 value )
{
	mTemporarySize = value;
	update();
}

qint64 Gauge::temporarySize() const
{
	return mTemporarySize;
}

void Gauge::paintEvent( QPaintEvent* event )
{
	Q_UNUSED( event );
	QPainter painter( this );
	const qint64 available = mSize;
	const qint64 used = mUsedSize;
	const qint64 free = mFreeSize;
	const qint64 total = used +mTemporarySize;
	const double usedPercent = available != 0 ? ( (double)used /available ) *100.00 : 0;
	const double temporaryPercent = available != 0 ? ( (double)mTemporarySize /available ) *100.00 : 0;
	const double neededPercent = available != 0 ? ( (double)total /available ) *100.00 : 0;
	const double maxPercent = qMax( 100.00, neededPercent );
	QRect workingRect = rect();
	
	if ( workingRect.width() %2 != 0 ) {
		workingRect.setWidth( workingRect.width() -1 );
	}
	
	// groove space
	QRect grooveRect = workingRect;
	grooveRect.setWidth( grooveRect.width() /maxPercent *100.00 );
	
	// used space
	QRect usedRect = workingRect;
	usedRect.setWidth( usedRect.width() /maxPercent *usedPercent );
	
	// overflow space
	QRect overflowRect = workingRect;
	overflowRect.moveLeft( usedRect.width() );
	overflowRect.setWidth( overflowRect.width() /maxPercent *temporaryPercent );
	
	// extra space
	QRect extraRect = workingRect;
	extraRect.moveLeft( usedRect.width() );
	extraRect.setWidth( extraRect.width() /maxPercent *qBound( 0.00, temporaryPercent, 100.00 -usedPercent ) );
	
	// painting
	QStyleOptionProgressBarV2 option;
	option.bottomToTop = false;
	option.invertedAppearance = false;
	option.orientation = Qt::Horizontal;
	option.maximum = 100;
	option.minimum = 0;
	option.progress = 0;
	option.text = QString::null;
	option.textAlignment = Qt::AlignCenter;
	option.textVisible = true;
	option.direction = layoutDirection();
	option.fontMetrics = QFontMetrics( font() );
	option.palette = palette();
	option.rect = grooveRect;
	option.state = QStyle::State_None;
	
	if ( isEnabled() ) {
		option.state |= QStyle::State_Active;
		option.state |= QStyle::State_Enabled;
	}
	
	if ( hasFocus() ) {
		option.state |= QStyle::State_HasFocus;
	}
	
	if ( option.orientation == Qt::Horizontal ) {
		option.state |= QStyle::State_Horizontal;
	}
	
	// overflow contents
	option.progress = 100;
	option.palette.setColor( QPalette::Highlight, QColor( 255, 181, 213, 255 ) );
	option.rect = overflowRect;
	
	style()->drawControl( QStyle::CE_ProgressBarContents, &option, &painter, this );
	
	// progressbar groove
	option.rect = grooveRect;
	
	style()->drawControl( QStyle::CE_ProgressBarGroove, &option, &painter, this );
	
	// extra space
	option.progress = 100;
	option.palette.setColor( QPalette::Highlight, QColor( 213, 255, 181, 255 ) );
	option.rect = extraRect;
	
	style()->drawControl( QStyle::CE_ProgressBarContents, &option, &painter, this );
	
	// progressbar content
	option.progress = usedPercent;
	option.palette.setColor( QPalette::Highlight, palette().color( QPalette::Highlight ) );
	option.rect = grooveRect;
	
	style()->drawControl( QStyle::CE_ProgressBarContents, &option, &painter, this );
	
	// progressbar text
	option.rect = grooveRect;
	option.text = tr( "%1 (%2%) / %3 Used - %4 (%5%) Free" )
		.arg( fileSizeToString( used ) )
		.arg( fileSizeAdaptString( available != 0 ? (double)used /available *100.00 : 0 ) )
		.arg( fileSizeToString( available ) )
		.arg( fileSizeToString( free ) )
		.arg( fileSizeAdaptString( available != 0 ? (double)free /available *100.00 : 0 ) );
	
	style()->drawControl( QStyle::CE_ProgressBarLabel, &option, &painter, this );
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
