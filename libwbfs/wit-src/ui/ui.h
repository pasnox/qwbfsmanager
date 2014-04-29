
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

#ifndef WIT_UI_H
#define WIT_UI_H

#include <getopt.h>
#include "lib-std.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			     defines			///////////////
///////////////////////////////////////////////////////////////////////////////

#define OPT_OLD_NEW 1		// 0 | 1 -> enable options --OLD --NEW

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  InfoOption_t			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct InfoOption_t
{
	int  id;
	char short_name;
	ccp  long_name;
	ccp  param;
	ccp  help;
	
} InfoOption_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  InfoCommand_t			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct InfoCommand_t
{
    int			id;		// id of the command
    bool		hidden;		// true: command is hidden
    bool		separator;	// true: print a separator above
    ccp			name1;		// main name
    ccp			name2;		// NULL or alternative name
    ccp			syntax;		// NULL or syntax string
    ccp			help;		// help text
    int			n_opt;		// number of options == elements of 'opt'
    const InfoOption_t	** opt;		// field with option info
    u8			* opt_allowed;	// field with OPT__N_SPECIFIC elements
					// 0: option permitted, 1: option allowed

} InfoCommand_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    InfoUI_t			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct InfoUI_t
{
    ccp			tool_name;	// name of the tool

    //----- commands -----

    int			n_cmd;		// == CMD__N
    const CommandTab_t	* cmd_tab;	// NULL or pointer to command table
    const InfoCommand_t	* cmd_info;	// pointer to 'CommandInfo[]'

    //----- options -----

    int			n_opt_specific;	// == OPT__N_SPECIFIC
    int			n_opt_total;	// == OPT__N_TOTAL
    const InfoOption_t	* opt_info;	// pointer to 'OptionInfo[]'
    u8			* opt_used;	// pointer to 'OptionUsed[]'
    const u8		* opt_index;	// pointer to 'OptionIndex[]'
    ccp			opt_short;	// pointer to 'OptionShort[]'
    const struct option	* opt_long;	// pointer to 'OptionLong[]'

} InfoUI_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    Interface			///////////////
///////////////////////////////////////////////////////////////////////////////

enum // some const
{
	OPT_MAX		=    100,	// max number of options
	OPT_USED_MASK	=   0x7f,	// mask to calculate usage count
	OPT_LONG_BASE	=   0x80,	// first index for "only long options"
	OPT_INDEX_SIZE	=  0x100,	// size of OptionIndex[]
};

///////////////////////////////////////////////////////////////////////////////

enumError RegisterOptionByIndex
(
    const InfoUI_t	* iu,		// valid pointer
    int			opt_index,	// index of option (OPT_*)
    int			level,		// the level of registration
    bool		is_env		// true: register environment pre setting
);

enumError RegisterOptionByName
(
    const InfoUI_t	* iu,		// valid pointer
    int			opt_name,	// short name of GO_* valus of option
    int			level,		// the level of registration
    bool		is_env		// true: register environment pre setting
);

enumError VerifySpecificOptions ( const InfoUI_t * iu, const CommandTab_t * cmd );
int GetOptionCount ( const InfoUI_t * iu, int option );
void DumpUsedOptions ( const InfoUI_t * iu, FILE * f, int indent );

///////////////////////////////////////////////////////////////////////////////

void PutLines
(
    FILE	* f,		// valid output stream
    int		indent,		// indent of output
    int		fw,		// field width of output
    int		first_line,	// length without prefix of already printed first line 
    ccp		prefix,		// NULL or prefix for each line
    ccp		text		// text to print
);

void PrintLines
(
    FILE	* f,		// valid output stream
    int		indent,		// indent of output
    int		fw,		// field width of output
    int		first_line,	// length without prefix of already printed first line 
    ccp		prefix,		// NULL or prefix for each line
    ccp		format,		// format string for vsnprintf()
    ...				// arguments for 'vsnprintf(format,...)'

)  __attribute__ ((__format__(__printf__,6,7)));

///////////////////////////////////////////////////////////////////////////////

void PrintHelp
(
    const InfoUI_t * iu,	// valid pointer
    FILE	* f,		// valid output stream
    int		indent,		// indent of output
    ccp		help_cmd,	// NULL or name of help command
    ccp		info		// NULL or poiner to additional text
);

void PrintHelpCmd
(
    const InfoUI_t * iu,	// valid pointer
    FILE	* f,		// valid output stream
    int		indent,		// indent of output
    int		cmd,		// index of command
    ccp		help_cmd,	// NULL or name of help command
    ccp		info		// NULL or poiner to additional text
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////				END			///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // WIT_UI_H

