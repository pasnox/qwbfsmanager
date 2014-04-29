
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "debug.h"
#include "libwbfs.h"
#include "lib-sf.h"
#include "wbfs-interface.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////                         data                    ///////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
    #define WC_GROW_SIZE  10
#else
    #define WC_GROW_SIZE 500
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////                      Setup data                 ///////////////
///////////////////////////////////////////////////////////////////////////////
// initialize WH

void InitializeWH ( WDF_Head_t * wh )
{
    ASSERT(wh);

    memset(wh,0,sizeof(*wh));
    memcpy(wh->magic,WDF_MAGIC,sizeof(wh->magic));

 #if WDF2_ENABLED

    wh->split_file_num_of = 1;

    if ( opt_wdf_version < 2 )
    {
	wh->wdf_version		= 1;
	wh->wdf_compatible	= 1;
	wh->wdf_head_size	= WDF_VERSION1_SIZE;
	//wh->align_factor	= 0;
	//wh->chunk_size_factor	= 0;
    }
    else
    {
	wh->wdf_version		= opt_wdf_version;
	wh->wdf_compatible	= WDF_COMPATIBLE;
	wh->wdf_head_size	= WDF_VERSION2_SIZE;
	wh->align_factor	= opt_wdf_align;
	wh->chunk_size_factor	= opt_wdf_align;	// [[2do]]
    }

 #else

    wh->wdf_version		= WDF_VERSION;
    wh->wdf_compatible		= WDF_COMPATIBLE;
    wh->wdf_head_size		= WDF_VERSION1_SIZE;
    //wh->align_factor		= 0;
    //wh->chunk_size_factor	= 0;

 #endif
}

///////////////////////////////////////////////////////////////////////////////
// initialize the WC

