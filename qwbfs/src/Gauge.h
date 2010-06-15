/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : Gauge.h
** Date      : 2010-06-15T23:21:10
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR HARDWARE OR YOUR DATA
** BECAUSE OF IMPROPER USAGE OF THIS SOFTWARE.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef GAUGE_H
#define GAUGE_H

#include <QWidget>

class Gauge : public QWidget
{
	Q_OBJECT
	
public:
	Gauge( QWidget* parent = 0 );
	virtual ~Gauge();
	
	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;
	
	qint64 size() const;
	qint64 usedSize() const;
	qint64 freeSize() const;
	qint64 temporarySize() const;
	
	static QString fileSizeAdaptString( double nb );
	static QString fileSizeToString( double size );

public slots:
	void setSize( qint64 value );
	void setUsedSize( qint64 value );
	void setFreeSize( qint64 value );
	void setTemporarySize( qint64 value );
	
protected:
	qint64 mSize;
	qint64 mUsedSize;
	qint64 mFreeSize;
	qint64 mTemporarySize;
	
	virtual void paintEvent( QPaintEvent* event );
};

#endif // GAUGE_H
