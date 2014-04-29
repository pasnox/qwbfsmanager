
/***************************************************************************
 *                    __            __ _ ___________                       *
 *                    \ \          / /| |____   ____|                      *
 *                     \ \        / / | |    | |                           *
 *                      \ \  /\  / /  | |    | |                           *
 *                       \ \/  \/ /   | |    | |                           *
 *                        \  /\  /    | |    | |                           *
 *                         \/  \/     |_|    |_|                           *
 *                                                                         *
 *                           Wiimms ISO Tools                              *
 *                         http://wit.wiimm.de/                            *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *   This file is part of the WIT project.                                 *
 *   Visit http://wit.wiimm.de/ for project details and sources.           *
 *                                                                         *
 *   Copyright (c) 2009-2012 by Dirk Clemens <wiimm@wiimm.de>              *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   See file gpl-2.0.txt or http://www.gnu.org/licenses/gpl-2.0.txt       *
 *                                                                         *
 ***************************************************************************/

#define _GNU_SOURCE 1
#define _XOPEN_SOURCE 1

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <errno.h>
#include <utime.h>
#include <ctype.h>
#include <dirent.h>

#if defined(__CYGWIN__)
  #include <cygwin/fs.h>
  #include <io.h>
#elif defined(__APPLE__)
  #include <sys/disk.h>
#elif defined(__linux__)
  #include <linux/fs.h>
#endif

#include "lib-std.h"
#include "lib-sf.h"
#include "wbfs-interface.h"

///////////////////////////////////////////////////////////////////////////////

#if defined(HAVE_FIEMAP) && defined(FS_IOC_FIEMAP)
  #undef HAVE_FIEMAP
  #define HAVE_FIEMAP 1
  #include <linux/fiemap.h>
#else
  #undef HAVE_FIEMAP
  #define HAVE_FIEMAP 0
#endif

#undef HAVE_FIBMAP
#if defined(FIGETBSZ) && defined(FIGETBSZ)
  #define HAVE_FIBMAP 1
#else
  #define HAVE_FIBMAP 0
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////                   file support                  ///////////////
///////////////////////////////////////////////////////////////////////////////

int opt_direct = 0;
enumIOMode opt_iomode = IOM__IS_DEFAULT | IOM_FORCE_STREAM;

//-----------------------------------------------------------------------------

void ScanIOMode ( ccp arg )
{
    const enumIOMode new_io = strtol(optarg,0,0); // [[2do]] error handling
    opt_iomode = new_io & IOM__IS_MASK;
    if ( verbose > 0 || opt_iomode != new_io )
	printf("IO mode set to %#0x.\n",opt_iomode);
    opt_iomode |= IOM_FORCE_STREAM;
}

///////////////////////////////////////////////////////////////////////////////

u32 GetHSS ( int fd, u32 default_value )
{
 #ifdef DKIOCGETBLOCKSIZE
    TRACE(" - try DKIOCGETBLOCKSIZE\n");
    {
	unsigned long size32 = 0;
	if ( ioctl(fd, DKIOCGETBLOCKSIZE, &size32 ) >= 0 && size32 )
	{
	    TRACE("GetHSS(%d) DKIOCGETBLOCKSIZE := %x = %u\n",fd,size32,size32);
	    return size32;
	}
    }
 #endif

 #ifdef BLKSSZGET
    TRACE(" - try BLKSSZGET\n");
    {
	unsigned long size32 = 0;
	if ( ioctl(fd, BLKSSZGET, &size32 ) >= 0 && size32 )
	{
	    TRACE("GetHSS(%d) BLKSSZGET := %lx = %lu\n",fd,size32,size32);
	    return size32;
	}
    }
 #endif

    TRACE("GetHSS(%d) default_value := %x = %u\n",fd,default_value,default_value);
    return default_value;
}

///////////////////////////////////////////////////////////////////////////////

static int valid_offset ( int fd, off_t off )
{
    noTRACE(" - valid_offset(%d,%llu=%#llx)\n",fd,(u64)off,(u64)off);
    char ch;
    if (   !off
	|| off > ~(off_t)0 / 2
	|| lseek(fd,off,SEEK_SET) == (off_t)-1 )
	return 0;
    return read(fd,&ch,1) == 1;
}

//-----------------------------------------------------------------------------

off_t GetBlockDevSize ( int fd )
{
    TRACE("GetBlockDevSize(%d)\n",fd);

 #ifdef BLKGETSIZE64
    TRACE(" - try BLKGETSIZE64\n");
    {
	unsigned long long size64 = 0;
	if ( ioctl(fd, BLKGETSIZE64, &size64 ) >= 0 && size64 )
	    return size64;
    }
 #endif

 #ifdef DKIOCGETBLOCKCOUNT
    TRACE(" - try DKIOCGETBLOCKCOUNT\n");
    {
	unsigned long long size64 = 0;
	if ( ioctl(fd, DKIOCGETBLOCKCOUNT, &size64 ) >= 0  && size64 )
	    return size64 * GetHSS(fd,HD_SECTOR_SIZE);
    }
 #endif

 #ifdef BLKGETSIZE
    TRACE(" - try BLKGETSIZE\n");
    {
	unsigned long size32 = 0;
	if ( ioctl(fd, BLKGETSIZE, &size32 ) >= 0 && size32 )
	    return (off_t)size32 * GetHSS(fd,HD_SECTOR_SIZE);
    }
 #endif

    TRACE(" - try lseek(SEEK_END)\n");
    {
	off_t off = 0;
	off = lseek(fd,0,SEEK_END);
	if ( off && off != (off_t)-1 )
	{
	    lseek(fd,0,SEEK_SET);
	    return off;
	}
    }

    TRACE(" - try binary search\n");
    off_t low, high;
    for ( low = 0, high = GiB; high > low && valid_offset(fd,high); high *= 2 )
	low = high;
    TRACE(" - low=%llx, high=%llx\n",(u64)low,(u64)high);

    while ( low < high - 1 )
    {
	const off_t mid = (low + high) / 2;
	if (valid_offset(fd,mid))
	    low = mid;
	else
	    high = mid;
    }
    lseek(fd,0,SEEK_SET);
    return low ? low + 1 : 0;
}

///////////////////////////////////////////////////////////////////////////////

