#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QObject>

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
	void setCacheDiskSize( qint64 sizeKb );
	
	qint64 cacheMemorySize() const;
	void setCacheMemorySize( qint64 sizeKb );
	
	bool cacheUseTemporaryPath() const;
	void setCacheUseTemporaryPath( bool useTemporary );
	
	void restoreState( UIMain* window ) const;
	void saveState( UIMain* window );

protected:
	QSettings* mSettings;
};

#endif // PROPERTIES_H
