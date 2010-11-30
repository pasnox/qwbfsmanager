/****************************************************************************
**
** 		Created using Monkey Studio v1.8.1.0
** Authors    : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : Monkey Studio Base Plugins
** FileName  : pUpdateChecker.h
** Date      : 2008-01-14T00:39:52
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
#ifndef PUPDATECHECKER_H
#define PUPDATECHECKER_H

#include <QObject>
#include <QWeakPointer>
#include <QDateTime>
#include <QUrl>

class QAction;

class pUpdateChecker : public QObject
{
	Q_OBJECT

public:
	pUpdateChecker( QWidget* parent = 0 );
	
	QAction* menuAction() const;
	QDateTime lastUpdated() const;
	QDateTime lastChecked() const;
	QUrl downloadsFeedUrl() const;
	QString version() const;
	QString versionString() const;
	QString versionDiscoveryPattern() const;

protected:
	QAction* mAction;
	QDateTime mLastUpdated;
	QDateTime mLastChecked;
	QUrl mDownloadsFeed;
	QString mVersion;
	QString mVersionString;
	QString mVersionDiscoveryPattern;
	QWeakPointer<QWidget> mParent;
	
	virtual bool eventFilter( QObject* object, QEvent* event );
	void localeChanged();

public slots:
	void setLastUpdated( const QDateTime& dateTime );
	void setLastChecked( const QDateTime& dateTime );
	void setDownloadsFeedUrl( const QUrl& url );
	void setVersion( const QString& version );
	void setVersionString( const QString& versionString );
	void setVersionDiscoveryPattern( const QString& pattern );
	void silentCheck();
	
protected slots:
	void checkForUpdate_triggered( bool show = true );
};

#endif // PUPDATECHECKER_H