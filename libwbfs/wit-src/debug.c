
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

#define WIT_DEBUG_C 1
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lib-std.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 trace functions		///////////////
///////////////////////////////////////////////////////////////////////////////

FILE * TRACE_FILE = 0;

unsigned GetTimerMSec();

///////////////////////////////////////////////////////////////////////////////

static void trace_helper ( int print_stderr, const char * format, va_list arg )
{
    if (!TRACE_FILE)
    {
	TRACE_FILE = fopen("_trace.tmp","wb");
	if (!TRACE_FILE)
	    TRACE_FILE = stderr;
    }

    unsigned msec = GetTimerMSec();

    if ( print_stderr || TRACE_FILE == stderr )
    {
	fflush(stdout);
	fprintf(stderr,"%4d.%03d  ",msec/1000,msec%1000);
	va_list arg2;
	va_copy(arg2,arg);
	vfprintf(stderr,format,arg2);
	va_end(arg2);
	fflush(stderr);
    }

    if ( TRACE_FILE != stderr )
    {
	fprintf(TRACE_FILE,"%4d.%03d  ",msec/1000,msec%1000);
	vfprintf(TRACE_FILE,format,arg);
	fflush(TRACE_FILE);
    }
}

///////////////////////////////////////////////////////////////////////////////

#undef TRACE_ARG_FUNC

void TRACE_ARG_FUNC ( const char * format, va_list arg )
{
    trace_helper(0,format,arg);
}

///////////////////////////////////////////////////////////////////////////////

#undef TRACE_FUNC

void TRACE_FUNC ( const char * format, ... )
{
    va_list arg;
    va_start(arg,format);
    trace_helper(0,format,arg);
    va_end(arg);
}

///////////////////////////////////////////////////////////////////////////////

#undef PRINT_ARG_FUNC

void PRINT_ARG_FUNC ( const char * format, va_list arg )
{
    trace_helper(1,format,arg);
}

///////////////////////////////////////////////////////////////////////////////

#undef PRINT_FUNC

void PRINT_FUNC ( const char * format, ... )
{
    va_list arg;
    va_start(arg,format);
    trace_helper(1,format,arg);
    va_end(arg);
}

///////////////////////////////////////////////////////////////////////////////

#undef WAIT_ARG_FUNC

void WAIT_ARG_FUNC ( const char * format, va_list arg )
{
    if ( format && *format )
	trace_helper(1,format,arg);
    PRINT_FUNC(">>>>>> PRESS RETURN: ");
    getchar();
}

///////////////////////////////////////////////////////////////////////////////

#undef WAIT_FUNC

