/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : Properties.h
** Date      : 2010-06-16T14:19:29
** License   : GPL2
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
#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QObject>
#include <QNetworkProxy>
#include <QDateTime>
#include <QLocale>

class QSettings;
class UIMain;

class Properties : public QObject
{
	Q_OBJECT
	
public:
	Properties( QObject* parent = 0 );
	virtual ~Properties();
	
	QString temporaryPath() const;
	
	QString cacheWorkingPath() const;
	void setCacheWorkingPath( const QString& path );
	
	qint64 cacheDiskSize() const;
	void setCacheDiskSize( qint64 sizeByte );
	
	bool cacheUseTemporaryPath() const;
	void setCacheUseTemporaryPath( bool useTemporary );
	
	QNetworkProxy::ProxyType proxyType() const;
	void setProxyType( QNetworkProxy::ProxyType type );
	
	QString proxyServer() const;
	void setProxyServer( const QString& server );
	
	int proxyPort() const;
	void setProxyPort( int port );
	
	QString proxyLogin() const;
	void setProxyLogin( const QString& login );
	
	QString proxyPassword() const;
	void setProxyPassword( const QString& password );
	
	QDateTime updateLastUpdated() const;
	void setUpdateLastUpdated( const QDateTime& dateTime );
	
	QDateTime updateLastChecked() const;
	void setUpdateLastChecked( const QDateTime& dateTime );
	
	QStringList translationsPaths() const;
	void setTranslationsPaths( const QStringList& translationsPaths );
	
	bool localeAccepted() const;
	void setLocaleAccepted( bool accepted );
	
	QLocale locale() const;
	void setLocale( const QLocale& locale );
	
	void restoreState( UIMain* window ) const;
	void saveState( UIMain* window );
	
	QString selectedPath() const;
	void setSelectedPath( const QString& path );
	
	QString selectedPartition() const;
	void setSelectedPartition( const QString& partition );
	
	static QString decrypt( const QByteArray& data );
	static QByteArray crypt( const QString& string );

protected:
	QSettings* mSettings;
};

#endif // PROPERTIES_H
