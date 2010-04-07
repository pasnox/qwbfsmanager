#ifndef DISCDELEGATE_H
#define DISCDELEGATE_H

#include <QStyledItemDelegate>

class DiscModel;

class DiscDelegate : public QStyledItemDelegate
{
	Q_OBJECT
	
public:
	DiscDelegate( DiscModel* parent = 0 );
	virtual ~DiscDelegate();
	
	virtual void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
	virtual QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const;

protected:
	DiscModel* mModel;
};

#endif // DISCDELEGATE_H
