#ifndef DISCDELEGATE_H
#define DISCDELEGATE_H

#include <QStyledItemDelegate>

namespace QWBFS {
namespace Model {

class DiscModel;

class DiscDelegate : public QStyledItemDelegate
{
	Q_OBJECT
	
public:
	DiscDelegate( QWBFS::Model::DiscModel* parent = 0 );
	virtual ~DiscDelegate();
	
	virtual void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
	virtual QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const;

protected:
	QWBFS::Model::DiscModel* mModel;
};

}; // Model
}; // QWBFS

#endif // DISCDELEGATE_H