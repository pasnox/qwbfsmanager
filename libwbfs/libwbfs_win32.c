#ifdef WIN32

#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <winioctl.h>

#include "libwbfs.h"

WCHAR* char2WCHAR( const char* in )
{
	const int length = strlen( in ) +1;
	const int needed = mbstowcs( 0, in, length );
	wchar_t out[ needed +1 ];
	mbstowcs( out, in, length );
	return out;
}

void *wbfs_open_file_for_read(char*filename)
{
	HANDLE *handle = CreateFile(char2WCHAR( filename ), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (handle == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "unable to open disc file\n");
		return 0;
	}
	return (void*)handle;
}

void *wbfs_open_file_for_write(char*filename)
{
	HANDLE *handle = CreateFile(char2WCHAR( filename ), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "unable to open file\n");
		return 0;
	}
	return (void*)handle;
}

int wbfs_read_file(void*handle, int len, void *buf)
{
	DWORD read;
	ReadFile((HANDLE)handle, buf, len, &read, NULL);
	return read;
}

void wbfs_close_file(void *handle)
{
	CloseHandle((HANDLE)handle);
}

void wbfs_file_reserve_space(void*handle,long long size)
{
	LARGE_INTEGER large;
	large.QuadPart = size;
	SetFilePointerEx((HANDLE)handle, large, NULL, FILE_BEGIN);
	SetEndOfFile((HANDLE)handle);
}

int wbfs_read_wii_file(void *_handle, u32 _offset, u32 count, void *buf)
{
	HANDLE *handle = (HANDLE *)_handle;
	LARGE_INTEGER large;
	DWORD read;
	u64 offset = _offset;
	
	offset <<= 2;
	large.QuadPart = offset;
	
	if (SetFilePointerEx(handle, large, NULL, FILE_BEGIN) == FALSE)
	{
		wbfs_error("error seeking in disc file");
		return 1;
	}
	
	read = 0;
	if ((ReadFile(handle, buf, count, &read, NULL) == FALSE) || !read)
	{
		wbfs_error("error reading wii disc sector");
		return 1;
	}

	if (read < count)
	{
		wbfs_warning("warning: requested %d, but read only %d bytes (trimmed or bad padded ISO)", count, read);
		wbfs_memset((u8*)buf+read, 0, count-read);
	}

	return 0;
}

int wbfs_write_wii_file(void *_handle, u32 lba, u32 count, void *buf)
{
	HANDLE *handle = (HANDLE *)_handle;
	LARGE_INTEGER large;
	DWORD written;
	u64 offset = lba;
	
	offset *= 0x8000;
	large.QuadPart = offset;
	
	if (SetFilePointerEx(handle, large, NULL, FILE_BEGIN) == FALSE)
	{
		fprintf(stderr,"\n\n%lld %p\n", offset, handle);
		wbfs_error("error seeking in wii disc sector (write)");
		return 1;
	}
	
	written = 0;
	if (WriteFile(handle, buf, count * 0x8000, &written, NULL) == FALSE)
	{
		wbfs_error("error writing wii disc sector");
		return 1;
	}

	if (written != count * 0x8000)
	{
		wbfs_error("error writing wii disc sector (size mismatch)");
		return 1;
	}
	
	return 0;
}

static int read_sector(void *_handle, u32 lba, u32 count, void *buf)
{
	HANDLE *handle = (HANDLE *)_handle;
	LARGE_INTEGER large;
	DWORD read;
	u64 offset = lba;
	
	offset *= 512ULL;
	large.QuadPart = offset;

	if (SetFilePointerEx(handle, large, NULL, FILE_BEGIN) == FALSE)
	{
		fprintf(stderr, "\n\n%lld %d %p\n", offset, count, _handle);
		wbfs_error("error seeking in hd sector (read)");
		return 1;
	}
	
	read = 0;
	if (ReadFile(handle, buf, count * 512ULL, &read, NULL) == FALSE)
	{
		wbfs_error("error reading hd sector");
		return 1;
	}
	
	return 0;
}

static int write_sector(void *_handle, u32 lba, u32 count, void *buf)
{
	HANDLE *handle = (HANDLE *)_handle;
	LARGE_INTEGER large;
	DWORD written;
	u64 offset = lba;

	offset *= 512ULL;
	large.QuadPart = offset;

	if (SetFilePointerEx(handle, large, NULL, FILE_BEGIN) == FALSE)
	{
		wbfs_error("error seeking in hd sector (write)");
		return 1;
	}
	
	written = 0;
	if (WriteFile(handle, buf, count * 512ULL, &written, NULL) == FALSE)
	{
		wbfs_error("error writing hd sector");
		return 1;
	}
	
	return 0;
  
}

static void close_handle(void *handle)
{
	CloseHandle((HANDLE *)handle);
}

static int get_capacity(char *fileName, u32 *sector_size, u32 *sector_count)
{
	DISK_GEOMETRY dg;
	PARTITION_INFORMATION pi;

	DWORD bytes;
	HANDLE *handle = CreateFile(char2WCHAR( fileName ), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);

	if (handle == INVALID_HANDLE_VALUE)
	{
		wbfs_error("could not open drive");
		return 0;
	}
	
	if (DeviceIoControl(handle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &dg, sizeof(DISK_GEOMETRY), &bytes, NULL) == FALSE)
	{
		CloseHandle(handle);
		wbfs_error("could not get drive geometry");
		return 0;
	}

	*sector_size = dg.BytesPerSector;

	if (DeviceIoControl(handle, IOCTL_DISK_GET_PARTITION_INFO, NULL, 0, &pi, sizeof(PARTITION_INFORMATION), &bytes, NULL) == FALSE)
	{
		CloseHandle(handle);
		wbfs_error("could not get partition info");
		return 0;
	}

	*sector_count = (u32)(pi.PartitionLength.QuadPart / dg.BytesPerSector);
	
	CloseHandle(handle);
	return 1;
}

wbfs_t *wbfs_try_open_hd(char *driveName, int reset)
{
	wbfs_error("no direct harddrive support");
	return 0;
}

wbfs_t *wbfs_try_open_partition(char *partitionLetter, int reset)
{
	HANDLE *handle;
	char drivePath[8] = "\\\\?\\Z:";
	
	u32 sector_size, sector_count;
	
	if (strlen(partitionLetter) != 1)
	{
		wbfs_error("bad drive name");
		return NULL;
	}

	drivePath[4] = partitionLetter[0];
	
	if (!get_capacity(drivePath, &sector_size, &sector_count))
	{
		return NULL;
	}
	
	handle = CreateFile(char2WCHAR( drivePath ), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	
	if (handle == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}
	
	return wbfs_open_partition(read_sector, write_sector, close_handle, handle, sector_size, sector_count, 0, reset);
}

wbfs_t *wbfs_try_open(char *disc, char *partition, int reset)
{
	wbfs_t *p = 0;
	
	if (partition)
	{
		p = wbfs_try_open_partition(partition,reset);
	}
	
	if (!p && !reset && disc)
	{
		p = 0;
	}
	else if(!p && !reset)
	{
		p = 0;
	}

	return p;
}

#endif

