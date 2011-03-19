#ifndef PPARTITIONMODEL_H
#define PPARTITIONMODEL_H

#include <QAbstractTableModel>

#include "pPartition.h"

class pPartitionModel : public QAbstractTableModel
{
	Q_OBJECT
	friend class PartitionComboBox;
	
public:
	enum Column {
		Icon = 0,
		Label = pPartition::Label,
		Device = pPartition::DevicePath,
		FileSystem = pPartition::FileSystem,
		Free = pPartition::FreeSize,
		Used = pPartition::UsedSize,
		Total = pPartition::TotalSize,
		LastCheck = pPartition::LastCheck
	};
	
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
	void updatePartition( const pPartition& partition );
	void removePartition( const QString& partition );
	
	QStringList customPartitions() const;
	void addPartitions( const QStringList& partitions );
	
	void dump() const;

public slots:
	void update();

protected:
	pPartitionList mPartitions;
	mutable void* mData;
	
	virtual void platformInit();
	virtual void platformDeInit();
	virtual pPartitionList partitions() const;
};

#endif // PPARTITIONMODEL_H