void WAIT_FUNC ( const char * format, ... )
{
    va_list arg;
    va_start(arg,format);
    WAIT_ARG_FUNC(format,arg);
    va_end(arg);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			alloc/free system		///////////////
///////////////////////////////////////////////////////////////////////////////

#if TRACE_ALLOC_MODE > 1
    #define MEM_FILLER_SIZE 8
    static u8 mem_filler[MEM_FILLER_SIZE];
#else
    #define MEM_FILLER_SIZE 0
#endif

///////////////////////////////////////////////////////////////////////////////

void my_free ( void * ptr )
{
    free(ptr);
}

///////////////////////////////////////////////////////////////////////////////

void * my_calloc  ( size_t nmemb, size_t size )
{
    void * res = calloc(nmemb,size);
    if (!res)
	PrintError(__FUNCTION__,__FILE__,__LINE__,0,ERR_OUT_OF_MEMORY,
		"Out of memory while calloc() %zu bytes (%zu*%zu=0x%zx)\n",
		nmemb*size, nmemb, size, nmemb*size );
    return res;
}

///////////////////////////////////////////////////////////////////////////////

void * my_malloc ( size_t size )
{
    void * res = malloc(size);
    if (!res)
	PrintError(__FUNCTION__,__FILE__,__LINE__,0,ERR_OUT_OF_MEMORY,
		"Out of memory while malloc() %zu bytes (0x%zx)\n",
		size, size );
    return res;
}

///////////////////////////////////////////////////////////////////////////////

void * my_realloc ( void * ptr, size_t size )
{
    void * res = realloc(ptr,size);
    if (!res)
	PrintError(__FUNCTION__,__FILE__,__LINE__,0,ERR_OUT_OF_MEMORY,
		"Out of memory while realloc() %zu bytes (0x%zx)\n",
		size, size );
    return res;
}

///////////////////////////////////////////////////////////////////////////////

char * my_strdup  ( ccp src )
{
    char * res = strdup(src);
    if (!res)
    {
	const uint size = src ? strlen(src)+1 : 0;
	PrintError(__FUNCTION__,__FILE__,__LINE__,0,ERR_OUT_OF_MEMORY,
		"Out of memory while strdup() %u bytes (0x%x)\n",
		size, size );
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////

char * my_strdup2 ( ccp src1, ccp src2 )
{
    const uint len1 = src1 ? strlen(src1) : 0;
    const uint len2 = src2 ? strlen(src2) : 0;
    char * res = my_malloc(len1+len2+1);
    if (len1)
	memcpy(res,src1,len1);
    if (len2)
	memcpy(res+len1,src2,len2);
    res[len1+len2] = 0;
    return res;
}

///////////////////////////////////////////////////////////////////////////////

void * my_memdup ( const void * src, size_t copylen )
{
    char * dest = my_malloc(copylen+1);
    memcpy(dest,src,copylen);
    dest[copylen] = 0;
    return dest;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if TRACE_ALLOC_MODE > 1
///////////////////////////////////////////////////////////////////////////////

typedef struct mem_info_t
{
    ccp		func;
    ccp		file;
    uint	line;
    u8		* data;
    uint	size;
    uint	seqnum;

} mem_info_t;

//-----------------------------------------------------------------------------

static mem_info_t * mem_list = 0;
static uint  mem_used	= 0;
static uint  mem_size	= 0;
static FILE *mem_log	= 0;
static uint  mem_seqnum	= 0;

///////////////////////////////////////////////////////////////////////////////

static FILE * OpenMemLog()
{
    static bool done = false;
    if (!done)
    {
	done = true;
	mem_log = fopen("_alloc-err-log.tmp","wb");
    }
    return mem_log;
}

///////////////////////////////////////////////////////////////////////////////

static void MemLog ( ccp func, ccp file, uint line, ccp info, u8 * ptr )
{
    fprintf(stderr,"MEM-ERR[%s,%p] %s @ %s#%u\n",
		info, ptr, func, file, line );
    fflush(stderr);

    if (OpenMemLog())
    {
	fprintf(mem_log,"[%s,%p] %s @ %s#%u\n\n",
		info, ptr, func, file, line );
	fflush(mem_log);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void MemLogItem ( ccp func, ccp file, uint line, ccp info, const mem_info_t * mi )
{
    if (func)
	fprintf(stderr,"MEM-ERR[%s] %s @ %s#%u\n",
		info, func, file, line );
    fprintf(stderr,"MEM-ERR[%s,%u] %s @ %s#%u\n",
		info, mi->seqnum, mi->func, mi->file, mi->line );
    fflush(stderr);

    if (OpenMemLog())
    {
	if (func)
	    fprintf(mem_log,"[%s] %s @ %s#%u\n",
		    info, func, file, line );
	fprintf(mem_log,"[%s,%u] %s @ %s#%u\n",
		    info, mi->seqnum, mi->func, mi->file, mi->line );
	HexDump16( mem_log, 10, 0, mi->data - MEM_FILLER_SIZE, 2*MEM_FILLER_SIZE );
	HexDump16( mem_log, 10, mi->size + MEM_FILLER_SIZE,
			mi->data + mi->size - MEM_FILLER_SIZE, 2 * MEM_FILLER_SIZE );
	fputc('\n',mem_log);
	fflush(mem_log);
    }
}

///////////////////////////////////////////////////////////////////////////////

uint FindMemInfoHelper ( u8 * data, uint size )
{
    int beg = 0;
    int end = mem_used - 1;
    while ( beg <= end )
    {
	uint idx = (beg+end)/2;
	mem_info_t * mi = mem_list + idx;
	if ( data < mi->data )
	    end = idx - 1 ;
	else if ( data > mi->data )
	    beg = idx + 1;
	else if ( size < mi->size )
	    end = idx - 1 ;
	else if ( size > mi->size )
	    beg = idx + 1;
	else
	{
	    noTRACE("FindMemMapHelper(%llx,%llx) FOUND=%d/%d/%d\n",
		    (u64)off, (u64)size, idx, mem_used, mem_size );
	    return idx;
	}
    }

    noTRACE("FindStringFieldHelper(%llx,%llx) failed=%d/%d/%d\n",
		(u64)off, (u64)size, beg, mem_used, mem_size );
    return beg;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeTraceAlloc()
{
    static bool done = false;
    if (done)
	return;

    OpenMemLog();

    memset(mem_filler,0xdc,sizeof(mem_filler));
    mem_filler[MEM_FILLER_SIZE/2] = 0xcd;
    mem_filler[MEM_FILLER_SIZE/3] = 0xcd;
}

///////////////////////////////////////////////////////////////////////////////

int CheckTraceAlloc ( ccp func, ccp file, uint line )
{
    int count = 0;

    mem_info_t *ptr, *end = mem_list + mem_used;
    for ( ptr = mem_list; ptr < end; ptr++ )
    {
	if (memcmp(ptr->data-MEM_FILLER_SIZE,mem_filler,sizeof(mem_filler)))
	{
	    MemLogItem(0,0,0,"BEG",ptr);
	    memcpy(ptr->data-MEM_FILLER_SIZE,mem_filler,sizeof(mem_filler));
	    count++;
	}

	if (memcmp(ptr->data+ptr->size,mem_filler,sizeof(mem_filler)))
	{
	    MemLogItem(0,0,0,"END",ptr);
	    memcpy(ptr->data+ptr->size,mem_filler,sizeof(mem_filler));
	    count++;
	}
    }

    if (count)
    {
	fprintf(stderr,"MEM-ERR: %u errors found -> %s @ %s#%u\n",
		count, func, file, line );
	fflush(stderr);

	if (OpenMemLog())
	{
	    fprintf(mem_log,"--- %u errors found -> %s @ %s#%u\n\n",
		count, func, file, line );
	    fflush(mem_log);
	}
    }
    return count;
}

///////////////////////////////////////////////////////////////////////////////

void DumpTraceAlloc ( ccp func, ccp file, uint line, FILE * f )
{
    CheckTraceAlloc(func,file,line);

    if (!f)
	return;

    fprintf(f,"---- MEM-DUMP [N=%u] -----\n",mem_used);
    mem_info_t *ptr, *end = mem_list + mem_used;
    for ( ptr = mem_list; ptr < end; ptr++ )
    {
	ccp file = strrchr(ptr->file,'/');
	file = file ? file+1 : ptr->file;
	fprintf(f,"%5u %p %5u %-20.20s %5u %-.25s\n",
		ptr->seqnum, ptr->data, ptr->size,
		ptr->func, ptr->line, file );
    }
    fflush(stderr);
}

///////////////////////////////////////////////////////////////////////////////

static mem_info_t * RegisterAlloc
	( ccp func, ccp file, uint line, u8 * data, uint size )
{
    if (! mem_seqnum)
	InitializeTraceAlloc();

    uint idx = FindMemInfoHelper(data,size);

    ASSERT( mem_used <= mem_size );
    if ( mem_used == mem_size )
    {
	mem_size += 1000 + mem_size/2;
	const uint alloc_size = mem_size * sizeof(*mem_list);
	mem_list = realloc(mem_list,alloc_size);
	if (!mem_list)
	    PrintError(__FUNCTION__,__FILE__,__LINE__,0,ERR_OUT_OF_MEMORY,
		"Out of memory while RegisterAlloc() %u bytes (0x%x)\n",
		alloc_size, alloc_size );
    }

    DASSERT( idx <= mem_used );
    mem_info_t * mi = mem_list + idx;
    memmove(mi+1,mi,(mem_used-idx)*sizeof(*mi));
    mem_used++;

    mi->func	= func;
    mi->file	= file;
    mi->line	= line;
    mi->data	= data;
    mi->size	= size;
    mi->seqnum	= mem_seqnum++;
    return mi;
}

//
///////////////////////////////////////////////////////////////////////////////
#endif // TRACE_ALLOC_MODE > 1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void * UnregisterAlloc ( ccp func, ccp file, uint line, u8 * data )
{
 #if TRACE_ALLOC_MODE > 1
    if (!data)
	return 0;

    int beg = 0;
    int end = mem_used - 1;
    while ( beg <= end )
    {
	uint idx = (beg+end)/2;
	mem_info_t * mi = mem_list + idx;
	if ( data < mi->data )
	    end = idx - 1 ;
	else if ( data > mi->data )
	    beg = idx + 1;
	else
	{
	    if (memcmp(data-MEM_FILLER_SIZE,mem_filler,sizeof(mem_filler)))
		MemLogItem(func,file,line,"BEG",mi);

	    if (memcmp(data+mi->size,mem_filler,sizeof(mem_filler)))
		MemLogItem(func,file,line,"END",mi);

	    memmove(mi,mi+1,(--mem_used-idx)*sizeof(*mi));
	    return data - MEM_FILLER_SIZE;
	}
    }
    MemLog(func,file,line,"NOT FOUND",data);
 #endif
    return data;
}

///////////////////////////////////////////////////////////////////////////////

void trace_free ( ccp func, ccp file, uint line, void * ptr )
{
    free(UnregisterAlloc(func,file,line,ptr));
}

///////////////////////////////////////////////////////////////////////////////

void * trace_malloc  ( ccp func, ccp file, uint line, size_t size )
{
    u8 * res = malloc( size + 2 * MEM_FILLER_SIZE );
    if (!res)
	PrintError(func,file,line,0,ERR_OUT_OF_MEMORY,
		"Out of memory while allocate %zu+%u bytes (0x%zx)\n",
		size, 2 * MEM_FILLER_SIZE, size + 2 * MEM_FILLER_SIZE );

 #if TRACE_ALLOC_MODE > 1
    memcpy(res,mem_filler,MEM_FILLER_SIZE);
    res += MEM_FILLER_SIZE;
    memcpy(res+size,mem_filler,MEM_FILLER_SIZE);
    RegisterAlloc(func,file,line,res,size);
 #endif

    return res;
}

///////////////////////////////////////////////////////////////////////////////

void * trace_calloc  ( ccp func, ccp file, uint line, size_t nmemb, size_t size )
{
    uint total_size = nmemb * size;
    void * res = trace_malloc(func,file,line,total_size);
    memset(res,0,total_size);
    return res;
}

///////////////////////////////////////////////////////////////////////////////

void * trace_realloc ( ccp func, ccp file, uint line, void *ptr, size_t size )
{
    ptr = UnregisterAlloc(func,file,line,ptr);
    void * res = realloc( ptr, size + 2 * MEM_FILLER_SIZE );
    if (!res)
	PrintError(func,file,line,0,ERR_OUT_OF_MEMORY,
		"Out of memory while re allocate %zu+%u bytes (0x%zx)\n",
		size, 2 * MEM_FILLER_SIZE, size + 2 * MEM_FILLER_SIZE );

 #if TRACE_ALLOC_MODE > 1
    memcpy(res,mem_filler,MEM_FILLER_SIZE);
    res += MEM_FILLER_SIZE;
    memcpy(res+size,mem_filler,MEM_FILLER_SIZE);
    RegisterAlloc(func,file,line,res,size);
 #endif

    return res;
}

///////////////////////////////////////////////////////////////////////////////

char * trace_strdup ( ccp func, ccp file, uint line, ccp src )
{
    const uint len = src ? strlen(src)+1 : 0;
    char * res = trace_malloc(func,file,line,len);
    memcpy(res,src,len);
    return res;
}

///////////////////////////////////////////////////////////////////////////////

char * trace_strdup2 ( ccp func, ccp file, uint line, ccp src1, ccp src2 )
{
    const uint len1 = src1 ? strlen(src1) : 0;
    const uint len2 = src2 ? strlen(src2) : 0;
    char * res = trace_malloc(func,file,line,len1+len2+1);
    if (len1)
	memcpy(res,src1,len1);
    if (len2)
	memcpy(res+len1,src2,len2);
    res[len1+len2] = 0;
    return res;
}

///////////////////////////////////////////////////////////////////////////////

void * trace_memdup ( ccp func, ccp file, uint line, const void * src, size_t copylen )
{
    char * dest = trace_malloc(func,file,line,copylen+1);
    memcpy(dest,src,copylen);
    dest[copylen] = 0;
    return dest;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 mem check			///////////////
///////////////////////////////////////////////////////////////////////////////

static u8	    mem_check_buf[0x100];
static const void * mem_check_ptr = 0;
static uint	    mem_check_size = 0;

///////////////////////////////////////////////////////////////////////////////

void MemCheckSetup ( const void * ptr, uint size )
{
    if ( ptr && size > 0 )
    {
	mem_check_ptr = ptr;
	if ( size > sizeof(mem_check_buf) )
	{
	    ERROR0(ERR_WARNING,
		"MemCheckSetup(), max watch size = %zx (<%x)",
		sizeof(mem_check_buf), size );
	    size = sizeof(mem_check_buf);
	}
	mem_check_size = size;
	memcpy(mem_check_buf,ptr,size);
    }
    else
	mem_check_size = 0;
}

///////////////////////////////////////////////////////////////////////////////

void MemCheck ( ccp func, ccp file, uint line )
{
    if (!mem_check_size)
	return;

    if ( memcmp(mem_check_buf,mem_check_ptr,mem_check_size) )
    {
	fprintf(stderr,"--- MemCheck: %p should be:\n",mem_check_ptr);
	HexDump16(stderr,0,0,mem_check_buf,mem_check_size);
	fprintf(stderr,"--- MemCheck: ... but is:\n");
	HexDump16(stderr,0,0,mem_check_ptr,mem_check_size);

	TRACE("--- MemCheck: %p should be:\n",mem_check_ptr);
	TRACE_HEXDUMP16(0,0,mem_check_buf,mem_check_size);
	TRACE("--- MemCheck: ... but is:\n");
	TRACE_HEXDUMP16(0,0,mem_check_ptr,mem_check_size);

	PrintError(func,file,line,0,ERR_FATAL,"MemCheck() failed!\n");
    }
    else
	TRACE("MemCheck OK: %s @ %s#%u\n",func,file,line);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////				END			///////////////
///////////////////////////////////////////////////////////////////////////////
