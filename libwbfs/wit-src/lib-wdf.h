
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

#ifndef WIT_LIB_WDF_H
#define WIT_LIB_WDF_H 1

#include "types.h"
#include "lib-std.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////                  WDF definitions                ///////////////
///////////////////////////////////////////////////////////////////////////////

// First a magic is defined to identify a WDF clearly. The magic should never be
// a possible Wii ISO image identification. Wii ISO images starts with the ID6.
// And so the WDF magic contains one contral character (CTRL-A) within.

#define WDF_MAGIC		"WII\1DISC"
#define WDF_MAGIC_SIZE		8
#define WDF_COMPATIBLE		1

// WDF head sizes

#define WDF_VERSION1_SIZE	sizeof(WDF_Head_t)
#define WDF_VERSION2_SIZE	sizeof(WDF_Head_t)

// the minimal size of holes in bytes that will be detected.

#define WDF_MIN_HOLE_SIZE	(sizeof(WDF_Chunk_t)+sizeof(WDF_Hole_t))

// WDF hole detection type
typedef u32 WDF_Hole_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct WDF_Head_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// This is the header of a WDF v1.
// Remember: Within a file the data is stored in network byte order (big endian)

typedef struct WDF_Head_t // split param repalced by others, 2012-09
{
	// magic and version number
	char magic[WDF_MAGIC_SIZE];	// WDF_MAGIC, what else!
	u32 wdf_version;		// WDF_VERSION

    #if 0 // first head definition, WDF v1 before 2012-09
	// split file support (not used, values are *,0,1)
	u32 split_file_id;		// for plausibility checks
	u32 split_file_index;		// zero based index ot this file
	u32 split_file_num_of;		// number of split files
    #else
	u32 wdf_head_size;		// size of version related WDF_Head_t
					// (WDF v1: ignored)
	u32 align_factor;		// info: all data is aligned with a multiple of #
					// (WDF v1: always 0, ignored)
	u32 wdf_compatible;		// this file is compatible down to version #
					// (WDF v1: always 1)
    #endif

	// virtual file infos
	u64 file_size;			// the size of the virtual file

	// data size of this file
	u64 data_size;			// the ISO data size in this file
					// (without header and chunk table)
	// chunks
    #if 0 // first head definition, WDF v1 before 2012-09
	u32 chunk_split_file;		// which split file contains the chunk table
    #else
	u32 chunk_size_factor;		// info: all chunk sizes are multiple of #
					// (WDF v1: always 0, ignored)
    #endif
	u32 chunk_n;			// total number of data chunks
	u64 chunk_off;			// the 'MAGIC + chunk_table' file offset

} __attribute__ ((packed)) WDF_Head_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////                struct WDF_Chunk_t               ///////////////
///////////////////////////////////////////////////////////////////////////////
// This is the chunk info of WDF.
// Remember: Within a file the data is stored in network byte order (big endian)

typedef struct WDF_Chunk_t
{
	// split_file_index is obsolete in WDF v2
	u32 ignored_split_file_index;	// which split file contains that chunk
					// (WDF v1: always 0, ignored)
	u64 file_pos;			// the virtual ISO file position
	u64 data_off;			// the data file offset
	u64 data_size;			// the data size

} __attribute__ ((packed)) WDF_Chunk_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////               interface for WDF_Head_t          ///////////////
///////////////////////////////////////////////////////////////////////////////

// initialize WH
void InitializeWH ( WDF_Head_t * wh );

// convert WH data, src + dest may point to same structure
void ConvertToNetworkWH ( WDF_Head_t * dest, const WDF_Head_t * src );
void ConvertToHostWH    ( WDF_Head_t * dest, const WDF_Head_t * src );

// helpers
size_t GetHeadSizeWDF ( u32 version );
size_t AdjustHeaderWDF ( WDF_Head_t * wh );

///////////////////////////////////////////////////////////////////////////////
///////////////               interface for WDF_Chunk_t         ///////////////
///////////////////////////////////////////////////////////////////////////////

// initialize WC
void InitializeWC ( WDF_Chunk_t * wc, int n_elem );

// convert WC data, src + dest may point to same structure
void ConvertToNetworkWC ( WDF_Chunk_t * dest, const WDF_Chunk_t * src );
void ConvertToHostWC    ( WDF_Chunk_t * dest, const WDF_Chunk_t * src );

//
///////////////////////////////////////////////////////////////////////////////
///////////////               SuperFile_t interface             ///////////////
///////////////////////////////////////////////////////////////////////////////

#undef SUPERFILE
#define SUPERFILE struct SuperFile_t
SUPERFILE;

// WDF reading support
enumError SetupReadWDF	( SUPERFILE * sf );
enumError ReadWDF	( SUPERFILE * sf, off_t off, void * buf, size_t size );
off_t     DataBlockWDF	( SUPERFILE * sf, off_t off, size_t hint_align, off_t * block_size );

// WDF writing support
enumError SetupWriteWDF	( SUPERFILE * sf );
enumError TermWriteWDF	( SUPERFILE * sf );
enumError WriteWDF	( SUPERFILE * sf, off_t off, const void * buf, size_t size );
enumError WriteSparseWDF( SUPERFILE * sf, off_t off, const void * buf, size_t size );
enumError WriteZeroWDF	( SUPERFILE * sf, off_t off, size_t size );

// chunk managment
WDF_Chunk_t * NeedChunkWDF ( SUPERFILE * sf, int at_index );

#undef SUPERFILE

//
///////////////////////////////////////////////////////////////////////////////
///////////////				etc			///////////////
///////////////////////////////////////////////////////////////////////////////

int SetWDF2Mode ( uint c, ccp align );
int ScanOptWDFAlign ( ccp arg );

//
///////////////////////////////////////////////////////////////////////////////
///////////////                          END                    ///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // WIT_LIB_WDF_H