enumError StatFile ( struct stat * st, ccp fname, int fd )
{
    ASSERT(st);

    TRACE("StatFile(%s,%d)\n",fname?fname:"",fd);

    // normalize parameters
    if ( fname && *fname )
	fd = -1;
    else
	fname = 0;

    if ( fname ? stat(fname,st) : fstat(fd,st) )
    {
	memset(st,0,sizeof(*st));
	return ERR_WARNING;
    }

    TRACE(" - st_dev:     %llu\n",(u64)st->st_dev);
    TRACE(" - st_ino:     %llu\n",(u64)st->st_ino);
    TRACE(" - st_mode:    %llx [REG=%u,DIR=%d,CHR=%d,BLK=%d,FIFO=%d,LNK=%d,SOCK=%d]\n",
			(u64)st->st_mode, S_ISREG(st->st_mode),
			S_ISDIR(st->st_mode), S_ISCHR(st->st_mode),
			S_ISBLK(st->st_mode), S_ISFIFO(st->st_mode),
			S_ISLNK(st->st_mode), S_ISSOCK(st->st_mode) );
    TRACE(" - st_nlink:   %llu\n",(u64)st->st_nlink);
    TRACE(" - st_uid:     %llu\n",(u64)st->st_uid);
    TRACE(" - st_gid:     %llu\n",(u64)st->st_gid);
    TRACE(" - st_rdev:    %llu\n",(u64)st->st_rdev);
    TRACE(" - st_size:    %10llx = %llu\n",(u64)st->st_size,(u64)st->st_size);
    TRACE(" - st_blksize: %10llx = %llu\n",(u64)st->st_blksize,(u64)st->st_blksize);
    TRACE(" - st_blocks:  %10llx = %llu\n",(u64)st->st_blocks,(u64)st->st_blocks);
    TRACE(" - st_atime:   %llu\n",(u64)st->st_atime);
    TRACE(" - st_mtime:   %llu\n",(u64)st->st_mtime);
    TRACE(" - st_ctime:   %llu\n",(u64)st->st_ctime);

    if ( !st->st_size && GetFileMode(st->st_mode) != FM_OTHER )
    {
	if (fname)
	    fd = open(fname,O_RDONLY);

	if ( fd != -1 )
	{
	    if ( GetFileMode(st->st_mode) > FM_PLAIN )
	    {
		st->st_size = GetBlockDevSize(fd);
		TRACE(" + st_size:    %llu\n",(u64)st->st_size);
	    }

	    if (!st->st_size)
	    {
		st->st_size = lseek(fd,0,SEEK_END);
		TRACE(" + st_size:    %llu\n",(u64)st->st_size);
		lseek(fd,0,SEEK_SET);
	    }

	    if (fname)
		close(fd);
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// initialize, reset and close files

void InitializeFile ( File_t * f )
{
    TRACE("#F# InitializeFile(%p)\n",f);
    ASSERT(f);
    ASSERT(ERR_OK==0);

    memset(f,0,sizeof(*f));
    f->fd = -1;
    f->sector_size = 512;
    f->fname = EmptyString;
    f->split_fname_format = EmptyString;
    f->slot = -1;
    f->already_created_mode = 2;

    // normalize 'opt_iomode'
    opt_iomode = opt_iomode & IOM__IS_MASK | IOM_FORCE_STREAM;

 #ifdef __CYGWIN__
    opt_iomode |= IOM_IS_WBFS_PART;
 #endif
}

///////////////////////////////////////////////////////////////////////////////

enumError XResetFile ( XPARM File_t * f, bool remove_file )
{
    TRACE("#F# ResetFile(%p,%d)\n",f,remove_file);
    ASSERT(f);
    enumError stat = XClearFile( XCALL f, remove_file );

    // save user settungs
    const bool open_flags		= f->open_flags;
    const bool disable_errors		= f->disable_errors;
    const bool create_directory		= f->create_directory;
    const bool allow_direct_io		= f->allow_direct_io;
    const int  already_created_mode	= f->already_created_mode;

    InitializeFile(f);

    // restore user settings
    f->open_flags		= open_flags;
    f->disable_errors		= disable_errors;
    f->create_directory		= create_directory;
    f->allow_direct_io		= allow_direct_io;
    f->already_created_mode	= already_created_mode;

    return stat;
}

///////////////////////////////////////////////////////////////////////////////

enumError XClearFile ( XPARM File_t * f, bool remove_file )
{
    ASSERT(f);
    TRACE("#F# ClearFile(%p,%d) fd=%d fp=%p\n",f,remove_file,f->fd,f->fp);

    enumError err = ERR_OK;
    if (f->split_f)
    {
	File_t **end, **ptr = f->split_f;
	for ( end = ptr + f->split_used; ptr < end; ptr++ )
	{
	    enumError err1 = XClearFile( XCALL *ptr, remove_file );
	    FREE(*ptr);
	    if ( err < err1 )
		err = err1;
	}
	FREE(f->split_f);
	f->split_f = 0;
	remove_file = false;
    };
    f->split_used = 0;

    enumError err1 = XCloseFile( XCALL f, remove_file );
    if ( err < err1 )
	err = err1;

    FreeString(f->fname);		f->fname		= EmptyString;
    FreeString(f->path);		f->path			= 0;
    FreeString(f->rename);		f->rename		= 0;
    FreeString(f->outname);		f->outname		= 0;
    FreeString(f->split_fname_format);	f->split_fname_format	= EmptyString;
    FreeString(f->split_rename_format);	f->split_rename_format	= 0;

    ASSERT(!f->is_caching);
    ASSERT(!f->cache);
    ASSERT(!f->cur_cache);

    f->last_error = err;
    if ( f->max_error < err )
	f->max_error = err;
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError XCloseFile ( XPARM File_t * f, bool remove_file )
{
    DASSERT(f);
    TRACE("#F# CloseFile(%p,%d) fd=%d fp=%p\n",f,remove_file,f->fd,f->fp);

    //----- pralloc support

    if ( !remove_file && f->prealloc_size > f->max_off && IsOpenF(f) )
    {
	PRINT("PREALLOC/CLOSE: pre=%llx, max=%llx   \n",
		(u64)f->prealloc_size, (u64)f->max_off );
	XSetSizeF(XCALL f,f->max_off);
    }


    //----- close file

    bool close_err = false;
    if ( f->fp )
	close_err = fclose(f->fp) != 0;
    else if ( f->fd != -1 )
	close_err = close(f->fd) != 0;

    f->fp =  0;
    f->fd = -1;

    enumError err = ERR_OK;
    if (close_err)
    {
	err = f->is_writing ? ERR_WRITE_FAILED : ERR_READ_FAILED;
	if (!f->disable_errors)
	    PrintError( XERROR1, err,
		"Close file failed: %s\n", f->fname );
    }


    //----- split file support & removing & renaming

    if (f->split_f)
    {
	File_t **end, **ptr = f->split_f;
	for ( end = ptr + f->split_used; ptr < end; ptr++ )
	{
	    TRACE("#S#%zd# close %s\n",ptr-f->split_f,(*ptr)->fname);
	    enumError err1 = XCloseFile( XCALL *ptr, remove_file );
	    if ( err < err1 )
		err = err1;
	}
    }
    else if ( !f->is_stdfile && S_ISREG(f->st.st_mode) )
    {
	ccp * path = f->path ? &f->path : &f->fname;
	if ( remove_file && *path && **path )
	{
	    TRACE("REMOVE: %s\n",*path);
	    unlink(*path);
	    FreeString(f->rename);
	    f->rename = 0;
	}
	else if (f->rename)
	{
	    TRACE("RENAME: %s\n",*path);
	    TRACE("    TO: %s\n",f->rename);
	    if (rename(*path,f->rename))
	    {
		if (!f->disable_errors)
		    ERROR1( ERR_CANT_CREATE,
			"Can't create file: %s\n", f->rename );
		if ( err < ERR_CANT_CREATE )
		    err = ERR_CANT_CREATE;
		unlink(*path);
	    }

	    FreeString(*path);
	    *path = f->rename;
	    f->rename = 0;
	}
    }


    //----- clean

    ClearCache(f);
    ResetMemMap(&f->prealloc_map);

    f->cur_off = f->file_off = 0;

    f->last_error = err;
    if ( f->max_error < err )
	f->max_error = err;

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError XSetFileTime ( XPARM File_t * f, FileAttrib_t * set_time )
{
    // try to change time and ignore error messages

    ASSERT(f);

    enumError err = ERR_OK;
    if ( set_time && set_time->mtime )
    {
	err = XCloseFile( XCALL f, false );

	struct timeval tval[2];
	tval[0].tv_sec = set_time->atime ? set_time->atime : set_time->mtime;
	tval[1].tv_sec = set_time->mtime;
	tval[0].tv_usec = tval[1].tv_usec = 0;

	if (f->split_f)
	{
	    File_t **end, **ptr = f->split_f;
	    for ( end = ptr + f->split_used; ptr < end; ptr++ )
	    {
		TRACE("XSetFileTime(%p,%p) fname=%s\n",f,set_time,(*ptr)->fname);
		utimes((*ptr)->fname,tval);
	    }
	}
	else
	{
	    TRACE("XSetFileTime(%p,%p) fname=%s\n",f,set_time,f->fname);
	    utimes(f->fname,tval);
	}
    }
    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError XOpenFileHelper
	( XPARM File_t * f, enumIOMode iomode, int default_flags, int force_flags )
{
    ASSERT(f);
    TRACE("#F# OpenFileHelper(%p,%x,%x,%x) dis=%d, mkdir=%d, fname=%s\n",
		f, iomode, default_flags, force_flags,
		f->disable_errors, f->create_directory, f->fname );

 #ifdef O_LARGEFILE
    TRACE("FORCE O_LARGEFILE\n");
    force_flags |= O_LARGEFILE;
 #endif

 #ifdef O_DIRECT
    TRACE("FORCE O_DIRECT = %d,%d\n",opt_direct,f->allow_direct_io);
    if ( opt_direct && f->allow_direct_io )
	force_flags |= O_DIRECT;
 #endif

    f->active_open_flags = ( f->open_flags ? f->open_flags : default_flags )
			 | force_flags;

    const int mode_mask = O_RDONLY|O_WRONLY|O_RDWR;
    int mode = f->active_open_flags & mode_mask;
    if ( !mode || mode == O_RDONLY )
    {
	mode = O_RDONLY;
	f->is_reading = true;
    }
    else if ( mode == O_WRONLY )
    {
	f->is_writing = true;
    }
    else
    {
	mode = O_RDWR;
	f->is_reading = true;
	f->is_writing = true;
    }
    f->active_open_flags = f->active_open_flags & ~mode_mask | mode;

    if ( f->fd == -1 )
    {
	TRACE("open %s, create-dir=%d\n",f->fname,f->create_directory);
     #ifdef __CYGWIN__
	char * temp = AllocNormalizedFilenameCygwin(f->fname);
	TRACE("open %p %p %s\n",f->fname,temp,temp);
	//FreeString(f->fname); // [[2do]] [memleak] -> forces a core dump -- mhhm
	f->fname = temp;
     #endif
	f->fd = open( f->fname, f->active_open_flags, 0666 );
	if ( f->fd == -1 && f->create_directory && f->is_writing )
	{
	    const enumError err = CreatePath(f->fname);
	    if (err)
		return err;
	    f->fd = open( f->fname, f->active_open_flags, 0666 );
	}
    }
    TRACE("#F# OpenFile '%s' fd=%d, dis=%d\n", f->fname, f->fd, f->disable_errors );

    if ( f->fd == -1 )
    {
	if ( f->active_open_flags & O_CREAT )
	{
	    f->max_error = f->last_error = ERR_CANT_CREATE;
	    if (!f->disable_errors)
		PrintError( XERROR1, f->last_error,
				"Can't create file: %s\n",f->fname);
	}
	else
	{
	    f->max_error = f->last_error = ERR_CANT_OPEN;
	    if (!f->disable_errors)
		PrintError( XERROR1, f->last_error,
				"Can't open file: %s\n",f->fname);
	}
    }
    else if (StatFile(&f->st,0,f->fd))
    {
	close(f->fd);
	f->fd = -1;
	f->max_error = f->last_error = ERR_READ_FAILED;
	if (!f->disable_errors)
	    PrintError( XERROR1, f->last_error,"Can't stat file: %s\n",f->fname);
    }
    else
    {
	CopyFileAttribStat(&f->fatt,&f->st,false);
	f->seek_allowed =  GetFileMode(f->st.st_mode) != FM_OTHER
			&& lseek(f->fd,0,SEEK_SET) != (off_t)-1;
	if ( iomode & opt_iomode || S_ISCHR(f->st.st_mode) )
	    XOpenStreamFile(XCALL f);
    }

    TRACE("#F# OpenFileHelper(%p) returns %d, fd=%d, fp=%p, seek-allowed=%d, rw=%d,%d\n",
		f, f->last_error, GetFD(f), GetFP(f), f->seek_allowed,
		f->is_reading, f->is_writing );
    return f->last_error;
}

///////////////////////////////////////////////////////////////////////////////

enumError XOpenFile ( XPARM File_t * f, ccp fname, enumIOMode iomode )
{
    ASSERT(f);
    TRACE("XOpenFile(%s)\n",fname);

    const bool no_fname = !fname;
    if (no_fname)
    {
	fname = f->fname;
	f->fname = 0;
    }

    XResetFile( XCALL f, false );

    f->is_stdfile = fname[0] == '-' && !fname[1];
    if (f->is_stdfile)
    {
	f->fd    = dup(fileno(stdin));
	f->fname = STRDUP("- (stdin)");
    }
    else
	f->fname = no_fname ? fname : STRDUP(fname);

    return XOpenFileHelper(XCALL f,iomode,O_RDONLY,O_RDONLY);
}

///////////////////////////////////////////////////////////////////////////////

enumError XOpenFileModify ( XPARM File_t * f, ccp fname, enumIOMode iomode )
{
    ASSERT(f);

    const bool no_fname = !fname;
    if (no_fname)
    {
	fname = f->fname;
	f->fname = 0;
    }

    XResetFile( XCALL f, false );

    f->fname = no_fname ? fname : STRDUP(fname);
    return XOpenFileHelper(XCALL f,iomode,O_RDWR,O_RDWR);
}

///////////////////////////////////////////////////////////////////////////////

enumError XCheckCreated
	( XPARM ccp fname, bool disable_errors, enumError err_code )
{
    if (!InsertStringField(&created_files,fname,false))
    {
	if (!disable_errors)
	    PrintError( XERROR0, err_code,
		"File already created: %s\n", fname );
	return err_code;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError XCreateFile
	( XPARM File_t * f, ccp fname, enumIOMode iomode, int overwrite )
{
    ASSERT(f);

    const int open_flags  = O_CREAT|O_WRONLY|O_TRUNC|O_EXCL;
    const int force_flags = O_CREAT;

    if (!fname)
    {
	fname = f->fname;
	f->fname = 0;
    }

    XResetFile( XCALL f, false );

    f->is_stdfile = fname[0] == '-' && !fname[1];
    if (f->is_stdfile)
    {
	f->fd    = dup(fileno(stdout));
	f->fname = STRDUP("- (stdout)");

	return XOpenFileHelper(XCALL f, iomode, open_flags, force_flags );
    }

    if (!stat(fname,&f->st))
    {
	if ( overwrite < 0 || f->disable_errors )
	    return ERR_ALREADY_EXISTS;

	if ( S_ISBLK(f->st.st_mode) || S_ISCHR(f->st.st_mode) )
	{
	    if (!overwrite)
		return PrintError( XERROR0, ERR_ALREADY_EXISTS,
		    "Can't overwrite block device: %s\n", fname );

	    f->fname = STRDUP(fname);
	    return XOpenFileHelper(XCALL f, iomode, O_WRONLY, 0 );
	}
	else
	{
	    if (!S_ISREG(f->st.st_mode))
		return PrintError( XERROR0, ERR_WRONG_FILE_TYPE,
		    "Not a plain file: %s\n", fname );

	    if (!overwrite)
		return PrintError( XERROR0, ERR_ALREADY_EXISTS,
		    "File already exists: %s\n", fname );
	}
    }

    TRACE("#F# CREATE: '%s' mkdir=%d\n",fname,f->create_directory);

    int flen = strlen(fname);
    if ( flen >= PATH_MAX )
    {
	// no temp name possible

	f->fname = STRDUP(fname);
	return XOpenFileHelper(XCALL f, iomode, open_flags, force_flags );
    }

    const bool disable_errors = f->disable_errors;
    TRACE("dis=%d\n",disable_errors);
    f->disable_errors = true;

    char fbuf[PATH_MAX+20], *XXXXXX;
    if ( flen >= 7 && !memcmp(fname+flen-7,".XXXXXX",7) )
    {
	memcpy(fbuf,fname,flen);
	fbuf[flen] = 0;
	XXXXXX = fbuf + flen - 6;
    }
    else if ( flen >= 11 && !memcmp(fname+flen-11,".XXXXXX.tmp",11) )
    {
	memcpy(fbuf,fname,flen);
	fbuf[flen] = 0;
	XXXXXX = fbuf + flen - 10;
    }
    else
    {
	if ( f->already_created_mode > 1 )
	{
	    if (XCheckCreated(XCALL fname,disable_errors,ERR_CANT_CREATE))
		goto abort;
	}
	else if ( f->already_created_mode > 0 )
	    XCheckCreated(XCALL fname,disable_errors,ERR_WARNING);

	f->rename = STRDUP(fname);

	ccp name = strrchr(fname,'/');
	name = name ? name+1 : fname;
	memcpy(fbuf,fname,name-fname);
	char * dest = fbuf + (name-fname);
	ASSERT( dest < fbuf + PATH_MAX );

	char * dest_name = dest;
	*dest++ = '.';
	dest = StringCopyE(dest,fbuf+PATH_MAX,name);
	if ( dest - dest_name > 50 )
	     dest = dest_name + 50;
	XXXXXX = dest + 1;
	StringCopyE(dest,fbuf+sizeof(fbuf),".XXXXXX.tmp");
	TRACE("#F# TEMP:   '%s'\n",fbuf);
    }


    //---------------------------------------------------------
    // I have seen the glibc function __gen_tempname() ;)
    //---------------------------------------------------------

    struct timeval tval;
    gettimeofday(&tval,NULL);
    u64 random_time_bits = (u64) tval.tv_usec << 16 ^ tval.tv_sec;
    u64 value = random_time_bits ^ getpid();

    static char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			    "abcdefghijklmnopqrstuvwxyz"
			    "0123456789";

    int i;
    for ( i = 0; i < 100; value += 7777, i++ )
    {
	u64 v = value;
	XXXXXX[0] = letters[v%62]; v /= 62;
	XXXXXX[1] = letters[v%62]; v /= 62;
	XXXXXX[2] = letters[v%62]; v /= 62;
	XXXXXX[3] = letters[v%62]; v /= 62;
	XXXXXX[4] = letters[v%62]; v /= 62;
	XXXXXX[5] = letters[v%62];

	f->fname = fbuf; // temporary assignment
	const enumError err = XOpenFileHelper(XCALL f,iomode,open_flags,open_flags);
	if ( err == ERR_OK || err == ERR_CANT_CREATE_DIR )
	{
	    noPRINT("#F# TEMP:   '%s'\n",fbuf);
	    f->fname = STRDUP(fbuf);
	    f->disable_errors = disable_errors;
	    return f->last_error = f->max_error = err;
	}
	f->fname = 0;
	XResetFile( XCALL f, false );
    }

    // cleanup
    f->fname = EmptyString;
    memset(&f->st,0,sizeof(f->st));

    if (!disable_errors)
	PrintError( XERROR0, ERR_CANT_CREATE,
			"Can't create temp file: %s\n", fname );

 abort:
    f->max_error = f->last_error = ERR_CANT_CREATE;
    f->disable_errors = disable_errors;
    TRACE("#F# CreateFile(%p) returns %d, fd=%d, fp=%p\n",
		f, f->last_error, f->fd, f->fp );
    return f->last_error;
}

///////////////////////////////////////////////////////////////////////////////

enumError XOpenStreamFile ( XPARM File_t * f )
{
    ASSERT(f);
    TRACE("#F# OpenStreamFile(%p) fd=%d, fp=%p\n",f,f->fd,f->fp);

    f->last_error = 0;
    if ( f->fd != -1 || !f->fp )
    {
	// flag 'b' is set for compatibilitiy only, linux ignores it
	ccp mode = !f->is_writing ? "rb" : f->is_reading ? "r+b" : "wb";
	TRACE("#F# open mode: '%s'\n",mode);
	f->fp = fdopen(f->fd,mode);
	if (!f->fp)
	{
	    f->last_error = ERR_CANT_OPEN;
	    if ( f->max_error < f->last_error )
		f->max_error = f->last_error;
	    if (!f->disable_errors)
		PrintError( XERROR1,  f->last_error, "Can't open file stream: %s\n",
			f->fname );
	}
	else
	    SeekF(f,f->file_off);
    }

    noTRACE("#F# OpenStreamFile(%p) returns %d, fd=%d, fp=%p, off=%llx\n",
		f, f->last_error, GetFD(f), GetFP(f), (u64)f->file_off );
    return f->last_error;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// file name generation

char * NormalizeFileName ( char * buf, char * end, ccp source, bool allow_slash )
{
    ASSERT(buf);
    ASSERT(end);
    ASSERT( buf <= end );
    char * dest = buf;
    TRACE("NormalizeFileName(%s,%d)\n",source,allow_slash);

    if (source)
    {
     #ifdef __CYGWIN__
	if (allow_slash)
	{
	    const int drv_len = IsWindowsDriveSpec(source);
	    if (drv_len)
	    {
		dest = StringCopyE(dest,end,"/cygdrive/c/");
		if ( dest < end )
		    dest[-2] = tolower((int)*source);
		source += drv_len;
	    }
	}
     #endif

	bool skip_space = true;
	while ( *source && dest < end )
	{
	    unsigned char ch = *source++;
	    if ( ch == ':' )
	    {
		if (!skip_space)
		    *dest++ = ' ';
		if ( dest + 2 <= end )
		{
		    *dest++ = '-';
		    *dest++ = ' ';
		}
		skip_space = true;
	    }
	    else
	    {
		if ( isalnum(ch)
			|| ( use_utf8
				? ch >= 0x80
				:    ch == 0xe4 // ä
				  || ch == 0xf6 // ö
				  || ch == 0xfc // ü
				  || ch == 0xdf // ß
				  || ch == 0xc4 // A
				  || ch == 0xd6 // Ö
				  || ch == 0xdc // Ü
			    )
			|| strchr("_+-=%'\"$%&,.!()[]{}<>",ch)
			|| ch == '/' && allow_slash )
		{
		    *dest++ = ch;
		    skip_space = false;
		}
	     #ifdef __CYGWIN__
		else if ( ch == '\\' && allow_slash )
		{
		    *dest++ = '/';
		    skip_space = false;
		}
	     #endif
		else if (!skip_space)
		{
		    *dest++ = ' ';
		    skip_space = true;
		}
	    }
	}
    }
    if ( dest > buf && dest[-1] == ' ' )
	dest--;

    ASSERT( dest <= end );
    return dest;
}

///////////////////////////////////////////////////////////////////////////////

uint ReduceToPathAndType
(
    char	*buf,		// valid return buffer
    uint	buf_size,	// size of 'buf'
    ccp		fname		// source: file name
)
{
    DASSERT(buf);
    DASSERT(buf_size>1);
    DASSERT(fname);

    StringCopyS(buf,buf_size,fname);
    char *dest = strrchr(buf,'/');
    dest = dest ? dest+1 : buf;
    ccp src = strrchr(fname,'.');
    if ( src && src > fname )
    {
	*dest++ = '*';
	while ( ( *dest++ = *src++ ) != 0 )
	    ;
	DASSERT( dest < buf + buf_size );
	*dest = 0;
	return buf - dest;
    }

    *buf = 0;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void SetFileName ( File_t * f, ccp source, bool allow_slash )
{
    char fbuf[PATH_MAX];
    char * dest = NormalizeFileName(fbuf, fbuf+sizeof(fbuf)-1, source, allow_slash );
    *dest++ = 0;
    const int len = dest - fbuf;

    FreeString(f->fname);
    if ( len <= 1 )
	f->fname = EmptyString;
    else
    {
	dest = MALLOC(len);
	memcpy(dest,fbuf,len);
	f->fname = dest;
    }
}

///////////////////////////////////////////////////////////////////////////////

void GenFileName ( File_t * f, ccp path, ccp name, ccp title, ccp id6, ccp ext )
{
    ASSERT(f);

    TRACE("GenFileName(,p=%s,n=%s,i=%s,e=%s)\n",
		path ? path : "",
		name ? name : "",
		id6  ? id6  : "",
		ext  ? ext  : "" );

    FreeString(f->fname);
    f->fname = EmptyString;

    //----- check stdout

    f->is_stdfile = name && name[0] == '-' && !name[1];
    if (f->is_stdfile)
    {
	f->fname = MinusString;
	return;
    }

    //----- evaluate 'path'

    char fbuf[PATH_MAX+20];

    if ( name && *name == '/' )
	path = 0;

    int plen = path ? strlen(path) : 0;
    if ( plen > PATH_MAX )
    {
	if (!realpath(path,fbuf))
	    strcpy(fbuf,"./");
	plen = strlen(fbuf);
    }
    else if (plen)
	memcpy(fbuf,path,plen);

    char * dest = fbuf + plen;
    if ( plen && dest[-1] != '/' )
	*dest++ = '/';

    TRACE(" >PATH: |%.*s|\n",(int)(dest-fbuf),fbuf);

    //----- evaluate 'name'

    char * end  = fbuf + sizeof(fbuf) -  15; // space for "[123456].ext"

    if (name)
    {
	// just copy it
	dest = StringCopyE(dest,end,name);
	TRACE(" >NAME: |%.*s|\n",(int)(dest-fbuf),fbuf);
    }
    else
    {
	dest = NormalizeFileName(dest,end,title,false);
	TRACE(" >TITL: |%.*s|\n",(int)(dest-fbuf),fbuf);

	if ( id6 && *id6 )
	{
	    dest += snprintf(dest,end-dest,"%s[%s]",
			dest > fbuf && dest[-1] == '/' ? "" : " ", id6 );
	    TRACE(" >ID6:  |%.*s|\n",(int)(dest-fbuf),fbuf);
	}
    }

    //----- ext handling

    if (ext)
    {
	const int elen = strlen(ext);
	if ( elen > 0
	    && ( dest - elen <= fbuf || memcmp(dest-elen,ext,elen) )
	    && dest + elen < end )
	{
	    TRACE("add ext %s\n",ext);
	    memcpy(dest,ext,elen);
	    dest += elen;
	    TRACE(" >EXT:  |%.*s|\n",(int)(dest-fbuf),fbuf);
	}
    }

    //----- term

    *dest = 0;
    f->fname = STRDUP(fbuf);
    TRACE("FNAME:      %s\n",fbuf);
}

///////////////////////////////////////////////////////////////////////////////

void GenDestFileName
	( File_t * f, ccp dest, ccp default_name, ccp ext, bool rm_std_ext )
{
    char fbuf[PATH_MAX];
    if ( rm_std_ext && default_name )
    {
	const size_t name_len = strlen(default_name);
	bool done = false;
	enumOFT oft;
	for ( oft = 0; oft < OFT__N && !done; oft++ )
	{
	    int i;
	    for ( i = 0; i < 2 && !done; i++ )
	    {
		ccp rm = i ? oft_info[oft].ext2 : oft_info[oft].ext1;
		if (!rm)
		    continue;

		const size_t rm_len = strlen(rm);
		const size_t cut_len = name_len - rm_len;
		if ( rm_len && rm_len <= name_len && !strcasecmp(rm,default_name+cut_len) )
		{
		    // got it!

		    if ( cut_len < sizeof(fbuf) )
		    {
			memcpy(fbuf,default_name,cut_len);
			fbuf[cut_len] = 0;
			default_name = fbuf;
		    }
		    done = true; // remove maximal 1 extension
		}
	    }
	}
    }

    if ( IsDirectory(dest,true) )
    {
	if ( dest && *dest && default_name )
	{
	    ccp file_part = strrchr(default_name,'/');
	    if ( file_part )
		default_name = file_part + 1;
	}
	GenFileName(f,dest,default_name,0,0,ext);
    }
    else
	GenFileName(f,0,dest,0,0,0);
}

///////////////////////////////////////////////////////////////////////////////

void GenImageFileName ( File_t * f, ccp dest, ccp default_name, enumOFT oft )
{
    ccp ext = (uint)oft < OFT__N ? oft_info[oft].ext1 : 0;
    GenDestFileName(f,dest,default_name,ext,true);
}

///////////////////////////////////////////////////////////////////////////////

const OFT_info_t oft_info[OFT__N+1] =
{
    // HINT: The extra 0 in "\0" is needed because of: oft_info[].ext1+1

    // { oft, attrib, iom,
    //     name, option, ext1, ext2, info },

    { OFT_UNKNOWN, 0, IOM__IS_DEFAULT,
	"?", 0, "\0", 0, "unkown file format" },

    { OFT_PLAIN, OFT_A_READ|OFT_A_WRITE|OFT_A_EXTEND|OFT_A_MODIFY, IOM_IS_IMAGE,
	"ISO", "--iso", ".iso", 0, "plain file" },

    { OFT_WDF,	OFT_A_READ|OFT_A_WRITE|OFT_A_EXTEND|OFT_A_MODIFY, IOM_IS_IMAGE,
	"WDF", "--wdf", ".wdf", 0, "Wii Disc Format" },

    { OFT_CISO,	OFT_A_READ|OFT_A_WRITE|OFT_A_MODIFY|OFT_A_NOSIZE, IOM_IS_IMAGE,
	"CISO", "--ciso", ".ciso", ".wbi", "Compact ISO" },

    { OFT_WBFS,	OFT_A_READ|OFT_A_WRITE|OFT_A_EXTEND|OFT_A_MODIFY|OFT_A_NOSIZE, IOM_IS_IMAGE,
	"WBFS", "--wbfs", ".wbfs", 0, "Wii Backup File System" },

    { OFT_WIA,	OFT_A_READ|OFT_A_WRITE|OFT_A_COMPR, IOM_IS_WIA,
	"WIA", "--wia", ".wia", 0, "compressed Wii ISO Archive" },

    { OFT_FST,	OFT_A_READ|OFT_A_WRITE|OFT_A_FST, IOM__IS_DEFAULT,
	"FST", "--fst", "\0", 0, "extracted File System" },

    { OFT__N, 0, IOM__IS_DEFAULT,
	0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////

enumOFT CalcOFT ( enumOFT force, ccp fname_dest, ccp fname_src, enumOFT def )
{
    if ( force > OFT_UNKNOWN && force < OFT__N )
	return force;

    ccp fname = IsDirectory(fname_dest,true) ? fname_src : fname_dest;
    if (fname)
    {
	const size_t len = strlen(fname);
	if ( len >= 4 )
	{
	    if ( !strcasecmp(fname+len-4,".wdf") )
		return OFT_WDF;

	    if ( !strcasecmp(fname+len-4,".wia") )
		return OFT_WIA;

	    if ( !strcasecmp(fname+len-4,".iso") )
		return OFT_PLAIN;

	    if ( !strcasecmp(fname+len-4,".wbi") )
		return OFT_CISO;

	    if ( len >= 5 )
	    {
		if ( !strcasecmp(fname+len-5,".wbfs") )
		    return OFT_WBFS;

		if ( !strcasecmp(fname+len-5,".ciso") )
		    return OFT_CISO;
	    }
	}
    }

    return def;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			split file support		///////////////
///////////////////////////////////////////////////////////////////////////////

static void ExtractSplitMap
(
    File_t		* f,		// main file
    File_t		* f2		// split file
)
{
    DASSERT(f);
    DASSERT(f2);

    if ( !f2->prealloc_done && f->prealloc_map.used )
    {
	const off_t split_end = f2->split_off + f2->split_filesize;
	MemMapItem_t ** end_field = f->prealloc_map.field + f->prealloc_map.used;
	MemMapItem_t **ptr;
	for ( ptr = f->prealloc_map.field; ptr < end_field; ptr++ )
	{
	    off_t beg = (*ptr)->off;
	    off_t end = beg + (*ptr)->size;
	    if ( beg < split_end && end > f2->split_off )
	    {
		if ( beg < f2->split_off )
		     beg = f2->split_off;
		if ( end > split_end )
		     end = split_end;
		PRINT(">>> PREALLOC: fd=%u, %9llx .. %9llx\n",f2->fd,(u64)beg,(u64)end);
		MemMapItem_t * item
		    = InsertMemMapTie(&f2->prealloc_map,beg-f2->split_off,end-beg);
		DASSERT(item);
		snprintf(item->info,sizeof(item->info),
			"v-off=%llx, %s\n",
			(u64)beg, wd_print_size(0,0,end-beg,false,WD_SIZE_AUTO) );
	    }
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

enumError XSetupSplitFile ( XPARM File_t *f, enumOFT oft, off_t split_size )
{
    ASSERT(f);
    if (f->split_f)
	return ERR_OK;

    if ( f->fd == -1 || !S_ISREG(f->st.st_mode) )
    {
	if (!f->disable_errors)
	    ERROR0(ERR_WARNING,
		"Split file support not possible: %s\n", f->fname );
	return ERR_WARNING;
    }

    // at very first: setup split file format

    FreeString(f->split_fname_format);
    FreeString(f->split_rename_format);
    if (f->rename)
    {
	f->split_fname_format = AllocSplitFilename(f->fname,OFT_PLAIN);
	f->split_rename_format = AllocSplitFilename(f->rename,oft);
    }
    else
	f->split_fname_format = AllocSplitFilename(f->fname,oft);

    char fname[PATH_MAX];

    if (!f->is_writing)
    {
	// for read only files: test if split files available
	snprintf(fname,sizeof(fname),f->split_fname_format,1);
	struct stat st;
	if (stat(fname,&st))
	{
	    bool found = false;
	    const int slen =  strlen(f->split_fname_format);
	    if ( slen > 2 && f->split_fname_format[slen-2] == '1' )
	    {
		int fw;
		for ( fw = '2'; fw <= '6'; fw++ )
		{
		    ((char*)f->split_fname_format)[slen-2] = fw;
		    snprintf(fname,sizeof(fname),f->split_fname_format,1);
		    if (!stat(fname,&st))
		    {
			found = true;
			break;
		    }
		}   
	    }
	    if (!found)
		return ERR_OK; // no split files found -> return
	}
	noPRINT("SPLIT-FNAME: %s\n",f->split_fname_format);
    }

    File_t ** list = CALLOC(MAX_SPLIT_FILES,sizeof(*list));
    File_t * first;
    *list = first = MALLOC(sizeof(File_t));

    // copy all but cache
    memcpy(first,f,sizeof(File_t));

    first->is_caching = false;
    first->cache = 0;
    first->cur_cache = 0;

    const bool have_stream = f->fp != 0;
    f->fp =  0;
    f->fd = -1;
    f->split_f = list;
    f->split_used = 1;

    if (f->rename)
    {
	// fname is only informative -> use the final name
	f->fname = STRDUP(f->rename);
	f->rename = 0;
    }
    else
	f->fname = STRDUP(f->fname);
    if (f->path)
	f->path = STRDUP(f->path);

    if ( oft == OFT_PLAIN || oft == OFT_CISO || oft == OFT_WBFS )
    {
	f->split_filesize = split_size ? split_size : DEF_SPLIT_SIZE_ISO;
	if ( DEF_SPLIT_FACTOR_ISO > 0 )
	{
	    f->split_filesize &= ~(DEF_SPLIT_FACTOR_ISO-1);
	    if (!f->split_filesize)
		f->split_filesize = DEF_SPLIT_FACTOR_ISO;
	}
    }
    else
    {
	f->split_filesize  = split_size ? split_size : DEF_SPLIT_SIZE;
	if ( DEF_SPLIT_FACTOR > 0 )
	{
	    f->split_filesize &= ~(DEF_SPLIT_FACTOR-1);
	    if (!f->split_filesize)
		f->split_filesize = DEF_SPLIT_FACTOR;
	}
    }

    first->split_filesize = first->st.st_size;
    first->split_fname_format = EmptyString;
    first->split_rename_format = 0;
    first->outname = 0;

    PRINT("#S#   Split setup, size=%llu, fname=%s\n",
	(u64)f->split_filesize, f->fname );
    PRINT("#S#0# split setup, size=%llu, fname=%s\n",
	(u64)first->split_filesize, first->fname );

    noTRACE(" fname:   %p %p\n",f->fname,first->fname);
    noTRACE(" path:    %p %p\n",f->path,first->path);
    noTRACE(" rename:  %p %p\n",f->rename,first->rename);
    noTRACE(" outname: %p %p\n",f->outname,first->outname);
    noTRACE(" split-fname-format:  %s\n",f->split_fname_format);
    noTRACE(" split-rename-format: %s\n",f->split_rename_format);

    ExtractSplitMap(f,first);

    if (f->is_reading)
    {
	int idx;
	for ( idx = 1; idx < MAX_SPLIT_FILES; idx++ )
	{
	    snprintf(fname,sizeof(fname),f->split_fname_format,idx);
	    struct stat st;
	    if (stat(fname,&st))
		break;

	    File_t * fi = MALLOC(sizeof(*f));
	    InitializeFile(fi);
	    fi->open_flags = f->active_open_flags;
	    fi->fname = STRDUP(fname);
	    enumError err = XOpenFileHelper( XCALL fi,
				have_stream ? IOM_FORCE_STREAM : IOM_NO_STREAM,
				f->active_open_flags, f->active_open_flags );
	    if (err)
	    {
		CloseFile(fi,false);
		FREE(fi);
		return err;
	    }

	    if (f->split_rename_format)
	    {
		ASSERT(!fi->rename);
		snprintf(fname,sizeof(fname),f->split_rename_format,idx);
		fi->rename = STRDUP(fname);
	    }

	    fi->split_off = f->split_f[idx-1]->split_off
			  + f->split_f[idx-1]->split_filesize;
	    fi->split_filesize = fi->st.st_size;

	    PRINT("#S#%u# Open %s, soff=%llx, ssize=%llx\n",
			idx, fname, (u64)fi->split_off, (u64)fi->split_filesize );
	    f->st.st_size += fi->st.st_size;
	    f->split_f[idx] = fi;
	}
	f->split_used = idx;
	f->fatt.size = f->st.st_size;
    }

    File_t * fi = f->split_f[f->split_used-1];
    ASSERT(fi);
    if ( fi->split_filesize < f->split_filesize )
    {
	TRACE("#S#%u# new ssize: %llx -> %llx\n",
		f->split_used-1, (u64)fi->split_filesize, (u64)f->split_filesize );
	fi->split_filesize = f->split_filesize;
    }

 #ifdef DEBUG
    {
	int i;
	for ( i = 0; i < f->split_used; i++ )
	    TRACE(" - %2d: ssize= %9llx\n",i,(u64)f->split_f[i]->split_filesize);
    }
 #endif

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError XCreateSplitFile ( XPARM File_t *f, uint split_idx )
{
    ASSERT( f );
    ASSERT( f->split_f );
    ASSERT( split_idx > 0 );
    TRACE("#S# CreateSplitFile() %u/%u/%u\n",split_idx,f->split_used,MAX_SPLIT_FILES);

    if ( split_idx > MAX_SPLIT_FILES )
    {
	if ( !f->disable_errors )
	    PrintError( XERROR1, ERR_WRITE_FAILED,
			"Max number of split files (%d,off=%llx) reached: %s\n",
			MAX_SPLIT_FILES, (u64)f->file_off, f->fname );

	f->last_error = ERR_WRITE_FAILED;
	if ( f->max_error < f->last_error )
	    f->max_error = f->last_error;
	return ERR_WRITE_FAILED;
    }

    while ( f->split_used <= split_idx )
    {
	File_t ** ptr = f->split_f + f->split_used;
	ASSERT(!*ptr);

	File_t * prev;
	if ( f->split_used > 0 )
	{
	    prev = ptr[-1];
	    if ( prev->is_writing )
		SetSizeF(prev,prev->split_filesize);
	}
	else
	    prev = 0;

	char fname[PATH_MAX];
	snprintf(fname,sizeof(fname),f->split_fname_format,f->split_used++);
	TRACE("#S#%u# Create %s\n",f->split_used-1,fname);

	File_t * f2 = MALLOC(sizeof(File_t));
	InitializeFile(f2);
	*ptr = f2;
	const int flags = O_CREAT|O_WRONLY|O_TRUNC|O_EXCL|f->active_open_flags;
	f2->fname = STRDUP(fname);
	enumError err
	    = XOpenFileHelper( XCALL f2,
				prev && prev->fp ? IOM_FORCE_STREAM : IOM_NO_STREAM,
				flags, flags );
	if (err)
	    return err;
	if (prev)
	    f2->split_off = prev->split_off + prev->split_filesize;
	f2->split_filesize = f->split_filesize;
	PRINT("SPLIT #%u: %9llx + %9llx\n",
		split_idx, (u64)f2->split_off,(u64)f2->split_filesize);
	if (f->split_rename_format)
	{
	    ASSERT(!f2->rename);
	    snprintf(fname,sizeof(fname),f->split_rename_format,f->split_used-1);
	    f2->rename = STRDUP(fname);
	}

	ExtractSplitMap(f,f2);
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError XFindSplitFile ( XPARM File_t *f, uint * p_index, off_t * p_off )
{
    ASSERT( f );
    ASSERT( f->split_f );
    ASSERT(p_index);
    ASSERT(p_off);
    off_t off = *p_off;
    TRACE("#S# XFindSplitFile(off=%llx) %u/%u\n",(u64)off,f->split_used,MAX_SPLIT_FILES);

    File_t ** ptr = f->split_f;
    for (;;)
    {
	if (!*ptr)
	{
	    enumError err = XCreateSplitFile( XCALL f, ptr-f->split_f );
	    if (err)
		return err;
	}

	File_t *cur = *ptr;
	ASSERT(cur);
	if ( off <= cur->split_filesize )
	{
	    *p_index = ptr - f->split_f;
	    *p_off = off;
	    return ERR_OK;
	}

	// skip this file
	off -= cur->split_filesize;
	ptr++;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ClearCache	( File_t * f )
{
    DASSERT(f);
    f->is_caching = false;
    while (f->cache)
    {
	FileCache_t * ptr = f->cache;
	f->cache = ptr->next;
	FREE((char*)ptr->data);
	FREE(ptr);
    }
    f->cur_cache = 0;
}

///////////////////////////////////////////////////////////////////////////////

void DefineCachedArea ( File_t * f, off_t off, size_t count )
{
    // this whole function assumes that count is small and that the hole
    // cache size is less than MAX(size_t)

    ASSERT(f);

    f->cur_cache = 0;
    f->is_caching = f->fd != -1 && f->is_reading && !f->is_writing;
    TRACE("DefineCachedArea(,%llx,%zx) fd=%d rw=%d,%d is_caching=%d, filesize=%llx\n",
		(u64)off, count, f->fd, f->is_reading, f->is_writing, f->is_caching,
		(u64)f->st.st_size );
    if (!f->is_caching)
	return;

    const size_t blocksize = f->st.st_blksize > HD_SECTOR_SIZE
			   ? f->st.st_blksize : HD_SECTOR_SIZE;

    off_t off_end = (( off + count + blocksize - 1 ) / blocksize ) * blocksize;
    off = ( off / blocksize ) * blocksize;
    count = (size_t)( off_end - off );

    if (f->st.st_size)
    {
	// limit cache to file size if filesize is known

	if ( off >= f->st.st_size )
	    return;

	if ( off + count > f->st.st_size )
	    count = f->st.st_size - off;
    }

    FileCache_t **pptr;
    for ( pptr = &f->cache; *pptr; pptr = &(*pptr)->next )
    {
	FileCache_t * ptr = *pptr;
	if ( off_end < ptr->off )
	    break;

	off_t ptr_end = ptr->off + ptr->count;
	if ( off < ptr_end || off == ptr_end && !ptr->data )
	{
	    // extend this cache element
	    if (ptr->data)
	    {
		// already cached -> try a smaller area
		if ( off_end > ptr_end )
		    DefineCachedArea(f,ptr_end,off_end-ptr_end);
		return;
	    }

	    off_t new_end = ptr->off + ptr->count;
	    if ( new_end < off_end )
		new_end = off_end;

	    if ( ptr->off < off )
		 ptr->off = off;
	    ptr->count = new_end - ptr->off;
	    TRACE("#F# CACHE ENTRY extended: o=%10llx n=%9zx\n",(u64)ptr->off,ptr->count);
	    return;
	}
    }

    // insert a new cache element
    FileCache_t * ptr = CALLOC(1,sizeof(FileCache_t));
    ptr->off   = off;
    ptr->count = count;
    ptr->next  = *pptr;
    *pptr = ptr;
    TRACE("#F# CACHE ENTRY inserted: o=%10llx n=%9zx\n",(u64)ptr->off,ptr->count);
}

//-----------------------------------------------------------------------------

void DefineCachedAreaISO ( File_t * f, bool head_only )
{
    ASSERT(f);
    if (head_only)
    {
	DefineCachedArea(f,0,CISO_HEAD_SIZE+FILE_PRELOAD_SIZE);
    }
    else
    {
	DefineCachedArea(f,0x0000000ull,0x200000);
	DefineCachedArea(f,0xf800000ull,0x800000);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError XAnalyzeWH ( XPARM File_t * f, WDF_Head_t * wh, bool print_err )
{
    ASSERT(wh);
    TRACE("AnalyzeWH()\n");

    if (memcmp(wh->magic,WDF_MAGIC,WDF_MAGIC_SIZE))
    {
	TRACE(" - magic failed\n");
	return ERR_NO_WDF;
    }

    if (!f->seek_allowed)
    {
	TRACE(" - seek not allowed\n");
	return print_err
		? PrintError( XERROR0, ERR_WRONG_FILE_TYPE,
			"Wrong file type: %s\n", f->fname )
		: ERR_WRONG_FILE_TYPE;
    }

    const size_t wdf_head_size = AdjustHeaderWDF(wh);

 #if WDF2_ENABLED
    if ( !wh->wdf_version || wh->wdf_compatible > WDF_VERSION )
 #else
    if ( !wh->wdf_version || wh->wdf_version > WDF_VERSION )
 #endif
    {
     #if WDF2_ENABLED
	PRINT(" - wrong WDF version: %u,%u\n",wh->wdf_version,wh->wdf_compatible);
     #else
	PRINT(" - wrong WDF version: %u\n",wh->wdf_version);
     #endif
	return print_err
		? PrintError( XERROR0, ERR_WDF_VERSION,
 #if WDF2_ENABLED
			"Only WDF version 1..%u supported but not version %u.\n",
 #else
			"Only WDF version %u supported but not version %u.\n",
 #endif
			WDF_VERSION, wh->wdf_version )
		: ERR_WDF_VERSION;
    }

    const u32 chunk_tab_size = wh->chunk_n * sizeof(WDF_Chunk_t);
    if ( f->st.st_size < wh->chunk_off + WDF_MAGIC_SIZE + chunk_tab_size )
    {
	// file size to short -> maybe a splitted file
	XSetupSplitFile(XCALL f,OFT_UNKNOWN,0);
    }

    if ( wh->chunk_off != wh->data_size + wdf_head_size
	|| wh->chunk_off + WDF_MAGIC_SIZE + chunk_tab_size != f->st.st_size )
    {
	PRINT(" - file size error\n");
	PRINT("   - %llx ? %llx = %llx + %zx\n",
		(u64)wh->chunk_off, wh->data_size + wdf_head_size,
		wh->data_size, wdf_head_size );
	PRINT("   - %llx + %x + %x = %llx ? %llx\n",
		(u64)wh->chunk_off, WDF_MAGIC_SIZE, chunk_tab_size,
		(u64)wh->chunk_off + WDF_MAGIC_SIZE + chunk_tab_size,
		(u64)f->st.st_size );

	return print_err
		? PrintError( XERROR0, ERR_WDF_INVALID, "Invalid WDF file: %s\n",f->fname )
		: ERR_WDF_INVALID;
    }

    TRACE(" - OK\n");
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static FileCache_t * XCacheHelper ( XPARM File_t * f, off_t off, size_t count )
{
    // This function does the following:
    // a) look into the cache table to find if (off,count) is part of an
    //    cache entry. I so, the result is a pointer to the first affected
    //    cache entry.
    // b) All cache entires up to off are filled with data. If 'off' is part
    //    of an entry this entry is filled completly.
    // c) If the result is not NULL a seek or a seek simulation is done.

    ASSERT(f);
    f->last_error = ERR_OK;
    if (!f->is_caching)
	return 0;

    // cur_cache is always pointing to the last used and loaded cache entry
    ASSERT( !f->cur_cache || f->cur_cache->data );

    FileCache_t * cptr = f->cur_cache;
    if ( !cptr || off < cptr->off )
	cptr = f->cache;

    f->is_caching = false; // disable cache access for sub calls
    while (cptr)
    {
	TRACE(" - SEARCH=%llx..%llx  CACHE=%llx..%llx\n",
	    (u64)off, (u64)off + count, (u64)cptr->off, (u64)cptr->off + cptr->count );

	if ( off + count <= cptr->off )
	{
	    cptr = 0;
	    break;
	}

	if (!cptr->data)
	{
	    // cache must be filled
	    char * data = MALLOC(cptr->count);
	    cptr->data = data;
	    TRACE(TRACE_RDWR_FORMAT, "#F# FILL CACHE",
		GetFD(f), GetFP(f), (u64)cptr->off,
		(u64)cptr->off+cptr->count, cptr->count, "" );
	    const enumError stat = XReadAtF(XCALL f,cptr->off,data,cptr->count);
	    if (stat)
	    {
		cptr = 0;
		break;
	    }
	}
	if ( off < cptr->off + cptr->count )
	    break;
	cptr = cptr->next;
    }
    f->is_caching = true; // restore cache access

    if (cptr)
    {
	// cache entry found!
	ASSERT( off + count > cptr->off );
	ASSERT( off < cptr->off + cptr->count );

	f->cur_off = off;
	f->cur_cache = cptr; // save the current ptr
	ASSERT(cptr->data);

	TRACE(TRACE_RDWR_FORMAT, "#F# CACHE FOUND",
		GetFD(f), GetFP(f), (u64)cptr->off,
		(u64)cptr->off+cptr->count, cptr->count, "" );
    }

    return cptr;
}

///////////////////////////////////////////////////////////////////////////////

static void PreallocHelper ( File_t *f )
{
    DASSERT(f);

 #ifndef NO_PREALLOC
    if ( !f->prealloc_done && f->prealloc_map.used )
    {
	f->prealloc_done = true;
	if (f->split_f)
	{
	    ExtractSplitMap(f,f->split_f[0]);
	    PreallocHelper(f->split_f[0]);
	}
	else
	{
	    if ( logging > 0 )
	    {
		printf("\n Preallocation table:\n");
		PrintMemMap(&f->prealloc_map,stdout,3);
		putchar('\n');
	    }

	    // prealloc largest block first

	    MemMapItem_t ** end_field = f->prealloc_map.field + f->prealloc_map.used;
	    for(;;)
	    {
		off_t found_size = 0;
		MemMapItem_t **ptr, *found = 0;
		for ( ptr = f->prealloc_map.field; ptr < end_field; ptr++ )
		    if ( (*ptr)->size > found_size )
		    {
			found = *ptr;
			found_size = found->size;
		    }
		if (!found)
		    break;

	#ifdef HAVE_FALLOCATE
		noPRINT("CALL fallocate(%d,0,%9llx,%9llx [%s])\n",
			    f->fd, (u64)found->off, (u64)found->size,
			    wd_print_size_1024(0,0,found->size,true) );
		fallocate(f->fd,0,found->off,found->size);
		noPRINT("TERM fallocate()\n");
	#elif HAVE_POSIX_FALLOCATE
		noPRINT("CALL posix_fallocate(%d,%9llx,%9llx [%s])\n",
			    f->fd, (u64)found->off, (u64)found->size,
			    wd_print_size_1024(0,0,found->size,true) );
		posix_fallocate(f->fd,found->off,found->size);
		noPRINT("TERM posix_fallocate()\n");
	#elif __APPLE__
		noPRINT("CALL fcntl(%d,F_PREALLOCATE,%9llx,%9llx [%s])\n",
			    f->fd, (u64)found->off, (u64)found->size,
			    wd_print_size_1024(0,0,found->size,true) );
		fstore_t fst;
		fst.fst_flags	= F_ALLOCATECONTIG;
		fst.fst_posmode	= F_PEOFPOSMODE;
		fst.fst_offset	= found->off;
		fst.fst_length	= found->size;
		fst.fst_bytesalloc = 0;
		fcntl( f->fd, F_PREALLOCATE, &fst );
		noPRINT("TERM fcntl()\n");
	#else
		#error "no preallocation support -> use -DNO_PREALLOC"
	#endif

		found->size = 0;
	    }
	}
    }
 #endif
}

///////////////////////////////////////////////////////////////////////////////

enumError XTellF ( XPARM File_t * f )
{
    ASSERT(f);

    if (f->split_f)
    {
	// file pos is pure virtual
	return f->cur_off;
    }

    f->file_off = f->fp
		    ? ftello(f->fp)
		    : f->fd != -1
			? lseek(f->fd,(off_t)0,SEEK_CUR)
			: (off_t)-1;

    if ( f->file_off == (off_t)-1 )
    {
	f->last_error = f->is_writing ? ERR_WRITE_FAILED : ERR_READ_FAILED;
	if ( f->max_error < f->last_error )
	    f->max_error = f->last_error;
	if (!f->disable_errors)
	    PrintError( XERROR1, f->last_error, "Tell failed [%c=%d]: %s\n",
			GetFT(f), GetFD(f), f->fname );
    }
    else
    {
	f->last_error = ERR_OK;
	f->tell_count++;
	if ( f->max_off < f->file_off )
	    f->max_off = f->file_off;
    }

    TRACE(TRACE_SEEK_FORMAT, "#F# TellF()", GetFD(f), GetFP(f), (u64)f->file_off, "" );
    f->cur_off = f->file_off;
    return f->last_error;
}

///////////////////////////////////////////////////////////////////////////////

enumError XSeekF ( XPARM File_t * f, off_t off )
{
    ASSERT(f);

    // sync virtual off
    f->cur_off = f->file_off;

    // if we are already at off and pure reading or pure writing -> all done
    if ( off == f->file_off && f->is_writing != f->is_reading )
	return f->last_error = ERR_OK;

    if (f->is_caching)
    {
	FileCache_t * cptr = XCacheHelper(XCALL f,off,1);
	if (cptr)
	    return f->last_error; // all done
    }

    if (!f->prealloc_done)
	PreallocHelper(f);

    if (f->split_f)
    {
	// file pos is pure virtual
	f->cur_off = off;
	return ERR_OK;
    }

    if (!f->seek_allowed)
    {
	if ( !f->is_reading && off >= f->file_off )
	{
	    // simulate seek with 'write zero' operations

	    off_t skip_size = off - f->file_off;
	    TRACE(TRACE_RDWR_FORMAT, "#F# SEEK SIMULATION",
			    GetFD(f), GetFP(f), (u64)f->file_off,
			    (u64)f->file_off+skip_size, (size_t)skip_size, "" );

	    while (skip_size)
	    {
		if (SIGINT_level>1)
		    return ERR_INTERRUPT;

		const size_t max_write = skip_size < sizeof(zerobuf)
				      ? (size_t)skip_size : sizeof(zerobuf);
		const enumError stat = XWriteF(XCALL f,zerobuf,max_write);
		if (stat)
		    return stat;
		skip_size -= max_write;
		noTRACE("R+W-Count: %llu + %llu\n",f->bytes_read,f->bytes_written);
	    }
	    ASSERT( SIGINT_level || off == f->file_off );
	    ASSERT( SIGINT_level || off == f->cur_off );
	    return ERR_OK;
	}

	if ( !f->is_writing && off >= f->file_off )
	{
	    // simulate seek with read operations

	    off_t skip_size = off - f->file_off;
	    TRACE(TRACE_RDWR_FORMAT, "#F# SEEK SIMULATION",
			    GetFD(f), GetFP(f), (u64)f->file_off,
			    (u64)f->file_off+skip_size, (size_t)skip_size, "" );

	    while (skip_size)
	    {
		if (SIGINT_level>1)
		    return ERR_INTERRUPT;

		const size_t max_read = skip_size < sizeof(iobuf)
				      ? (size_t)skip_size : sizeof(iobuf);
		const enumError stat = XReadF(XCALL f,iobuf,max_read);
		if (stat)
		    return stat;
		skip_size -= max_read;
		noTRACE("R+W-Count: %llu + %llu\n",f->bytes_read,f->bytes_written);
	    }
	    ASSERT( SIGINT_level || off == f->file_off );
	    ASSERT( SIGINT_level || off == f->cur_off );
	    return ERR_OK;
	}

	if ( f->is_caching )
	{
	    char buf[100];
	    snprintf(buf,sizeof(buf),"ID=%s, OFF=%llx, SIZE=%x",
		    f->id6_src, (u64)f->cache_info_off, (u32)f->cache_info_size );
	    fprintf(stderr,
		    "\n"
		    "************************************************************************\n"
		    "*****  It seems, that the caching area for the game is too small!  *****\n"
		    "*****  Please report this to the author.                           *****\n"
		    "*****  Technical data: %-43s *****\n"
		    "************************************************************************\n"
		    "\n", buf );
	}
    }

    TRACE(TRACE_SEEK_FORMAT, "#F# SeekF()",
		GetFD(f), GetFP(f), (u64)off, off < f->max_off ? " <" : "" );

    const bool failed = f->fp
			? fseeko(f->fp,off,SEEK_SET) == (off_t)-1
			: f->fd == -1 || lseek(f->fd,off,SEEK_SET) == (off_t)-1;

    enumError err;
    if (failed)
    {
	err = f->is_writing ? ERR_WRITE_FAILED : ERR_READ_FAILED;
	f->last_error = err;
	if ( f->max_error < f->last_error )
	    f->max_error = f->last_error;
	if (!f->disable_errors)
	    PrintError( XERROR1, f->last_error,
			"Seek failed [%c=%d,%llu]: %s\n",
			GetFT(f), GetFD(f), (u64)off, f->fname );
	f->file_off = (off_t)-1;
    }
    else
    {
	err = ERR_OK;
	f->seek_count++;
	if ( f->max_off < f->file_off )
	    f->max_off = f->file_off;
    }

    f->cur_off = f->file_off = off;
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError XSetSizeF ( XPARM File_t * f, off_t size )
{
    ASSERT(f);
    TRACE(TRACE_SEEK_FORMAT, "#F# SetSizeF()",
		GetFD(f), GetFP(f), (u64)size,
		size < f->max_off ? " <" : size > f->max_off ? " >" : "" );

    if (!f->prealloc_done)
	PreallocHelper(f);

    if (f->split_f)
    {
	f->max_off = size;

	uint index;
	enumError err = XFindSplitFile( XCALL f, &index, &size );
	if (err)
	    return err;
	ASSERT( index < MAX_SPLIT_FILES );
	File_t ** ptr = f->split_f + index;
	XSetSizeF(XCALL *ptr,size);

	int count = f->split_used - index;
	f->split_used = index+1;
	while ( count-- > 1 )
	{
	    ptr++;
	    ASSERT(*ptr);
	    XCloseFile( XCALL *ptr, true );
	    FREE(*ptr);
	    *ptr = 0;
	}
	return ERR_OK;
    }

    //--------------------------------------------------

    if (f->fp)
	fflush(f->fp); // [[2do]] ? error handling

    if ( !f->seek_allowed && f->cur_off <= size )
    {
	if (!XSeekF(XCALL f,size))
	    return f->last_error;
    }
    else if (ftruncate(f->fd,size))
    {
	f->last_error = ERR_WRITE_FAILED;
	if ( f->max_error < f->last_error )
	    f->max_error = f->last_error;
	if (!f->disable_errors)
	    PrintError( XERROR1, f->last_error,
			"Set file size failed [%c=%d,%llu]: %s\n",
			GetFT(f), GetFD(f), (u64)size, f->fname );
	return f->last_error;
    }

    PRINT("fd=%d truncated to %llx\n",f->fd,(u64)size);
    f->setsize_count++;
    f->max_off = size;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError XPreallocateF
(
    XPARM			// debugging and tracing infos
    File_t	* f,		// valid file, writing
    off_t	off,		// offset
    off_t	size		// needed size
)
{
    DASSERT(f);
    enumError err = ERR_OK;
    if ( size && prealloc_mode > PREALLOC_OFF )
    {
	noPRINT("PREALLOC/FILE fd=%d, %llx+%llx max=%llx: %s\n",
		f->fd, (u64)off, (u64)size, (u64)f->max_off, f->fname );

	const off_t max = off + size;
	if ( f->prealloc_size < max )
	     f->prealloc_size = max;

	InsertMemMapTie(&f->prealloc_map,off,size);
    }
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError XReadF ( XPARM File_t * f, void * iobuf, size_t count )
{
    ASSERT(f);

    if (f->is_caching)
    {
	TRACE("#F# --- cache\n");
	TRACE(TRACE_RDWR_FORMAT, "#F# ReadF()+C",
		GetFD(f), GetFP(f), (u64)f->cur_off, (u64)f->cur_off+count, count,
		f->cur_off < f->max_off ? " <" : "" );

	off_t my_off = f->cur_off;
	while (count)
	{
	    FileCache_t * cptr = XCacheHelper(XCALL f,my_off,count);
	    if (f->last_error)
		return f->last_error;
	    if (!cptr)
		break;

	    // there is an overlap
	    TRACE("   my_off=%llx, count=%zx, cptr->off=%llx, cptr->count=%zx\n",
			(u64)my_off, count, (u64)cptr->off, cptr->count );
	    //ASSERT( my_off + count > cptr->off );
	    ASSERT( cptr->off + cptr->count > my_off );

	    if ( cptr->off > my_off )
	    {
		// read data from file

		TRACELINE;
		const size_t size = cptr->off - my_off;
		f->is_caching = false; // disable cache operations
		const enumError stat = XReadAtF(XCALL f,my_off,iobuf,size);
		f->is_caching = true; // restore cache operations
		if (stat)
		    return stat;
		iobuf = (void*)( (char*)iobuf + size );
		count -= size;
		my_off += size;
	    }

	    if ( count > 0 )
	    {
		TRACELINE;
		ASSERT( my_off >= cptr->off );
		ASSERT( my_off <  cptr->off + cptr->count );
		ASSERT( cptr->data );
		const u32 delta = my_off - cptr->off;
		ASSERT ( delta < cptr->count );
		size_t size = cptr->count - delta;
		if ( size > count )
		    size = count;
		TRACE(TRACE_RDWR_FORMAT, "#F# COPY FROM CACHE",
			GetFD(f), GetFP(f), (u64)my_off, (u64)my_off+size, size, "" );
		memcpy(iobuf,cptr->data+delta,size);
		iobuf = (void*)( (char*)iobuf + size );
		count -= size;
		my_off += size;
		f->bytes_read += size;
	    }

	    f->cur_off = my_off;
	    if (!count)
		return ERR_OK;
	}
    }

    //--------------------------------------------------

    if (f->split_f)
    {
	TRACE("#S# ---\n");
	TRACE(TRACE_RDWR_FORMAT, "#S#   ReadF()",
		GetFD(f), GetFP(f), (u64)f->cur_off, (u64)f->cur_off+count, count,
		f->cur_off < f->max_off ? " <" : "" );

	File_t ** ptr = f->split_f;
	off_t off = f->cur_off;
	f->cur_off = 0;

	while ( count > 0 )
	{
	    if (!*ptr)
	    {
		ptr--;
		f->cur_off += off + count;
		f->bytes_read += count;
		return XReadAtF( XCALL *ptr, off, iobuf, count );
	    }

	    File_t *cur = *ptr;
	    ASSERT(cur);
	    TRACE("#S#%zd# off=%llx cur_of=%llx count=%zx fsize=%llx\n",
			ptr-f->split_f, (u64)off, (u64)f->cur_off, count, (u64)cur->split_filesize );

	    if ( off < cur->split_filesize )
	    {
		// read from this file
		const off_t max_count = cur->split_filesize - off;
		const size_t stream_count = count < max_count ? count : (size_t)max_count;
		enumError err = XReadAtF( XCALL cur, off, iobuf, stream_count );
		if (err)
		    return err;
		f->bytes_read += stream_count;
		count -= stream_count;
		iobuf = (char*)iobuf + stream_count;
		f->cur_off += off + stream_count;
		off = 0;
	    }
	    else
	    {
		// skip this file
		f->cur_off += cur->split_filesize;
		off -= cur->split_filesize;
	    }
	    ptr++;
	}
	if ( f->max_off < f->cur_off )
	    f->max_off = f->cur_off;

	TRACE("#S#x# off=%9llx cur_of=%9llx count=%zx\n", (u64)off, (u64)f->cur_off, count );
	return ERR_OK;
    }

    //--------------------------------------------------

    if ( f->cur_off != f->file_off )
    {
	const int stat = XSeekF(XCALL f,f->cur_off);
	if (stat)
	    return stat;
	// f->cur_off and f->file_off may differ because of cache access
    }

    TRACE(TRACE_RDWR_FORMAT, "#F# ReadF()",
		GetFD(f), GetFP(f), (u64)f->cur_off, (u64)f->cur_off+count, count,
		f->cur_off < f->max_off ? " <" : "" );

    if ( f->read_behind_eof && ( f->st.st_size > 0 || f->is_writing ) )
    {
	const off_t max_read = f->st.st_size > f->file_off
					? f->st.st_size - f->file_off
					: 0;

	TRACE("read_behind_eof=%d, st_size=%llx, count=%zx, max_read=%llx\n",
		f->read_behind_eof, (u64)f->st.st_size, count, (u64)max_read );

	if ( count > max_read )
	{
	    if ( f->read_behind_eof == 1 )
	    {
		f->read_behind_eof = 2;
		if ( !f->disable_errors )
		    PrintError( XERROR0, ERR_WARNING,
			"Read behind eof -> zero filled [%c=%d,%llu+%zu]: %s\n",
			GetFT(f), GetFD(f),
			(u64)f->file_off, count, f->fname );
	    }
	    size_t fill_count = count - (size_t)max_read;
	    count = (size_t)max_read;
	    memset(iobuf+count,0,fill_count);

	    if (!count)
		return ERR_OK;
	}
    }

    bool err;
    size_t read_count = 0;
    if ( f->fd == -1 )
    {
	err = true;
	errno = 0;
    }
    else if (!count)
	err = false;
    else if (f->fp)
    {
	read_count = fread(iobuf,1,count,f->fp);
	err = read_count < count && errno;
	iobuf = (void*)( (char*)iobuf + read_count );
    }
    else
    {
	err = false;
	size_t size = count;
	while (size)
	{
	    ssize_t rstat = read(f->fd,iobuf,size);
	    if ( rstat <= 0 )
	    {
		err = rstat < 0;
		break;
	    }
	    read_count += rstat;
	    size -= rstat;
	    iobuf = (void*)( (char*)iobuf + rstat );
	}
    }

    if ( err || read_count < count && !f->read_behind_eof )
    {
	if ( !f->disable_errors && f->last_error != ERR_READ_FAILED )
	    PrintError( XERROR1, ERR_READ_FAILED,
			"Read failed [%c=%d,%llu+%zu]: %s\n",
			GetFT(f), GetFD(f),
			(u64)f->file_off, count, f->fname );
	f->last_error = ERR_READ_FAILED;
	if ( f->max_error < f->last_error )
	     f->max_error = f->last_error;
	f->file_off = (off_t)-1;
    }
    else
    {
	f->read_count++;
	f->bytes_read += read_count;
	f->file_off += read_count;
	if ( f->max_off < f->file_off )
	    f->max_off = f->file_off;
	if ( read_count < count )
	{
	    if (!f->st.st_size)
		f->st.st_size = f->file_off;
	    f->cur_off = f->file_off;
	    return XReadAtF(XCALL f,f->file_off,iobuf,count-read_count);
	}
    }

    f->cur_off = f->file_off;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError XWriteF ( XPARM File_t * f, const void * iobuf, size_t count )
{
    ASSERT(f);

    if (!f->prealloc_done)
	PreallocHelper(f);

    if (f->split_f)
    {
	TRACE("#S# ---\n");
	TRACE(TRACE_RDWR_FORMAT, "#S#   WriteF()",
		GetFD(f), GetFP(f), (u64)f->cur_off, (u64)f->cur_off+count, count,
		f->cur_off < f->max_off ? " <" : "" );

	File_t ** ptr = f->split_f;
	off_t off = f->cur_off;
	f->cur_off = 0;

	while ( count > 0 )
	{
	    if (!*ptr)
	    {
		enumError err = XCreateSplitFile( XCALL f, ptr-f->split_f );
		if (err)
		    return err;
	    }

	    File_t *cur = *ptr;
	    ASSERT(cur);
	    TRACE("#S#%zd# off=%llx cur_of=%llx count=%zx fsize=%llx\n",
			ptr-f->split_f, (u64)off, (u64)f->cur_off, count, (u64)cur->split_filesize );

	    if ( off < cur->split_filesize )
	    {
		// write to this file
		const off_t max_count = cur->split_filesize - off;
		const size_t stream_count = count < max_count ? count : (size_t)max_count;
		enumError err = XWriteAtF( XCALL cur, off, iobuf, stream_count );
		if (err)
		    return err;
		f->bytes_written += stream_count;
		count -= stream_count;
		iobuf = (char*)iobuf + stream_count;
		f->cur_off += off + stream_count;
		off = 0;
	    }
	    else
	    {
		// skip this file
		f->cur_off += cur->split_filesize;
		off -= cur->split_filesize;
	    }
	    ptr++;
	}
	if ( f->max_off < f->cur_off )
	    f->max_off = f->cur_off;

	TRACE("#S#x# off=%9llx cur_of=%9llx count=%zx\n", (u64)off, (u64)f->cur_off, count );
	return ERR_OK;
    }

    //--------------------------------------------------

    ASSERT( f->cur_off == f->file_off ); // no cache while writing
    TRACE(TRACE_RDWR_FORMAT, "#F# WriteF()",
		GetFD(f), GetFP(f), (u64)f->file_off, (u64)f->file_off+count, count,
		f->file_off < f->max_off ? " <" : "" );

    bool err;
    if (f->fp)
	err = count && fwrite(iobuf,count,1,f->fp) != 1;
    else if ( f->fd != -1 )
    {
	err = false;
	size_t size = count;
	while (size)
	{
	    ssize_t wstat = write(f->fd,iobuf,size);
	    if ( wstat <= 0 )
	    {
		err = true;
		break;
	    }
	    size -= wstat;
	    iobuf = (void*)( (char*)iobuf + wstat );
	}
    }
    else
    {
	err = true;
	errno = 0;
    }

    if (err)
    {
	if ( !f->disable_errors && f->last_error != ERR_WRITE_FAILED )
	    PrintError( XERROR1, ERR_WRITE_FAILED,
			"Write failed [%c=%d,%llu+%zu]: %s\n",
			GetFT(f), GetFD(f),
			(u64)f->file_off, count, f->fname );
	f->last_error = ERR_WRITE_FAILED;
	if ( f->max_error < f->last_error )
	    f->max_error = f->last_error;
	f->file_off = (off_t)-1;
    }
    else
    {
	f->write_count++;
	f->bytes_written += count;
	f->file_off += count;
	if ( f->max_off < f->file_off )
	    f->max_off = f->file_off;
    }

    f->cur_off = f->file_off;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError XReadAtF ( XPARM File_t * f, off_t off, void * iobuf, size_t count )
{
    ASSERT(f);
    noTRACE("#F# ReadAtF(fd=%d,o=%llx,%p,n=%zx)\n",f->fd,(u64)off,iobuf,count);
    f->cache_info_off  = off;
    f->cache_info_size = count;
    const enumError stat = XSeekF(XCALL f,off);
    return stat ? stat : XReadF(XCALL f,iobuf,count);
}

///////////////////////////////////////////////////////////////////////////////

enumError XWriteAtF ( XPARM File_t * f, off_t off, const void * iobuf, size_t count )
{
    ASSERT(f);
    noTRACE("#F# WriteAtF(fd=%d,o=%llx,%p,n=%zx)\n",f->fd,(u64)off,iobuf,count);
    const enumError stat = XSeekF(XCALL f,off);
    return stat ? stat : XWriteF(XCALL f,iobuf,count);
}

///////////////////////////////////////////////////////////////////////////////

enumError XWriteZeroAtF ( XPARM File_t * f, off_t off, size_t count )
{
    TRACE(TRACE_RDWR_FORMAT, "#F# WriteZeroAtF()",
		GetFD(f), GetFP(f), (u64)off, (u64)off+count, count,
		off < f->max_off ? " <" : "" );

    enumError err = XSeekF(XCALL f,off);
    while ( !err && count > 0 )
    {
	const size_t wsize = count < sizeof(zerobuf) ? count : sizeof(zerobuf);
	err = XWriteF(XCALL f, zerobuf, wsize );
	count -= wsize;
    }
    return err;
}

///////////////////////////////////////////////////////////////////////////////
// [zero]

enumError XZeroAtF ( XPARM File_t * f, off_t off, size_t count )
{
    ASSERT(f);
    TRACE(TRACE_RDWR_FORMAT, "#F# ZeroAtF()",
		GetFD(f), GetFP(f), (u64)off, (u64)off+count, count,
		off < f->max_off ? " <" : "" );

    if ( !f->is_reading || !S_ISREG(f->st.st_mode) )
	return XWriteZeroAtF(XCALL f,off,count);

    //----- check file growing

    off_t last_off = off + count;
    if ( last_off > f->max_off )
    {
	const off_t max_off = f->max_off;
	const enumError err = XSetSizeF( XCALL f, last_off );
	if ( err || off >= max_off )
	    return err;

	ASSERT( count > last_off - max_off );
	count -= last_off - max_off;
	last_off = max_off;
    }

    //----- try to align to blocks

    char buf[0x20000];
    const size_t blocksize = f->st.st_blksize < HD_SECTOR_SIZE
				? HD_SECTOR_SIZE
				: f->st.st_blksize > sizeof(buf)
					? sizeof(buf)
					: f->st.st_blksize;

    if ( off/blocksize != (last_off-1)/blocksize )
    {
	// align to blocksize
	const size_t count1 = blocksize - off % blocksize;
	ASSERT( count1 < count );
	const enumError err = XZeroAtF(XCALL f,off,count1);
	if (err)
	    return err;
	off   += count1;
	count -= count1;
    }

    //----- the main loop

    while ( count > 0 )
    {
	//--- read data

	const size_t rsize = count < sizeof(buf) ? count : sizeof(buf);
	const enumError err = XReadAtF(XCALL f,off,buf,rsize);
	if (err)
	    return err;

	//--- iterate read data

	char * ptr = buf;
	char * end = buf + rsize;

	while ( ptr < end )
	{
	    //--- find begining of non zero data

	    while ( ptr < end && !*ptr )
		ptr++;

	    //--- set start block aligned

	    size_t start = ((ptr-buf)/blocksize)*blocksize;
	    ptr = buf + start + blocksize;

	    //--- find zero block

	    while ( ptr < end )
	    {
		char * bl_end = ptr + blocksize;
		if ( bl_end > end )
		    bl_end = end;
		while ( ptr < bl_end && !*ptr )
		    ptr++;
		if ( ptr == bl_end )
		    break;
		ptr = bl_end;
	    }

	    if ( ptr > end )
		ptr = end;

	    const enumError err = XWriteZeroAtF(XCALL f, off+start, ptr-buf-start );
	    if (!err)
		return err;
	}

	off   += rsize;
	count -= rsize;
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int WrapperReadSector ( void * handle, u32 lba, u32 count, void * iobuf )
{
    ASSERT(handle);
    SuperFile_t * sf = (SuperFile_t*)handle;

    TRACE("WrapperReadSector(fd=%d,lba=%x,count=%x) sector-size=%u\n",
		GetFD(&sf->f), lba, count, sf->f.sector_size );

    if (SIGINT_level>1)
	return ERR_INTERRUPT;

    return ReadAtF(
		&sf->f,
		(off_t)lba * sf->f.sector_size,
		iobuf,
		count * sf->f.sector_size );
}

///////////////////////////////////////////////////////////////////////////////

int WrapperWriteSector ( void * handle, u32 lba, u32 count, void * iobuf )
{
    ASSERT(handle);
    SuperFile_t * sf = (SuperFile_t*)handle;

    TRACE("WBFS: WrapperWriteSector(fd=%d,lba=%x,count=%x) sector-size=%u\n",
		GetFD(&sf->f), lba, count, sf->f.sector_size );

    if (SIGINT_level>1)
	return ERR_INTERRUPT;

    return WriteAtF(
		&sf->f,
		(off_t)lba * sf->f.sector_size,
		iobuf,
		count * sf->f.sector_size );
}

///////////////////////////////////////////////////////////////////////////////

int GetFD ( const File_t * f )
{
    return !f ? -1 : f->split_used > 0 ? f->split_f[0]->fd : f->fd;
}

///////////////////////////////////////////////////////////////////////////////

FILE * GetFP ( const File_t * f )
{
    return !f ? 0 : f->split_used > 0 ? f->split_f[0]->fp : f->fp;
}

///////////////////////////////////////////////////////////////////////////////

char GetFT ( const File_t * f )
{
    if (!f)
	return '%';

    if ( f->split_used > 0 )
	f = f->split_f[0];

    return f->fp ? 'S' : f->fd != -1 ? 'F' : '-';
}

///////////////////////////////////////////////////////////////////////////////

bool IsOpenF ( const File_t * f )
{
    return f && ( f->split_used > 0 ? f->split_f[0]->fd : f->fd ) != -1;
}

///////////////////////////////////////////////////////////////////////////////

bool IsSplittedF ( const File_t * f )
{
    return f && f->split_used > 0;
}

///////////////////////////////////////////////////////////////////////////////

bool IsDirectory ( ccp fname, bool answer_if_empty )
{
    if ( !fname || !*fname )
	return answer_if_empty;

    if ( *fname == '-' && !fname[1] )
	return false;

    if ( fname[strlen(fname)-1] == '/' )
	return true;

    struct stat st;
    return !stat(fname,&st) && S_ISDIR(st.st_mode);
}

///////////////////////////////////////////////////////////////////////////////

enumFileMode GetFileMode ( mode_t mode )
{
    return S_ISREG(mode) ? FM_PLAIN
	 : S_ISBLK(mode) ? FM_BLKDEV
	 : S_ISCHR(mode) ? FM_CHRDEV
	 : 0;
}

ccp GetFileModeText ( enumFileMode mode, bool longtext, ccp fail_text )
{
    switch (mode)
    {
	case FM_PLAIN:
	    return longtext ? "PLAIN FILE" : "PLAIN";

	case FM_BLKDEV:
	    return longtext ? "BLOCK DEVICE" : "BLOCK";

	case FM_CHRDEV:
	    return longtext ? "CHARACTER DEVICE" : "CHAR";

	default:
	    return fail_text;
    }
}

///////////////////////////////////////////////////////////////////////////////

enumError CreatePath ( ccp fname )
{
    TRACE("CreatePath(%s)\n",fname);

    char buf[PATH_MAX], *dest = buf;
    StringCopyS(buf,sizeof(buf),fname);

    for(;;)
    {
	// skip slashes
	while ( *dest == '/' )
	    dest++;

	// search end of current directory
	while ( *dest && *dest != '/' )
	    dest++;
	if (!*dest)
	    break;

	*dest = 0;
	if ( mkdir(buf,0777) && errno != EEXIST && !IsDirectory(buf,0) )
	{
	    noTRACE("CREATE-DIR: %s -> err=%d (ENOTDIR=%d)\n",buf,errno,ENOTDIR);
	    if ( errno == ENOTDIR )
	    {
		while ( dest > buf && *dest != '/' )
		    dest--;
		if ( dest > buf )
		    *dest = 0;
	    }
	    return ERROR1( ERR_CANT_CREATE_DIR,
		errno == ENOTDIR
			? "Not a directory: %s\n"
			: "Can't create directory: %s\n", buf );
	}
	TRACE("CREATE-DIR: %s -> OK\n",buf);
	*dest++ = '/';
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

s64 GetFileSize
(
    ccp			path1,		// NULL or part 1 of path
    ccp			path2,		// NULL or part 2 of path
    s64			not_found_val,	// return value if no regular file found
    FileAttrib_t	* fatt,		// not NULL: store file attributes
    bool		fatt_max	// true: store max values to 'fatt'
)
{
    char pathbuf[PATH_MAX];
    ccp path = PathCatPP(pathbuf,sizeof(pathbuf),path1,path2);
    TRACE("GetFileSize(%s,%lld)\n",path,not_found_val);

    struct stat st;
    if ( stat(path,&st) || !S_ISREG(st.st_mode) )
    {
	if ( fatt && !fatt_max )
	    memset(fatt,0,sizeof(*fatt));
	return not_found_val;
    }

    if (fatt)
	CopyFileAttribStat(fatt,&st,fatt_max);

    return st.st_size;
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadFile
(
    ccp			path1,		// NULL or part #1 of path
    ccp			path2,		// NULL or part #2 of path
    size_t		skip,		// skip num of bytes before reading
    void		* data,		// destination buffer, size = 'size'
    size_t		size,		// size to read
    bool		silent,		// true: suppress printing of error messages
    FileAttrib_t	* fatt,		// not NULL: store file attributes
    bool		fatt_max	// true: store max values to 'fatt'
)
{
    // [[2do]] error handling

    ASSERT(data);
    if ( fatt && !fatt_max )
	memset(fatt,0,sizeof(*fatt));

    if (!size)
	return ERR_OK;

    char pathbuf[PATH_MAX];
    ccp path = PathCatPP(pathbuf,sizeof(pathbuf),path1,path2);
    TRACE("LoadFile(%s,%zu,%zu,%d)\n",path,skip,size,silent);

    FILE * f = fopen(path,"rb");
    if (!f)
    {
	if (!silent)
	    ERROR1(ERR_CANT_OPEN,"Can't open file: %s\n",path);
	return ERR_CANT_OPEN;
    }

    if (fatt)
    {
	struct stat st;
	if (!fstat(fileno(f),&st))
	    CopyFileAttribStat(fatt,&st,fatt_max);
    }

    if ( skip > 0 )
	fseek(f,skip,SEEK_SET);

    size_t read_stat = fread(data,1,size,f);
    fclose(f);

    if ( read_stat == size )
	return ERR_OK;

    noPRINT("D=%p, s=%zu/%zu: %s\n",data,read_stat,size,path);
    if ( read_stat >= 0 && read_stat < size )
	memset((char*)data+read_stat,0,size-read_stat);

    return ERR_WARNING;
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveFile ( ccp path1, ccp path2, bool create_dir,
		     void * data, size_t size, bool silent )
{
    ASSERT(data);

    char pathbuf[PATH_MAX];
    ccp path = PathCatPP(pathbuf,sizeof(pathbuf),path1,path2);
    TRACE("SaveFile(%s,%zx,%d)\n",path,size,silent);

    FILE * f = fopen(path,"wb");
    if (!f)
    {
	if (create_dir)
	{
	    CreatePath(path);
	    f = fopen(path,"wb");
	}

	if (!f)
	{
	    if (!silent)
		ERROR1(ERR_CANT_CREATE,"Can't create file: %s\n",path);
	    return ERR_CANT_CREATE;
	}
    }

    enumError err = ERR_OK;
    size_t stat = fwrite(data,1,size,f);
    if ( stat != size )
    {
	err = ERR_WRITE_FAILED;
	if (!silent)
	    ERROR1(err,"Writo to file failed: %s\n",path);
    }
    fclose(f);

    return err;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ClearFileID
(
    File_t		* f
)
{
    DASSERT(f);
    memset( f->id6_src,  0, sizeof(f->id6_src) );
    memset( f->id6_dest, 0, sizeof(f->id6_dest) );
}

///////////////////////////////////////////////////////////////////////////////

void SetFileID
(
    File_t		* f,
    const void		* new_id,
    int			id_length
)
{
    DASSERT(f);
    DASSERT(id_length>=0);

    memset( f->id6_src, 0, sizeof(f->id6_src) );
    memcpy( f->id6_src, new_id, id_length < 6 ? id_length : 6 );
    memcpy( f->id6_dest, f->id6_src, sizeof(f->id6_dest) );
    noPRINT("$$$ SetFileID: |%s|%s|\n",f->id6_src,f->id6_dest);
}

///////////////////////////////////////////////////////////////////////////////

bool SetPatchFileID
(
    File_t		* f,
    const void		* new_id,
    int			id_length
)
{
    DASSERT(f);
    DASSERT(id_length>=0);

    memset( f->id6_src, 0, sizeof(f->id6_src) );
    memcpy( f->id6_src, new_id, id_length < 6 ? id_length : 6 );
    const bool stat = CopyPatchDiscId( f->id6_dest, f->id6_src );
    noPRINT("$$$ SetPatchFileID: |%s|%s| stat=%d\n",f->id6_src,f->id6_dest,stat);
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                  FileAttrib_t                   ///////////////
///////////////////////////////////////////////////////////////////////////////

FileAttrib_t * NormalizeFileAttrib
(
    FileAttrib_t	* fa		// valid attribute
)
{
    DASSERT(fa);

    if ( fa->size < 0 )
	fa->size = 0;

    time_t mtime = fa->mtime;
    if (!mtime)
    {
	mtime = fa->itime;
	if (!mtime)
	{
	    mtime = fa->ctime;
	    if (!mtime)
		mtime = fa->atime;
	}
    }
    fa->mtime = mtime;

    if (!fa->itime)
	fa->itime = fa->ctime > mtime ? fa->ctime : mtime;

    if (!fa->ctime)
	fa->ctime = fa->itime > mtime ? fa->itime : mtime;

    if (!fa->atime)
	fa->atime = fa->itime > fa->ctime ? fa->itime : fa->ctime;

    return fa;
}

//-----------------------------------------------------------------------------

FileAttrib_t * MaxFileAttrib
(
    FileAttrib_t	* dest,		// source and destination attribute
    const FileAttrib_t	* src		// NULL or second source attribute
)
{
    DASSERT(dest);
    if (src)
    {
	if ( dest->size < src->size )
	     dest->size = src->size;
	if ( dest->itime < src->itime )
	     dest->itime = src->itime;
	if ( dest->mtime < src->mtime )
	     dest->mtime = src->mtime;
	if ( dest->ctime < src->ctime )
	     dest->ctime = src->ctime;
	if ( dest->atime < src->atime )
	     dest->atime = src->atime;
    }
    return dest;
}

//-----------------------------------------------------------------------------

FileAttrib_t * CopyFileAttrib
(
    FileAttrib_t	* dest,		// valid destination attribute
    const FileAttrib_t	* src		// valid source attribute
)
{
    DASSERT(src);
    DASSERT(dest);

    memcpy(dest,src,sizeof(*dest));
    return NormalizeFileAttrib(dest);
}

//-----------------------------------------------------------------------------

FileAttrib_t * CopyFileAttribStat
(
    FileAttrib_t	* dest,		// valid destination attribute
    const struct stat	* src,		// NULL or source
    bool		maximize	// true store max values to 'dest'
)
{
    DASSERT(dest);

    if (src)
    {
	FileAttrib_t temp_fatt;
	FileAttrib_t * fatt = maximize ? &temp_fatt : dest;

	memset(fatt,0,sizeof(*fatt));
	fatt->size = src->st_size;

	if ( S_ISREG(src->st_mode) )
	{
	    fatt->mtime = src->st_mtime;
	    fatt->ctime = src->st_ctime;
	    fatt->atime = src->st_atime;
	}
	NormalizeFileAttrib(fatt);

	if (maximize)
	    MaxFileAttrib(dest,fatt);
    }
    else if (!maximize)
	memset(dest,0,sizeof(*dest));

    return dest;
}

//-----------------------------------------------------------------------------

FileAttrib_t * CopyFileAttribInode
	( FileAttrib_t * dest, const struct wbfs_inode_info_t * src, off_t size )
{
    ASSERT(src);
    ASSERT(dest);

    dest->size  = size;
    if (wbfs_is_inode_info_valid(0,src))
    {
	dest->itime = ntoh64(src->itime);
	dest->mtime = ntoh64(src->mtime);
	dest->ctime = ntoh64(src->ctime);
	dest->atime = ntoh64(src->atime);
    }

    return NormalizeFileAttrib(dest);
}

//-----------------------------------------------------------------------------

FileAttrib_t * CopyFileAttribDiscInfo
	( FileAttrib_t * dest, const struct WDiscInfo_t * src )
{
    ASSERT(src);
    ASSERT(dest);

    return CopyFileAttribInode(dest,&src->dhead.iinfo,src->size);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			split file support		///////////////
///////////////////////////////////////////////////////////////////////////////

int CalcSplitFilename ( char * buf, size_t buf_size, ccp path, enumOFT oft )
{
    const int needed_space = oft == OFT_WBFS ? 6 : 7;
    const int max_path_len = PATH_MAX - needed_space;

    if (!path)
	path = "";
    TRACE("CalcSplitFilename(%s,%d)\n",path,oft);

    size_t plen = strlen(path);
    if ( plen > 0 && oft == OFT_WBFS )
	plen--;
    if ( plen > max_path_len )
	plen = max_path_len;
    ccp path_end = path + plen;

    char * dest = buf;
    if ( buf_size > needed_space )
    {
	char * end = dest + buf_size - needed_space;
	while ( dest < end && path < path_end )
	{
	    if ( *path == '%' )
	    {
		*dest++ = '%';
		*dest++ = '%';
		path++;
	    }
	    else
		*dest++	= *path++;
	}

	if ( oft != OFT_WBFS )
	    *dest++ = '.';
	*dest++ = '%';
	*dest++ = '0';
	*dest++ = '1';
	*dest++ = 'u';
    }
    *dest = 0;
    return dest-buf;
}

//-----------------------------------------------------------------------------

char * AllocSplitFilename ( ccp path, enumOFT oft )
{
    char buf[2*PATH_MAX];
    CalcSplitFilename(buf,sizeof(buf),path,oft);
    return STRDUP(buf);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			GetFileSystemMap()		///////////////
///////////////////////////////////////////////////////////////////////////////
#if HAVE_FIEMAP
///////////////////////////////////////////////////////////////////////////////

 static enumError GetFileMapHelper_FIEMAP
 (
    FileMap_t		* fm,		// file map
    File_t		* file,		// file to analyze
    struct fiemap	* fmap,		// valid fiemap structure
    uint		n_elem		// number of 'fmap->fm_extents' elements
 )
 {
    DASSERT(fm);
    DASSERT(file);
    DASSERT(fmap);
    DASSERT(n_elem>0);

    memset(fmap,0,sizeof(fmap));

    u64 last_off = 0;
    while ( last_off < file->st.st_size )
    {
	fmap->fm_start	  = last_off;
	fmap->fm_length	  = file->st.st_size - last_off;
	fmap->fm_flags	  = FIEMAP_FLAG_SYNC;
	fmap->fm_extent_count = n_elem;
	int stat = ioctl(file->fd,FS_IOC_FIEMAP,fmap);
	if ( stat < 0 )
	    return ERROR1(ERR_READ_FAILED,
		    "Can't read file mapping (FIEMAP): %s\n",file->fname);

	PRINT("STAT=%d, N=%d\n",stat,fmap->fm_mapped_extents);
	if (!fmap->fm_mapped_extents)
	    break;

	const struct fiemap_extent *fe = fmap->fm_extents;
	const struct fiemap_extent *fe_end = fe + fmap->fm_mapped_extents;
	for ( ; fe < fe_end; fe++ )
	{
	    AppendFileMap( fm, fe->fe_logical + file->split_off,
				fe->fe_physical, fe->fe_length);
	    noPRINT("   -> %9llx %9llx %9llx\n",
			fe->fe_logical, fe->fe_physical, fe->fe_length );
	}
	if ( fe[-1].fe_flags & FIEMAP_EXTENT_LAST )
	    break;
	last_off = fe[-1].fe_logical + fe[-1].fe_length;
    }

    return ERR_OK;
 }

 //////////////////////////////////////////////////////////////////////////////

 static enumError GetFileSystemMap_FIEMAP
 (
    FileMap_t		* fm,		// file map
    File_t		* file		// file to analyze
 )
 {
    DASSERT(fm);
    DASSERT(file);

    struct fiemap *fmap;
    const uint MAX_ELEM = 1024;
    const uint ALLOC_SIZE = sizeof(*fmap) + sizeof(*fmap->fm_extents) * MAX_ELEM;
    PRINT("ALLOC FIEMAP: %zu + %zu * %u = %u\n",
		sizeof(*fmap), sizeof(*fmap->fm_extents), MAX_ELEM, ALLOC_SIZE );
    fmap = MALLOC(ALLOC_SIZE);

    enumError err = ERR_OK;
    if (file->split_f)
    {
	File_t **end, **ptr = file->split_f;
	for ( end = ptr + file->split_used; err == ERR_OK && ptr < end; ptr++ )
	    err = GetFileMapHelper_FIEMAP(fm,*ptr,fmap,MAX_ELEM);
    }
    else
	err = GetFileMapHelper_FIEMAP(fm,file,fmap,MAX_ELEM);

    FREE(fmap);
    return err;
 }

///////////////////////////////////////////////////////////////////////////////
#endif // HAVE_FIEMAP
///////////////////////////////////////////////////////////////////////////////
#if HAVE_FIBMAP
///////////////////////////////////////////////////////////////////////////////

 static enumError GetFileSystemMap_FIBMAP
 (
    FileMap_t		* fm,		// file map
    File_t		* file		// file to analyze
 )
 {
    DASSERT(fm);
    DASSERT(file);

    if (file->split_f)
    {
	File_t **end, **ptr = file->split_f;
	for ( end = ptr + file->split_used; ptr < end; ptr++ )
	{
	    enumError err = GetFileSystemMap_FIBMAP(fm,*ptr);
	    if (err)
		return err;
	}
	return ERR_OK;
    }

    int blocksize;
    if ( ioctl(file->fd,FIGETBSZ,&blocksize) < 0 )
	return ERROR1(ERR_READ_FAILED,
		"Can't read block size (FIGETBSZ): %s\n",file->fname);

    uint block, n_blocks = ( file->st.st_size + blocksize - 1 ) / blocksize;
    PRINT("FIBMAP: %u blocks * %u bytes\n",n_blocks,blocksize);

    for ( block = 0; block < n_blocks; block++ )
    {
	uint param = block;
	if ( ioctl(file->fd,FIBMAP,&param) < 0 )
	    return ERROR1(ERR_READ_FAILED,
		"Can't read file mapping (FIBMAP): %s\n",file->fname);

	AppendFileMap( fm, block * (u64)blocksize + file->split_off,
				param * (u64)blocksize, blocksize );
    }

    return ERR_OK;
 }

///////////////////////////////////////////////////////////////////////////////
#endif // HAVE_FIBMAP
///////////////////////////////////////////////////////////////////////////////

bool HaveFileSystemMapSupport()
{
 #if HAVE_FIEMAP || HAVE_FIBMAP
    return true;
 #else
    return false;
 #endif
}

///////////////////////////////////////////////////////////////////////////////

enumError GetFileSystemMap
(
    FileMap_t		* fm,		// file map
    bool		init_fm,	// true: initialize 'fm', false: reset 'fm'
    File_t		* file		// file to analyze
)
{
    DASSERT(fm);
    DASSERT(file);
    if (init_fm)
	InitializeFileMap(fm);
    else
	ResetFileMap(fm);

    enumError stat = ERR_JOB_IGNORED;
    if (S_ISREG(file->st.st_mode))
    {

     #if HAVE_FIEMAP

	// first, we try FIEMAP
	stat = GetFileSystemMap_FIEMAP(fm,file);
	if (!stat)
	    return ERR_OK;
     #endif

     #if HAVE_FIBMAP

	// if failed, we try FIBMAP
	stat = GetFileSystemMap_FIBMAP(fm,file);
	if (!stat)
	    return ERR_OK;
     #endif

    }
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			cygwin support			///////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef __CYGWIN__

 int IsWindowsDriveSpec ( ccp src )
 {
    if ( ( *src >= 'a' && *src <= 'z' || *src >= 'A' && *src <= 'Z' )
	&& src[1] == ':' )
    {
	if (!src[2])
	    return 2;

	if ( src[2] == '/' || src[2] == '\\' )
	    return 3;
    }
    return 0;
 }

#endif // __CYGWIN__

///////////////////////////////////////////////////////////////////////////////

#ifdef __CYGWIN__

 int NormalizeFilenameCygwin ( char * buf, size_t bufsize, ccp src )
 {
    static char prefix[] = "/cygdrive/";

    if ( bufsize < sizeof(prefix) + 5 )
    {
	*buf = 0;
	return 0;
    }

    char * end = buf + bufsize - 1;
    char * dest = buf;

    if (   ( *src >= 'a' && *src <= 'z' || *src >= 'A' && *src <= 'Z' )
	&& src[1] == ':'
	&& ( src[2] == 0 || src[2] == '/' || src[2] == '\\' ))
    {
	memcpy(buf,prefix,sizeof(prefix));
	dest = buf + sizeof(prefix)-1;
	*dest++ = tolower((int)*src); // cygwin needs the '(int)'
	*dest = 0;
	if (IsDirectory(buf,false))
	{
	    *dest++ = '/';
	    src += 2;
	    if (*src)
		src++;
	}
	else
	    dest = buf;
    }
    ASSERT( dest < buf + bufsize );

    while ( dest < end && *src )
	if ( *src == '\\' )
	{
	    *dest++ = '/';
	    src++;
	}
	else
	    *dest++ = *src++;

    *dest = 0;
    ASSERT( dest < buf + bufsize );
    return dest - buf;
 }

#endif // __CYGWIN__

///////////////////////////////////////////////////////////////////////////////

#ifdef __CYGWIN__

 char * AllocNormalizedFilenameCygwin ( ccp source )
 {
    char buf[PATH_MAX];
    const int len = NormalizeFilenameCygwin(buf,sizeof(buf),source);
    char * result = MALLOC(len+1);
    memcpy(result,buf,len+1);
    ASSERT(buf[len]==0);
    return result;
 }

#endif // __CYGWIN__

///////////////////////////////////////////////////////////////////////////////

void SetDest ( ccp dest, bool mkdir )
{
 #ifdef __CYGWIN__
    opt_dest = IsWindowsDriveSpec(dest)
		? AllocNormalizedFilenameCygwin(dest)
		: dest;
 #else
    opt_dest = dest;
 #endif
    opt_mkdir = mkdir;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			cert support			///////////////
///////////////////////////////////////////////////////////////////////////////

int AddCertFile ( ccp fname, int unused )
{
    SuperFile_t sf;
    InitializeSF(&sf);
    enumError err = OpenSF(&sf,fname,true,false);
    if (!err)
    {
	if ( sf.f.ftype & FT_A_ISO )
	{
	    wd_disc_t * disc = OpenDiscSF(&sf,false,false);
	    if (disc)
	    {
		int ip;
		for ( ip = 0; ip < disc->n_part; ip++ )
		{
		    wd_part_t * part = disc->part + ip;
		    if ( part->is_enabled
			&& !wd_load_part(part,true,false,true)
			&& part->cert )
		    {
			cert_append_data(&global_cert,part->cert,part->ph.cert_size,true);
		    }
		}
	    }
	}
// [[2do]] [[ft-id]]
	else if ( sf.f.ftype & (FT_ID_CERT_BIN|FT_ID_TIK_BIN|FT_ID_TMD_BIN) )
	{
	    const size_t load_size = sf.file_size < sizeof(iobuf)
				   ? sf.file_size : sizeof(iobuf);
	    err = ReadSF(&sf,0,iobuf,load_size);
	    if (!err)
	    {
		size_t skip = 0;
		if ( sf.f.ftype & FT_ID_TIK_BIN )
		    skip = sizeof(wd_ticket_t);
		else if ( sf.f.ftype & FT_ID_TMD_BIN )
		{
		    wd_tmd_t * tmd = (wd_tmd_t*)iobuf;
		    skip = sizeof(wd_tmd_t)
			 + ntohs(tmd->n_content) * sizeof(wd_tmd_content_t);
		}

		if ( skip < load_size )
		    cert_append_data(&global_cert,iobuf+skip,load_size-skip,true);
	    }
	}
    }

    ResetSF(&sf,0);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

