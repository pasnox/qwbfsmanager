/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : PaypalDonationWidget.cpp
** Date      : 2010-20-16T16:19:29
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**
** In addition, as a special exception, the copyright holders give permission
** to link this program with the OpenSSL project's "OpenSSL" library (or with
** modified versions of it that use the same license as the "OpenSSL"
** library), and distribute the linked executables. You must obey the GNU
** General Public License in all respects for all of the code used other than
** "OpenSSL".  If you modify file(s), you may extend this exception to your
** version of the file(s), but you are not obligated to do so. If you do not
** wish to do so, delete this exception statement from your version.
**
****************************************************************************/
#include "PaypalDonationWidget.h"
#include "datacache/DataNetworkCache.h"

#include <QEvent>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QDebug>

Q_GLOBAL_STATIC( DataNetworkCache, networkCache );

PaypalDonationWidget::PaypalDonationWidget( QWidget* parent )
	: QLabel( parent )
{
	setCursor( Qt::PointingHandCursor );
	setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum ) );
	
	mQueryItems[ "path" ] = "https://www.paypal.com/cgi-bin/webscr";
	mQueryItems[ "cmd" ] = "_donations";
	mQueryItems[ "bn" ] = QUrl::fromPercentEncoding( "PP%2dDonationsBF%3abtn_donate_SM%2egif%3aNonHosted" );
	
	localeChanged();
	
	connect( networkCache(), SIGNAL( dataCached( const QUrl& ) ), this, SLOT( networkCache_dataCached( const QUrl& ) ) );
	connect( networkCache(), SIGNAL( error( const QString&, const QUrl& ) ), this, SLOT( networkCache_error( const QString&, const QUrl& ) ) );
	connect( networkCache(), SIGNAL( invalidated() ), this, SLOT( networkCache_invalidated() ) );
}

PaypalDonationWidget::~PaypalDonationWidget()
{
}

bool PaypalDonationWidget::event( QEvent* event )
{
	switch ( event->type() ) {
		case QEvent::Show:
			localeChanged();
			break;
		case QEvent::LocaleChange:
			localeChanged();
			break;
		default:
			break;
	}
	
	return QLabel::event( event );
}

void PaypalDonationWidget::mousePressEvent( QMouseEvent* event )
{
	QLabel::mousePressEvent( event );
	
	QDesktopServices::openUrl( url() );
}

QUrl PaypalDonationWidget::pixmapUrl( const QString& locale )
{
	return QUrl( QString( "https://www.paypal.com/%1/i/btn/btn_donate_LG.gif" ).arg( locale ) );
}

QString PaypalDonationWidget::actionPost() const
{
	return mQueryItems.value( "path" );
}

void PaypalDonationWidget::setActionPost( const QString& value )
{
	mQueryItems[ "path" ] = value;
}

QString PaypalDonationWidget::businessId() const
{
	return mQueryItems.value( "business" );
}

void PaypalDonationWidget::setBusinessId( const QString& value )
{
	mQueryItems[ "business" ] = value;
}

QString PaypalDonationWidget::itemName() const
{
	return mQueryItems.value( "item_name" );
}

void PaypalDonationWidget::setItemName( const QString& value )
{
	mQueryItems[ "item_name" ] = value;
}

QString PaypalDonationWidget::itemId() const
{
	return mQueryItems.value( "item_number" );
}

void PaypalDonationWidget::setItemId( const QString& value )
{
	mQueryItems[ "item_number" ] = value;
}

QString PaypalDonationWidget::currencyCode() const
{
	return mQueryItems.value( "currency_code" );
}

void PaypalDonationWidget::setCurrencyCode( const QString& value )
{
	mQueryItems[ "currency_code" ] = value;
}

QPixmap PaypalDonationWidget::pixmap( const QUrl& url ) const
{
	QPixmap pixmap = networkCache()->cachedPixmap( url );
	
	if ( pixmap.isNull() ) {
		pixmap = QPixmap( ":/icons/donation.png" );
	}
	
	return pixmap;
}

DataNetworkCache* PaypalDonationWidget::cache() const
{
	return networkCache();
}

QUrl PaypalDonationWidget::url() const
{
	QUrl url( mQueryItems.value( "path" ) );
	QList<QPair<QString, QString> > queryItems;
	
	foreach ( const QString& key, mQueryItems.keys() ) {
		if ( key == "path" ) {
			continue;
		}
		
		queryItems << qMakePair( key, mQueryItems[ key ] );
	}
	
	url.setQueryItems( queryItems );
	
	return url;
}

void PaypalDonationWidget::localeChanged()
{
	mQueryItems[ "lc" ] = locale().name().section( "_", 1 );
	
	setText( tr( "Donation" ) );
	setToolTip( tr( "Make a donation via Paypal" ) );
	
	updatePixmap();
}

void PaypalDonationWidget::updatePixmap()
{
	const QUrl url = pixmapUrl( locale().name() );
	
	networkCache_dataCached( url );
	
	if ( !networkCache()->hasCachedData( url ) ) {
		networkCache()->cacheData( url );
	}
}

void PaypalDonationWidget::networkCache_dataCached( const QUrl& url )
{
	const QPixmap pixmap = this->pixmap( url );
	setPixmap( pixmap );
}

void PaypalDonationWidget::networkCache_error( const QString& message, const QUrl& url )
{
	qWarning() << message << url;
}

void PaypalDonationWidget::networkCache_invalidated()
{
	updatePixmap();
}
