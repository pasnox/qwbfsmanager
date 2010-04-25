/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : Gauge.h
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
