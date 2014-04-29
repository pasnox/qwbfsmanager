
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

#ifndef WIT_TYPES_H
#define WIT_TYPES_H 1

#define _GNU_SOURCE 1

#include <sys/types.h>
#include <stdint.h>
#include <limits.h>

///////////////////////////////////////////////////////////////////////////////

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
//typedef uint64_t	u64;	// is 'long unsigned' in 64 bit linux
typedef long long unsigned u64;

typedef int8_t		s8;
typedef int16_t		s16;
typedef int32_t		s32;
//typedef int64_t	s64;	// is 'long int' in 64 bit linux
typedef long long int	s64;

typedef u16		be16_t;
typedef u32		be32_t;
typedef u64		be64_t;

typedef unsigned char	uchar;
typedef unsigned int	uint;
typedef unsigned long	ulong;

#ifndef __cplusplus
typedef enum bool { false, true } __attribute__ ((packed)) bool;
#endif

///////////////////////////////////////////////////////////////////////////////

typedef const char *	ccp;
typedef char		id6_t[7];
typedef u8		sha1_hash_t[20];

///////////////////////////////////////////////////////////////////////////////

#endif // WIT_TYPES_H

