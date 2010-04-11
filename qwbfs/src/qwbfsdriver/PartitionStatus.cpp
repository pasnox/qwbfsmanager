#include "PartitionStatus.h"

using namespace QWBFS::Partition;

Status::Status( const QWBFS::Partition::Handle& handle )
{
	const u32 blockCount = handle.isValid() ? wbfs_count_usedblocks( handle.ptr() ) : 0;
	size = handle.isValid() ? (double)handle.ptr()->n_wbfs_sec *handle.ptr()->wbfs_sec_sz : -1;
	used = handle.isValid() ? (double)( handle.ptr()->n_wbfs_sec -blockCount ) *handle.ptr()->wbfs_sec_sz : -1;
	free = handle.isValid() ? (double)(blockCount) *handle.ptr()->wbfs_sec_sz : -1;
}
