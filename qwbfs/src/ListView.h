#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QListView>

#include "wiitdb/Covers.h"

class QHeaderView;
class pNetworkAccessManager;

namespace QWBFS {
class Driver;
namespace Model {
struct Disc;
class DiscModel;
class DiscDelegate;
}; // Model
}; // QWBFS

typedef QPair<QRect, QModelIndex> QItemViewPaintPair;
typedef QList<QItemViewPaintPair> QItemViewPaintPairs;

#define HEADER_HEIGHT 21

class ListView : public QListView
{
	Q_OBJECT
	
public:
	ListView( QWidget* parent = 0 );
	virtual ~ListView();
	
	void initialize( QWBFS::Driver* driver, pNetworkAccessManager* manager );
	void setViewMode( QListView::ViewMode mode );
	void setViewIconType( QWBFS::WiiTDB::Scan scan );
	
	QWBFS::WiiTDB::Scan viewIconType() const;
	QWBFS::Driver* driver() const;
	QWBFS::Model::DiscModel* model() const;

protected slots:
	void header_sortIndicatorChanged( int logicalIndex, Qt::SortOrder order );

protected:
	QWBFS::WiiTDB::Scan mIconType;
	QWBFS::Driver* mDriver;
	QWBFS::Model::DiscModel* mModel;
	QWBFS::Model::DiscDelegate* mDelegate;
	QHeaderView* mHeader;
	
	virtual void resizeEvent( QResizeEvent* event );
	virtual void mousePressEvent( QMouseEvent* event );
	virtual void startDrag( Qt::DropActions supportedActions );
	
	// dirty code get from QAbstractItemView for avoid drag & drop bug (no preview when dragging)
	
	QItemViewPaintPairs draggablePaintPairs( const QModelIndexList& indexes, QRect* r ) const;
	QPixmap renderToPixmap( const QModelIndexList& indexes, QRect* r ) const;
	QStyleOptionViewItemV4 viewOptionsV4() const;
	void clearOrRemove();
};

#endif // LISTVIEW_H
