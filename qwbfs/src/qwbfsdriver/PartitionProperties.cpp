#include "PartitionProperties.h"

using namespace QWBFS::Partition;

Properties::Properties( const QString& _partition )
{
	reset = false;
	partition = _partition;
}
