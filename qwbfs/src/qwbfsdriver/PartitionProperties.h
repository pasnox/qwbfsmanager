#ifndef PARTITIONPROPERTIES_H
#define PARTITIONPROPERTIES_H

#include <QString>

namespace QWBFS {
namespace Partition {

struct Properties
{
	Properties();
	
	bool force;
	bool reset;
	QString disk;
	QString partition;
};

}; // Partition
}; // QWBFS

#endif // PARTITIONPROPERTIES_H
