/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : ListView.h
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
	pNetworkAccessManager* cacheManager() const;

protected slots:
	void header_sortIndicatorChanged( int logicalIndex, Qt::SortOrder order );

protected:
	QWBFS::WiiTDB::Scan mIconType;
	QWBFS::Driver* mDriver;
	QWBFS::Model::DiscModel* mModel;
	QWBFS::Model::DiscDelegate* mDelegate;
	pNetworkAccessManager* mCacheManager;
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
