#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QListView>

#include "wiitdb/Covers.h"

class pNetworkAccessManager;

namespace QWBFS {
class Driver;
namespace Model {
struct Disc;
class DiscModel;
class DiscDelegate;
}; // Model
}; // QWBFS

class ListView : public QListView
{
	Q_OBJECT
	
public:
	ListView( QWidget* parent = 0 );
	virtual ~ListView();
	
	void initialize( QWBFS::Driver* driver, pNetworkAccessManager* manager );
	void setViewMode( QListView::ViewMode mode );
	void setViewIconType( QWBFS::WiiTDB::Covers::Type type );
	
	QWBFS::WiiTDB::Covers::Type viewIconType() const;
	QWBFS::Driver* driver() const;
	QWBFS::Model::DiscModel* model() const;

protected:
	QWBFS::WiiTDB::Covers::Type mIconType;
	QWBFS::Driver* mDriver;
	QWBFS::Model::DiscModel* mModel;
	QWBFS::Model::DiscDelegate* mDelegate;
};

#endif // LISTVIEW_H