void InitializeWC ( WDF_Chunk_t * wc, int n_elem )
{
    ASSERT(wc);
    ASSERT(n_elem>0);

    memset(wc,0,sizeof(*wc)*n_elem);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////        convert data to network byte order       ///////////////
///////////////////////////////////////////////////////////////////////////////

#undef CONV32
#undef CONV64
#define CONV32(var) dest->var = htonl(src->var)
#define CONV64(var) dest->var = hton64(src->var)

///////////////////////////////////////////////////////////////////////////////

void ConvertToNetworkWH ( WDF_Head_t * dest, const WDF_Head_t * src )
{
    ASSERT(dest);
    ASSERT(src);

    // initialize this again before exporting
    memcpy(dest->magic,WDF_MAGIC,sizeof(dest->magic));

    CONV32(wdf_version);
    CONV32(wdf_head_size);
    CONV32(align_factor);
    CONV32(wdf_compatible);

    CONV64(file_size);
    CONV64(data_size);

    CONV32(chunk_size_factor);
    CONV32(chunk_n);
    CONV64(chunk_off);
}

///////////////////////////////////////////////////////////////////////////////

void ConvertToNetworkWC ( WDF_Chunk_t * dest, const WDF_Chunk_t * src )
{
    ASSERT(dest);
    ASSERT(src);

    CONV32(ignored_split_file_index);
    CONV64(file_pos);
    CONV64(data_off);
    CONV64(data_size);
}

///////////////////////////////////////////////////////////////////////////////

// clear defines
#undef CONV32
#undef CONV64

//
///////////////////////////////////////////////////////////////////////////////
///////////////         convert data to host byte order         ///////////////
///////////////////////////////////////////////////////////////////////////////

#undef CONV32
#undef CONV64
#define CONV32(var) dest->var = ntohl(src->var)
#define CONV64(var) dest->var = ntoh64(src->var)

///////////////////////////////////////////////////////////////////////////////

void ConvertToHostWH ( WDF_Head_t * dest, const WDF_Head_t * src )
{
    ASSERT(dest);
    ASSERT(src);

    memcpy(dest->magic,src->magic,sizeof(dest->magic));

    CONV32(wdf_version);
    CONV32(wdf_head_size);
    CONV32(align_factor);
    CONV32(wdf_compatible);

    CONV64(file_size);
    CONV64(data_size);

    CONV32(chunk_size_factor);
    CONV32(chunk_n);
    CONV64(chunk_off);
}

///////////////////////////////////////////////////////////////////////////////

void ConvertToHostWC ( WDF_Chunk_t * dest, const WDF_Chunk_t * src )
{
    ASSERT(dest);
    ASSERT(src);

    CONV32(ignored_split_file_index);
    CONV64(file_pos);
    CONV64(data_off);
    CONV64(data_size);
}

///////////////////////////////////////////////////////////////////////////////

// clear defines
#undef CONV32
#undef CONV64

//
///////////////////////////////////////////////////////////////////////////////
///////////////                     WDF: helpers                ///////////////
///////////////////////////////////////////////////////////////////////////////

size_t GetHeadSizeWDF ( u32 version )
{
    static size_t WDFHeadSizeTab[] =
    {
	0,
	WDF_VERSION1_SIZE,
	WDF_VERSION2_SIZE,
    };
    ASSERT( sizeof(WDFHeadSizeTab)/sizeof(*WDFHeadSizeTab) >= WDF_VERSION+1 );

    return version < sizeof(WDFHeadSizeTab)/sizeof(*WDFHeadSizeTab)
		? WDFHeadSizeTab[version] : 0;
}

///////////////////////////////////////////////////////////////////////////////

size_t AdjustHeaderWDF ( WDF_Head_t * wh )
{
    DASSERT(wh);

 #if WDF2_ENABLED

    if ( wh->wdf_compatible == 1 )
    {
	const u32 calced_wdf_head_size = wh->chunk_off - wh->data_size;
	if ( calced_wdf_head_size == WDF_VERSION1_SIZE )
	{
	    noPRINT("AdjustHeaderWDF() v=%u,%u, size=%u\n",
		    wh->wdf_version, wh->wdf_compatible, calced_wdf_head_size );
	    wh->wdf_version   = 1;
	    wh->wdf_head_size = WDF_VERSION1_SIZE;
	    wh->align_factor  = 1;
	}
    }
    return wh->wdf_head_size;

 #else

    return wh->chunk_off - wh->data_size;

 #endif
}

///////////////////////////////////////////////////////////////////////////////
// chunk managment

WDF_Chunk_t * NeedChunkWDF ( SuperFile_t * sf, int index )
{
    ASSERT(sf);
    ASSERT( index >= 0 );
    ASSERT( index <= sf->wc_used );
    ASSERT( sf->wc_used <= sf->wc_size );

    if ( sf->wc_used == sf->wc_size )
    {
	// grow the field

	TRACE("#W# NeedChunkWDF() GROW %d ->%d [use3d=%d]\n",
		sf->wc_size, sf->wc_size+WC_GROW_SIZE, sf->wc_used );

	sf->wc_size += WC_GROW_SIZE;
	sf->wc = REALLOC(sf->wc,sf->wc_size*sizeof(*sf->wc));
    }
    ASSERT( sf->wc_used < sf->wc_size );

    WDF_Chunk_t * wc = sf->wc + index;

    if ( index < sf->wc_used )
	memmove( wc+1, wc, (sf->wc_used-index)*sizeof(*wc) );

    sf->wc_used++;
    TRACE("#W# NeedChunkWDF() return %p [idx=%zd/%d/%d]\n",
		wc, wc-sf->wc, sf->wc_used, sf->wc_size );
    InitializeWC(wc,1);
    return wc;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                       read WDF                  ///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SetupReadWDF ( SuperFile_t * sf )
{
    TRACE("#W# SetupReadWDF(%p) wc=%p wbfs=%p\n",sf,sf->wc,sf->wbfs);
    if ( sf->wc || sf->wbfs )
	return ERR_OK;

    ASSERT(sf);
    CleanSF(sf);
    InitializeWH(&sf->wh); // reset data

    if ( sf->f.seek_allowed && sf->f.st.st_size < sizeof(WDF_Head_t) )
	return ERR_NO_WDF;

    WDF_Head_t wh;
    enumError stat = ReadAtF(&sf->f,0,&wh,sizeof(wh));
    if (stat)
	return stat;
    TRACE("#W#  - header read\n");

    //----- fix old style header (WDF v1)

    ConvertToHostWH(&wh,&wh);
    if ( wh.wdf_version == 1 )
    {
	wh.wdf_compatible	= 1;
	wh.wdf_head_size	= WDF_VERSION1_SIZE;
	wh.align_factor		= 0;
	wh.chunk_size_factor	= 0;
    }

    //----- test header

    stat = AnalyzeWH(&sf->f,&wh,true);
    if (stat)
	return stat;

    //----- test chunk table magic

    char magic[WDF_MAGIC_SIZE];
    stat = ReadAtF(&sf->f,wh.chunk_off,magic,sizeof(magic));
    if (stat)
	return stat;

    if (memcmp(magic,WDF_MAGIC,WDF_MAGIC_SIZE))
	goto invalid;

    const int chunk_tab_size = wh.chunk_n * sizeof(WDF_Chunk_t);
    WDF_Chunk_t *wc = MALLOC(chunk_tab_size);

    stat = ReadAtF(&sf->f,wh.chunk_off+WDF_MAGIC_SIZE,wc,chunk_tab_size);
    if (stat)
	return stat;
    TRACE("#W#  - chunk table read\n");

    sf->wc = wc;
    sf->wc_used = sf->wc_size = wh.chunk_n;

    int idx;
    for ( idx = 0; idx < wh.chunk_n; idx++, wc++ )
    {
	ConvertToHostWC(wc,wc);
	if ( idx && wc->file_pos < wc[-1].file_pos + wc[-1].data_size )
	    goto invalid;
    }
    TRACE("#W#  - chunk loop exits with ok\n");

    // check last chunk
    wc = sf->wc + sf->wc_used - 1;
    if ( wc->file_pos + wc->data_size != wh.file_size )
	    goto invalid;

    memcpy(&sf->wh,&wh,sizeof(sf->wh));
    sf->file_size	= sf->wh.file_size;
    sf->f.max_off	= sf->wh.chunk_off;
    sf->max_virt_off	= sf->wh.file_size;
    SetupIOD(sf,OFT_WDF,OFT_WDF);

    TRACE("#W# WDF FOUND!\n");
    return ERR_OK;

 invalid:
    return ERROR0(ERR_WDF_INVALID,"Invalid WDF file: %s\n",sf->f.fname);
}

///////////////////////////////////////////////////////////////////////////////

enumError ReadWDF ( SuperFile_t * sf, off_t off, void * buf, size_t count )
{
    ASSERT(sf);
    ASSERT(sf->wc);
    ASSERT(sf->wc_used);

    TRACE("#W# -----\n");
    TRACE(TRACE_RDWR_FORMAT, "#W# ReadWDF()",
		GetFD(&sf->f), GetFP(&sf->f), (u64)off, (u64)off+count, count, "" );

    if ( off + count > sf->wh.file_size )
    {
	if (!sf->f.read_behind_eof)
	{
	    if ( !sf->f.disable_errors )
		ERROR0( ERR_READ_FAILED, "Read behind eof [%c,%llx+%zx]: %s\n",
		    sf->f.fp ? 'S' : sf->f.fd != -1 ? 'F' : '-',
		    (u64)off, count, sf->f.fname );
	    return ERR_READ_FAILED;
	}

	const off_t max_read = sf->wh.file_size > off
					? sf->wh.file_size - off
					: 0;
	ASSERT( count > max_read );

	if ( sf->f.read_behind_eof == 1 )
	{
	    sf->f.read_behind_eof = 2;
	    if ( !sf->f.disable_errors )
		ERROR0( ERR_WARNING, "Read behind eof -> zero filled [%c,%llx+%zx]: %s\n",
		    sf->f.fp ? 'S' : sf->f.fd != -1 ? 'F' : '-',
		    (u64)off, count, sf->f.fname );
	}

	size_t fill_count = count - (size_t)max_read;
	count = (size_t)max_read;
	memset((char*)buf+count,0,fill_count);

	if (!count)
	    return ERR_OK;
    }

    // find chunk header
    WDF_Chunk_t * wc = sf->wc;
    const int used_m1 = sf->wc_used - 1;
    int beg = 0, end = used_m1;
    ASSERT( beg <= end );
    while ( beg < end )
    {
	int idx = (beg+end)/2;
	wc = sf->wc + idx;
	if ( off < wc->file_pos )
	    end = idx-1;
	else if ( idx < used_m1 && off >= wc[1].file_pos )
	    beg = idx + 1;
	else
	    beg = end = idx;
    }
    wc = sf->wc + beg;

    noTRACE("#W#  - FOUND #%03d: off=%09llx ds=%llx, off=%09llx\n",
	    beg, wc->file_pos, wc->data_size, (u64)off );
    ASSERT( off >= wc->file_pos );
    ASSERT( beg == used_m1 || off < wc[1].file_pos );

    char * dest = buf;
    while ( count > 0 )
    {
	noTRACE("#W# %d/%d count=%zd off=%llx,%llx \n",
		beg, sf->wc_used, count, (u64)off, (u64)wc->file_pos );

	if ( off < wc->file_pos )
	{
	    const u64 max_size = wc->file_pos - off;
	    const u32 fill_size = max_size < count ? (u32)max_size : count;
	    TRACE("#W# >FILL %p +%8zx = %p .. %x\n",
		    buf, dest-(ccp)buf, dest, fill_size );
	    memset(dest,0,fill_size);
	    count -= fill_size;
	    off  += fill_size;
	    dest += fill_size;
	    if (!count)
		break;
	}

	if ( off >= wc->file_pos && off < wc->file_pos + wc->data_size )
	{
	    // we want a part of this
	    const u64 delta     = off - wc->file_pos;
	    const u64 max_size  = wc->data_size - delta;
	    const u32 read_size = max_size < count ? (u32)max_size : count;
	    TRACE("#W# >READ %p +%8zx = %p .. %x <- %10llx\n",
		    buf, dest-(ccp)buf, dest, read_size, wc->data_off+delta );
	    int stat = ReadAtF(&sf->f,wc->data_off+delta,dest,read_size);
	    if (stat)
		return stat;
	    count -= read_size;
	    off  += read_size;
	    dest += read_size;
	    if (!count)
		break;
	}

	wc++;
	if ( ++beg >= sf->wc_used )
	{
	    TRACE("ERR_WDF_INVALID\n");
	    return ERR_WDF_INVALID;
	}
    }

    TRACE("#W#  - done, dest = %p\n",dest);
    return ERR_OK;
}


///////////////////////////////////////////////////////////////////////////////

off_t DataBlockWDF
	( SuperFile_t * sf, off_t off, size_t hint_align, off_t * block_size )
{
    if ( off >= sf->wh.file_size )
	return DataBlockStandard(sf,off,hint_align,block_size);

    //--- find chunk header

    WDF_Chunk_t * wc = sf->wc;
    const int used_m1 = sf->wc_used - 1;
    int beg = 0, end = used_m1;
    ASSERT( beg <= end );
    while ( beg < end )
    {
	int idx = (beg+end)/2;
	wc = sf->wc + idx;
	if ( off < wc->file_pos )
	    end = idx-1;
	else if ( idx < used_m1 && off >= wc[1].file_pos )
	    beg = idx + 1;
	else
	    beg = end = idx;
    }
    wc = sf->wc + beg;

    while ( off >= wc->file_pos + wc->data_size )
    {
	if ( ++beg >= sf->wc_used )
	    return DataBlockStandard(sf,off,hint_align,block_size);
	wc++;
    }
    noPRINT("WC: %llx %llx %llx\n",wc->file_pos,wc->data_off,wc->data_size);

    if ( off < wc->file_pos )
	 off = wc->file_pos;


    //--- calc block_size, ignore holes < 4k

    if ( block_size )
    {
	if ( hint_align < HD_BLOCK_SIZE )
	    hint_align = HD_BLOCK_SIZE;

	for (;;)
	{
	    if ( ++beg >= sf->wc_used
		    || wc[1].file_pos - (wc->file_pos+wc->data_size) >= hint_align )
		break;
	    wc++;
	}
	*block_size = wc->data_size - ( off - wc->file_pos );
    }


    //--- term

    return off;
}

///////////////////////////////////////////////////////////////////////////////

void FileMapWDF ( SuperFile_t * sf, FileMap_t *fm )
{
    DASSERT(sf);
    DASSERT(fm);
    DASSERT(!fm->used);

    const WDF_Chunk_t *wc = sf->wc;
    const WDF_Chunk_t *end = wc + sf->wc_used;
    for ( ; wc < end; wc++ )
	AppendFileMap(fm,wc->file_pos,wc->data_off,wc->data_size);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                     write WDF                   ///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SetupWriteWDF ( SuperFile_t * sf )
{
    ASSERT(sf);
    TRACE("#W# SetupWriteWDF(%p)\n",sf);

    InitializeWH(&sf->wh);

 #if WDF2_ENABLED
    const uint head_size = sf->wh.wdf_head_size;
 #else
    const uint head_size = WDF_VERSION1_SIZE;
 #endif

    if (sf->src)
    {
	u64 size = 0;
	if (sf->raw_mode)
	    size = sf->f.st.st_size;
	else
	{
	    wd_disc_t * disc = OpenDiscSF(sf->src,false,true);
	    if (disc)
		size = wd_count_used_disc_size(disc,1,0);
	}

	if (size)
	{
	    const enumError err = PreallocateF(&sf->f,0,size+head_size);
	    if (err)
	    {
		TRACE("#W# SetupWriteWDF() returns %d\n",err);
		return err;
	    }
	}
    }

    SetupIOD(sf,OFT_WDF,OFT_WDF);
    sf->max_virt_off = 0;
    sf->wh.magic[0] = '-'; // write a 'not complete' indicator
    const enumError err = WriteAtF(&sf->f,0,&sf->wh,head_size);

    sf->wc_used = 0;

    // first chunk with file_pos=0, count=0, off=x
    WDF_Chunk_t * wc = NeedChunkWDF(sf,0);
    wc->data_off = sf->f.file_off;

    TRACE("#W# SetupWriteWDF() returns %d\n",err);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError TermWriteWDF ( SuperFile_t * sf )
{
    ASSERT(sf);
    ASSERT(sf->wc);
    TRACE("#W# TermWriteWDF(%p)\n",sf);

    WDF_Chunk_t * wc = sf->wc + sf->wc_used - 1;
    const u64 last_pos = wc->file_pos + wc->data_size;
    if ( sf->file_size < last_pos )
    {
	// correction for double layer discs [[2do]]
	sf->file_size = last_pos;
    }
    else if ( sf->file_size > last_pos )
    {
	wc = NeedChunkWDF(sf,sf->wc_used);
	wc->file_pos = sf->file_size;
	wc->data_off = sf->f.max_off;
    }

    int i = 0;
    for ( wc = sf->wc; i < sf->wc_used; i++, wc++ )
	ConvertToNetworkWC(wc,wc);

 #if WDF2_ENABLED
    const uint head_size = sf->wh.wdf_head_size;
 #else
    const uint head_size = WDF_VERSION1_SIZE;
 #endif

    sf->wh.chunk_n	= sf->wc_used;
    sf->wh.chunk_off	= sf->f.max_off;
    sf->wh.file_size	= sf->file_size;
    sf->wh.data_size	= sf->wh.chunk_off - head_size;

    WDF_Head_t wh;
    ConvertToNetworkWH(&wh,&sf->wh);

    // write the magic behind the data (use header)
    int stat = WriteAtF( &sf->f, sf->wh.chunk_off, &wh.magic, sizeof(wh.magic) );

    // write the chunk table
    if (!stat)
	stat = WriteF( &sf->f, sf->wc, sf->wh.chunk_n * sizeof(*sf->wc) );

    // write the header
    if (!stat)
	stat = WriteAtF( &sf->f, 0, &wh, head_size );

    TRACE("#W# TermWriteWDF() returns %d\n",stat);
    return stat;
}

///////////////////////////////////////////////////////////////////////////////

enumError WriteWDF ( SuperFile_t * sf, off_t off, const void * buf, size_t count )
{
    ASSERT(sf);
    ASSERT(sf->wc);

    TRACE("#W# -----\n");
    TRACE(TRACE_RDWR_FORMAT, "#W# WriteWDF()",
		GetFD(&sf->f), GetFP(&sf->f), (u64)off, (u64)off+count, count,
		off < sf->max_virt_off ? " <" : "" );
    TRACE(" - off = %llx,%llx, fs = %llx\n",
		(u64)sf->f.file_off, (u64)sf->f.max_off, (u64)sf->file_size );

    if (!count)
	return ERR_OK;

    // adjust the file size
    const off_t data_end = off + count;
    if ( sf->file_size < data_end )
	 sf->file_size = data_end;

    ASSERT( sf->wc_used > 0 );
    const int used_m1 = sf->wc_used - 1;

    if ( off >= sf->max_virt_off )
    {
	// SPECIAL CASE:
	//    the current virtual file will be extended
	//    -> no need to search chunks

	if ( off <= sf->max_virt_off + WDF_MIN_HOLE_SIZE )
	{
	    // maybe an extend of the last chunk -> get the last chunk
	    WDF_Chunk_t * wc = sf->wc + used_m1;
	    if ( wc->data_off + wc->data_size == sf->f.max_off )
	    {
		// yes, it is the last written chunk
		const u32 skip = off - sf->max_virt_off;

		// adjust max_virt_off
		sf->max_virt_off = off + count;

		const enumError err
		    = WriteAtF(&sf->f,skip+wc->data_off+wc->data_size,buf,count);
		wc->data_size += skip + count;
		return err;
	    }
	}

	// adjust max_virt_off
	sf->max_virt_off = off + count;

	// create a new chunk at end of file
	WDF_Chunk_t * wc = NeedChunkWDF(sf,sf->wc_used);
	wc->file_pos  = off;
	wc->data_off  = sf->f.max_off;
	wc->data_size = count;
	return WriteAtF(&sf->f,wc->data_off,buf,count);
    }

    // search chunk header with a binary search
    WDF_Chunk_t * wc = sf->wc;
    int beg = 0, end = used_m1;
    ASSERT( beg <= end );
    while ( beg < end )
    {
	int idx = (beg+end)/2;
	wc = sf->wc + idx;
	if ( off < wc->file_pos )
	    end = idx-1;
	else if ( idx < used_m1 && off >= wc[1].file_pos )
	    beg = idx + 1;
	else
	    beg = end = idx;
    }
    wc = sf->wc + beg;

    TRACE("#W#  - FOUND #%03d: off=%09llx ds=%llx, off=%09llx\n",
	    beg, wc->file_pos, wc->data_size, (u64)off );
    ASSERT( off >= wc->file_pos );
    ASSERT( beg == used_m1 || off < wc[1].file_pos );

    ccp src = buf;
    while ( count > 0 )
    {
	TRACE("#W# %d/%d count=%zd off=%llx,%llx \n",
		beg, sf->wc_used, count, (u64)off, wc->file_pos );

	if ( off < wc->file_pos )
	{
	    const u64 max_size = wc->file_pos - off;
	    const u32 wr_size = max_size < count ? (u32)max_size : count;

	    TRACE("#W# >CREATE#%02d    %p +%8zx = %10llx .. +%4x\n",
			beg, buf, src-(ccp)buf, (u64)off, wr_size );

	    // create a new chunk
	    wc = NeedChunkWDF(sf,beg);
	    wc->file_pos   = off;
	    wc->data_off  = sf->f.max_off;
	    wc->data_size = wr_size;

	    // write data & return
	    const enumError stat = WriteAtF(&sf->f,wc->data_off,src,wr_size);
	    if (stat)
		return stat;

	    wc++;
	    beg++;

	    count -= wr_size;
	    off  += wr_size;
	    src += wr_size;
	    if (!count)
		break;
	}

	if ( off >= wc->file_pos && off < wc->file_pos + wc->data_size )
	{
	    // we want a part of this
	    const u64 delta     = off - wc->file_pos;
	    const u64 max_size  = wc->data_size - delta;
	    const u32 wr_size = max_size < count ? (u32)max_size : count;

	    TRACE("#W# >OVERWRITE#%02d %p +%8zx = %10llx .. +%4x, delta=%lld\n",
			beg, buf, src-(ccp)buf, (u64)off, wr_size, delta );

	    const enumError stat = WriteAtF(&sf->f,wc->data_off+delta,src,wr_size);
	    if (stat)
		return stat;

	    count -= wr_size;
	    off  += wr_size;
	    src += wr_size;
	    if (!count)
		break;
	}

	wc++;
	if ( ++beg >= sf->wc_used )
	    return WriteWDF(sf,off,src,count);
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError WriteSparseWDF
	( SuperFile_t * sf, off_t off, const void * buf, size_t count )
{
    return SparseHelper(sf,off,buf,count,WriteWDF,WDF_MIN_HOLE_SIZE);
}

///////////////////////////////////////////////////////////////////////////////

enumError WriteZeroWDF ( SuperFile_t * sf, off_t off, size_t count )
{
    ASSERT(sf);
    ASSERT(sf->wc);

    TRACE("#W# -----\n");
    TRACE(TRACE_RDWR_FORMAT, "#W# WriteZeroWDF()",
		GetFD(&sf->f), GetFP(&sf->f), (u64)off, (u64)off+count, count,
		off < sf->max_virt_off ? " <" : "" );
    TRACE(" - off = %llx,%llx,%llx\n",
		(u64)sf->f.file_off, (u64)sf->f.max_off, (u64)sf->max_virt_off);

    if (!count)
	return ERR_OK;

    // adjust the file size
    const off_t data_end = off + count;
    if ( sf->file_size < data_end )
	sf->file_size = data_end;

    ASSERT( sf->wc_used > 0 );
    const int used_m1 = sf->wc_used - 1;

    if ( off >= sf->max_virt_off )
	return ERR_OK;

    // search chunk header with a binary search
    WDF_Chunk_t * wc = sf->wc;
    int beg = 0, end = used_m1;
    ASSERT( beg <= end );
    while ( beg < end )
    {
	int idx = (beg+end)/2;
	wc = sf->wc + idx;
	if ( off < wc->file_pos )
	    end = idx-1;
	else if ( idx < used_m1 && off >= wc[1].file_pos )
	    beg = idx + 1;
	else
	    beg = end = idx;
    }
    wc = sf->wc + beg;

    TRACE("#W#  - FOUND #%03d: off=%09llx ds=%llx, off=%09llx\n",
	    beg, wc->file_pos, wc->data_size, (u64)off );
    ASSERT( off >= wc->file_pos );
    ASSERT( beg == used_m1 || off < wc[1].file_pos );

    WDF_Chunk_t * last_wc = sf->wc + sf->wc_used;
    for ( ; wc < last_wc || wc->file_pos < data_end; wc++ )
    {
	off_t end = wc->file_pos + wc->data_size;
	TRACE("loop: wc=%llx,%llx,%llx off=%llx, end=%llx\n",
	    wc->file_pos, wc->data_off, wc->data_size, (u64)off, (u64)end );
	if ( off >= end )
	    continue;

	if ( off < wc->file_pos )
	    off = wc->file_pos;
	if ( end > data_end )
	    end = data_end;
	if ( off < end )
	{
	    const enumError err
		= WriteZeroAtF( &sf->f, wc->data_off+(off-wc->file_pos), end-off );
	    if (err)
		return err;
	}
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////				etc			///////////////
///////////////////////////////////////////////////////////////////////////////

int SetWDF2Mode ( uint vers, ccp align )
{
    output_file_type = OFT_WDF;
    const int stat = ScanOptWDFAlign(align);
    opt_wdf_version = vers;
    return stat;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptWDFAlign ( ccp arg )
{
    if (!arg)
	return 0;

    u32 align;
    enumError stat = ScanSizeOptU32(
		&align,			// u32 * num
		arg,			// ccp source
		1,			// default_factor1
		0,			// int force_base
		"wdf-align",		// ccp opt_name
		0,			// u64 min
		WDF_MAX_ALIGN,		// u64 max
		0,			// u32 multiple
		1,			// u32 pow2
		true			// bool print_err
		) != ERR_OK;

    if (!stat)
    {
	if (align)
	{
	    opt_wdf_version = 2;
	    opt_wdf_align   = align;
	}
	else
	    opt_wdf_version = 1;
    }
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                          END                    ///////////////
///////////////////////////////////////////////////////////////////////////////

