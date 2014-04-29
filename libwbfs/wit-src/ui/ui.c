
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

#include <ctype.h>
#include <getopt.h>

#include "lib-std.h"
#include "version.h"
#include "ui.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			register options		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError RegisterOptionByIndex
(
    const InfoUI_t	* iu,		// valid pointer
    int			opt_index,	// index of option (OPT_*)
    int			level,		// the level of registration
    bool		is_env		// true: register environment pre setting
)
{
    ASSERT(iu);
    DASSERT(iu->opt_info);
    DASSERT(iu->opt_used);
    DASSERT(iu->opt_index);

    if ( level <= 0 )
	return ERR_OK;

    if ( opt_index >= 0
	 && opt_index < iu->n_opt_total
	 && opt_index < OPT_INDEX_SIZE )
    {
	TRACE("OPT: RegisterOptionByIndex(,%02x,%d,%d) opt_index=%02x,%s\n",
		    opt_index, level, is_env, opt_index,
		    iu->opt_info[opt_index].long_name );
	u8 * obj = iu->opt_used + opt_index;
	u32 count = *obj;
	if (is_env)
	{
	    if ( count < 0x7f )
	    {
		count += level;
		*obj = count < 0x7f ? count : 0x7f;
	    }
	}
	else
	{
	    if ( count < 0x80 )
		count = 0x80;
	    count += level;
	    *obj = count < 0xff ? count : 0xff;
	}

	return ERR_OK;
    }

    PRINT("OPT: RegisterOptionbyIndex(,%02x/%02x/%02x,%d,%d) => WARNING\n",
		opt_index, iu->n_opt_total, OPT_INDEX_SIZE, level, is_env );
    TRACE("OPT: RegisterOptionbyIndex(,%02x/%02x/%02x,%d,%d) => WARNING\n",
		opt_index, iu->n_opt_total, OPT_INDEX_SIZE, level, is_env );
    return ERR_WARNING;
}

///////////////////////////////////////////////////////////////////////////////

enumError RegisterOptionByName
(
    const InfoUI_t	* iu,		// valid pointer
    int			opt_name,	// short name of GO_* valus of option
    int			level,		// the level of registration
    bool		is_env		// true: register environment pre setting
)
{
    ASSERT(iu);
    DASSERT(iu->opt_index);

    if ( opt_name >= 0 && opt_name < OPT_INDEX_SIZE )
	return RegisterOptionByIndex(iu,iu->opt_index[opt_name],level,is_env);

    PRINT("OPT: RegisterOptionbyName(,%02x,%d,%d) => WARNING\n",
		opt_name, level, is_env );
    TRACE("OPT: RegisterOptionbyName(,%02x,%d,%d) => WARNING\n",
		opt_name, level, is_env );
    return ERR_WARNING;
}

///////////////////////////////////////////////////////////////////////////////

enumError VerifySpecificOptions ( const InfoUI_t * iu, const CommandTab_t * cmd )
{
    ASSERT(iu);
    ASSERT(cmd);

    ASSERT( cmd->id > 0 && cmd->id < iu->n_cmd );
    const InfoCommand_t * ic = iu->cmd_info + cmd->id;
    ASSERT(ic->opt_allowed);
    u8 * allow  = ic->opt_allowed;
    u8 * active = iu->opt_used;

    TRACE("ALLOWED SPECIFIC OPTIONS:\n");
    TRACE_HEXDUMP(10,0,0,-20,allow,iu->n_opt_specific);
    TRACE("ACTIVE SPECIFIC OPTIONS:\n");
    TRACE_HEXDUMP(10,0,0,-20,active,iu->n_opt_specific);

    enumError err = ERR_OK;
    int idx;
    for ( idx = 0; idx < iu->n_opt_specific; idx++, allow++, active++ )
    {
	if (!*allow)
	{
	    if ( *active & 0x80 )
	    {
		const InfoOption_t * io = iu->opt_info + idx;
		if (io->short_name)
		    ERROR0(ERR_SEMANTIC,"Command '%s' don't allow the option --%s (-%c).\n",
				cmd->name1, io->long_name, io->short_name );
		else
		    ERROR0(ERR_SEMANTIC,"Command '%s' don't allow the option --%s.\n",
				cmd->name1, io->long_name );
		err = ERR_SEMANTIC;
		
	    }
	    else 
		*active = 0; // clear environ settings
	}
    }
    return err;
}

