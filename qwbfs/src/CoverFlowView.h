#ifndef COVERFLOWVIEW_H
#define COVERFLOWVIEW_H

#include "ofi-labs-pictureflow/pictureflow-qt/pictureflow.h"

#include <QPersistentModelIndex>

class QScrollBar;
class QAbstractItemModel;
class QModelIndex;
class QTimer;

class CoverFlowView : public PictureFlow
{
	Q_OBJECT
	
public:
	CoverFlowView( QWidget* parent = 0 );
	virtual ~CoverFlowView();
	
	void setModel( QAbstractItemModel* model );
	QAbstractItemModel* model() const;
	
	void setColumn( int column );
	int column() const;
	
	void setDisplayTextColumn( int column );
	int displayTextColumn() const;

protected:
	QScrollBar* mScrollBar;
	QAbstractItemModel* mModel;
	QPersistentModelIndex mRootIndex;
	int mColumn;
	int mDisplayTextColumn;
	QTimer* mTimer;
	
	virtual bool event( QEvent* event );
	virtual void resizeEvent( QResizeEvent* event );
	virtual void paintEvent( QPaintEvent* event );
	
	QModelIndex modelIndex( int index, int column = -1 ) const;
	virtual QPixmap pixmap( int index ) const;
	
protected slots:
	void _q_dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight );
	void _q_layoutChanged();
	void _q_modelReset();
	void _q_rowsInserted( const QModelIndex& parent, int start, int end );
	void _q_rowsMoved( const QModelIndex& sourceParent, int sourceStart, int sourceEnd, const QModelIndex& destinationParent, int destinationRow );
	void _q_rowsRemoved( const QModelIndex& parent, int start, int end );
	
	void delayedResizeEvent();
	void initialize( int index = 0 );
	void updateScrollBarState();
	void preload();

signals:
	void centerIndexChanged( const QModelIndex& index );
};

#endif // COVERFLOWVIEW_H
