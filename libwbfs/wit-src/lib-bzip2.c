
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

/******************/
  #ifndef NO_BZIP2
/******************/

//#include <bzlib.h>
#include "libbz2/bzlib.h"
#include "lib-bzip2.h"

/************************************************************************
 **  BZIP2 support: http://www.bzip.org/1.0.5/bzip2-manual-1.0.5.html  **
 ************************************************************************/

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    BZIP2 helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetMessageBZIP2
(
    int			err,		// error code
    ccp			unkown_error	// result for unkown error codes
)
{
    switch (err)
    {
	case BZ_OK:			return "OK";

	case BZ_CONFIG_ERROR:		return "CONFIG ERROR";
	case BZ_DATA_ERROR:		return "DATA ERROR";
	case BZ_DATA_ERROR_MAGIC:	return "DATA ERROR MAGIC";
	case BZ_IO_ERROR:		return "IO ERROR";
	case BZ_MEM_ERROR:		return "MEM ERROR";
	case BZ_PARAM_ERROR:		return "PARAM ERROR";
	case BZ_SEQUENCE_ERROR:		return "SEQUENCE ERROR";
	case BZ_STREAM_END:		return "STREAM END";
	case BZ_UNEXPECTED_EOF:		return "UNEXPECTED EOF";
    }

    return unkown_error;
};

///////////////////////////////////////////////////////////////////////////////

int CalcCompressionLevelBZIP2
(
    int			compr_level	// valid are 1..9 / 0: use default value
)
{
    return compr_level < 1
		? 5
		: compr_level < 9
			? compr_level
			: 9;
}

///////////////////////////////////////////////////////////////////////////////

u32 CalcMemoryUsageBZIP2
(
    int			compr_level,	// valid are 1..9 / 0: use default value
    bool		is_writing	// false: reading mode, true: writing mode
)
{
    compr_level = CalcCompressionLevelBZIP2(compr_level);
    return is_writing
		? ( 4 + 8 * compr_level ) * 102400
		: ( 1 + 4 * compr_level ) * 102400;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BZIP2 writing			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError EncBZIP2_Open
(
    BZIP2_t		* bz,		// data structure, will be initialized
    File_t		* file,		// destination file
    int			compr_level	// valid are 1..9 / 0: use default value
)
{
    DASSERT(bz);
    DASSERT(file);
    DASSERT(file->fp);

    bz->file = file;
    bz->compr_level = CalcCompressionLevelBZIP2(compr_level);

    int bzerror;
    bz->handle = BZ2_bzWriteOpen(&bzerror,file->fp,bz->compr_level,0,0);
    if ( !bz || bzerror != BZ_OK )
    {
	if (bz->handle)
	{
	    BZ2_bzWriteClose(0,bz->handle,0,0,0);
	    bz->handle = 0;
	}

	return ERROR0(ERR_BZIP2,
		"Error while opening bzip2 stream: %s\n-> bzip2 error: %s\n",
		bz->file->fname, GetMessageBZIP2(bzerror,"?") );
	return 0;
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError EncBZIP2_Write
(
    BZIP2_t		* bz,		// created by EncBZIP2_Open()
    const void		* data,		// data to write
    size_t		data_size	// size of data to write
)
{
    DASSERT(bz);
    DASSERT(bz->handle);

    if (data_size)
    {
	int bzerror;
	BZ2_bzWrite(&bzerror,bz->handle,(u8*)data,data_size);
	if ( bzerror != BZ_OK )
	{
	    BZ2_bzWriteClose(0,bz->handle,0,0,0);
	    bz->handle = 0;

	    return ERROR0(ERR_BZIP2,
		    "Error while writing bzip2 stream: %s\n-> bzip2 error: %s\n",
		    bz->file->fname, GetMessageBZIP2(bzerror,"?") );
	}
	bz->file->bytes_written += data_size;
    }
    
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError EncBZIP2_Close
(
    BZIP2_t		* bz,		// NULL or created by EncBZIP2_Open()
    u32			* bytes_written	// not NULL: store written bytes
)
{
    u32 written = 0;
    if ( bz && bz->handle )
    {
	int bzerror;
	BZ2_bzWriteClose(&bzerror,bz->handle,0,0,&written);
	bz->handle = 0;

	if ( bzerror != BZ_OK )
	    return ERROR0(ERR_BZIP2,
		"Error while closing bzip2 stream: %s\n-> bzip2 error: %s\n",
		bz->file->fname, GetMessageBZIP2(bzerror,"?") );

    }

    if (bytes_written)
	*bytes_written = written;

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			BZIP2 reading			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError DecBZIP2_Open
(
    BZIP2_t		* bz,		// data structure, will be initialized
    File_t		* file		// source file
)
{
    DASSERT(file);
    DASSERT(file->fp);

    bz->file = file;

    int bzerror;
    bz->handle = BZ2_bzReadOpen(&bzerror,file->fp,0,0,0,0);
    if ( !bz->handle || bzerror != BZ_OK )
    {
	if (bz->handle)
	{
	    BZ2_bzReadClose(0,bz->handle);
	    bz->handle = 0;
	}

	return ERROR0(ERR_BZIP2,
		"Error while opening bzip2 stream: %s\n-> bzip2 error: %s\n",
		bz->file->fname, GetMessageBZIP2(bzerror,"?") );
    }
    
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError DecBZIP2_Read
(
    BZIP2_t		* bz,		// created by DecBZIP2_Open()
    void		* buf,		// destination buffer
    size_t		buf_size,	// size of destination buffer
    u32			* buf_written	// not NULL: store bytes written to buf
)
{
    DASSERT(bz);
    DASSERT(bz->handle);
    DASSERT(buf);

    int bzerror;
    const u32 written = BZ2_bzRead(&bzerror,bz->handle,buf,buf_size);
    noPRINT("BZREAD, num=%x, buf_size=%zx, err=%d\n",written,buf_size,bzerror);
    if ( bzerror != BZ_STREAM_END )
    {
	BZ2_bzReadClose(0,bz->handle);
	bz->handle = 0;

	return ERROR0(ERR_BZIP2,
		"Error while reading bzip2 stream: %s\n-> bzip2 error: %s\n",
		bz->file->fname, GetMessageBZIP2(bzerror,"?") );
    }

    bz->file->bytes_read += written;
    if (buf_written)
	*buf_written = written;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError DecBZIP2_Close
(
    BZIP2_t		* bz		// NULL or created by DecBZIP2_Open()
)
{
    if ( bz && bz->handle )
    {
	int bzerror;
	BZ2_bzReadClose(&bzerror,bz->handle);
	bz->handle = 0;

	if ( bzerror != BZ_OK )
	    return ERROR0(ERR_BZIP2,
		    "Error while closing bzip2 stream: %s\n-> bzip2 error: %s\n",
		    bz->file->fname, GetMessageBZIP2(bzerror,"?") );
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

/*********************/
  #endif // !NO_BZIP2
/*********************/

