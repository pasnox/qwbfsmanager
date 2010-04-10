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
