/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : PartitionHandle.h
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
#ifndef PARTITIONHANDLE_H
#define PARTITIONHANDLE_H

#include <QSharedData>

#include <libwbfs.h>

#include "qwbfsdriver/PartitionProperties.h"

namespace QWBFS {
namespace Partition {

namespace Internal {

class HandleData : public QSharedData
{
public:
	HandleData( const QWBFS::Partition::Properties& properties = QWBFS::Partition::Properties() );
	HandleData( const HandleData& other );
	~HandleData();
	
	QWBFS::Partition::Properties properties;
	wbfs_t* handle;
};

}; // Internal

class Handle
{
public:
	Handle( const QWBFS::Partition::Properties& properties = QWBFS::Partition::Properties() );
	Handle( const QString& partition );
	~Handle();
	
	bool isValid() const;
	wbfs_t* ptr() const;
	
	QWBFS::Partition::Properties properties() const;
	
	bool reset() const;
	QString partition() const;
	
protected:
	QSharedDataPointer<Internal::HandleData> d;
};

}; // Partition
}; // QWBFS

#endif // PARTITIONHANDLE_H
