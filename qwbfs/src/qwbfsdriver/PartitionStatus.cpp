#include "PartitionStatus.h"

using namespace QWBFS::Partition;

Status::Status( const QWBFS::Partition::Handle& handle )
{
	blocks = handle.isValid() ? wbfs_count_usedblocks( handle.ptr() ) : -1;
	size = handle.isValid() ? (double)handle.ptr()->n_wbfs_sec *handle.ptr()->wbfs_sec_sz : -1;
	used = handle.isValid() ? (double)( handle.ptr()->n_wbfs_sec -blocks ) *handle.ptr()->wbfs_sec_sz : -1;
	free = handle.isValid() ? (double)(blocks) *handle.ptr()->wbfs_sec_sz : -1;
}
