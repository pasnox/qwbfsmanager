/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : PartitionDiscHandle.h
** Date      : 2010-04-25T13:05:33
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a crossplatform WBFS Manager developed using Qt4/C++.
** It's currently working under Unix/Linux, Mac OS X, and build under windows (but not yet working).
** 
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR WII CONSOLE OR WII PARTITION
** BECAUSE OF IMPROPER USAGE OF THIS SOFTWARE.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef PARTITIONDISCHANDLE_H
#define PARTITIONDISCHANDLE_H

#include <QSharedData>

#include "qwbfsdriver/PartitionHandle.h"

namespace QWBFS {
namespace Partition {

namespace Internal {

class DiscHandleData : public QSharedData
{
public:
	DiscHandleData( const QWBFS::Partition::Handle& handle = QWBFS::Partition::Handle(), const QString& discId = QString::null );
	DiscHandleData( const DiscHandleData& other );
	~DiscHandleData();
	
	wbfs_disc_t* handle;
	QString discId;
	int index;
};

}; // Internal

struct DiscHandle
{
	DiscHandle( const QWBFS::Partition::Handle& handle, const QString& discId );
	~DiscHandle();
	
	bool isValid() const;
	wbfs_disc_t* ptr() const;
	
	QString discId() const;
	int index() const;
	QString isoName() const;
	
protected:
	QSharedDataPointer<Internal::DiscHandleData> d;
};

}; // Partition
}; // QWBFS

#endif // PARTITIONDISCHANDLE_H