///////////////////////////////////////////////////////////////////////////////

int GetOptionCount ( const InfoUI_t * iu, int option )
{
    ASSERT(iu);
    ASSERT(iu->opt_info);
    ASSERT(iu->opt_used);

    if ( option > 0 && option <  iu->n_opt_total )
    {
	TRACE("GetOptionCount(,%02x) option=%s => %d\n",
		option, iu->opt_info[option].long_name,
		iu->opt_used[option] & OPT_USED_MASK );
	return iu->opt_used[option] & OPT_USED_MASK;
    }

    TRACE("GetOptionCount(,%02x) => INVALID [-1]\n",option);
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

void DumpUsedOptions ( const InfoUI_t * iu, FILE * f, int indent )
{
    ASSERT(iu);
    ASSERT(iu->opt_info);
    ASSERT(iu->opt_used);

    TRACE("OptionUsed[]:\n");
    TRACE_HEXDUMP16(9,0,iu->opt_used,iu->n_opt_total+1);

    if (!f)
	return;

    indent = NormalizeIndent(indent);

    int i;
    for ( i = 0; i < iu->n_opt_total; i++ )
    {
	const u8 opt = iu->opt_used[i];
	if (opt)
	    fprintf(f,"%*s%s %s %2u* [%02x,%s]\n",
			indent, "",
			i <= iu->n_opt_specific ? "CMD" : "GLB",
			opt & 0x80 ? "PRM" : "ENV",
			opt & OPT_USED_MASK,
			i, iu->opt_info[i].long_name );
    }
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 text helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

void PutLines
(
    FILE	* f,		// valid output stream
    int		indent,		// indent of output
    int		fw,		// field width of output
    int		first_line,	// length without prefix of already printed first line 
    ccp		prefix,		// NULL or prefix for each line
    ccp		text		// text to print
)
{
    DASSERT(f);
    DASSERT( indent >= 0 );

    if (!prefix)
	prefix = "";
    TRACE("PutLines(,%d,%d,%d,%.10s,%.20s)\n",indent,fw,first_line,prefix,text);
    fw -= strlen(prefix);
    if ( fw < 10 )
	fw = 10;

    ccp prefix1 = "";
    int indent1, fw1;
    if (  indent > first_line )
    {
	indent1 = indent - first_line;
	fw1 = fw - indent;
    }
    else
    {
	indent1 = 0;
	fw1 = fw - first_line;
    }

    fw -= indent;
    if ( fw < 20 )
	fw = 20;

    if ( fw1 < 20 )
    {
	fputc('\n',f);
	indent1 = indent;
	fw1 = fw;
	prefix1 = prefix;
    }
    
    while ( *text )
    {
	// skip blank and control
	if ( *text == '\n' )
	{
	    // don't skip spaces behind a LF ==> needed for tables
	    while ( *text > 0 && *text < ' ' )
		text++;
	}
	else
	{
	    // but ignore spaces on an automatic line break
	    while ( *text > 0 && *text <= ' ' )
		text++;
	}

	// setup
	ccp start = text, last_blank = text;
	ccp max = text + fw1;

	while ( text < max && *text && *text != '\n' )
	{
	    if ( *text > 0 && *text <= ' ' )
		last_blank = text;
	    text++;
	}

	// set back to last blank
	if ( last_blank > start && (u8)*text > ' ' )
	    text = last_blank;

	// print out
	if ( *text || text > start )
	    fprintf(f,"%s%*s%.*s\n", prefix1, indent1, "", (int)(text-start), start );

	// use standard values for next lines
	indent1 = indent;
	fw1 = fw;
	prefix1 = prefix;
    }
}

///////////////////////////////////////////////////////////////////////////////

void PrintLines
(
    FILE	* f,		// valid output stream
    int		indent,		// indent of output
    int		fw,		// field width of output
    int		first_line,	// length without prefix of already printed first line 
    ccp		prefix,		// NULL or prefix for each line
    ccp		format,		// format string for vsnprintf()
    ...				// arguments for 'vsnprintf(format,...)'
)
{
    DASSERT(f);
    DASSERT(format);

    char msg[5000];

    va_list arg;
    va_start(arg,format);
    vsnprintf(msg,sizeof(msg),format,arg);
    va_end(arg);

    PutLines(f,indent,fw,first_line,prefix,msg);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   print help			///////////////
///////////////////////////////////////////////////////////////////////////////

void PrintHelp
(
    const InfoUI_t * iu,	// valid pointer
    FILE	* f,		// valid output stream
    int		indent,		// indent of output
    ccp		help_cmd,	// NULL or name of help command
    ccp		info		// NULL or additional text
)
{
    int cmd = 0;
    if (iu->n_cmd)
    {
	char buf[100];

	ParamList_t * param;
	for ( param = first_param; param; param = param->next )
	    if ( param->arg && *param->arg )
	    {
		int cmd_stat;
		const CommandTab_t * ct = ScanCommand(&cmd_stat,param->arg,iu->cmd_tab);
		if (ct)
		    cmd = ct->id;
		else
		{
		    StringCat2S(buf,sizeof(buf),"+",param->arg);
		    ct = ScanCommand(&cmd_stat,buf,iu->cmd_tab);
		    if (ct)
			cmd = ct->id;
		}
		break;
	    }
    }
    
    PrintHelpCmd(iu,f,indent,cmd,help_cmd,info);
}

///////////////////////////////////////////////////////////////////////////////

void PrintHelpCmd
(
    const InfoUI_t * iu,	// valid pointer
    FILE	* f,		// valid output stream
    int		indent,		// indent of output
    int		cmd,		// index of command
    ccp		help_cmd,	// NULL or name of help command
    ccp		info		// NULL or poiner to additional text
)
{
    ASSERT(iu);
    if (!f)
	return;
    indent = NormalizeIndent(indent);

    if ( cmd < 0 || cmd >= iu->n_cmd )
	cmd = 0;
    const InfoCommand_t * ic = iu->cmd_info + cmd;

    const int fw = GetTermWidth(80,40) - 1;

    if (!cmd)
	snprintf(iobuf,sizeof(iobuf),"%s",iu->tool_name );
    else if (ic->name2)
	snprintf(iobuf,sizeof(iobuf),"%s %s|%s",iu->tool_name,ic->name1,ic->name2);
    else
	snprintf(iobuf,sizeof(iobuf),"%s %s",iu->tool_name,ic->name1);

    int len = fprintf(f,"\n%*s%s : ",indent,"",iobuf) - 1;
    PutLines(f,indent+len,fw,len,0,ic->help);

    len = fprintf(f,"\n%*sSyntax: ", indent, "" ) - 1;
    PutLines(f,indent+len,fw,len,0,ic->syntax);
    fputc('\n',f);

    if ( !cmd && iu->n_cmd )
    {
	fprintf(f,"%*sCommands:\n\n",indent,"");

	int fw1 = 0, fw2 = 0;
	const InfoCommand_t * ic;

	for ( ic = iu->cmd_info; ic->name1; ic++ )
	    if (!ic->hidden)
	    {

		const int len = strlen(ic->name1);
		if ( fw1 < len )
		     fw1 = len;
		if ( ic->name2 )
		{
		    const int len = strlen(ic->name2);
		    if ( fw2 < len )
			 fw2 = len;
		}
	    }
	const int fw12 = fw1 + ( fw2 ? fw2 + 3 : 0 );

	for ( ic = iu->cmd_info+1; ic->name1; ic++ )
	    if (!ic->hidden)
	    {
		if (ic->separator)
		    fputc('\n',f);
		int len;
		if ( ic->name2 )
		    len = fprintf(f,"%*s  %-*s | %-*s : ",
			    indent, "", fw1, ic->name1, fw2, ic->name2 );
		else
		    len = fprintf(f,"%*s  %-*s : ",
			    indent, "", fw12, ic->name1 );
		PutLines(f,indent+len,fw,len,0,ic->help);
	    }
	
	if (help_cmd)
	    fprintf(f,
		"\n%*sType '%s %s command' to get command specific help.\n\n",
		indent, "", iu->tool_name, help_cmd );
    }
    
    if ( ic->n_opt )
    {
	fprintf(f,"%*s%sptions:\n\n",
		indent,"", !cmd && iu->n_cmd ? "Global o" : "O" );

	const InfoOption_t ** iop;

	// calculate the base option field width
	int base_opt_fw = 0;
	for ( iop = ic->opt; *iop; iop++ )
	{
	    const InfoOption_t * io = *iop;
	    if (io->long_name)
	    {
		int len = strlen(io->long_name);
		if (io->param)
		    len += 1 + strlen(io->param);
		if ( base_opt_fw < len )
		     base_opt_fw = len;
	    }
	}
	
	const int max_fw = 2 + (fw+5)/8;
	if ( base_opt_fw > max_fw )
	     base_opt_fw = max_fw;

	// calculate the option field width
	int opt_fw = 0;
	for ( iop = ic->opt; *iop; iop++ )
	{
	    const InfoOption_t * io = *iop;
	    if (io->long_name)
	    {
		int len = strlen(io->long_name);
		if (io->param)
		    len += 1 + strlen(io->param);
		if ( len <= base_opt_fw && opt_fw < len )
		    opt_fw = len;
	    }
	}
	opt_fw += indent + 9;

	for ( iop = ic->opt; *iop; iop++ )
	{
	    const InfoOption_t * io = *iop;
	    if ( !io->short_name && !io->long_name )
	    {
		fputc('\n',f);
		continue;
	    }

	    int len;
	    if (!io->short_name)
		len = fprintf(f,"%*s     --%s%s%s ",
			indent, "",
			io->long_name,
			io->param ? " " : "",
			io->param ? io->param : "" );
	    else if (!io->long_name)
		len = fprintf(f,"%*s  -%c%s%s ",
			indent, "",
			io->short_name,
			io->param ? " " : "",
			io->param ? io->param : "" );
	    else
		len = fprintf(f,"%*s  -%c --%s%s%s ",
			indent, "",
			io->short_name,
			io->long_name,
			io->param ? " " : "",
			io->param ? io->param : "" );

	    if ( len > opt_fw )
	    {
		fputc('\n',f);
		PutLines(f,opt_fw,fw,0,0,io->help);
	    }
	    else
		PutLines(f,opt_fw,fw,len,0,io->help);
	}
	fputc('\n',f);
    }

    if (info)
	fputs(info,f);
	
    if (!cmd)
	fprintf(f,"%*sMore help is available at %s%s\n\n",
		indent, "", URI_HOME, iu->tool_name );
    else if (!ic->hidden)
    {
	char *dest = iobuf;
	ccp src;
	for ( src = ic->name1; *src; src++ )
	    *dest++ = *src == '_' ? '-' : tolower((int)*src);
	*dest = 0;

	fprintf(f,"%*sMore help is available at %scmd/%s/%s\n\n",
		indent, "", URI_HOME, iu->tool_name, iobuf );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////				END			///////////////
///////////////////////////////////////////////////////////////////////////////

