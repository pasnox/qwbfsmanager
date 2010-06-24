/****************************************************************************
**
** 		Created using Monkey Studio v1.8.1.0
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : Monkey Studio Base Plugins
** FileName  : pUpdateCheckerDialog.cpp
** Date      : 2008-01-14T00:39:51
** License   : GPL2
** Comment   : This header has been automatically generated, if you are the original author, or co-author, fill free to replace/append with your informations.
** Home Page : http://www.monkeystudio.org
**
	Copyright (C) 2005 - 2008  Filipe AZEVEDO & The Monkey Studio Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
****************************************************************************/
#include "pUpdateCheckerDialog.h"
#include "pUpdateChecker.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPushButton>
#include <QDesktopServices>
#include <QDebug>

// pUpdateItem

pUpdateItem::pUpdateItem( pUpdateChecker* updateChecker, const QDomElement& element )
{
	mUpdateChecker = updateChecker;
	
	QDomNodeList nodes = element.childNodes();
	
	for ( int i = 0; i < nodes.count(); i++ ) {
		const QDomElement el = nodes.at( i ).toElement();
		const QString name = el.tagName();
		
		if ( name == "updated" ) {
			mDatas[ pUpdateItem::Updated ] = el.firstChild().toText().data();
		}
		else if ( name == "id" ) {
			mDatas[ pUpdateItem::Id ] = el.firstChild().toText().data();
		}
		else if ( name == "link" ) {
			mDatas[ pUpdateItem::Link ] = el.attribute( "href" );
		}
		else if ( name == "title" ) {
			mDatas[ pUpdateItem::Title ] = el.firstChild().toText().data().trimmed();
		}
		else if ( name == "author" ) {
			mDatas[ pUpdateItem::Author ] = el.firstChild().firstChild().toText().data();
		}
		else if ( name == "content" ) {
			mDatas[ pUpdateItem::Content ] = el.firstChild().toText().data().trimmed();
		}
	}
}

bool pUpdateItem::operator<( const pUpdateItem& other ) const
{
	return pVersion( version() ) < pVersion( other.version() );
}

bool pUpdateItem::operator>( const pUpdateItem& other ) const
{
	return pVersion( version() ) > pVersion( other.version() );
}

bool pUpdateItem::operator<( const pVersion& other ) const
{
	return pVersion( version() ) < other;
}

bool pUpdateItem::operator>( const pVersion& other ) const
{
	return pVersion( version() ) > other;
}

QDateTime pUpdateItem::updated() const
{
	return QDateTime::fromString( mDatas.value( pUpdateItem::Updated ), Qt::ISODate );
}

QString pUpdateItem::id() const
{
	return mDatas.value( pUpdateItem::Id );
}

QUrl pUpdateItem::link() const
{
	return QUrl( mDatas.value( pUpdateItem::Link ) );
}

QString pUpdateItem::title() const
{
	return mDatas.value( pUpdateItem::Title );
}

QString pUpdateItem::author() const
{
	return mDatas.value( pUpdateItem::Author );
}

QString pUpdateItem::content() const
{
	return mDatas.value( pUpdateItem::Content );
}

QString pUpdateItem::toolTip() const
{
	return content().replace(
		QRegExp( "<a.*</a>" ), pUpdateCheckerDialog::tr( QT_TRANSLATE_NOOP( "pUpdateCheckerDialog", "Updated on %1 by %2" ) )
			.arg( updated().toString( Qt::DefaultLocaleLongDate ) )
			.arg( author() )
	);
}

bool pUpdateItem::isFeatured() const
{
	return content().contains( "Featured", Qt::CaseInsensitive );
}

QString pUpdateItem::displayText() const
{
	return content().split( "\n" ).value( 1 ).trimmed().append( " ( " ).append( title() ).append( " ) " );
}

QString pUpdateItem::versionString() const
{
	if ( !mUpdateChecker ) {
		return QString::null;
	}
	
	const QString text = title();
	QRegExp rx( mUpdateChecker->versionDiscoveryPattern() );
	
	if ( rx.exactMatch( text ) ) {
		return rx.cap( 1 );
	}
	
	return QString::null;
}

pVersion pUpdateItem::version() const
{
	return pVersion( versionString() );
}

bool pUpdateItem::isValid() const
{
	return !mDatas.isEmpty();
}

// pUpdateCheckerDialog

