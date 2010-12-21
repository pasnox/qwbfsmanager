#if defined( __linux__) || defined(__APPLE__) || defined(__CYGWIN__)
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(__linux__)
#include <linux/fs.h>
#elif defined(__CYGWIN__)
#include <cygwin/fs.h>
#else
#include <sys/disk.h>
#endif
#include <fcntl.h>
#include <unistd.h>

#include "libwbfs.h"

void *wbfs_open_file_for_read(char*filename)
{
        FILE*f = fopen(filename,"r");
	if (!f)
		wbfs_fatal("unable to open file\n");
        return (void*)f;
}
void *wbfs_open_file_for_write(char*filename)
{
        FILE*f = fopen(filename,"w");
	if (!f)
		wbfs_fatal("unable to open file\n");
        return (void*)f;
}
int wbfs_read_file(void*handle, int len, void *buf)
{
        return fread(buf,len,1,(FILE*)handle);
}
int wbfs_close_file(void *handle)
{
        return fclose((FILE*)handle) == 0 ? 0 : 1;
}
int wbfs_file_reserve_space(void*handle, long long size)
{
        FILE*f=(FILE*)handle;
		int result;
        fseeko(f, size-1ULL, SEEK_SET);
        result = fwrite("", 1, 1, f);
		return result == 1 ? 0 : 1;
}
void wbfs_file_truncate(void *handle,long long size)
{
        ftruncate(fileno((FILE*)handle),size);
}
int wbfs_read_wii_file(void*handle,u32 offset,u32 count,void*iobuf)
{
	FILE*fp =handle;
	u64 off = offset;
	off<<=2;

	if (fseeko(fp, off, SEEK_SET))
	{
		wbfs_error("error seeking in disc file (%ld)",off);
                return 1;
        }
        if (fread(iobuf, count, 1, fp) != 1){
                wbfs_error("error reading disc 2");
                return 1;
	}
	return 0;
}

int wbfs_write_wii_file(void*handle,u32 lba,u32 count,void*iobuf)
{
	FILE*fp=handle;
	u64 off = lba;
	off *=0x8000;

	if (fseeko(fp, off, SEEK_SET))
        {
		wbfs_error("error seeking in disc file (%lld)",off);
                return 1;
        }
        if (fwrite(iobuf, count*0x8000, 1, fp) != 1){
                wbfs_error("error writing disc file");
                return 1;
        }
        return 0;
}

static int wbfs_fread_sector(void *_fp,u32 lba,u32 count,void*buf)
{
	FILE*fp =_fp;
	u64 off = lba;
	off*=512ULL;
	if (fseeko(fp, off, SEEK_SET))
	{
		fprintf(stderr,"\n\n%lld %d %p\n",off,count,_fp);
		wbfs_error("error seeking in disc partition");
		return 1;
	}
	if (fread(buf, count*512ULL, 1, fp) != 1){
		wbfs_error("error reading disc 1");
		return 1;
	}
	return 0;
  
}
static int wbfs_fwrite_sector(void *_fp,u32 lba,u32 count,void*buf)
{
	FILE*fp =_fp;
	u64 off = lba;
	off*=512ULL;
	if (fseeko(fp, off, SEEK_SET))
	{
		wbfs_error("error seeking in disc file");
		return 1;
	}
	if (fwrite(buf, count*512ULL, 1, fp) != 1){
		wbfs_error("error writing disc");
		return 1;
	}
	return 0;
  
}
static int get_capacity(char *file,u32 *sector_size,u32 *n_sector)
{
	int fd = open(file,O_RDONLY);
	int ret;
	if(fd<0){
		return 0;
	}
#if defined(__linux__) || defined(__CYGWIN__)
        if (sizeof(void *) == 8) {
          unsigned long long sec_size;
          ret = ioctl(fd,BLKSSZGET,&sec_size);
          *sector_size = (unsigned int) sec_size;
        } else
          ret = ioctl(fd,BLKSSZGET,sector_size);
#else //__APPLE__
	ret = ioctl(fd,DKIOCGETBLOCKSIZE,sector_size);
#endif
	if(ret<0)
	{
		FILE *f;
		close(fd);
		f = fopen(file,"r");
		fseeko(f,0,SEEK_END);
		*n_sector = ftello(f)/512;
		*sector_size = 512;
		fclose(f);
		return 1;
	}
#if defined(__linux__) || defined(__CYGWIN__)
        if (sizeof(void *) == 8) {
          unsigned long long n_sec;
          ret = ioctl(fd,BLKGETSIZE,&n_sec);
          *n_sector = (unsigned int) n_sec;
        } else
          ret = ioctl(fd,BLKGETSIZE,n_sector);
#else //__APPLE__
	long long my_n_sector;
	ret = ioctl(fd,DKIOCGETBLOCKCOUNT,&my_n_sector);
	*n_sector = (long)my_n_sector;
#endif
	close(fd);
	if(*sector_size>512)
		*n_sector*=*sector_size/512;
	if(*sector_size<512)
		*n_sector/=512/ *sector_size;
	return 1;
}
wbfs_t *wbfs_try_open_hd(char *fn,int reset)
{
	u32 sector_size, n_sector;
	if(!get_capacity(fn,&sector_size,&n_sector))
		return NULL;
	FILE *f = fopen(fn,"r+");
	if (!f)
		return NULL;
	return wbfs_open_hd(wbfs_fread_sector,wbfs_fwrite_sector,f,
			    sector_size ,n_sector,reset);
}
wbfs_t *wbfs_try_open_partition(char *fn,int reset)
{
	u32 sector_size, n_sector;
	if(!get_capacity(fn,&sector_size,&n_sector))
		return NULL;
	FILE *f = fopen(fn,"r+");
	if (!f)
		return NULL;
	return wbfs_open_partition(wbfs_fread_sector,wbfs_fwrite_sector,f,
				   sector_size ,n_sector,0,reset);
}
wbfs_t *wbfs_try_open(char *disc,char *partition, int reset)
{
	wbfs_t *p = 0;
	if(partition)
		p = wbfs_try_open_partition(partition,reset);
	if (!p && !reset && disc)
		p = wbfs_try_open_hd(disc,0);
	else if(!p && !reset){
		char buffer[32];
		int i;
#if defined(__linux__) || defined(__CYGWIN__)
		for (i='b';i<'z';i++)
		{
			snprintf(buffer,32,"/dev/sd%c",i);
			p = wbfs_try_open_hd(buffer,0);
			if (p)
			{
				fprintf(stderr,"using %s\n",buffer);
				return p;
			}
			snprintf(buffer,32,"/dev/hd%c",i);
			p = wbfs_try_open_hd(buffer,0);
			if (p)
			{
				fprintf(stderr,"using %s\n",buffer);
				return p;
			}
		}			 
#else
		int j;
		for (i=0;i<10;i++)
			for (j=0;j<10;j++){
				snprintf(buffer,32,"/dev/disk%ds%d",i,j);
				p = wbfs_try_open_partition(buffer,0);
				if (p)
				{
					fprintf(stderr,"using %s\n",buffer);
					return p;
				}	    
			}
#endif
		wbfs_error("cannot find any wbfs partition (verify permissions))");
	}
	return p;
	
}

#endif //__linux__ or __APPLE__
