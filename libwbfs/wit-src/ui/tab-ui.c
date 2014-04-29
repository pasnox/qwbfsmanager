
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

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  text controls			///////////////
///////////////////////////////////////////////////////////////////////////////

//	\1 : the following text is only for the built in help
//	\2 : the following text is only for the web site
//	\3 : the following text is for both, built in help and web site
//	\4 : replace by '<' for html tags on web site

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  declarations			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum enumType
{
	//----- base types

	T_END		=    0x0001,  // end of list
	T_DEF_TOOL	=    0x0002,  // start of a new tool
	T_DEF_CMD	=    0x0004,  // define a command
	T_SEP_CMD	=    0x0008,  // define a command separator
	T_DEF_OPT	=    0x0010,  // define an option
	T_SEP_OPT	=    0x0020,  // define an option separator
	T_GRP_BEG	=    0x0040,  // start option definitions for a group
	T_CMD_BEG	=    0x0080,  // start option definitions for a command
	T_CMD_OPT	=    0x0100,  // allowed option for command
	T_COPY_CMD	=    0x0200,  // copy options of other command
	T_COPY_GRP	=    0x0400,  // copy options of group
	T_ALL_OPT	=    0x0800,  // allow all options

	//----- option flags

	F_OPT_COMMAND	=  0x010000,  // option is command specific
	F_OPT_GLOBAL	=  0x020000,  // option is global
	F_OPT_MULTIUSE	=  0x040000,  // multiple usage of option possible
	F_OPT_PARAM	=  0x080000,  // option needs a parameter
	F_OPT_OPTPARAM	=  0x100000,  // option accepts a optional parameter
	F_SEPARATOR	=  0x200000,  // separator element
	F_SUPERSEDE	=  0x400000,  // supersedes all other commands and options

	F_OPT_XPARAM	=  F_OPT_PARAM | F_OPT_OPTPARAM,


	//----- global flags

	F_HIDDEN	= 0x1000000,  // option is hidden from help


	//----- option combinations

	T_OPT_C		= T_DEF_OPT | F_OPT_COMMAND,
	T_OPT_CM	= T_DEF_OPT | F_OPT_COMMAND | F_OPT_MULTIUSE,
	T_OPT_CP	= T_DEF_OPT | F_OPT_COMMAND                  | F_OPT_PARAM,
	T_OPT_CMP	= T_DEF_OPT | F_OPT_COMMAND | F_OPT_MULTIUSE | F_OPT_PARAM,
	T_OPT_CO	= T_DEF_OPT | F_OPT_COMMAND                  | F_OPT_OPTPARAM,
	T_OPT_CMO	= T_DEF_OPT | F_OPT_COMMAND | F_OPT_MULTIUSE | F_OPT_OPTPARAM,

	T_OPT_G		= T_DEF_OPT | F_OPT_GLOBAL,
	T_OPT_GM	= T_DEF_OPT | F_OPT_GLOBAL  | F_OPT_MULTIUSE,
	T_OPT_GP	= T_DEF_OPT | F_OPT_GLOBAL                   | F_OPT_PARAM,
	T_OPT_GMP	= T_DEF_OPT | F_OPT_GLOBAL  | F_OPT_MULTIUSE | F_OPT_PARAM,
	T_OPT_GO	= T_DEF_OPT | F_OPT_GLOBAL                   | F_OPT_OPTPARAM,
	T_OPT_GMO	= T_DEF_OPT | F_OPT_GLOBAL  | F_OPT_MULTIUSE | F_OPT_OPTPARAM,

	T_OPT_S		= T_DEF_OPT | F_OPT_GLOBAL | F_SUPERSEDE,


	T_COPT		= T_CMD_OPT,
	T_COPT_M	= T_CMD_OPT | F_OPT_MULTIUSE,


	//----- hidden options and commands (hide from help)

	H_DEF_TOOL	= F_HIDDEN | T_DEF_TOOL,
	H_DEF_CMD	= F_HIDDEN | T_DEF_CMD,

	H_OPT_C		= F_HIDDEN | T_OPT_C,
	H_OPT_CM	= F_HIDDEN | T_OPT_CM,
	H_OPT_CP	= F_HIDDEN | T_OPT_CP,
	H_OPT_CMP	= F_HIDDEN | T_OPT_CMP,
	H_OPT_CO	= F_HIDDEN | T_OPT_CO,
	H_OPT_CMO	= F_HIDDEN | T_OPT_CMO,

	H_OPT_G		= F_HIDDEN | T_OPT_G,
	H_OPT_GM	= F_HIDDEN | T_OPT_GM,
	H_OPT_GP	= F_HIDDEN | T_OPT_GP,
	H_OPT_GMP	= F_HIDDEN | T_OPT_GMP,
	H_OPT_GO	= F_HIDDEN | T_OPT_GO,
	H_OPT_GMO	= F_HIDDEN | T_OPT_GMO,

	H_COPT		= F_HIDDEN | T_COPT,
	H_COPT_M	= F_HIDDEN | T_COPT_M,

} enumType;

