#ifndef PARTITIONCOMBOBOX_H
#define PARTITIONCOMBOBOX_H

#include <QComboBox>

class pPartitionModel;

class PartitionComboBox : public QComboBox
{
	Q_OBJECT
	
public:
	PartitionComboBox( QWidget* parent = 0 );
	virtual ~PartitionComboBox();
	
	static pPartitionModel* partitionModel();

protected slots:
	void addPartition();

protected:
	void paintEvent( QPaintEvent* event );
};

#endif // PARTITIONCOMBOBOX_H
