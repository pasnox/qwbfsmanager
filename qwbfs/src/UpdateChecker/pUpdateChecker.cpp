/****************************************************************************
**
** 		Created using Monkey Studio v1.8.1.0
** Authors    : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : Monkey Studio Base Plugins
** FileName  : pUpdateChecker.cpp
** Date      : 2008-01-14T00:39:52
** License   : GPL
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
#include "pUpdateChecker.h"
#include "pUpdateCheckerDialog.h"

#include <pIconManager.h>

#include <QTimer>
#include <QApplication>

pUpdateChecker::pUpdateChecker( QWidget* parent )
	: QObject( parent )
{
	mParent = parent;
	
	mAction = new QAction( this );
	mAction->setIcon( pIconManager::pixmap( "pUpdateChecker.png", ":/icons" ) );
	
	localeChanged();
	
	QApplication::instance()->installEventFilter( this );
	
	connect( mAction, SIGNAL( triggered() ), this, SLOT( checkForUpdate_triggered() ) );
}

bool pUpdateChecker::eventFilter( QObject* object, QEvent* event )
{
	switch ( event->type() ) {
		case QEvent::LocaleChange:
			localeChanged();
			break;
		default:
			break;
	}
	
	return QObject::eventFilter( object, event );
}

QAction* pUpdateChecker::menuAction() const
{
	return mAction;
}

QDateTime pUpdateChecker::lastUpdated() const
{
	return mLastUpdated;
}

void pUpdateChecker::setLastUpdated( const QDateTime& dateTime )
{
	mLastUpdated = dateTime;
}

QDateTime pUpdateChecker::lastChecked() const
{
	return mLastChecked;
}

void pUpdateChecker::setLastChecked( const QDateTime& dateTime )
{
	mLastChecked = dateTime;
}

QUrl pUpdateChecker::downloadsFeedUrl() const
{
	return mDownloadsFeed;
}

void pUpdateChecker::setDownloadsFeedUrl( const QUrl& url )
{
	mDownloadsFeed = url;
}

QString pUpdateChecker::version() const
{
	return mVersion;
}

void pUpdateChecker::setVersion( const QString& version )
{
	mVersion = version;
}

QString pUpdateChecker::versionString() const
{
	return mVersionString;
}

void pUpdateChecker::setVersionString( const QString& versionString )
{
	mVersionString = versionString;
}

QString pUpdateChecker::versionDiscoveryPattern() const
{
	return mVersionDiscoveryPattern;
}

void pUpdateChecker::setVersionDiscoveryPattern( const QString& pattern )
{
	mVersionDiscoveryPattern = pattern;
}

void pUpdateChecker::silentCheck()
{
	checkForUpdate_triggered( false );
}

void pUpdateChecker::localeChanged()
{
	mAction->setText( tr( "Update" ) );
	mAction->setToolTip( tr( "Check for update" ) );
}

void pUpdateChecker::checkForUpdate_triggered( bool show )
{
	pUpdateCheckerDialog* dlg = new pUpdateCheckerDialog( this, mParent.data() );
	
	if ( show ) {
		dlg->open();
	}
}