///////////////////////////////////////////////////////////////////////////////

typedef struct info_t
{
	enumType type;		// entry type
	ccp c_name;		// the C name
	ccp namelist;		// list of names
	ccp param;		// name of parameter
	ccp help;		// help text

	int index;		// calculated index

} info_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			some helper macros		///////////////
///////////////////////////////////////////////////////////////////////////////

#define TEXT_WWT_OPT_REPAIR \
	"This option defines how to repair WBFS errors." \
	" The parameter is a comma separated list of the following keywords," \
	" case is ignored:" \
	" @NONE, FBT, INODES, STANDARD," \
	" RM-INVALID, RM-OVERLAP, RM-FREE, RM-EMPTY, RM-ALL, ALL@." \
	"\n " \
	" All keywords can be prefixed by '+' to enable that option," \
	" by a '-' to disable it or" \
	" by a '=' to enable that option and disable all others."

#define TEXT_OPT_CHUNK_MODE(def) \
	"Defines an operation mode for {--chunk-size} and {--max-chunks}." \
	" Allowed keywords are @'ANY'@ to allow any values," \
	" @'32K'@ to force chunk sizes with a multiple of 32 KiB," \
	" @'POW2'@ to force chunk sizes >=32K and with a power of 2" \
	" or @'ISO'@ for ISO images (more restrictive as @'POW2'@," \
	" best for USB loaders)." \
	" The case of the keyword is ignored." \
	" The default key is @'" def "'@." \
	"\n " \
	" @--chm@ is a shortcut for @--chunk-mode@."

#define TEXT_EXTRACT_LONG \
	"Print a summary line while extracting files." \
	" If set at least twice, print a status line for each extracted files."

#define TEXT_OPT_FILES \
	" This option can be used multiple times to extend the rule list." \
	" Rules beginning with a '+' or a '-' are allow or deny rules rules." \
	" Rules beginning with a ':' are macros for predefined rule sets." \
	"\1\n " \
	" See http://wit.wiimm.de/info/file-filter.html" \
	" for more details about file filters."

///////////////////////////////////////////////////////////////////////////////

#define TEXT_DIFF_QUIET \
	"Be quiet and print only error messages and failure messages on mismatch." \
	" The comparison is aborted at the first mismatch for each source image." \
	" If set twice print nothing and report the diff result only as exit status" \
	" and the complete comparison is aborted at the first mismatch at all."

#define TEXT_DIFF_VERBOSE \
	"The default is to print only differ messages." \
	" If set success messages and summaries are printed too." \
	" If set at least twice, a progress counter is printed too."

#define TEXT_DIFF_FILE_LIMIT \
	"This option is only used if comparing discs on file level." \
	" If not set or set to null, then all files will be compared." \
	" If set to a value greater than comparison is aborted for" \
	" the current source image if the entered number of files differ." \
	" This option is ignored in quiet mode."

#define TEXT_DIFF_LIMIT \
	"If not set, the comparison of the current file is aborted" \
	" if a mismatch is found." \
	" If set, the comparison is aborted after @'limit'@ mismatches." \
	" To compare the whole file use the special value @0@." \
	" This option is ignored in quiet mode."

#define TEXT_DIFF_LONG \
	"If set, a status line with the offset is printed for each found mismatch." \
	" If set twice, an additional hex dump of the first bytes is printed." \
	" If set 3 or 4 times, the limit is set to 10 or unlimited" \
	" if option {--limit} is not already set." \
	" This option is ignored in quiet mode."

#define TEXT_DIFF_BLOCK_SIZE \
	"If a mismatch is found in raw or disc mode then the comparison" \
	" is continued with the next block. This option sets the block size." \
	" The default value is @32K@ (Wii sector size)." \
	" This option is ignored in quiet mode."

//
///////////////////////////////////////////////////////////////////////////////
///////////////			the info table			///////////////
///////////////////////////////////////////////////////////////////////////////

info_t info_tab[] =
{
    #include "tab-wit.inc"
    #include "tab-wwt.inc"
    #include "tab-wdf.inc"
    #include "tab-wfuse.inc"

    { T_END, 0,0,0,0 }
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    E N D			///////////////
///////////////////////////////////////////////////////////////////////////////
