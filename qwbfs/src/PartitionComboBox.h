#ifndef PARTITIONCOMBOBOX_H
#define PARTITIONCOMBOBOX_H

#include <QComboBox>

class pPartitionModel;
class QToolButton;

class PartitionComboBox : public QComboBox
{
	Q_OBJECT
	
public:
	PartitionComboBox( QWidget* parent = 0 );
	virtual ~PartitionComboBox();
	
	static pPartitionModel* partitionModel();

protected slots:
	void modelChanged();
	void addPartition();

protected:
	QToolButton* mButton;
	
	virtual bool event( QEvent* event );
	virtual void paintEvent( QPaintEvent* event );
};

#endif // PARTITIONCOMBOBOX_H
