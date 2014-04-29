
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

#ifndef WIT_DEBUG_H
#define WIT_DEBUG_H 1

//
///////////////////////////////////////////////////////////////////////////////
///////////////			DEBUG and TRACING		///////////////
///////////////////////////////////////////////////////////////////////////////
//
//  -------------------
//   DEBUG and TRACING
//  -------------------
//
//  If symbol 'DEBUG' or symbol _DEBUG' is defined, then and only then
//  DEBUG and TRACING is enabled.
//
//  There are to function like macros defined:
//
//     TRACE ( const char * format, ... );
//        Print to console only if TRACING is enabled.
//        Ignored when TRACING is disabled.
//        It works like the well known printf() function and include flushing.
//
//      TRACE_IF ( bool condition, const char * format, ... );
//        Like TRACE(), but print only if 'condition' is true.
//
//      TRACELINE
//        Print out current line and source.
//
//      TRACE_SIZEOF ( object_or_type );
//        Print out `sizeof(object_or_type)´
//
//      ASSERT(cond);
//	  If condition is false: print info and exit program.
//
//
//  There are more macros with a preceding 'no' defined:
//
//      noTRACE ( const char * format, ... );
//      noTRACE_IF ( bool condition, const char * format, ... );
//      noTRACELINE ( bool condition, const char * format, ... );
//      noTRACE_SIZEOF ( object_or_type );
//      noASSERT(cond);
//
//      If you add a 'no' before a TRACE-Call it is disabled always.
//      This makes the usage and disabling of multi lines traces very easy.
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdarg.h>

///////////////////////////////////////////////////////////////////////////////

#if defined(RELEASE)
    #undef TESTTRACE
    #undef DEBUG
    #undef _DEBUG
    #undef DEBUG_ASSERT
    #undef _DEBUG_ASSERT
    #undef WAIT_ENABLED
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined(IGNORE_DEBUG)
    #undef DEBUG
    #undef _DEBUG
#endif

///////////////////////////////////////////////////////////////////////////////

#undef TRACE
#undef TRACE_IF
#undef TRACELINE
#undef TRACE_SIZEOF

///////////////////////////////////////////////////////////////////////////////

extern FILE * TRACE_FILE;
void TRACE_FUNC ( const char * format, ...)
	__attribute__ ((__format__(__printf__,1,2)));
void TRACE_ARG_FUNC ( const char * format, va_list arg );

void PRINT_FUNC ( const char * format, ...)
	__attribute__ ((__format__(__printf__,1,2)));
void PRINT_ARG_FUNC ( const char * format, va_list arg );

void WAIT_FUNC ( const char * format, ...)
	__attribute__ ((__format__(__printf__,1,2)));
void WAIT_ARG_FUNC ( const char * format, va_list arg );

///////////////////////////////////////////////////////////////////////////////

#undef TEST0 // never defined

#ifdef TESTTRACE

    #undef DEBUG
    #define DEBUG 1

    #undef TEST
    #define TEST 1

    #undef WAIT_ENABLED
    #define WAIT_ENABLED 1

    #undef NEW_FEATURES
    #define NEW_FEATURES 1

    #define TRACE_FUNC printf
    #define PRINT_FUNC printf
    #define WAIT_FUNC  printf

    #define TRACE_ARG_FUNC vprintf
    #define PRINT_ARG_FUNC vprintf
    #define WAIT_ARG_FUNC  vprintf

#endif

///////////////////////////////////////////////////////////////////////////////

#if defined(DEBUG) || defined(_DEBUG)

    #define HAVE_TRACE 1

    #undef DEBUG
    #define DEBUG 1

    #undef DEBUG_ASSERT
    #define DEBUG_ASSERT 1

    #define TRACE(...) TRACE_FUNC(__VA_ARGS__)
    #define TRACE_IF(cond,...) if (cond) TRACE_FUNC(__VA_ARGS__)
    #define TRACELINE TRACE_FUNC("line #%d @ %s\n",__LINE__,__FILE__)
    #define TRACE_SIZEOF(t) TRACE_FUNC("%7zd ==%6zx/hex == sizeof(%s)\n",sizeof(t),sizeof(t),#t)

    #define HEXDUMP(i,a,af,rl,d,c) HexDump(stdout,i,a,af,rl,d,c);
    #define HEXDUMP16(i,a,d,c) HexDump16(stdout,i,a,d,c);
    #define TRACE_HEXDUMP(i,a,af,rl,d,c) HexDump(TRACE_FILE,i,a,af,rl,d,c);
    #define TRACE_HEXDUMP16(i,a,d,c) HexDump16(TRACE_FILE,i,a,d,c);

