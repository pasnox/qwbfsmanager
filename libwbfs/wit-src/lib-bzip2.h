
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

#ifndef WIT_LIB_BZIP2_H
#define WIT_LIB_BZIP2_H 1
#ifndef NO_BZIP2

#define _GNU_SOURCE 1

#include "lib-std.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef _BZLIB_H
    typedef void BZFILE;
#endif

//-----------------------------------------------------------------------------

typedef struct BZIP2_t
{
    File_t		* file;		// IO file
    BZFILE		* handle;	// bzip2 handle
    int			compr_level;	// active compression level

} BZIP2_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetMessageBZIP2
(
    int			err,		// error code
    ccp			unkown_error	// result for unkown error codes
);

//-----------------------------------------------------------------------------

int CalcCompressionLevelBZIP2
(
    int			compr_level	// valid are 1..9 / 0: use default value
);

//-----------------------------------------------------------------------------

u32 CalcMemoryUsageBZIP2
(
    int			compr_level,	// valid are 1..9 / 0: use default value
    bool		is_writing	// false: reading mode, true: writing mode
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BZIP2 writing			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError EncBZIP2_Open
(
    BZIP2_t		* bz,		// data structure, will be initialized
    File_t		* file,		// destination file
    int			compr_level	// valid are 1..9 / 0: use default value
);

//-----------------------------------------------------------------------------

enumError EncBZIP2_Write
(
    BZIP2_t		* bz,		// created by EncBZIP2_Open()
    const void		* data,		// data to write
    size_t		data_size	// size of data to write
);

//-----------------------------------------------------------------------------

enumError EncBZIP2_Close
(
    BZIP2_t		* bz,		// NULL or created by EncBZIP2_Open()
    u32			* bytes_written	// not NULL: store written bytes
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BZIP2 reading			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DecBZIP2_Open
(
    BZIP2_t		* bz,		// data structure, will be initialized
    File_t		* file		// source file
);

//-----------------------------------------------------------------------------

enumError DecBZIP2_Read
(
    BZIP2_t		* bz,		// created by DecBZIP2_Open()
    void		* buf,		// destination buffer
    size_t		buf_size,	// size of destination buffer
    u32			* buf_written	// not NULL: store bytes written to buf
);

//-----------------------------------------------------------------------------

enumError DecBZIP2_Close
(
    BZIP2_t		* bz		// NULL or created by DecBZIP2_Open()
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////				END			///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // !NO_BZIP2
#endif // WIT_LIB_BZIP2_H 1