pUpdateCheckerDialog::pUpdateCheckerDialog( pUpdateChecker* updateChecker, QWidget* parent )
	: QDialog( parent )
{
	Q_ASSERT( updateChecker );
	
	mUpdateChecker = updateChecker;
	
	setupUi( this );
	setAttribute( Qt::WA_DeleteOnClose );
	setAttribute( Qt::WA_MacSmallSize );
	dbbButtons->button( QDialogButtonBox::Yes )->setEnabled( false );
	
	foreach ( QWidget* widget, findChildren<QWidget*>() ) {
		widget->setAttribute( Qt::WA_MacSmallSize );
	}
	
	mAccessManager = new QNetworkAccessManager( this );
	
	localeChanged();
	
	connect( mAccessManager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( accessManager_finished( QNetworkReply* ) ) );
	
	mAccessManager->get( QNetworkRequest( mUpdateChecker->downloadsFeedUrl() ) );
}

bool pUpdateCheckerDialog::event( QEvent* event )
{
	switch ( event->type() ) {
		case QEvent::LocaleChange:
			localeChanged();
			break;
		default:
			break;
	}
	
	return QDialog::event( event );
}

void pUpdateCheckerDialog::localeChanged()
{
	retranslateUi( this );
	lVersion->setText( tr( "You are using version <b>%1</b> (%2)." ).arg( mUpdateChecker->version() ).arg( mUpdateChecker->versionString() ) );
	dbbButtons->button( QDialogButtonBox::Yes )->setText( tr( "Download" ) );
}

void pUpdateCheckerDialog::accessManager_finished( QNetworkReply* reply )
{
	const pVersion currentVersion( mUpdateChecker->version() );
	const QDateTime lastUpdated = mUpdateChecker->lastUpdated();
	//const QDateTime lastCheck = mUpdateChecker->lastChecked();
	
	if ( reply->error() != QNetworkReply::NoError ) {
		lwVersions->addItem( new QListWidgetItem( tr( "An error occur: %1" ).arg( reply->errorString() ) ) );
	}
	else {
		QDomDocument document;
		
		if ( document.setContent( reply->readAll() ) ) {
			const QString updatedText = document.elementsByTagName( "updated" ).at( 0 ).firstChild().toText().data();
			const QDateTime updated = QDateTime::fromString( updatedText, Qt::ISODate );
			const QDomNodeList entries = document.elementsByTagName( "entry" );
			
			for ( int i = 0; i < entries.count(); i++ ) {
				const QDomElement element = entries.at( i ).toElement();
				
				const pUpdateItem updateItem( mUpdateChecker, element );
				
				if ( updateItem.isFeatured() && updateItem > currentVersion ) {
					QListWidgetItem* item = new QListWidgetItem( updateItem.displayText() );
					
					item->setToolTip( updateItem.toolTip() );
					item->setData( Qt::UserRole, QVariant::fromValue( updateItem ) );
					lwVersions->addItem( item );
				}
			}
			
			mUpdateChecker->setLastUpdated( updated );
			
			qWarning() << lastUpdated << updated << ( lastUpdated < updated );
			
			if ( lwVersions->count() > 0 ) {				
				if ( !isVisible() && lastUpdated > updated ) {
					open();
				}
			}
			else {
				QListWidgetItem* item = new QListWidgetItem( tr( "You are running the last available version." ) );
				
				item->setFlags( Qt::NoItemFlags );
				lwVersions->addItem( item );
				
				if ( !isVisible() ) {
					close();
				}
			}
		}
		else {
			lwVersions->addItem( new QListWidgetItem( tr( "An error occur while parsing xml, retry later." ) ) );
		}
	}
	
	mUpdateChecker->setLastChecked( QDateTime::currentDateTime() );
}

void pUpdateCheckerDialog::on_lwVersions_itemSelectionChanged()
{
	QListWidgetItem* item = lwVersions->selectedItems().value( 0 );
	const pUpdateItem updateItem = item ? item->data( Qt::UserRole ).value<pUpdateItem>() : pUpdateItem();
	
	dbbButtons->button( QDialogButtonBox::Yes )->setEnabled( updateItem.isValid() );
}

void pUpdateCheckerDialog::accept()
{
	QListWidgetItem* item = lwVersions->selectedItems().value( 0 );
	const pUpdateItem updateItem = item->data( Qt::UserRole ).value<pUpdateItem>();
	
	QDesktopServices::openUrl( updateItem.link() );
	QDialog::accept();
}