#else

    #define HAVE_TRACE 0

    #undef DEBUG

    #define TRACE(...)
    #define TRACE_IF(cond,...)
    #define TRACELINE
    #define TRACE_SIZEOF(t)
    #define HEXDUMP(i,a,af,rl,d,c)
    #define HEXDUMP16(a,i,d,c)
    #define TRACE_HEXDUMP(i,a,af,rl,d,c)
    #define TRACE_HEXDUMP16(i,a,d,c)

#endif

///////////////////////////////////////////////////////////////////////////////

#undef ASSERT
#undef ASSERT_MSG

#if defined(DEBUG_ASSERT) || defined(_DEBUG_ASSERT)

    #define HAVE_ASSERT 1

    #undef DEBUG_ASSERT
    #define DEBUG_ASSERT 1

    #define ASSERT(a) if (!(a)) ERROR0(ERR_FATAL,"ASSERTION FAILED !!!\n")
    #define ASSERT_MSG(a,...) if (!(a)) ERROR0(ERR_FATAL,__VA_ARGS__)

#else

    #define HAVE_ASSERT 0

    #undef DEBUG_ASSERT
    #define ASSERT(cond)
    #define ASSERT_MSG(a,...)

#endif

///////////////////////////////////////////////////////////////////////////////

#undef PRINT
#undef PRINT_IF
#undef BINGO
#undef HAVE_PRINT

#undef HAVE_PRINT0	// always false
#define HAVE_PRINT0 0

#if defined(DEBUG) && defined(TEST)

    #define HAVE_PRINT 1

    #define PRINT(...) PRINT_FUNC(__VA_ARGS__)
    #define PRINT_IF(cond,...) if (cond) PRINT_FUNC(__VA_ARGS__)
    #define BINGO PRINT_FUNC("BINGO! %s() #%d @ %s\n",__FUNCTION__,__LINE__,__FILE__)

#else

    #define HAVE_PRINT	HAVE_TRACE

    #define PRINT	TRACE
    #define PRINT_IF	TRACE_IF
    #define BINGO	TRACELINE

#endif

///////////////////////////////////////////////////////////////////////////////

#undef WAIT
#undef WAIT_IF
#undef HAVE_WAIT

#if defined(WAIT_ENABLED)

    #define HAVE_WAIT 1

    #define WAIT(...) WAIT_FUNC(__VA_ARGS__)
    #define WAIT_IF(cond,...) if (cond) WAIT_FUNC(__VA_ARGS__)

#else

    #define HAVE_WAIT 0

    #define WAIT(...)
    #define WAIT_IF(cond,...)

#endif

///////////////////////////////////////////////////////////////////////////////

#undef DASSERT
#undef DASSERT_MSG
#undef HAVE_DASSERT

#if defined(DEBUG) || defined(TEST)

    #define HAVE_DASSERT 1

    #define DASSERT ASSERT
    #define DASSERT_MSG ASSERT_MSG

#else

    #define HAVE_DASSERT 0

    #define DASSERT(cond)
    #define DASSERT_MSG(a,...)

#endif

///////////////////////////////////////////////////////////////////////////////

// prefix 'no' deactivates traces

#undef noTRACE
#undef noTRACE_IF
#undef noTRACELINE
#undef noTRACE_SIZEOF
#undef noPRINT
#undef noPRINT_IF
#undef noWAIT
#undef noWAIT_IF
#undef noASSERT


#ifdef TESTTRACE

    #define noTRACE		TRACE
    #define noTRACE_IF		TRACE_IF
    #define noTRACELINE		TRACELINE
    #define noTRACE_SIZEOF	TRACE_SIZEOF
    #define noPRINT		PRINT
    #define noPRINT_IF		PRINT_IF
    #define noWAIT		WAIT
    #define noWAIT_IF		WAIT_IF
    #define noASSERT		ASSERT
    #define noASSERT_MSG	ASSERT_MSG

#else

    #define noTRACE(...)
    #define noTRACE_IF(cond,...)
    #define noTRACELINE
    #define noTRACE_SIZEOF(t)
    #define noPRINT(...)
    #define noPRINT_IF(...)
    #define noWAIT(...)
    #define noWAIT_IF(...)
    #define noASSERT(cond)
    #define noASSERT_MSG(cond,...)

#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			alloc/free system		///////////////
///////////////////////////////////////////////////////////////////////////////

// define TRACE_ALLOC_MODE
//
//  0: standard malloc with out of memory detection
//  1: like mode #0, but with source identification on out of memory
//  2: like mode #1, but alloc debuging enabled

