/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : DiscModel.h
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
#ifndef DISCMODEL_H
#define DISCMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QItemSelection>

#include "Disc.h"

namespace QWBFS {
class Driver;
namespace Model {

class DiscModel : public QAbstractItemModel
{
	Q_OBJECT
	
public:
	typedef QPair<int, int> PairIntInt;
	
	DiscModel( QObject* parent = 0, QWBFS::Driver* driver = 0 );
	virtual ~DiscModel();
	
	virtual int columnCount( const QModelIndex& parent = QModelIndex() ) const;
	virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
	virtual QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
	virtual QModelIndex parent( const QModelIndex& index ) const;
	virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;
	
	virtual bool hasChildren( const QModelIndex& parent = QModelIndex() ) const;
	virtual Qt::ItemFlags flags( const QModelIndex& index ) const;
	virtual bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );
	virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
	
	virtual bool dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent );
	virtual QMimeData* mimeData( const QModelIndexList& indexes ) const;
	virtual QStringList mimeTypes() const;
	
	void insertDiscs( int index, const QWBFS::Model::DiscList& discs );
	void addDiscs( const QWBFS::Model::DiscList& discs );
	void setDiscs( const QWBFS::Model::DiscList& discs );
	void setDisc( const QModelIndex& index, const QWBFS::Model::Disc& disc );
	QWBFS::Model::DiscList discs() const;
	QWBFS::Model::DiscList discs( const QModelIndexList& indexes );
	QWBFS::Model::DiscList discs( const QItemSelection& selection );
	QWBFS::Model::Disc disc( const QModelIndex& index ) const;
	QModelIndex index( const QWBFS::Model::Disc& disc ) const;
	QString discId( const QModelIndex& index ) const;
	void removeSelection( const QItemSelection& selection );
	void updateDisc( const QWBFS::Model::Disc& disc );
	qint64 size() const;

public slots:
	void clear();

protected:
	Driver* mDriver;
	QWBFS::Model::DiscList mDiscs;
	
	static QStringList mMimeTypes;

signals:
	void countChanged( int count );
};

struct SelectionRangePairLessThanSorter
{
	bool operator()( const DiscModel::PairIntInt& left, const DiscModel::PairIntInt& right ) const;
};

struct SelectionRangePairGreaterThanSorter
{
	bool operator()( const DiscModel::PairIntInt& left, const DiscModel::PairIntInt& right ) const;
};

}; // Models
}; // QWBFS

#endif // DISCMODEL_H
