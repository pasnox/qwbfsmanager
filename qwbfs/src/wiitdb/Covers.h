#ifndef COVERS_H
#define COVERS_H

#include <QString>
#include <QUrl>

namespace QWBFS {
namespace WiiTDB {

class Covers
{
public:
	enum Type
	{
		HQ,
		Cover,
		_3D,
		Disc,
		DiscCustom,
		Full
	};
	
	Covers( const QString& id );
	virtual ~Covers();

	Covers( const Covers& other );
	Covers& operator=( const Covers& other );
	bool operator==( const Covers& other ) const;
	bool operator!=( const Covers& other ) const;
	
	QUrl url( Type type ) const;
	static QUrl url( Type type, const QString& id );

protected:
	QString mId;
};

}; // WiiTDB
}; // QWBDFS

#endif // COVERS_H
