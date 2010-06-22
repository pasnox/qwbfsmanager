#ifndef PTRANSLATIONDIALOG_H
#define PTRANSLATIONDIALOG_H

#include <QDialog>
#include <QHash>

class pTranslationManager;
class QTreeWidgetItem;

namespace Ui {
	class pTranslationDialog;
};

class pTranslationDialog : public QDialog
{
	Q_OBJECT

public:
	pTranslationDialog( pTranslationManager* translationManager, QWidget* parent = 0 );
	virtual ~pTranslationDialog();
	
	virtual bool event( QEvent* event );
	
	QString selectedLocale() const;
	
	static QString getLocale( pTranslationManager* translationManager, QWidget* parent = 0 );

protected:
	Ui::pTranslationDialog* ui;
	pTranslationManager* mTranslationManager;
	QHash<QString, QTreeWidgetItem*> mRootItems;
	
	void localeChanged();
	QTreeWidgetItem* newItem( const QLocale& locale );
	QTreeWidgetItem* rootItem( const QLocale& locale );

protected slots:
	void on_tbLocate_clicked();
	void on_tbReload_clicked();
};

#endif // PTRANSLATIONDIALOG_H
