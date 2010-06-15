/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : PartitionDiscHandle.h
** Date      : 2010-06-15T23:21:10
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR HARDWARE OR YOUR DATA
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
