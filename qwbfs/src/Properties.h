#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QObject>

class QSettings;

class Properties : public QObject
{
	Q_OBJECT
	
public:
	Properties( QObject* parent = 0 );
	virtual ~Properties();
	
	QString temporaryPath() const;
	
	QString cacheWorkingPath() const;
	void setCacheWorkingPath( const QString& path );
	
	int cacheDiskSize() const;
	void setCacheDiskSize( int sizeKb );
	
	int cacheMemorySize() const;
	void setCacheMemorySize( int sizeKb );
	
	bool cacheUseTemporaryPath() const;
	void setCacheUseTemporaryPath( bool useTemporary );

protected:
	QSettings* mSettings;
};

#endif // PROPERTIES_H
