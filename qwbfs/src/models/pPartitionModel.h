/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : Fresh Library
** FileName  : pPartitionModel.h
** Date      : 2011-02-20T00:41:35
** License   : LGPL v3
** Home Page : http://bettercodes.org/projects/fresh
** Comment   : Fresh Library is a Qt 4 extension library providing set of new core & gui classes.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Leser General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/
#ifndef PPARTITIONMODEL_H
#define PPARTITIONMODEL_H

/*!
	\file pPartitionModel.h
	\brief A model presenting the available partitions.
	\author Filipe Azevedo aka Nox P\@sNox <pasnox@gmail.com>
*/

#include <QAbstractTableModel>

#include "pPartition.h"

/*!
	\ingroup Core
	\class pPartitionModel
	\brief A model presenting the available partitions.
*/
class pPartitionModel : public QAbstractTableModel
{
	Q_OBJECT
	friend class PartitionComboBox;
	
public:
	/*!
		Create an instance of the model having \a parent as parent.
	*/
	pPartitionModel( QObject* parent = 0 );
	/*!
		Destroys the model instance.
	*/
	virtual ~pPartitionModel();
	/*!
		Reimplemented.
	*/
	virtual int columnCount( const QModelIndex& parent = QModelIndex() ) const;
	/*!
		Reimplemented.
	*/
	virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
	/*!
		Reimplemented.
	*/
	virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;
	/*!
		Reimplemented.
	*/
	virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
	/*!
		Reimplemented.
	*/
	virtual bool insertRow( int row, const QModelIndex& parent = QModelIndex() );
	/*!
		Reimplemented.
	*/
	virtual bool insertRows( int row, int count, const QModelIndex& parent = QModelIndex() );
	//virtual void	sort ( int column, Qt::SortOrder order = Qt::AscendingOrder )
	/*!
		Return the pPartition at \a index.
	*/
	pPartition partition( const QModelIndex& index ) const;
	/*!
		Return the QModelIndex of \a partition for \a column.
	*/
	QModelIndex index( const pPartition& partition, int column = 0 ) const;
	/*!
		Add \a partition if it does not already exists and is valid.
	*/
	void addPartition( const QString& partition );
	/*!
		Add \a partitions if they are not exists and are valid.
	*/
	void addPartitions( const QStringList& partitions );
	/*!
		Update \a partition.
	*/
	void updatePartition( const pPartition& partition );
	/*!
		Remove \a partition.
	*/
	void removePartition( const QString& partition );
	/*!
		Return the list of all partitions.
	*/
	pPartitionList partitions() const;
	/*!
		Return the list of custom partitions.
		\see pPartition::isCustom().
	*/
	QStringList customPartitions() const;
	
	void dump() const;

public slots:
	/*!
		Update the list of partitions.
		\note Linux and Mac OS X should not need this as they are automatically updated by listening the OS events.
	*/
	void update();

protected:
	pPartitionList mPartitions;
	mutable void* mData;
	
	virtual void platformInit();
	virtual void platformDeInit();
	virtual void platformUpdate();
};

#endif // PPARTITIONMODEL_H
