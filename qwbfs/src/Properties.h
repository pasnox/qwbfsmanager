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
	
	QString temporaryDirectory() const;
	
	QString workingPath() const;
	void setCacheWorkingPath( const QString& path );
	
	int diskCacheSize() const;
	void setDiskCacheSize( int sizeKb );
	
	int memoryCacheSize() const;
	void setMemoryCacheSize( int sizeKb );
	
	bool useTemporaryCachePath() const;
	void setUseTemporaryCachePath( bool useTemporary );

protected:
	QSettings* mSettings;
};

#endif // PROPERTIES_H
