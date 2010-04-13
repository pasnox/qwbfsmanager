#ifndef PARTITIONPROPERTIES_H
#define PARTITIONPROPERTIES_H

#include <QString>

namespace QWBFS {
namespace Partition {

struct Properties
{
	Properties( const QString& partition = QString::null );
	
	bool reset;
	QString partition;
};

}; // Partition
}; // QWBFS

#endif // PARTITIONPROPERTIES_H
