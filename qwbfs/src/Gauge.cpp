#include "Gauge.h"

#include <QPainter>
#include <QDebug>

Gauge::Gauge( QWidget* parent )
	: QWidget( parent )
{
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
	
	// border
	QRect borderRect = workingRect.adjusted( 0, 0, -1, -1 );
	borderRect.setWidth( borderRect.width() /maxPercent *100.00 );
	
	painter.save();
	painter.setPen( QColor( 0, 0, 0, 255 ) );
	painter.setBrush( QColor( 255, 255, 255, 255 ) );
	painter.drawRect( borderRect );
	painter.restore();
	
	// used space
	QRect usedRect = workingRect.adjusted( 1, 1, 0, -1 );
	usedRect.setWidth( usedRect.width() /maxPercent *usedPercent );
	
	painter.save();
	painter.setPen( Qt::NoPen );
	painter.setBrush( QColor( 38, 125, 255, 180 ) );
	painter.drawRect( usedRect );
	painter.restore();
	
	// extra space
	QRect extraRect = workingRect.adjusted( 1, 1, 0, -1 );
	extraRect.moveLeft( usedRect.width() +1 );
	extraRect.setWidth( extraRect.width() /maxPercent *temporaryPercent );
	
	painter.save();
	painter.setPen( Qt::NoPen );
	painter.setBrush( QColor( 255, 46, 43, 180 ) );
	painter.drawRect( extraRect );
	painter.restore();
	
	// temporary space
	QRect temporaryRect = workingRect.adjusted( 1, 1, 0, -1 );
	temporaryRect.moveLeft( usedRect.width() +1 );
	temporaryRect.setWidth( temporaryRect.width() /maxPercent *qBound( 0.00, temporaryPercent, 100.00 -usedPercent ) );
	
	painter.save();
	painter.setPen( Qt::NoPen );
	painter.setBrush( QColor( 168, 255, 87, 255 ) );
	painter.drawRect( temporaryRect );
	painter.restore();
	
	// used space
	const QString text = tr( "%1 (%2%) / %3 Used - %4 (%5%) Free" )
		.arg( fileSizeToString( used ) )
		.arg( fileSizeAdaptString( available != 0 ? (double)used /available *100.00 : 0 ) )
		.arg( fileSizeToString( available ) )
		.arg( fileSizeToString( free ) )
		.arg( fileSizeAdaptString( available != 0 ? (double)free /available *100.00 : 0 ) );
	const int flags = Qt::AlignCenter | Qt::TextWordWrap;
	
	painter.save();
	painter.setPen( QColor( 0, 0, 0, 255 ) );
	painter.setBrush( Qt::NoBrush );
	painter.drawText( borderRect, flags, text );
	painter.restore();
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
