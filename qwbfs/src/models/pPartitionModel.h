#ifndef PPARTITIONMODEL_H
#define PPARTITIONMODEL_H

#include <QAbstractTableModel>

#include "pPartition.h"

class pPartitionModel : public QAbstractTableModel
{
	Q_OBJECT
	friend class PartitionComboBox;
	
public:
	pPartitionModel( QObject* parent = 0 );
	virtual ~pPartitionModel();
	
	virtual int columnCount( const QModelIndex& parent = QModelIndex() ) const;
	virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
	virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;
	
	virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
	virtual bool insertRow( int row, const QModelIndex& parent = QModelIndex() );
	virtual bool insertRows( int row, int count, const QModelIndex& parent = QModelIndex() );
	//virtual void	sort ( int column, Qt::SortOrder order = Qt::AscendingOrder )
	
	pPartition partition( const QModelIndex& index ) const;
	QModelIndex index( const pPartition& partition, int column = 0 ) const;
	
	void addPartition( const QString& partition );
	void addPartitions( const QStringList& partitions );
	void updatePartition( const pPartition& partition );
	void removePartition( const QString& partition );
	pPartitionList partitions() const;
	QStringList customPartitions() const;
	
	void dump() const;

public slots:
	void update();

protected:
	pPartitionList mPartitions;
	mutable void* mData;
	
	virtual void platformInit();
	virtual void platformDeInit();
	virtual void platformUpdate();
};

#endif // PPARTITIONMODEL_H