// define ENABLE_MEM_CHECK
//  0: disabled
//  1: enable a watch point for a memory location

#ifdef TEST
    #define TRACE_ALLOC_MODE 2
    #define ENABLE_MEM_CHECK 0
#elif DEBUG
    #define TRACE_ALLOC_MODE 1
    #define ENABLE_MEM_CHECK 0
#else
    #define TRACE_ALLOC_MODE 1
    #define ENABLE_MEM_CHECK 0
#endif

///////////////////////////////////////////////////////////////////////////////

#if ENABLE_MEM_CHECK
    extern void MemCheckSetup ( const void * ptr, unsigned int size );
    extern void MemCheck ( const char * func, const char * file, unsigned int line );
    #define MEM_CHECK_SETUP(p,s) MemCheckSetup(p,s)
    #define MEM_CHECK MemCheck(__FUNCTION__,__FILE__,__LINE__)
#else
    #define MEM_CHECK_SETUP(p)
    #define MEM_CHECK
#endif

///////////////////////////////////////////////////////////////////////////////

void   my_free    ( void * ptr );
void * my_calloc  ( size_t nmemb, size_t size );
void * my_malloc  ( size_t size );
void * my_realloc ( void * ptr, size_t size );
char * my_strdup  ( const char * src );
char * my_strdup2 ( const char * src1, const char * src2 );
void * my_memdup  ( const void * src, size_t copylen );

void   trace_free    ( const char * func, const char * file, unsigned int line,
			void * ptr );
void * trace_calloc  ( const char * func, const char * file, unsigned int line,
			size_t nmemb, size_t size );
void * trace_malloc  ( const char * func, const char * file, unsigned int line,
			size_t size );
void * trace_realloc ( const char * func, const char * file, unsigned int line,
			void *ptr, size_t size );
char * trace_strdup  ( const char * func, const char * file, unsigned int line,
			const char * src );
char * trace_strdup2 ( const char * func, const char * file, unsigned int line,
			const char * src1, const char * src2 );
void * trace_memdup ( const char * func, const char * file, unsigned int line,
			const void * src, size_t copylen );

#if TRACE_ALLOC_MODE > 1
    void InitializeTraceAlloc();
    int  CheckTraceAlloc ( const char * func, const char * file, unsigned int line );
    void DumpTraceAlloc ( const char * func, const char * file, unsigned int line, FILE * f );
#endif

///////////////////////////////////////////////////////////////////////////////

#if TRACE_ALLOC_MODE > 1
    #define FREE(ptr) trace_free(__FUNCTION__,__FILE__,__LINE__,ptr)
    #define CALLOC(nmemb,size) trace_calloc(__FUNCTION__,__FILE__,__LINE__,nmemb,size)
    #define MALLOC(size) trace_malloc(__FUNCTION__,__FILE__,__LINE__,size)
    #define REALLOC(ptr,size) trace_realloc(__FUNCTION__,__FILE__,__LINE__,ptr,size)
    #define STRDUP(src) trace_strdup(__FUNCTION__,__FILE__,__LINE__,src)
    #define STRDUP2(src1,src2) trace_strdup2(__FUNCTION__,__FILE__,__LINE__,src1,src2)
    #define MEMDUP(src,size) trace_memdup(__FUNCTION__,__FILE__,__LINE__,src,size)
    #define INIT_TRACE_ALLOC  InitializeTraceAlloc()
    #define CHECK_TRACE_ALLOC CheckTraceAlloc(__FUNCTION__,__FILE__,__LINE__)
    #define DUMP_TRACE_ALLOC(f)  DumpTraceAlloc(__FUNCTION__,__FILE__,__LINE__,f)
#else
    #define FREE(ptr) my_free(ptr)
    #define CALLOC(nmemb,size) my_calloc(nmemb,size)
    #define MALLOC(size) my_malloc(size)
    #define REALLOC(ptr,size) my_realloc(ptr,size)
    #define STRDUP(src) my_strdup(src)
    #define STRDUP2(src1,src2) my_strdup2(src1,src2)
    #define MEMDUP(src,size) my_memdup(src,size)
    #define INIT_TRACE_ALLOC
    #define CHECK_TRACE_ALLOC
    #define DUMP_TRACE_ALLOC(f)
#endif

#ifndef WIT_DEBUG_C
    #define free	do_not_use_free
    #define calloc	do_not_use_calloc
    #define malloc	do_not_use_malloc
    #define realloc	do_not_use_realloc
    #undef  strdup
    #define strdup	do_not_use_strdup
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // WIT_DEBUG_H
