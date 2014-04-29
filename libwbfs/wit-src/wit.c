
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
#include <sys/time.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

#include "debug.h"
#include "version.h"
#include "wiidisc.h"
#include "lib-std.h"
#include "lib-sf.h"
#include "titles.h"
#include "iso-interface.h"
#include "wbfs-interface.h"
#include "match-pattern.h"
#include "crypt.h"

#include "ui-wit.c"
#include "logo.inc"

//-----------------------------------------------------------------------------

enumError cmd_mix();

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define TITLE WIT_SHORT ": " WIT_LONG " v" VERSION " r" REVISION \
	" " SYSTEM " - " AUTHOR " - " DATE

//
///////////////////////////////////////////////////////////////////////////////

static void help_exit ( bool xmode )
{
    fputs( TITLE "\n", stdout );

    if (xmode)
    {
	int cmd;
	for ( cmd = 0; cmd < CMD__N; cmd++ )
	    PrintHelpCmd(&InfoUI,stdout,0,cmd,0,0);
    }
    else
	PrintHelpCmd(&InfoUI,stdout,0,0,"HELP",0);

    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_version_section ( bool print_header )
{
    if (print_header)
	fputs("[version]\n",stdout);

    const u32 base = 0x04030201;
    const u8 * e = (u8*)&base;
    const u32 endian = be32(e);

    printf( "prog=" WIT_SHORT "\n"
	    "name=" WIT_LONG "\n"
	    "version=" VERSION "\n"
	    "beta=%d\n"
	    "revision=" REVISION  "\n"
	    "system=" SYSTEM "\n"
	    "endian=%u%u%u%u %s\n"
	    "author=" AUTHOR "\n"
	    "date=" DATE "\n"
	    "url=" URI_HOME WIT_SHORT "\n"
	    "\n"
	    , BETA_VERSION
	    , e[0], e[1], e[2], e[3]
	    , endian == 0x01020304 ? "little"
		: endian == 0x04030201 ? "big" : "mixed" );
}

///////////////////////////////////////////////////////////////////////////////

static void version_exit()
{
    if (print_sections)
	print_version_section(true);
    else if (long_count)
	print_version_section(false);
    else
	fputs( TITLE "\n", stdout );

    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

void print_title ( FILE * f )
{
    static bool done = false;
    if (!done)
    {
	done = true;
	if (print_sections)
	    print_version_section(true);
	else if ( verbose >= 1 && f == stdout )
	    fprintf(f,"\n%s\n\n",TITLE);
	else
	    fprintf(f,"*****  %s  *****\n",TITLE);
    }
}

///////////////////////////////////////////////////////////////////////////////

static const CommandTab_t * current_command = 0;

static void hint_exit ( enumError stat )
{
    if ( current_command )
	fprintf(stderr,
	    "-> Type '%s help %s' (pipe it to a pager like 'less') for more help.\n\n",
	    progname, CommandInfo[current_command->id].name1 );
    else
	fprintf(stderr,
	    "-> Type '%s -h' or '%s help' (pipe it to a pager like 'less') for more help.\n\n",
	    progname, progname );
    exit(stat);
}

///////////////////////////////////////////////////////////////////////////////

#define SYNTAX_ERROR syntax_error(__FUNCTION__,__FILE__,__LINE__)

static void syntax_error ( ccp func, ccp file, uint line )
{
    if ( current_command
	&& current_command->id >= 0
	&& current_command->id < InfoUI.n_cmd )
    {
	const InfoCommand_t *ic = InfoUI.cmd_info + current_command->id;
	if (strchr(ic->syntax,'\n'))
	{
	    ccp src = ic->syntax;
	    char *dest = iobuf;
	    while (*src)
		if ( (*dest++ = *src++) == '\n' )
		{
		    *dest++ = ' ';
		    *dest++ = ' ';
		    *dest++ = ' ';
		}
	    *dest = 0;
	    PrintError(func,file,line,0,ERR_SYNTAX,"Syntax:\n   %s\n",iobuf);
	}
	else
	    PrintError(func,file,line,0,ERR_SYNTAX,"Syntax: %s\n",ic->syntax);
    }
    else
	PrintError(func,file,line,0,ERR_SYNTAX,"Syntax Error!\n");
    exit(ERR_SYNTAX);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command TEST			///////////////
///////////////////////////////////////////////////////////////////////////////

// common commands of 'wwt' and 'wit'
#define IS_WIT 1
#include "wwt+wit-cmd.c"

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_test()
{
 #if 1 || !defined(TEST) // test options

    return cmd_test_options();

 #elif 0

    char buf[20];
    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
    {
	ccp arg = param->arg;
	while ( arg && *arg )
	{
	    arg = ScanArgID(buf,arg,false);
	    if (*buf)
	    {
		printf("%s ",buf);
		if (!*arg)
		    printf("= %s\n",param->arg);
	    }
	    else
	    {
		printf("ERR= %s\n",arg);
		break;
	    }
	}
    }
    return ERR_OK;

 #elif 1

    int i;
    ParamList_t * param;
    for ( i = 1, param = first_param; param; param = param->next, i++ )
	printf("%3d.: |%s|\n",i,param->arg);

    printf("----\n");
    for ( i = 1, param = first_param; param; param = param->next, i++ )
    {
	AtExpandParam(&param);
	printf("%3d.: |%s|\n",i,param->arg);
    }
    return ERR_OK;

 #elif 1

    printf("\n  partition selector:\n");
    wd_print_select(stdout,6,&part_selector);
    putchar('\n');
    return ERR_OK;

 #elif 1

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
    {
	u8 buf[10];
	int count;
	ccp res = ScanHexHelper(buf,sizeof(buf),&count,param->arg,99);
	printf("\ncount=%d |%s|\n",count,res);
	HEXDUMP16(0,0,buf,sizeof(buf));
    }
    return ERR_OK;

 #elif 1

    {
	ccp msg = "Dieses ist ein langer Satz, "
		  "der noch ein wenig länger ist "
		  "und noch ein wenig länger ist "
		  "und hier endet.\n";
	ERROR1(ERR_WARNING,"%s%s",msg,msg);
	ERROR1(ERR_ERROR,"%s%s",msg,msg);
    }
    return ERR_OK;

 #elif 1

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
    {
	char id6[7];
	enumFileType ftype = IsFST(param->arg,id6);
	printf("%05x %-6s %s\n",ftype,id6,param->arg);
    }
    return ERR_OK;

 #else

    int i, max = 5;
    for ( i=1; i <= max; i++ )
    {
	fprintf(stderr,"sleep 20 sec (%d/%d)\n",i,max);
	sleep(20);
    }
    return ERR_OK;

 #endif
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command ANAID			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_anaid()
{
    const bool print_header = !OptionUsed[OPT_NO_HEADER];
    if (print_header)
	printf(	"\n"
		" HEX       ASCII   Game Title\n"
		"%s\n",
		sep_79);

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
    {
	id6_t id6;
	memset(id6,0,sizeof(id6));
	bool valid = false;
	uint alen = strlen(param->arg);

	if ( alen >= 8 )
	    valid = ScanHexSilent( id6, 4, param->arg + alen - 8) == ERR_OK;
	else if ( alen >= 1 && alen <= 6 )
	{
	    ccp src = param->arg;
	    char *dest = id6;
	    while (*src)
		*dest++ = toupper((int)*src++);
	    valid = true;
	}


	if (valid)
	    printf(" %02x%02x%02x%02x  %-6s  %s\n",
		(uchar)id6[0], (uchar)id6[1], (uchar)id6[2], (uchar)id6[3],
		id6, GetTitle(id6,"?") );
	else
	    printf("? %s\n",param->arg);
    }

    if (print_header)
	putchar('\n');

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command CERT			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_cert()
{
    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AtFileHelper(param->arg,0,0,AddCertFile);

    cert_add_root(); // if not already inserted

    FILE * f = 0;
    if ( opt_dest && *opt_dest )
    {
	if (opt_mkdir)
	    CreatePath(opt_dest);
	f = fopen(opt_dest,"wb");
	if (!f)
	    return ERROR0(ERR_CANT_CREATE,"Can't create cert file: %s\n",opt_dest);
    }

    FilePattern_t * pat_select   = file_pattern + PAT_FILES;
    FilePattern_t * pat_fakesign = file_pattern + PAT_FAKE_SIGN;

    int i;
    for ( i = 0; i < global_cert.used; i++ )
    {
	cert_item_t * item = global_cert.cert + i;
	if (MatchFilePattern(pat_select,item->name,'-'))
	{
	    if ( pat_fakesign->is_active
			&& MatchFilePattern(pat_fakesign,item->name,'-') )
	    {
		cert_fake_sign(item);
	    }

	    if ( !f || verbose > 0 )
	    {
		Dump_CERT_Item(stdout,0,item,i,long_count+1,&global_cert);
		fputc('\n',stdout);
	    }

	    if ( f && item->head )
		fwrite(item->head,1,item->cert_size,f);
	}
    }

    if (f)
	fclose(f);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command CREATE			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_create()
{
    if ( n_param < 1 )
    {
	ERROR0(ERR_SYNTAX,"Missing sub command for CREATE.\n");
	hint_exit(ERR_SYNTAX);
    }

    //----- find sub command

    enum // sub commands
    {
	SC_TICKET,
	SC_TMD,
    };


    static const CommandTab_t tab[] =
    {						    // min + max param
	{ SC_TICKET,	"TICKET",	"TIK",		0 | 0x100 * 2 },
	{ SC_TMD,	"TMD",		0,		0 | 0x100 * 1 },

	{ 0,0,0,0 }
    };

    ParamList_t * param = first_param;
    ccp cmd_name = param->arg;
    int cmd_stat;
    const CommandTab_t * cmd = ScanCommand(&cmd_stat,cmd_name,tab);
    if (!cmd)
    {
	PrintCommandError(tab,cmd_name,cmd_stat,"sub command");
	hint_exit(ERR_SYNTAX);
    }

    param = param->next;


    //----- find destination file

    if ( !param )
    {
	ERROR0(ERR_SYNTAX,"Missing filename for CREATE.\n");
	hint_exit(ERR_SYNTAX);
    }

    char pbuf1[PATH_MAX], pbuf2[PATH_MAX], namebuf[50];
    ccp path = PathCatPP(pbuf1,sizeof(pbuf1),opt_dest,param->arg);
    if (IsDirectory(path,0))
    {
	ccp src = cmd->name1;
	char * dest = namebuf;
	while (*src)
	    *dest++ = tolower((int)*src++);
	DASSERT( dest < namebuf + sizeof(namebuf) );

	src = ".bin";
	while (*src)
	    *dest++ = tolower((int)*src++);
	DASSERT( dest < namebuf + sizeof(namebuf) );
	*dest = 0;

	path = PathCatPP(pbuf2,sizeof(pbuf2),path,namebuf);
    }

    param = param->next;


    //----- check number of params

    const int max_param = ( cmd->opt & 0xff00 ) >> 8;
    const int min_param = cmd->opt & 0xff;
    n_param -= 2;
    if ( n_param < min_param || n_param > max_param )
    {
	ERROR0(ERR_SYNTAX,"Wrong number of arguments.\n");
	hint_exit(ERR_SYNTAX);
    }

    if ( testmode || verbose > 0 )
	printf("Create %s: %s\n",cmd->name1,path);


    //----- execute

    switch(cmd->id)
    {
      case SC_TICKET:
	{
	    wd_ticket_t tik;
	    ticket_setup(&tik,modify_id /* [[id]] */);

	    if (param)
	    {
		if ( *param->arg && strcmp(param->arg,"-") )
		{
		    const enumError err
			= ScanHex(tik.ticket_id,sizeof(tik.ticket_id),param->arg);
		    if (err)
			return err;
		}
		param = param->next;
	    }

	    if (param)
	    {
		if ( *param->arg && strcmp(param->arg,"-") )
		{
		    u8 key[WII_KEY_SIZE];
		    const enumError err = ScanHex(key,sizeof(key),param->arg);
		    if (err)
			return err;
		    wd_encrypt_title_key(&tik,key);
		}
		param = param->next;
	    }

	    ticket_fake_sign(&tik,sizeof(tik));
	    if ( verbose > 1 )
		Dump_TIK_MEM(stdout,2,&tik,0);
	    if (!testmode)
	    {
		const enumError err = SaveFile(path,0,opt_mkdir,
						&tik,sizeof(tik),false);
		if (!err)
		    return err;
	    }
	}
	break;

      case SC_TMD:
	{
	    char tmd_buf[WII_TMD_GOOD_SIZE];
	    wd_tmd_t * tmd = (wd_tmd_t*)tmd_buf;
	    tmd_setup(tmd,sizeof(tmd_buf),modify_id /* [[id]] */);

	    if (opt_ios_valid)
		tmd->sys_version = hton64(opt_ios);

	    if (param)
	    {
		if ( *param->arg && strcmp(param->arg,"-") )
		{
		    const enumError err
			= ScanHex(tmd->content[0].hash,
				sizeof(tmd->content[0].hash),param->arg);
		    if (err)
			return err;
		}
		param = param->next;
	    }

	    tmd_fake_sign(tmd,sizeof(tmd_buf));
	    if ( verbose > 1 )
		Dump_TMD_MEM(stdout,2,tmd,1,0);
	    if (!testmode)
	    {
		const enumError err = SaveFile(path,0,opt_mkdir,
						tmd,sizeof(tmd_buf),false);
		if (!err)
		    return err;
	    }
	}
	break;
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command FILELIST		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_filelist ( SuperFile_t * sf, Iterator_t * it )
{
    ASSERT(sf);
    ASSERT(it);

    printf("%s\n", it->long_count ? it->real_path : sf->f.fname );
    return ERR_OK;
}

//-----------------------------------------------------------------------------

static enumError cmd_filelist()
{
    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    encoding |= ENCODE_F_FAST; // hint: no encryption needed

    Iterator_t it;
    InitializeIterator(&it);
    it.func		= exec_filelist;
    it.act_non_exist	= ignore_count > 0 ? ACT_IGNORE : ACT_ALLOW;
    it.act_non_iso	= ignore_count > 1 ? ACT_IGNORE : ACT_ALLOW;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= allow_fst ? ACT_ALLOW : ACT_IGNORE;
    it.long_count	= long_count;
    const enumError err = SourceIterator(&it,1,true,false);
    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command FILETYPE		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_filetype ( SuperFile_t * sf, Iterator_t * it )
{
    ASSERT(sf);
    ASSERT(it);

    const bool print_header = !OptionUsed[OPT_NO_HEADER];
    ccp ftype = GetNameFT(sf->f.ftype,0);

    if ( it->long_count > 1 )
    {
	if ( print_header && !it->done_count++  )
	    printf("\n"
		"file     disc   size reg split\n"
		"type     ID6     MIB ion   N  %s\n"
		"%s\n",
		it->long_count > 2 ? "real path" : "filename", sep_79 );

	char split[10] = " -";
	if ( sf->f.split_used > 1 )
	    snprintf(split,sizeof(split),"%2d",sf->f.split_used);

	ccp region = "-   ";
	char size[10] = "   -";
	if (sf->f.id6_dest[0])
	{
	    region = GetRegionInfo(sf->f.id6_dest[3])->name4;
	    u32 count = CountUsedIsoBlocksSF(sf,&part_selector);
	    if (count)
		snprintf(size,sizeof(size),"%4u",
			(count+WII_SECTORS_PER_MIB/2)/WII_SECTORS_PER_MIB);
	}

	printf("%-8s %-6s %s %s %s  %s\n",
		ftype, sf->f.id6_dest[0] ? sf->f.id6_dest : "-",
		size, region, split,
		it->long_count > 2 ? it->real_path : sf->f.fname );
    }
    else if (it->long_count)
    {
	if ( print_header && !it->done_count++  )
	    printf("\n"
		"file     disc  split\n"
		"type     ID6     N  filename\n"
		"%s\n", sep_79 );

	char split[10] = " -";
	if ( sf->f.split_used > 1 )
	    snprintf(split,sizeof(split),"%2d",sf->f.split_used);
	printf("%-8s %-6s %s  %s\n",
		ftype, sf->f.id6_dest[0] ? sf->f.id6_dest : "-",
		split, sf->f.fname );
    }
    else
    {
	if ( print_header && !it->done_count++  )
	    printf("\n"
		"file\n"
		"type     filename\n"
		"%s\n", sep_79 );

	printf("%-8s %s\n", ftype, sf->f.fname );
    }

    return ERR_OK;
}

//-----------------------------------------------------------------------------

static enumError cmd_filetype()
{
    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    encoding |= ENCODE_F_FAST; // hint: no encryption needed

    Iterator_t it;
    InitializeIterator(&it);
    it.func		= exec_filetype;
    it.act_non_exist	= ignore_count > 0 ? ACT_IGNORE : ACT_ALLOW;
    it.act_non_iso	= ignore_count > 1 ? ACT_IGNORE : ACT_ALLOW;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= !allow_fst ? ACT_IGNORE
					 : long_count > 1 ? ACT_EXPAND : ACT_ALLOW;
    it.long_count	= long_count;
    const enumError err = SourceIterator(&it,1,true,false);

    if ( !OptionUsed[OPT_NO_HEADER] && it.done_count )
	putchar('\n');

    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command ISOSIZE			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_isosize ( SuperFile_t * sf, Iterator_t * it )
{
    ASSERT(sf);
    ASSERT(it);

    wd_disc_t * disc = 0;
    if (sf->f.id6_dest[0])
    {
	disc = OpenDiscSF(sf,true,true);
	if (disc)
	    wd_filter_usage_table(disc,wdisc_usage_tab,0);
    }

    const bool print_header = !OptionUsed[OPT_NO_HEADER];
    const int size_fw = wd_get_size_fw(opt_unit,4);
    const u64 blocks = disc ? wd_count_used_blocks(wdisc_usage_tab,1) : 0;
    it->sum += blocks;

    if ( it->long_count > 1 )
    {
	const int wbfs_fw = size_fw > 5 ? size_fw : 5;
	if ( !it->done_count++ && print_header )
	    printf("\n"
		"   ISO %*s %*s %*s %*s\n"
		"blocks %*s %*s %*s %*s  %s\n"
		"%s\n",
		size_fw, "ISO",
		size_fw, ".wbfs",
		size_fw, "500g",
		size_fw, "  3t",
		size_fw, wd_get_size_unit(opt_unit,"?"),
		wbfs_fw, "file",
		size_fw, "WBFS",
		size_fw, "WBFS",
		it->long_count > 2 ? "real path" : "filename",
		sep_79 );

	if (blocks)
	{
	    // wbfs: size=10g => block size = 2 MiB
	    const u32 wfile = 1 + wd_count_used_blocks( wdisc_usage_tab,
						2 * WII_SECTORS_PER_MIB );
	    // wbfs: size=500g => block size = 8 MiB
	    const u32 w500g = wd_count_used_blocks( wdisc_usage_tab,
						8 * WII_SECTORS_PER_MIB );

	    // wbfs: size=3t => block size = 8 MiB
	    const u32 w3t   = wd_count_used_blocks( wdisc_usage_tab,
						64 * WII_SECTORS_PER_MIB );

	    printf("%6llu %*s %*s %*s %*s  %s\n",
		blocks,
		size_fw, wd_print_size(0,0,blocks*WII_SECTOR_SIZE,false,opt_unit),
		wbfs_fw, wd_print_size(0,0,2ull*MiB*wfile,false,opt_unit),
		size_fw, wd_print_size(0,0,8ull*MiB*w500g,false,opt_unit),
		size_fw, wd_print_size(0,0,64ull*MiB*w3t,false,opt_unit),
		sf->f.fname );
	}
	else	
	    printf("     - %*s %*s %*s  %s\n",
		size_fw, "-",
		wbfs_fw, "-",
		size_fw, "-",
		sf->f.fname );
    }
    else if ( it->long_count )
    {
	if ( !it->done_count++ && print_header )
	    printf("\n"
		"   ISO %*s\n"
		"blocks %*s  filename\n"
		"%s\n",
		size_fw, "ISO",
		size_fw, wd_get_size_unit(opt_unit,"?"),
		sep_79 );

	if (blocks)
	    printf("%6llu %*s  %s\n",
		blocks,
		size_fw, wd_print_size(0,0,blocks*WII_SECTOR_SIZE,false,opt_unit),
		sf->f.fname );
	else	
	    printf("     - %*s  %s\n", size_fw, "-", sf->f.fname );
    }
    else
    {
	if ( !it->done_count++ && print_header )
	    printf("\n"
		"%*s\n"
		"%*s  filename\n"
		"%s\n",
		size_fw, "ISO",
		size_fw, wd_get_size_unit(opt_unit,"?"),
		sep_79 );

	ccp size = "-";
	if (blocks)
	    size = wd_print_size(0,0,blocks*WII_SECTOR_SIZE,false,opt_unit);
	printf("%*s  %s\n", size_fw, size, sf->f.fname );
    }

    return ERR_OK;
}

//-----------------------------------------------------------------------------

static enumError cmd_isosize()
{
    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    encoding |= ENCODE_F_FAST; // hint: no encryption needed

    wd_size_mode_t unit_mode = opt_unit & WD_SIZE_M_MODE;
    if ( unit_mode == WD_SIZE_DEFAULT || unit_mode == WD_SIZE_WD_SECT && long_count )
	unit_mode = WD_SIZE_M;
    const wd_size_mode_t sum_unit = opt_unit & ~WD_SIZE_M_MODE | unit_mode;
    const wd_size_mode_t total_unit = opt_unit & WD_SIZE_M_BASE | WD_SIZE_AUTO;
    opt_unit = sum_unit | WD_SIZE_F_AUTO_UNIT;

    Iterator_t it;
    InitializeIterator(&it);
    it.act_non_exist	= ignore_count > 0 ? ACT_IGNORE : ACT_ALLOW;
    it.act_non_iso	= ignore_count > 1 ? ACT_IGNORE : ACT_ALLOW;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= allow_fst ? ACT_EXPAND : ACT_IGNORE;
    it.act_wbfs		= ACT_EXPAND;
    it.long_count	= long_count;

    enumError err = SourceIterator(&it,1,true,true);
    if ( err <= ERR_WARNING )
    {
	it.func = exec_isosize;
	err = SourceIteratorCollected(&it,0,1,false);
    }

    if ( !OptionUsed[OPT_NO_HEADER] && it.done_count )
    {
	if ( it.done_count > 1 )
	{
	    const u64 total_size = it.sum * WII_SECTOR_SIZE;
	    ccp size1 = wd_print_size(0,0,total_size,false,sum_unit);
	    ccp size2 = wd_print_size(0,0,total_size,false,total_unit);

	    if (long_count)
	    {
		if (strcmp(size1,size2))
		    printf("%s\nTotal: %llu blocks (%s = %s) in %u files\n",
			sep_79, it.sum, size1, size2, it.done_count );
		else
		    printf("%s\nTotal: %llu blocks (%s) in %u files\n",
			sep_79, it.sum, size1, it.done_count );
	    }
	    else
	    {
		const int size_fw = wd_get_size_fw(opt_unit,4)+4;
		if (strcmp(size1,size2))
		    printf("%s\n%*s (%s) in %u files\n",
			    sep_79, size_fw, size1, size2, it.done_count );
		else
		    printf("%s\n%*s in %u files\n",
			    sep_79, size_fw, size1, it.done_count );
	    }
	}
	putchar('\n');
    }

    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command DUMP			///////////////
///////////////////////////////////////////////////////////////////////////////

static void dump_data ( off_t base, u32 off4, off_t size, ccp text )
{
    const u64 off = (off_t)off4 << 2;
    const u64 end = off + size;
    printf("    %-5s %9llx .. %9llx -> %9llx .. %9llx, size:%10llx/hex =%11llu\n",
		text, off, end, (u64)base+off, (u64)base+end, (u64)size, (u64)size );
}

//-----------------------------------------------------------------------------

enumError exec_dump ( SuperFile_t * sf, Iterator_t * it )
{
    TRACE("exec_dump()");
    ASSERT(sf);
    ASSERT(it);
    fflush(0);

    if ( sf->f.ftype & FT_A_ISO )
	return Dump_ISO(stdout,0,sf,it->real_path,opt_show_mode,it->long_count);

    if ( sf->f.ftype & FT_ID_DOL )
	return Dump_DOL(stdout,0,sf,it->real_path);

    if ( sf->f.ftype & FT_ID_FST_BIN )
	return Dump_FST_BIN(stdout,0,sf,it->real_path,opt_show_mode);

    if ( sf->f.ftype & FT_ID_CERT_BIN )
	return Dump_CERT_BIN(stdout,0,sf,it->real_path,1);

    if ( sf->f.ftype & FT_ID_TIK_BIN )
	return Dump_TIK_BIN(stdout,0,sf,it->real_path);

    if ( sf->f.ftype & FT_ID_TMD_BIN )
	return Dump_TMD_BIN(stdout,0,sf,it->real_path);

    if ( sf->f.ftype & FT_ID_HEAD_BIN )
	return Dump_HEAD_BIN(stdout,0,sf,it->real_path);

    if ( sf->f.ftype & FT_ID_BOOT_BIN )
	return Dump_BOOT_BIN(stdout,0,sf,it->real_path);

    if ( sf->f.ftype & FT_ID_PATCH )
	return Dump_PATCH(stdout,0,sf,it->real_path);

    return OptionUsed[OPT_IGNORE]
		? ERR_OK
		: ERROR0(ERR_INVALID_FILE,
			"Can't dump this file type: %s\n",sf->f.fname);
}

//-----------------------------------------------------------------------------

static enumError cmd_dump()
{
    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    encoding |= ENCODE_F_FAST; // hint: no encryption needed

    Iterator_t it;
    InitializeIterator(&it);
    it.func		= exec_dump;
    it.act_known	= ACT_ALLOW;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= allow_fst ? ACT_EXPAND : ACT_IGNORE;
    it.long_count	= long_count;

    enumError err = SourceIterator(&it,0,false,true);
    if ( err <= ERR_WARNING )
	err = SourceIteratorCollected(&it,0,1,false);
    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    command ID6 --long			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_print_id ( SuperFile_t * sf, Iterator_t * it )
{
    DASSERT(sf);

    wd_disc_t * disc = OpenDiscSF(sf,true,true);
    if (!disc)
	return ERR_OK;

    wd_part_t *part = it->long_count > 1 ? disc->part : disc->main_part;

    if ( !part || !disc->n_part )
	printf("%6s  --   --    --  ", wd_print_id(&disc->dhead,6,0));
    else
    {
	const u32 *tick_id = (u32*)(part->ph.ticket.title_id+4);
	printf("%6s %4s %4s %6s",
		wd_print_id(&disc->dhead,6,0),
		tick_id[0] ? wd_print_id(tick_id,4,0) : "-- ",
		part->tmd ? wd_print_id(part->tmd->title_id+4,4,0) : "-- ",
		wd_print_id(&part->boot,6,0) );
    }

    printf(" %-6s %s  %s\n",
	sf->wbfs_id6[0] ? sf->wbfs_id6 : "  --",
	wd_print_part_name(0,0,part->part_type,WD_PNAME_NAME_NUM_9),
	sf->f.fname );

    if ( it->long_count < 2 )
	return ERR_OK;

    int pi;
    for ( pi = 1; pi < disc->n_part; pi++ )
    {
	part = wd_get_part_by_index(disc,pi,0);
	const u32 *tick_id = (u32*)(part->ph.ticket.title_id+4);
	printf("     > %4s %4s %-13s %s\n",
		tick_id[0] ? wd_print_id(tick_id,4,0) : "-- ",
		part->tmd ? wd_print_id(part->tmd->title_id+4,4,0) : "-",
		wd_print_id(&part->boot,6,0),
		wd_print_part_name(0,0,part->part_type,WD_PNAME_NAME_NUM_9) );
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_id6_long()
{
    const bool print_header = !OptionUsed[OPT_NO_HEADER];
    if (print_header)
	printf(	"\n"
		" DISC TICKET TMD  BOOT   WBFS\n"
		"  ID    ID   ID    ID     ID   Partition  File\n"
		"%.79s\n", sep_79);

    Iterator_t it;
    InitializeIterator(&it);
    it.func		= exec_print_id;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= allow_fst ? ACT_EXPAND : ACT_IGNORE;
    it.long_count	= long_count;

    enumError err = SourceIterator(&it,0,true,false);
    ResetIterator(&it);
    if (print_header)
	putchar('\n');
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command ID6			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_collect ( SuperFile_t * sf, Iterator_t * it )
{
    ASSERT(sf);
    ASSERT(it);
    ASSERT(it->wlist);

    WDiscInfo_t wdi;
    InitializeWDiscInfo(&wdi);
    CalcWDiscInfo(&wdi,sf);

    WDiscList_t * wl = it->wlist;
    WDiscListItem_t * item = AppendWDiscList(wl,&wdi);
    if ( OptionUsed[OPT_REALPATH] )
    {
	int len = strlen(it->real_path);
	if ( it->real_filename && sf->f.ftype & FT_A_WDISC )
	{
	    ccp slash = strrchr(it->real_path,'/');
	    if (slash)
		len = slash - it->real_path;
	}
	item->fname = MEMDUP(it->real_path,len);
    }
    else if ( it->real_filename && sf->f.path && *sf->f.path )
    {
	item->fname = sf->f.path;
	sf->f.path  = EmptyString;
    }
    else
    {
	item->fname = sf->f.fname;
	sf->f.fname = EmptyString;
    }
    TRACE("WLIST: %d/%d\n",wl->used,wl->size);

    item->used_blocks = wdi.used_blocks;
    item->size_mib = (sf->f.fatt.size+MiB/2)/MiB;
    wl->total_size_mib += item->size_mib;

    item->ftype = sf->f.ftype;
    if ( sf->f.ftype & FT_ID_WBFS && sf->wbfs && sf->wbfs->disc )
    {
	item->wbfs_slot = sf->wbfs->disc->slot;
	item->wbfs_fragments = wbfs_get_disc_fragments(sf->wbfs->disc,0);
    }
    else
    {
	item->wbfs_slot = -1;
	item->wbfs_fragments = 0;
    }
    CopyFileAttrib(&item->fatt,&sf->f.fatt);

    ResetWDiscInfo(&wdi);

    if ( print_sections && scan_progress > 0 && !it->scan_progress )
    {
	printf("[progress:found]\n");
	PrintSectWDiscListItem(stdout,item,0);
	putchar('\n');
	fflush(stdout);
    }

    return ERR_OK;
}

//-----------------------------------------------------------------------------

static enumError cmd_id6()
{
    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    encoding |= ENCODE_F_FAST; // hint: no encryption needed

    if (long_count)
	return cmd_id6_long();

    StringField_t id6_list;
    InitializeStringField(&id6_list);

    WDiscList_t wlist;
    InitializeWDiscList(&wlist);

    Iterator_t it;
    InitializeIterator(&it);
    it.func		= exec_collect;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= allow_fst ? ACT_EXPAND : ACT_IGNORE;
    it.long_count	= long_count;
    it.wlist		= &wlist;

    enumError err = SourceIterator(&it,0,true,false);
    ResetIterator(&it);
    if ( err > ERR_WARNING )
	return err;

    AppendWListID6(&id6_list,0,&wlist,long_count>0);
    ResetWDiscList(&wlist);

    int i;
    for ( i = 0; i < id6_list.used; i++ )
	printf("%s\n",id6_list.field[i]);

    ResetStringField(&id6_list);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command MAPPING			///////////////
///////////////////////////////////////////////////////////////////////////////

static void print_fragments
    ( SuperFile_t * sf, FileMap_t *fm, ccp title, ccp off1, ccp off2 )
{
    uint i;
    u64 max1a = 0x1000000, max1b = 0x1000;
    u64 max2a =    0x1000, max2b = 0x1000, max_s = 0x1000;
    u64 align1a = 0, align1b = 0, align2a = 0, align2b = 0, align_s = 0;
    for ( i = 0; i < fm->used; i++ )
    {
	const FileMapItem_t *mi = fm->field + i;

	align1a |= mi->src_off;
	if ( max1a < mi->src_off )
	     max1a = mi->src_off;

	u64 temp = mi->src_off + mi->size;
	align1b |= temp;
	if ( max1b < temp )
	     max1b = temp;

	align2a |= mi->dest_off;
	if ( max2a < mi->dest_off )
	     max2a = mi->dest_off;

	temp = mi->dest_off + mi->size;
	align2b |= temp;
	if ( max2b < temp )
	     max2b = temp;

	align_s |= mi->size;
	if ( max_s < mi->size )
	     max_s = mi->size;
    }

    char buf[50];
    const uint fw_1a = snprintf(buf,sizeof(buf),"%llx",max1a);
    const uint fw_1b = snprintf(buf,sizeof(buf),"%llx",max1b-1);
    const uint fw_2a = snprintf(buf,sizeof(buf),"%llx",max2a);
    const uint fw_2b = snprintf(buf,sizeof(buf),"%llx",max2b-1);
    const uint fw_s  = snprintf(buf,sizeof(buf),"%llx",max_s);

    const uint fw_h1  = fw_1a + 4 + fw_1b;
    const uint fw_h1b = fw_h1/2 + 3;
    const uint fw_h1a = fw_h1 - fw_h1b;

    const uint fw_h2  = fw_2a + 4 + fw_2b;
    const uint fw_h2b = fw_h2/2 + 3;
    const uint fw_h2a = fw_h2 - fw_h2b;

    const uint fw_sep = fw_h1 + fw_h2 + fw_s + 16;

    printf(
	"\n"
	"%s of %s:%s\n"
	"\n"
	"          %*s%-*s -> %*s%-*s : %*s\n"
	"   index  %*s%-*s -> %*s%-*s : %*s\n"
	"  %.*s\n",
	title, oft_info[sf->iod.oft].name, sf->f.fname,
	fw_h1a, "", fw_h1b, off1,
	fw_h2a, "", fw_h2b, off2,
	fw_s, "chunk",
	fw_h1a, "", fw_h1b, "offset",
	fw_h2a, "", fw_h2b, "offset",
	fw_s, "size",
	fw_sep, wd_sep_200 );

    for ( i = 0; i < fm->used; i++ )
    {
	const FileMapItem_t *mi = fm->field + i;
	printf("%7d.  %*llx .. %*llx -> %*llx .. %*llx : %*llx\n",
		i+1,
		fw_1a, mi->src_off,  fw_1b, mi->src_off + mi->size - 1,
		fw_2a, mi->dest_off, fw_2b, mi->dest_off + mi->size - 1,
		fw_s, mi->size );
    }

    if (long_count)
    {
	printf("  %.*s\n",fw_sep,wd_sep_200);

	if ( long_count != 1 )
	    printf(
		"   align: %*llx    %*llx    %*llx    %*llx   %*llx\n",
		fw_1a, GetAlign64(align1a), fw_1b, GetAlign64(align1b),
		fw_2a, GetAlign64(align2a), fw_2b, GetAlign64(align2b),
		fw_s,  GetAlign64(align_s) );

	if ( long_count != 2 )
	    printf(
		"   align:%*s %*s %*s %*s %*s\n",
		fw_1a + 1, wd_print_size_1024(0,0,GetAlign64(align1a),false),
		fw_1b + 3, wd_print_size_1024(0,0,GetAlign64(align1b),false),
		fw_2a + 3, wd_print_size_1024(0,0,GetAlign64(align2a),false),
		fw_2b + 3, wd_print_size_1024(0,0,GetAlign64(align2b),false),
		fw_s  + 2, wd_print_size_1024(0,0,GetAlign64(align_s),false) );
    }
}

///////////////////////////////////////////////////////////////////////////////

static enumError exec_fragments ( SuperFile_t * sf, Iterator_t * it )
{
    DASSERT(sf);
    DASSERT(it);

    if ( !it->done_count++ && brief_count && !OptionUsed[OPT_NO_HEADER] )
	printf(	"\n"
		"image filesys real\n"
		"_____fragments____ %s type file\n"
		"%s\n",
		long_count ? "aligning " : "", sep_79);

    const bool is_reg = S_ISREG(sf->f.st.st_mode);
    FileMap_t fm1, fm2, fm3, *fm0 = is_reg ? &fm1 : &fm3;
    
    if (!GetFileMapSF(sf,fm0,true))
    {
	if (brief_count)
	    printf("%4s %6s %6s %s %-4s %s\n",
		"-", "-", "-",
		long_count ? "    -    " : "",
		oft_info[sf->iod.oft].name, sf->f.fname );
	else
	    printf("\n* No file map for %s:%s\n",
		oft_info[sf->iod.oft].name, sf->f.fname );
	ResetFileMap(fm0);
	return 0;
    }

    bool have_system_map;
    if (is_reg)
    {
	have_system_map = GetFileSystemMap(&fm2,true,&sf->f) == ERR_OK;
	if (have_system_map)
	    CombineFileMaps(&fm3,true,&fm1,&fm2);
	else
	    InitializeFileMap(&fm3);
    }
    else
    {
	have_system_map = true;
	InitializeFileMap(&fm1);
	InitializeFileMap(&fm2);
    }

    if (brief_count)
    {
	char buf[20];
	if (long_count)
	{
	    const FileMap_t *fm = have_system_map ? &fm3 : &fm1;
	    uint i;
	    u64 align = 0;
	    for ( i = 0; i < fm->used; i++ )
	    {
		const FileMapItem_t *mi = fm->field + i;
		align |= mi->src_off | mi->dest_off | mi->size;
	    }
	    wd_print_size( buf, sizeof(buf), GetAlign64(align),
				true, WD_SIZE_AUTO|WD_SIZE_F_SMALL_VAL );
	}
	else
	    *buf = 0;

	if (!is_reg)
	    printf("%4s %6s %6u %s %-4s %s\n",
		"-", "-", fm3.used, buf,
		oft_info[sf->iod.oft].name, sf->f.fname );
	else if (have_system_map)
	    printf("%4u %6u %6u %s %-4s %s\n",
		fm1.used, fm2.used, fm3.used, buf,
		oft_info[sf->iod.oft].name, sf->f.fname );
	else
	    printf("%4u %6s %6s %s %-4s %s\n",
		fm1.used, "-", "-", buf,
		oft_info[sf->iod.oft].name, sf->f.fname );
    }
    else
    {
	if (have_system_map)
	{
	    if ( verbose > 0 && is_reg )
	    {
		print_fragments(sf,&fm1,"Image fragments","virtual","image");
		print_fragments(sf,&fm2,"Filesystem fragments","image","filesystem");
	    }
	    print_fragments(sf,&fm3,"Fragments","virtual","filesystem");
	    if ( verbose > 0 )
		putchar('\n');
	}
	else
	    print_fragments(sf,&fm1,"Image fragments [NO FS-MAP]","virtual","image");
	putchar('\n');
    }

    ResetFileMap(&fm1);
    ResetFileMap(&fm2);
    ResetFileMap(&fm3);
    return 0;
}

//-----------------------------------------------------------------------------

static enumError cmd_fragments()
{
    if (!HaveFileSystemMapSupport())
	ERROR0(ERR_WARNING,
	    "This version of %s can determine the file system mapping"
	    " only for WBFS partitions.",
	    progname);

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    encoding |= ENCODE_F_FAST; // hint: no encryption needed

    Iterator_t it;
    InitializeIterator(&it);
    it.func		= exec_fragments;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= allow_fst ? ACT_EXPAND : ACT_IGNORE;
    it.long_count	= long_count;
    it.progress_enabled	= 0;

    enumError err = SourceIterator(&it,0,true,false);
    if ( it.done_count++ && brief_count && !OptionUsed[OPT_NO_HEADER] )
	putchar('\n');
    ResetIterator(&it);

    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command LIST			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_list ( int long_level )
{
    if ( long_level > 0 )
    {
	RegisterOptionByIndex(&InfoUI,OPT_LONG,long_level,false);
	long_count += long_level;
    }

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    encoding |= ENCODE_F_FAST; // hint: no encryption needed

    WDiscList_t wlist;
    InitializeWDiscList(&wlist);

    if ( long_count > 3 )
	OptionUsed[OPT_REALPATH] = 1;

    Iterator_t it;
    InitializeIterator(&it);
    it.func		= exec_collect;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= allow_fst ? ACT_EXPAND : ACT_IGNORE;
    it.long_count	= long_count;
    it.real_filename	= print_sections > 0;
    it.wlist		= &wlist;
    it.progress_t_file	= "disc";
    it.progress_t_files	= "discs";
    if (print_sections)
	it.scan_progress = false;

    it.scan_progress	= false;

    enumError err = SourceIterator(&it,1,true,false);
    ResetIterator(&it);
    if ( err > ERR_WARNING )
	return err;

    SortWDiscList(&wlist,sort_mode,SORT_TITLE, OptionUsed[OPT_UNIQUE] ? 1 : 0 );

    //------------------------------

    if ( print_sections )
    {
	printf(	"\n[summary]\n"
		"total-discs=%u\n"
		"total-size=%llu\n",
		wlist.used, wlist.total_size_mib * (u64)MiB );

	char buf[10];
	snprintf(buf,sizeof(buf),"%u",wlist.used-1);
	const int fw = strlen(buf);

	int i;
	WDiscListItem_t *witem;
	for ( i = 0, witem = wlist.first_disc; i < wlist.used; i++, witem++ )
	{
	    printf("\n[disc-%0*u]\n",fw,i);
	    PrintSectWDiscListItem(stdout,witem,0);
	}
	return ERR_OK;
    }

    //------------------------------

    char footer[200];
    int footer_len = 0;

    WDiscListItem_t *witem, *wend = wlist.first_disc + wlist.used;

    const bool print_header = !OptionUsed[OPT_NO_HEADER];
    const bool line2 = long_count > 2;

    PrintTime_t pt;
    int opt_time = opt_print_time;
    if ( long_count > 1 )
	opt_time = EnablePrintTime(opt_time);
    SetupPrintTime(&pt,opt_time);

    if ( ( opt_unit & WD_SIZE_M_MODE ) == WD_SIZE_DEFAULT )
	opt_unit = opt_unit & ~WD_SIZE_M_MODE | WD_SIZE_M;
    const wd_size_mode_t column_unit = opt_unit | WD_SIZE_F_AUTO_UNIT;
    const wd_size_mode_t total_unit = opt_unit & WD_SIZE_M_BASE | WD_SIZE_AUTO;
    const int size_fw = wd_get_size_fw(column_unit,4);
    const u64 total_size = wlist.total_size_mib * (u64)MiB;

    int max_name_wd = 0;
    if (print_header)
    {
	for ( witem = wlist.first_disc; witem < wend; witem++ )
	{
	    const int plen = strlen( witem->title
					? witem->title : witem->name64 );
	    if ( max_name_wd < plen )
		max_name_wd = plen;

	    if ( line2 && witem->fname )
	    {
		const int flen = strlen(witem->fname);
		if ( max_name_wd < flen )
		    max_name_wd = flen;
	    }
	}

	ccp size1 = wd_print_size(0,0,total_size,false,opt_unit);
	ccp size2 = wd_print_size(0,0,total_size,false,total_unit);
	footer_len = strcmp(size1,size2)
			? snprintf(footer,sizeof(footer),
				"Total: %u disc%s, %s ~ %s used.",
				wlist.used, wlist.used == 1 ? "" : "s",
				size1, size2 )
			: snprintf(footer,sizeof(footer),
				"Total: %u disc%s, %s used.",
				wlist.used, wlist.used == 1 ? "" : "s",
				size1 );
    }

    if (long_count)
    {
	int part_info_fw = pt.wd + size_fw - 3;
	if ( part_info_fw < 6 )
	    part_info_fw = 6;

	if (print_header)
	{
	    int n1, n2;
	    putchar('\n');
	    printf("ID6   %s %*s Reg.  %n%d discs (%s)%n\n",
		    pt.head, size_fw, wd_get_size_unit(column_unit,"?"),
		    &n1, wlist.used,
		    wd_print_size(0,0,total_size,false,total_unit), &n2 );
	    max_name_wd += n1;
	    if ( max_name_wd < n2 )
		max_name_wd = n2;

	    if (line2)
		printf("  n(p) %-*s type     %ssource\n",
			part_info_fw, "p-info",
			OptionUsed[OPT_REALPATH] ? "real " : "" );

	    if ( max_name_wd < footer_len )
		max_name_wd = footer_len;
	    printf("%.*s\n", max_name_wd, wd_sep_200);
	}

	for ( witem = wlist.first_disc; witem < wend; witem++ )
	{
	    const u64 size = witem->fatt.size
				? (u64)witem->fatt.size : witem->size_mib*(u64)MiB;
	    printf("%s%s %*s %s  %s\n",
		    witem->id6, PrintTime(&pt,&witem->fatt),
		    size_fw,
		    wd_print_size(0,0,size,false,column_unit),
		    GetRegionInfo(witem->id6[3])->name4,
		    witem->title ? witem->title : witem->name64 );
	    if (line2)
		printf("%6d %-*s %-8s %s\n",
		    witem->n_part, part_info_fw, witem->part_info,
		    GetNameFT(witem->ftype,0),
		    witem->fname ? witem->fname : "" );
	}
    }
    else
    {
	if (print_header)
	{
	    int n1, n2;
	    putchar('\n');
	    printf("ID6    %s %n%d disc%s (%s)%n\n",
		    pt.head, &n1,
		    wlist.used, wlist.used == 1 ? "" : "s",
		    wd_print_size(0,0,total_size,false,total_unit), &n2 );
	    max_name_wd += n1;
	    if ( max_name_wd < n2 )
		max_name_wd = n2;
	    if ( max_name_wd < footer_len )
		max_name_wd = footer_len;
	    printf("%.*s\n", max_name_wd, wd_sep_200 );
	}

	for ( witem = wlist.first_disc; witem < wend; witem++ )
	    printf("%s%s  %s\n", witem->id6, PrintTime(&pt,&witem->fatt),
			witem->title ? witem->title : witem->name64 );
    }

    if (print_header)
	printf("%.*s\n%s\n\n", max_name_wd, wd_sep_200, footer );

    ResetWDiscList(&wlist);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command FILES			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_files ( SuperFile_t * fi, Iterator_t * it )
{
    ASSERT(fi);
    if ( fi->f.ftype & FT_ID_FST_BIN )
	return Dump_FST_BIN(stdout,0,fi,it->real_path,it->show_mode&~SHOW_INTRO);

// [[2do]] [[ft-id]]
    if ( fi->f.ftype & FT__SPC_MASK || !fi->f.id6_dest[0] )
	return OptionUsed[OPT_IGNORE]
		? ERR_OK
		: PrintErrorFT(&fi->f,FT_A_WII_ISO);

    wd_disc_t * disc = OpenDiscSF(fi,true,true);
    if (!disc)
	return ERR_WDISC_NOT_FOUND;
    wd_select(disc,&part_selector);

    char prefix[PATH_MAX];
    if ( it->show_mode & SHOW_PATH )
	PathCatPP(prefix,sizeof(prefix),fi->f.fname,"/");
    else
	*prefix = 0;

    WiiFst_t fst;
    InitializeFST(&fst);
    CollectFST(&fst,disc,GetDefaultFilePattern(),false,0,prefix_mode,true);

    sort_mode = SortFST(&fst,sort_mode,SORT_NAME);
    if ( (sort_mode&SORT__MODE_MASK) != SORT_OFFSET )
	it->show_mode &= ~SHOW_UNUSED;

    DumpFilesFST(stdout,0,&fst,ConvertShow2PFST(it->show_mode,0),prefix);
    ResetFST(&fst);

    return disc->invalid_part ? ERR_WPART_INVALID : ERR_OK;
}

//-----------------------------------------------------------------------------

static enumError cmd_files ( int long_level )
{
    if ( long_level > 0 )
    {
	RegisterOptionByIndex(&InfoUI,OPT_LONG,long_level,false);
	long_count += long_level;
    }

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    encoding |= ENCODE_F_FAST; // hint: no encryption needed

    Iterator_t it;
    InitializeIterator(&it);
    //it.act_non_iso	= OptionUsed[OPT_IGNORE] ? ACT_IGNORE : ACT_WARN;
    it.act_non_iso	= ACT_ALLOW;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= allow_fst ? ACT_EXPAND : ACT_IGNORE;
    it.long_count	= long_count;

    if ( it.show_mode & SHOW__DEFAULT )
    {
	switch (long_count)
	{
	    case 0:  it.show_mode = SHOW_F_HEAD; break;
	    case 1:  it.show_mode = SHOW_F_HEAD | SHOW_SIZE | SHOW_F_DEC; break;
	    case 2:  it.show_mode = SHOW_F_HEAD | SHOW__ALL & ~SHOW_PATH; break;
	    default: it.show_mode = SHOW_F_HEAD | SHOW__ALL; break;
	}
    }
    if ( OptionUsed[OPT_NO_HEADER] )
	it.show_mode &= ~SHOW_F_HEAD;
    if ( it.show_mode & (SHOW_F_DEC|SHOW_F_HEX) )
	it.show_mode |= SHOW_SIZE;

    enumError err = SourceIterator(&it,0,false,true);
    if ( err <= ERR_WARNING )
    {
	it.func = exec_files;
	err = SourceIteratorCollected(&it,0,1,false);
    }
    ResetIterator(&it);

    if ( it.show_mode & SHOW_F_HEAD )
	putchar('\n');
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command DIFF			///////////////
///////////////////////////////////////////////////////////////////////////////

static Diff_t diff = {0};

static enumError cmd_diff ( bool file_level )
{
    if ( verbose > 0 )
	print_title(stdout);

    if (file_level)
    {
	FilePattern_t * pat = file_pattern + PAT_DEFAULT;
	pat->rules.used = 0;
	AddFilePattern("+",PAT_DEFAULT);
	PRINT("FIRST FILE LEVEL\n");
	RegisterOptionByIndex(&InfoUI,OPT_FILES,1,false);
    }
    else if (OptionUsed[OPT_FILES])
    {
	FilePattern_t * pat = GetDefaultFilePattern();
	SetupFilePattern(pat);
    }

    if (!opt_dest)
    {
	if (!first_param)
	    return ERROR0(ERR_MISSING_PARAM, "Missing destination parameter\n" );

	ParamList_t * param;
	for ( param = first_param; param->next; param = param->next )
	    ;
	ASSERT(param);
	ASSERT(!param->next);
	SetDest(param->arg,false);
	param->arg = 0;
    }

    SetupDiff(&diff,long_count);

    if ( output_file_type == OFT_UNKNOWN && allow_fst && IsFST(opt_dest,0) )
	output_file_type = OFT_FST;
    if ( output_file_type == OFT_FST && !allow_fst )
	output_file_type = OFT_UNKNOWN;

    if ( prefix_mode <= WD_IPM_AUTO )
	prefix_mode = WD_IPM_PART_NAME;

    FilePattern_t * pat = GetDefaultFilePattern();
    if (SetupFilePattern(pat))
	encoding |= ENCODE_F_FAST; // hint: no encryption needed

    int done_count = 0;
    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	if (param->arg)
	{
	    done_count++;
	    AppendStringField(&source_list,param->arg,true);
	}
    if (!done_count)
	SYNTAX_ERROR; // no return

    Iterator_t it;
    InitializeIterator(&it);
    it.act_non_iso	= OptionUsed[OPT_IGNORE] ? ACT_IGNORE : ACT_WARN;
    it.act_non_exist	= it.act_non_iso;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= allow_fst ? ACT_EXPAND : ACT_IGNORE;
    it.long_count	= long_count;

    if ( testmode > 1 )
    {
	it.func = exec_filetype;
	enumError err = SourceIterator(&it,1,false,false);
	CloseDiff(&diff);
	ResetIterator(&it);
	printf("DESTINATION: %s\n",opt_dest);
	return err;
    }

    enumError err = SourceIterator(&it,0,false,true);
    if ( err <= ERR_WARNING )
    {
	enumError exec_copy ( SuperFile_t * fi, Iterator_t * it );
	it.func = exec_copy;
	it.diff_it = true;
	err = SourceIteratorCollected(&it,0,2,false);
	if ( err == ERR_OK && it.exists_count )
	    err = ERR_ALREADY_EXISTS;
    }

    if ( !err && it.diff_count )
	err = ERR_DIFFER;
    enumError err2 = CloseDiff(&diff);
    ResetIterator(&it);
    return err > err2 ? err : err2;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command EXTRACT			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_extract ( SuperFile_t * fi, Iterator_t * it )
{
    ASSERT(fi);
    PRINT("exec_extract(%s)\n",fi->f.fname);
    if (!fi->f.id6_dest[0])
	return ERR_OK;
    fflush(0);

    wd_disc_t * disc = OpenDiscSF(fi,true,true);
    if (!disc)
	return ERR_WDISC_NOT_FOUND;

    char dest_dir[PATH_MAX];
    SubstFileNameBuf(dest_dir,sizeof(dest_dir)-1,fi,0,opt_dest,OFT_UNKNOWN);
    const enumError err
	= NormalizeExtractPath( dest_dir, sizeof(dest_dir), dest_dir, it->overwrite );
    if (err)
	return err;

    if ( testmode || verbose >= 0 )
    {
	if (print_sections)
	{
	    printf(
		"[EXTRACT]\n"
		"test-mode=%d\n"
		"job-counter=%d\n"
		"job-total=%d\n"
		"source-path=%s\n"
		"source-real-path=%s\n"
		"source-type=%s\n"
		"source-n-split=%d\n"
		"dest-path=%s\n"
		"dest-type=FST\n"
		"\n"
		,testmode>0
		,it->source_index+1
		,it->source_list.used
		,fi->f.fname
		,it->real_path
		,oft_info[fi->iod.oft].name
		,fi->f.split_used
		,dest_dir
		);
	}
	else
	{
	    char split_buf[10];
	    const int count_fw
		= snprintf(split_buf,sizeof(split_buf),"%u",it->source_list.used );
	    if ( fi->f.split_used > 1 )
		snprintf(split_buf,sizeof(split_buf),"*%u",fi->f.split_used);
	    else
		*split_buf = 0;

	    printf( "%s: %sEXTRACT %*u/%u %s%s:%s -> %s\n",
		progname, testmode ? "WOULD " : "",
		count_fw, it->source_index+1, it->source_list.used,
		oft_info[fi->iod.oft].name,
		split_buf, fi->f.fname, dest_dir );
	}
	fflush(0);

	if (testmode)
	    return ERR_OK;
    }

    return ExtractImage( fi, dest_dir, it->overwrite, OptionUsed[OPT_PRESERVE] != 0 );
}

//-----------------------------------------------------------------------------

static enumError cmd_extract()
{
    if ( verbose >= 0 )
	print_title(stdout);

    if (!opt_dest)
    {
	if (!first_param)
	    return ERROR0(ERR_MISSING_PARAM, "Missing destination parameter\n" );

	ParamList_t * param;
	for ( param = first_param; param->next; param = param->next )
	    ;
	ASSERT(param);
	ASSERT(!param->next);
	SetDest(param->arg,false);
	param->arg = 0;
    }

    int done_count = 0;
    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	if (param->arg)
	{
	    done_count++;
	    AppendStringField(&source_list,param->arg,true);
	}
    if (!done_count)
	SYNTAX_ERROR; // no return

    encoding |= ENCODE_F_FAST; // hint: no encryption needed

    Iterator_t it;
    InitializeIterator(&it);
    it.act_non_iso	= OptionUsed[OPT_IGNORE] ? ACT_IGNORE : ACT_WARN;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= allow_fst ? ACT_EXPAND : ACT_IGNORE;
    it.overwrite	= OptionUsed[OPT_OVERWRITE] ? 1 : 0;

    enumError err = SourceIterator(&it,0,false,true);
    if ( err <= ERR_WARNING )
    {
	it.func = exec_extract;
	err = SourceIteratorCollected(&it,0,2,false);
	if ( err == ERR_OK && it.exists_count )
	    err = ERR_ALREADY_EXISTS;
    }
    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command COPY			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_copy ( SuperFile_t * fi, Iterator_t * it )
{
    ASSERT(fi);
    PRINT("exec_copy(%s)\n",fi->f.fname);
    if (!fi->f.id6_dest[0])
	return ERR_OK;
    fflush(0);

    const bool convert_it
		=  it->convert_it
		&& !it->diff_it
		&& ( output_file_type == OFT_UNKNOWN
			|| output_file_type == fi->iod.oft );

    ccp fname = fi->f.path ? fi->f.path : fi->f.fname;
    const enumOFT oft = convert_it
			? fi->iod.oft
			: CalcOFT(output_file_type,opt_dest,fname,fi->iod.oft);

    SuperFile_t fo;
    InitializeSF(&fo);
    SetupIOD(&fo,oft,oft);
    fo.src = fi;

    if (convert_it)
	fo.f.fname = STRDUP(fname);
    else
    {
	TRACE("COPY, mkdir=%d\n",opt_mkdir);
	fo.f.create_directory = opt_mkdir;
	ccp oname = fi->f.outname ? fi->f.outname : fname;
 #if 0 // [[obsolete]] 2011-11
	if ( oft == OFT_WBFS && fi->f.id6_dest[0] )
	{
	    // use ID6 as default filename
	    ccp pathend = strrchr(oname,'/');
	    if (pathend)
	    {
		const int len = pathend - oname + 1;
		memcpy(iobuf,oname,len);
		strcpy(iobuf+len,fi->f.id6_dest);
		oname = iobuf;
	    }
	    else
		oname = fi->f.id6_dest;
	}
 #endif
	GenImageFileName(&fo.f,opt_dest,oname,oft);
	SubstFileNameSF(&fo,fi,0);

	if ( it->update && !StatFile(&fo.f.st,fo.f.fname,-1) )
	    return ERR_OK;
    }

    const bool raw_mode = part_selector.whole_disc || !fi->f.id6_dest[0];
    fo.raw_mode = raw_mode;

    ccp job_mode = raw_mode
			? "RAW"
			: it->diff_it && OptionUsed[OPT_FILES]
				? "FILES"
				: "SCRUB";

    if ( testmode || verbose >= 0 )
    {
	ccp job = it->diff_it ? "DIFF" : convert_it ? "CONVERT" : "COPY";

	if (print_sections)
	{
	    printf(
		"[%s]\n"
		"test-mode=%d\n"
		"data-mode=%s\n"
		"job-counter=%d\n"
		"job-total=%d\n"
		"source-path=%s\n"
		"source-real-path=%s\n"
		"source-type=%s\n"
		"source-n-split=%d\n"
		"dest-path=%s\n"
		"dest-type=%s\n"
		"\n"
		,job
		,testmode>0
		,job_mode
		,it->source_index+1
		,it->source_list.used
		,fi->f.fname
		,it->real_path
		,oft_info[fi->iod.oft].name
		,fi->f.split_used
		,fo.f.fname
		,oft_info[oft].name
		);
	}
	else
	{
	    char split_buf[10];
	    const int count_fw
		= snprintf(split_buf,sizeof(split_buf),"%u",it->source_list.used );
	    if ( fi->f.split_used > 1 )
		snprintf(split_buf,sizeof(split_buf),"*%u",fi->f.split_used);
	    else
		*split_buf = 0;

	    printf( "* %s%s/%s %*u/%u %s%s:%s %s %s:%s\n",
		testmode ? "WOULD " : "", job, job_mode,
		count_fw, it->source_index+1, it->source_list.used,
		oft_info[fi->iod.oft].name, split_buf, fi->f.fname,
		it->diff_it ? ":" : "->",
		oft_info[oft].name, fo.f.fname );
	}
	fflush(0);

	if (testmode)
	{
	    ResetSF(&fo,0);
	    return ERR_OK;
	}
    }


    //----- diff image

    if (it->diff_it)
    {
	enumError err = OpenSF(&fo,0,it->act_non_iso||it->act_wbfs>=ACT_ALLOW,0);
	if (err)
	{
	    it->diff_count++;
	    return ERR_OK;
	}

      if ( newmode >= 0 ) // [[2do]] remove old mode 2011-02-23
      {
	if (!OpenDiffSource(&diff,fi,&fo,true))
	    return ERR_DIFFER;

	err = opt_patch_file && *opt_patch_file
		? DiffPatchSF( &diff, GetDefaultFilePattern(), prefix_mode )
		: !raw_mode && OptionUsed[OPT_FILES]
		    ? DiffFilesSF( &diff, GetDefaultFilePattern(), prefix_mode )
		    : DiffSF( &diff, raw_mode );

	if ( err == ERR_OK && !CloseDiffSource(&diff,print_sections>0) )
	    err = ERR_DIFFER;

	if ( err == ERR_DIFFER )
	{
	    it->diff_count++;
	    err = ERR_OK;
	    if ( verbose >= 0 && print_sections )
		printf(	"[differ]\n"
			"data-mode=%s\n"
			"job-counter=%d\n"
			"job-total=%d\n"
			"source-path=%s\n"
			"source-type=%s\n"
			"dest-path=%s\n"
			"dest-type=%s\n"
			"\n"
			,job_mode
			,it->source_index+1
			,it->source_list.used
			,fi->f.fname
			,oft_info[fi->iod.oft].name
			,fo.f.fname
			,oft_info[oft].name
			);
	}
	it->done_count++;
	ResetSF(&fo,0);
	return err;
      }
      else
      {
	err = !raw_mode && OptionUsed[OPT_FILES]
		    ? oldDiffFilesSF( fi, &fo, it->long_count,
				    GetDefaultFilePattern(), prefix_mode )
		    : oldDiffSF( fi, &fo, it->long_count, raw_mode );

	if ( err == ERR_DIFFER )
	{
	    it->diff_count++;
	    err = ERR_OK;
	    if ( verbose >= 0 )
	    {
		if (print_sections)
		printf(
		    "[differ]\n"
		    "data-mode=%s\n"
		    "job-counter=%d\n"
		    "job-total=%d\n"
		    "source-path=%s\n"
		    "source-type=%s\n"
		    "dest-path=%s\n"
		    "dest-type=%s\n"
		    "\n"
		    ,job_mode
		    ,it->source_index+1
		    ,it->source_list.used
		    ,fi->f.fname
		    ,oft_info[fi->iod.oft].name
		    ,fo.f.fname
		    ,oft_info[oft].name
		    );
		else
		    printf( "! ISOs differ: %s:%s : %s:%s\n",
			    oft_info[fi->iod.oft].name, fi->f.fname,
			    oft_info[fo.iod.oft].name, fo.f.fname );
	    }
	}
	it->done_count++;
	ResetSF(&fo,0);
	return err;
      }
    }


    //----- copy image

    enumError err = CopyImage(  fi, &fo, oft,
				it->overwrite,
				OptionUsed[OPT_PRESERVE] != 0,
				it->remove_source );
    if ( err == ERR_ALREADY_EXISTS )
    {
	it->exists_count++;
	return ERR_OK;
    }

    if ( !err && OptionUsed[OPT_DIFF] )
    {
	it->diff_it = true;
	err = exec_copy(fi,it);
	it->diff_it = false;
    }

    return err;
}

//-----------------------------------------------------------------------------

static enumError cmd_copy()
{
    if ( OptionUsed[OPT_FST] )
	return cmd_extract();

    if ( verbose >= 0 )
	print_title(stdout);

    if (!opt_dest)
    {
	if (!first_param)
	    return ERROR0(ERR_MISSING_PARAM, "Missing destination parameter!\n" );

	ParamList_t * param;
	for ( param = first_param; param->next; param = param->next )
	    ;
	ASSERT(param);
	ASSERT(!param->next);
	SetDest(param->arg,false);
	param->arg = 0;
    }

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	if (param->arg)
	    AppendStringField(&source_list,param->arg,true);
    if ( !source_list.used && !recurse_list.used )
	SYNTAX_ERROR; // no return

    Iterator_t it;
    InitializeIterator(&it);
    it.act_non_iso	= OptionUsed[OPT_IGNORE] ? ACT_IGNORE : ACT_WARN;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= allow_fst ? ACT_EXPAND : ACT_IGNORE;
    it.overwrite	= OptionUsed[OPT_OVERWRITE] ? 1 : 0;
    it.update		= OptionUsed[OPT_UPDATE]    ? 1 : 0;
    it.remove_source	= OptionUsed[OPT_REMOVE]    ? 1 : 0;

    if ( testmode == 2 )
    {
	it.func = exec_filetype;
	enumError err = SourceIterator(&it,1,false,false);
	ResetIterator(&it);
	printf("DESTINATION: %s\n",opt_dest);
	return err;
    }

    enumError err = SourceIterator(&it,0,false,true);
    if ( err <= ERR_WARNING )
    {
	if ( testmode > 1 )
	    DumpIdField(stdout,0,&it.source_list);
	else
	{
	    it.func = exec_copy;
	    err = SourceIteratorCollected(&it,0,2,false);
	    if ( err == ERR_OK && it.exists_count )
		err = ERR_ALREADY_EXISTS;
	}
    }
    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command CONVERT			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_convert()
{
    if ( verbose >= 0 )
	print_title(stdout);

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    Iterator_t it;
    InitializeIterator(&it);
    it.act_non_iso	= OptionUsed[OPT_IGNORE] ? ACT_IGNORE : ACT_WARN;
    it.act_wbfs		= it.act_non_iso;
    it.act_gc		= ACT_ALLOW;
    it.convert_it	= true;
    it.overwrite	= true;
    it.remove_source	= true;

    if ( testmode > 1 )
    {
	it.func = exec_filetype;
	enumError err = SourceIterator(&it,1,false,false);
	ResetIterator(&it);
	printf("DESTINATION: %s\n",opt_dest);
	return err;
    }

    enumError err = SourceIterator(&it,0,false,true);
    if ( err <= ERR_WARNING )
    {
	it.func = exec_copy;
	err = SourceIteratorCollected(&it,0,2,false);
	if ( err == ERR_OK && it.exists_count )
	    err = ERR_ALREADY_EXISTS;
    }
    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command EDIT			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_edit ( SuperFile_t * fi, Iterator_t * it )
{
    if (!fi->f.id6_dest[0])
	return ERR_OK;
    fflush(0);

    const OFT_info_t * oinfo = oft_info + fi->iod.oft;
    if ( !(oinfo->attrib & OFT_A_MODIFY) )
	return ERROR0(ERR_WARNING,"Can't modify file type '%s': %s\n",
		    oinfo->name, fi->f.fname );

#ifdef TEST
    if ( testmode || verbose >= 0 )
    {
	if (print_sections)
	{
	    printf(
		"[EDIT]\n"
		"test-mode=%d\n"
		"job-counter=%d\n"
		"job-total=%d\n"
		"source-path=%s\n"
		"source-real-path=%s\n"
		"source-type=%s\n"
		"source-n-split=%d\n"
		"\n"
		,testmode>0
		,it->source_index+1
		,it->source_list.used
		,fi->f.fname
		,it->real_path
		,oft_info[fi->iod.oft].name
		,fi->f.split_used
		);
	}
	else
	{
	    char split_buf[10];
	    const int count_fw
		= snprintf(split_buf,sizeof(split_buf),"%u",it->source_list.used );
	    if ( fi->f.split_used > 1 )
		snprintf(split_buf,sizeof(split_buf),"*%u",fi->f.split_used);
	    else
		*split_buf = 0;

	    printf(" - %sEDIT %*u/%u %s%s:%s\n",
		testmode ? "WOULD " : "",
		count_fw, it->source_index+1, it->source_list.used,
		oft_info[fi->iod.oft].name, split_buf, fi->f.fname );
	}
	fflush(0);

	if (testmode)
	    return ERR_OK;
    }
#else
    if (testmode)
    {
	printf( "%s: WOULD EDIT %s:%s\n", progname, oinfo->name, fi->f.fname );
	return ERR_OK;
    }

    if ( verbose >= 0 )
	printf( "%s: EDIT %s:%s\n", progname, oinfo->name, fi->f.fname );
#endif

    OpenDiscSF(fi,true,true);
    wd_disc_t * disc = fi->disc1;

    enumError err = ERR_OK;
    if ( disc && disc->reloc )
    {
	PRINT("EDIT PHASE I\n");
	const wd_reloc_t * reloc = disc->reloc;
	u32 idx;
	for ( idx = 0; idx < WII_MAX_SECTORS && !err; idx++, reloc++ )
	    if ( *reloc & (WD_RELOC_F_PATCH|WD_RELOC_F_HASH)
		&& !( *reloc & WD_RELOC_F_LAST ) )
	    {
		TRACE(" - WRITE SECTOR %x, off %llx\n",idx,idx*(u64)WII_SECTOR_SIZE);
		err = CopyRawData(fi,fi,idx*(u64)WII_SECTOR_SIZE,WII_SECTOR_SIZE);
	    }

	PRINT("EDIT PHASE II\n");
	reloc = disc->reloc;
	for ( idx = 0; idx < WII_MAX_SECTORS && !err; idx++, reloc++ )
	    if ( *reloc & WD_RELOC_F_LAST )
	    {
		TRACE(" - WRITE SECTOR %x, off %llx\n",idx,idx*(u64)WII_SECTOR_SIZE);
		err = CopyRawData(fi,fi,idx*(u64)WII_SECTOR_SIZE,WII_SECTOR_SIZE);
	    }
    }

    ResetSF( fi, !err && OptionUsed[OPT_PRESERVE] ? &fi->f.fatt : 0 );
    return err;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_edit()
{
    if ( verbose >= 0 )
	print_title(stdout);

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    Iterator_t it;
    InitializeIterator(&it);
    it.act_non_iso	= OptionUsed[OPT_IGNORE] ? ACT_IGNORE : ACT_WARN;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_ALLOW;

    if ( testmode > 1 )
    {
	it.func = exec_filetype;
	enumError err = SourceIterator(&it,1,false,false);
	ResetIterator(&it);
	return err;
    }

    enumError err = SourceIterator(&it,0,false,true);
    if ( err <= ERR_WARNING )
    {
	it.func = exec_edit;
	it.open_modify = !testmode;
	err = SourceIteratorCollected(&it,0,2,false);
	if ( err == ERR_OK && it.exists_count )
	    err = ERR_ALREADY_EXISTS;
    }
    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command IMGFILES		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_imgfiles ( SuperFile_t * fi, Iterator_t * it )
{
    DASSERT(fi);
    DASSERT(it);
    PRINT("IMGFILES [%u,%x] %s\n",fi->f.split_used,fi->f.ftype&FT_A_WDISC,fi->f.fname);
    if ( fi->f.ftype & FT_A_WDISC )
	return 0;

    char eol = OptionUsed[OPT_NULL] ? 0 : '\n';
    int n = fi->f.split_used;
    if (n)
    {
	File_t **f = fi->f.split_f;
	while ( n-- > 0 )
	{
	    printf("%s%c",(*f)->fname,eol);
	    f++;
	}
    }
    else
	printf("%s%c",fi->f.fname,eol);

    return ERR_OK;
}

//-----------------------------------------------------------------------------

static enumError cmd_imgfiles()
{
    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    encoding |= ENCODE_F_FAST; // hint: no encryption needed

    Iterator_t it;
    InitializeIterator(&it);
    it.act_non_iso	= OptionUsed[OPT_IGNORE] ? ACT_IGNORE : ACT_WARN;
    it.act_wbfs_disc	= it.act_non_iso;
    it.act_wbfs		= ACT_ALLOW;
    it.act_gc		= ACT_ALLOW;

    enumError err = SourceIterator(&it,0,false,true);
    if ( err <= ERR_WARNING )
    {
	it.func = exec_imgfiles;
	err = SourceIteratorCollected(&it,0,2,false);
    }
    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command REMOVE			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_remove ( SuperFile_t * fi, Iterator_t * it )
{
    DASSERT(fi);
    DASSERT(it);
    PRINT("REMOVE [%u] %s\n",fi->f.split_used,fi->f.fname);
    if ( fi->f.ftype & FT_A_WDISC )
	return 0;

    int n = fi->f.split_used;

    char split_buf[10];
    const int count_fw
	= snprintf(split_buf,sizeof(split_buf),"%u",it->source_list.used );
    if ( fi->f.split_used > 1 )
	snprintf(split_buf,sizeof(split_buf),"*%u",n);
    else
	*split_buf = 0;

    printf(" - %sREMOVE %*u/%u %s%s:%s\n",
	testmode ? "WOULD " : "",
	count_fw, it->source_index+1, it->source_list.used,
	oft_info[fi->iod.oft].name, split_buf, fi->f.fname );

    if (!testmode)
	RemoveSF(fi);

    return ERR_OK;
}

//-----------------------------------------------------------------------------

static enumError cmd_remove()
{
    if ( verbose >= 0 )
	print_title(stdout);

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    encoding |= ENCODE_F_FAST; // hint: no encryption needed

    Iterator_t it;
    InitializeIterator(&it);
    it.act_non_iso	= OptionUsed[OPT_IGNORE] ? ACT_IGNORE : ACT_WARN;
    it.act_wbfs_disc	= it.act_non_iso;
    it.act_wbfs		= ACT_ALLOW;
    it.act_gc		= ACT_ALLOW;

    if ( testmode > 1 )
    {
	it.func = exec_filetype;
	enumError err = SourceIterator(&it,1,false,false);
	ResetIterator(&it);
	return err;
    }

    enumError err = SourceIterator(&it,0,false,true);
    if ( err <= ERR_WARNING )
    {
	it.func = exec_remove;
	err = SourceIteratorCollected(&it,0,2,false);
    }
    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command MOVE			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_move ( SuperFile_t * fi, Iterator_t * it )
{
    DASSERT(fi);
    DASSERT(it);
    if ( fi->f.ftype & FT_A_WDISC )
	return 0;

    SuperFile_t fo;
    InitializeSF(&fo);
    SetupIOD(&fo,fi->iod.oft,fi->iod.oft);
    fo.f.create_directory = opt_mkdir;

    ccp oname = fi->iod.oft == OFT_WBFS && fi->f.id6_dest[0]
			? fi->f.id6_dest
			: fi->f.outname
				? fi->f.outname
				: fi->f.fname;
    GenImageFileName(&fo.f,opt_dest,oname,fi->iod.oft);
    SubstFileNameSF(&fo,fi,0);

    if ( strcmp(fi->f.fname,fo.f.fname) )
    {
	const int stat_status = stat(fo.f.fname,&fo.f.st);
	if ( !stat_status
		&& fi->f.st.st_dev == fo.f.st.st_dev
		&& fi->f.st.st_ino == fo.f.st.st_ino )
	{
	    // src==dest => nothing to do!
	}
	else if ( !it->overwrite && !stat_status )
	{
	    ERROR0(ERR_CANT_CREATE,"File already exists: %s\n",fo.f.fname);
	}
	else if (!CheckCreated(fo.f.fname,false,ERR_CANT_CREATE))
	{
	    if ( testmode || verbose >= 0 )
	    {
	      if (print_sections)
	      {
		printf(
		    "[MOVE]\n"
		    "test-mode=%d\n"
		    "job-counter=%d\n"
		    "job-total=%d\n"
		    "source-path=%s\n"
		    "source-real-path=%s\n"
		    "source-type=%s\n"
		    "source-n-split=%d\n"
		    "dest-path=%s\n"
		    "\n"
		    ,testmode>0
		    ,it->source_index+1
		    ,it->source_list.used
		    ,fi->f.fname
		    ,it->real_path
		    ,oft_info[fi->iod.oft].name
		    ,fi->f.split_used
		    ,fo.f.fname
		    );
	      }
	      else
	      {
		char split_buf[10];
		const int count_fw
		    = snprintf(split_buf,sizeof(split_buf),"%u",it->source_list.used );
		if ( fi->f.split_used > 1 )
		    snprintf(split_buf,sizeof(split_buf),"*%u",fi->f.split_used);
		else
		    *split_buf = 0;

		printf(" - %sMOVE %*u/%u %s%s:%s -> %s\n",
		    testmode ? "WOULD " : "",
		    count_fw, it->source_index+1, it->source_list.used,
		    oft_info[fi->iod.oft].name, split_buf, fi->f.fname, fo.f.fname );
	      }
	      fflush(0);
	    }

	    CloseSF(fi,0);
	    if (!testmode)
	    {
		int stat = 0;
		if (fi->f.split_used)
		{
		    char * format = iobuf + sizeof(iobuf);
		    CalcSplitFilename(format,sizeof(iobuf)/2,fo.f.fname,fo.iod.oft);
		    noPRINT(">> |%s|\n",format);
		    int i;
		    for ( i = 0; !stat && i < fi->f.split_used; i++ )
		    {
			ccp dest;
			if (i)
			{
			    dest = iobuf;
			    snprintf(iobuf,sizeof(iobuf)/2,format,i);
			}
			else
			    dest = fo.f.fname;
			File_t * f = fi->f.split_f[i];
			DASSERT(f);
			PRINT("rename %s -> %s\n",f->fname,dest);
			stat = rename(f->fname,dest);
			if ( stat && opt_mkdir )
			{
			    CreatePath(dest);
			    stat = rename(f->fname,dest);
			}
		    }
		}
		else
		{
		    stat = rename(fi->f.fname,fo.f.fname);
		    if ( stat && opt_mkdir )
		    {
			CreatePath(fo.f.fname);
			stat = rename(fi->f.fname,fo.f.fname);
		    }
		}
		if (stat)
		    return ERROR1(ERR_CANT_CREATE,
					"Can't create file: %s\n", fo.f.fname );
	    }
	}
    }

    ResetSF(&fo,0);
    return ERR_OK;
}

//-----------------------------------------------------------------------------

static enumError cmd_move()
{
    if ( verbose >= 0 )
	print_title(stdout);

    if (!opt_dest)
    {
	if (!first_param)
	    return ERROR0(ERR_MISSING_PARAM, "Missing destination parameter\n" );

	ParamList_t * param;
	for ( param = first_param; param->next; param = param->next )
	    ;
	ASSERT(param);
	ASSERT(!param->next);
	SetDest(param->arg,false);
	param->arg = 0;
    }

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    encoding |= ENCODE_F_FAST; // hint: no encryption needed

    Iterator_t it;
    InitializeIterator(&it);
    it.act_non_iso	= OptionUsed[OPT_IGNORE] ? ACT_IGNORE : ACT_WARN;
    it.act_wbfs_disc	= it.act_non_iso;
    it.act_wbfs		= ACT_ALLOW; //HaveEscapeChar(opt_dest) ? it.act_non_iso : ACT_ALLOW;
    it.act_gc		= ACT_ALLOW;
    it.overwrite	= OptionUsed[OPT_OVERWRITE] ? 1 : 0;

    if ( testmode > 1 )
    {
	it.func = exec_filetype;
	enumError err = SourceIterator(&it,1,false,false);
	ResetIterator(&it);
	printf("DESTINATION: %s\n",opt_dest);
	return err;
    }

    enumError err = SourceIterator(&it,0,false,true);
    if ( err <= ERR_WARNING )
    {
	it.func = exec_move;
	err = SourceIteratorCollected(&it,0,2,false);
    }
    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command RENAME			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_rename ( SuperFile_t * fi, Iterator_t * it )
{
    ParamList_t *param, *plus_param = 0;
    for ( param = first_param; param; param = param->next )
    {
	if ( !plus_param && param->selector[0] == '+' )
	    plus_param = param;
	else if ( !strcmp(param->selector,fi->f.id6_src) )
	    break;
    }

    if (!param)
    {
	if (!plus_param)
	    return ERR_OK;
	param = plus_param;
    }
    param->count++;

    bool change_wbfs	= 0 != OptionUsed[OPT_WBFS];
    bool change_iso	= 0 != OptionUsed[OPT_ISO];
    if ( !change_wbfs && !change_iso )
	change_wbfs = change_iso = true;

    if ( fi->iod.oft == OFT_WBFS )
    {
	ASSERT(fi->wbfs);
	return RenameWDisc( fi->wbfs, param->id6, param->arg,
			    change_wbfs, change_iso, verbose, testmode );
    }

    const int bufsize = WII_TITLE_OFF + WII_TITLE_SIZE;
    char buf[bufsize];

    enumError err = ReadSF(fi,0,buf,sizeof(buf));
    if (err)
	return err;

    if (RenameISOHeader( buf, fi->f.fname,
			 param->id6, param->arg, verbose, testmode ))
    {
	err = WriteSF(fi,0,buf,sizeof(buf));
	if (err)
	    return err;
    }
    return err;
}

//-----------------------------------------------------------------------------

static enumError cmd_rename ( bool rename_id )
{
    if ( verbose >= 0 )
	print_title(stdout);

    if ( !source_list.used && !recurse_list.used )
	SYNTAX_ERROR; // no return
	//return ERROR0( ERR_MISSING_PARAM, "Missing source files.\n");

    enumError err = CheckParamRename(rename_id,!rename_id,false);
    if (err)
	return err;
    if ( !n_param )
	return ERROR0(ERR_MISSING_PARAM, "Missing renaming parameters.\n" );

    Iterator_t it;
    InitializeIterator(&it);
    it.open_modify	= !testmode;
    it.act_non_iso	= OptionUsed[OPT_IGNORE] ? ACT_IGNORE : ACT_WARN;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_ALLOW;
    it.long_count	= long_count;

    if ( testmode > 1 )
    {
	it.func = exec_filetype;
	enumError err = SourceIterator(&it,1,false,false);
	ResetIterator(&it);
	printf("DESTINATION: %s\n",opt_dest);
	return err;
    }

    it.func = exec_rename;
    err = SourceIterator(&it,0,false,false);
    ResetIterator(&it);
    return err;

    // [[2do]]: not found
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command VERIFY			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_verify ( SuperFile_t * fi, Iterator_t * it )
{
    ASSERT(fi);
    ASSERT(it);
    if (!fi->f.id6_dest[0])
	return ERR_OK;
    fflush(0);

    it->done_count++;
    if ( testmode || verbose >= 999 )
    {
	printf( "%s: %sVERIFY %s:%s\n",
		progname, testmode ? "WOULD " : "",
		oft_info[fi->iod.oft].name, fi->f.fname );
	if (testmode)
	    return ERR_OK;
    }

    Verify_t ver;
    InitializeVerify(&ver,fi);
    ver.long_count = it->long_count;
    if ( opt_limit >= 0 )
    {
	ver.max_err_msg = opt_limit;
	if (!ver.verbose)
	    ver.verbose = 1;
    }
    if ( it->source_list.used > 1 )
    {
	ver.disc_index = it->source_index+1;
	ver.disc_total = it->source_list.used;
    }

    const enumError err = VerifyDisc(&ver);
    if ( err == ERR_DIFFER )
	it->diff_count++;
    ResetVerify(&ver);
    return err;
}

//-----------------------------------------------------------------------------

static enumError cmd_verify()
{
    if ( verbose >= 0 )
	print_title(stdout);

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    Iterator_t it;
    InitializeIterator(&it);
    it.act_non_iso	= OptionUsed[OPT_IGNORE] ? ACT_IGNORE : ACT_WARN;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_WARN;
    it.act_fst		= ACT_IGNORE;
    it.long_count	= long_count;

    if ( testmode > 1 )
    {
	it.func = exec_filetype;
	enumError err = SourceIterator(&it,1,false,false);
	ResetIterator(&it);
	printf("DESTINATION: %s\n",opt_dest);
	return err;
    }

    enumError err = SourceIterator(&it,0,false,true);
    if ( err <= ERR_WARNING )
    {
	it.func = exec_verify;
	err = SourceIteratorCollected(&it,0,2,true);
	if ( err == ERR_OK && it.diff_count )
	    err = ERR_DIFFER;
    }
    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    command SKELETONIZE			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError exec_skeletonize ( SuperFile_t * fi, Iterator_t * it )
{
    ASSERT(fi);
    ASSERT(it);
    if (!fi->f.id6_dest[0])
	return ERR_OK;
    fflush(0);

    it->done_count++;
    return Skeletonize(fi,0,it->source_index+1,it->source_list.used);
}

//-----------------------------------------------------------------------------

static enumError cmd_skeletonize()
{
    if ( verbose >= 0 )
	print_title(stdout);

    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AppendStringField(&source_list,param->arg,true);

    Iterator_t it;
    InitializeIterator(&it);
    it.act_non_iso	= OptionUsed[OPT_IGNORE] ? ACT_IGNORE : ACT_WARN;
    it.act_wbfs		= ACT_EXPAND;
    it.act_gc		= ACT_ALLOW;
    it.act_fst		= allow_fst ? ACT_EXPAND : ACT_IGNORE;
    it.long_count	= long_count;

    if ( testmode > 1 )
    {
	it.func = exec_filetype;
	enumError err = SourceIterator(&it,1,false,false);
	ResetIterator(&it);
	printf("DESTINATION: %s\n",opt_dest);
	return err;
    }

    enumError err = SourceIterator(&it,0,false,true);
    if ( err <= ERR_WARNING )
    {
	it.func = exec_skeletonize;
	err = SourceIteratorCollected(&it,0,2,true);
    }
    ResetIterator(&it);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                   check options                 ///////////////
///////////////////////////////////////////////////////////////////////////////

enumError CheckOptions ( int argc, char ** argv, bool is_env )
{
    TRACE("CheckOptions(%d,%p,%d) optind=%d\n",argc,argv,is_env,optind);

    optind = 0;
    int err = 0;

    for(;;)
    {
      const int opt_stat = getopt_long(argc,argv,OptionShort,OptionLong,0);
      if ( opt_stat == -1 )
	break;

      RegisterOptionByName(&InfoUI,opt_stat,1,is_env);

      switch ((enumGetOpt)opt_stat)
      {
	case GO__ERR:		err++; break;

	case GO_VERSION:	version_exit();
	case GO_HELP:		help_exit(false);
	case GO_XHELP:		help_exit(true);
	case GO_WIDTH:		err += ScanOptWidth(optarg); break;
	case GO_QUIET:		verbose = verbose > -1 ? -1 : verbose - 1; break;
	case GO_VERBOSE:	verbose = verbose <  0 ?  0 : verbose + 1; break;
	case GO_PROGRESS:	progress++; break;
	case GO_SCAN_PROGRESS:	scan_progress++; break;
	case GO_LOGGING:	logging++; break;
	case GO_ESC:		err += ScanEscapeChar(optarg) < 0; break;
	case GO_IO:		ScanIOMode(optarg); break;
	case GO_FORCE:		opt_force++; break;
	case GO_DIRECT:		opt_direct++; break;

	case GO_TITLES:		AtFileHelper(optarg,0,0,AddTitleFile); break;
	case GO_UTF_8:		use_utf8 = true; break;
	case GO_NO_UTF_8:	use_utf8 = false; break;
	case GO_LANG:		lang_info = optarg; break;
	case GO_CERT:		AtFileHelper(optarg,0,0,AddCertFile); break;
	case GO_FAKE_SIGN:	err += ScanRule(optarg,PAT_FAKE_SIGN); break;

	case GO_TEST:		testmode++; break;

#if OPT_OLD_NEW
	case GO_OLD:		newmode = -1; break;
	case GO_NEW:		newmode = +1; break;
#endif

	case GO_SOURCE:		AppendStringField(&source_list,optarg,false); break;
	case GO_NO_EXPAND:	opt_no_expand = true; break;
	case GO_RECURSE:	AppendStringField(&recurse_list,optarg,false); break;
	case GO_RDEPTH:		err += ScanOptRDepth(optarg); break;
	case GO_AUTO:		opt_source_auto++; break;

	case GO_INCLUDE:	AtFileHelper(optarg,SEL_ID,SEL_FILE,AddIncludeID); break;
	case GO_EXCLUDE:	AtFileHelper(optarg,SEL_ID,SEL_FILE,AddExcludeID); break;
	case GO_INCLUDE_PATH:	AtFileHelper(optarg,0,0,AddIncludePath); break;
	case GO_EXCLUDE_PATH:	AtFileHelper(optarg,0,0,AddExcludePath); break;
	case GO_INCLUDE_FIRST:	include_first = true; break;

	case GO_ONE_JOB:	job_limit = 1; break;
	case GO_IGNORE:		ignore_count++; break;
	case GO_IGNORE_FST:	allow_fst = false; break;
	case GO_IGNORE_SETUP:	ignore_setup = true; break;
	case GO_LINKS:		opt_links = true; break;

	case GO_PSEL:		err += ScanOptPartSelector(optarg); break;
	case GO_RAW:		part_selector.whole_disc
					= part_selector.whole_part = true; break;
	case GO_FLAT:		opt_flat++; break;
	case GO_COPY_GC:	opt_copy_gc = true; break;
	case GO_NO_LINK:	opt_no_link = true; break;
	case GO_NEEK:		SetupNeekMode(); break;
	case GO_HOOK:		opt_hook = 1; break;
	case GO_ENC:		err += ScanOptEncoding(optarg); break;
	case GO_REGION:		err += ScanOptRegion(optarg); break;
	case GO_COMMON_KEY:	err += ScanOptCommonKey(optarg); break;
	case GO_IOS:		err += ScanOptIOS(optarg); break;
	case GO_MODIFY:		err += ScanOptModify(optarg); break;
	case GO_NAME:		err += ScanOptName(optarg); break;
	case GO_ID:		err += ScanOptId(optarg); break;
	case GO_DISC_ID:	err += ScanOptDiscId(optarg); break;
	case GO_BOOT_ID:	err += ScanOptBootId(optarg); break;
	case GO_TICKET_ID:	err += ScanOptTicketId(optarg); break;
	case GO_TMD_ID:		err += ScanOptTmdId(optarg); break;
	case GO_WBFS_ID:	err += ScanOptWbfsId(optarg); break;
	case GO_FILES:		err += ScanRule(optarg,PAT_FILES); break;
	case GO_RM_FILES:	err += ScanRule(optarg,PAT_RM_FILES); break;
	case GO_ZERO_FILES:	err += ScanRule(optarg,PAT_ZERO_FILES); break;
	case GO_IGNORE_FILES:	err += ScanRule(optarg,PAT_IGNORE_FILES); break;
	case GO_REPL_FILE:	err += ScanOptFile(optarg,false); break;
	case GO_ADD_FILE:	err += ScanOptFile(optarg,true); break;
	case GO_TRIM:		err += ScanOptTrim(optarg); break;
	case GO_ALIGN:		err += ScanOptAlign(optarg); break;
	case GO_ALIGN_PART:	err += ScanOptAlignPart(optarg); break;
	case GO_ALIGN_FILES:	opt_align_files = true; break;
	case GO_DISC_SIZE:	err += ScanOptDiscSize(optarg); break;
	case GO_OVERLAY:	break;
	case GO_PATCH_FILE:	opt_patch_file = optarg; break;
	case GO_DEST:		SetDest(optarg,false); break;
	case GO_DEST2:		SetDest(optarg,true); break;
	case GO_SPLIT:		opt_split++; break;
	case GO_SPLIT_SIZE:	err += ScanOptSplitSize(optarg); break;
	case GO_PREALLOC:	err += ScanPreallocMode(optarg); break;
	case GO_TRUNC:		opt_truncate++; break;
	case GO_CHUNK_MODE:	err += ScanChunkMode(optarg); break;
	case GO_CHUNK_SIZE:	err += ScanChunkSize(optarg); break;
	case GO_MAX_CHUNKS:	err += ScanMaxChunks(optarg); break;
	case GO_COMPRESSION:	err += ScanOptCompression(false,optarg); break;
	case GO_MEM:		err += ScanOptMem(optarg,true); break;
	case GO_PRESERVE:	break;
	case GO_UPDATE:		break;
	case GO_OVERWRITE:	break;
	case GO_DIFF:		break;
	case GO_REMOVE:		break;

	case GO_WDF:		output_file_type = OFT_WDF; break;
	case GO_WIA:		err += ScanOptCompression(true,optarg); break;
	case GO_ISO:		output_file_type = OFT_PLAIN; break;
	case GO_CISO:		output_file_type = OFT_CISO; break;
	case GO_WBFS:		output_file_type = OFT_WBFS; break;
	case GO_FST:		output_file_type = OFT_FST; break;

    #if WDF2_ENABLED > 1
	case GO_WDF1:		SetWDF2Mode(1,0); break;
	case GO_WDF2:		err += SetWDF2Mode(2,optarg); break;
	case GO_WDF_ALIGN:	err += ScanOptWDFAlign(optarg); break;
    #endif

	case GO_ITIME:		SetTimeOpt(PT_USE_ITIME|PT_F_ITIME); break;
	case GO_MTIME:		SetTimeOpt(PT_USE_MTIME|PT_F_MTIME); break;
	case GO_CTIME:		SetTimeOpt(PT_USE_CTIME|PT_F_CTIME); break;
	case GO_ATIME:		SetTimeOpt(PT_USE_ATIME|PT_F_ATIME); break;

	case GO_LONG:		long_count++; break;
	case GO_BRIEF:		brief_count++; break;
	case GO_NUMERIC:	break;
	case GO_TECHNICAL:	opt_technical++; break;
	case GO_REALPATH:	break;
	case GO_UNIQUE:	    	break;
	case GO_NO_HEADER:	break;
	case GO_NULL:		break;
	case GO_OLD_STYLE:	print_old_style++; break;
	case GO_SECTIONS:	print_sections++; break;
	case GO_SHOW:		err += ScanOptShow(optarg); break;
	case GO_UNIT:		err += ScanOptUnit(optarg); break;
	case GO_SORT:		err += ScanOptSort(optarg); break;

	case GO_PMODE:
	    {
		const int new_pmode = ScanPrefixMode(optarg);
		if ( new_pmode == -1 )
		    err++;
		else
		    prefix_mode = new_pmode;
	    }
	    break;

	case GO_TIME:
	    if ( ScanAndSetPrintTimeMode(optarg) == PT__ERROR )
		err++;
	    break;

	case GO_LIMIT:
	    {
		u32 limit;
		if (ScanSizeOptU32(&limit,optarg,1,0,"limit",0,INT_MAX,0,0,true))
		    err++;
		else
		    opt_limit = limit;
	    }
	    break;

	case GO_FILE_LIMIT:
	    {
		u32 file_limit;
		if (ScanSizeOptU32(&file_limit,optarg,1,0,"fil-limit",0,INT_MAX,0,0,true))
		    err++;
		else
		    opt_file_limit = file_limit;
	    }
	    break;

	case GO_BLOCK_SIZE:
	    {
		u32 block_size;
		if (ScanSizeOptU32(&block_size,optarg,1,0,"block-size",0,INT_MAX,0,0,true))
		    err++;
		else
		    opt_block_size = block_size;
	    }
	    break;

	case GO_JOB_LIMIT:
	    {
		u32 limit;
		if (ScanSizeOptU32(&limit,optarg,1,0,"job-limit",0,UINT_MAX,0,0,true))
		    err++;
		else
		    job_limit = limit ? limit : ~(u32)0;
	    }
	    break;

	// no default case defined
	//	=> compiler checks the existence of all enum values

      }
    }
    NormalizeIdOptions();
    if ( OptionUsed[OPT_NO_HEADER] )
	opt_show_mode &= ~SHOW_F_HEAD;

 #ifdef DEBUG
    DumpUsedOptions(&InfoUI,TRACE_FILE,11);
 #endif

    if ( verbose > 3 && !is_env )
    {
	print_title(stdout);
	printf("PROGRAM_NAME   = %s\n",progname);
	if (lang_info)
	    printf("LANG_INFO      = %s\n",lang_info);
	ccp * sp;
	for ( sp = search_path; *sp; sp++ )
	    printf("SEARCH_PATH[%td] = %s\n",sp-search_path,*sp);
	printf("\n");
    }

    return !err ? ERR_OK : max_error ? max_error : ERR_SYNTAX;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                   check command                 ///////////////
///////////////////////////////////////////////////////////////////////////////

enumError CheckCommand ( int argc, char ** argv )
{
    TRACE("CheckCommand(%d,) optind=%d\n",argc,optind);

    if ( optind >= argc )
    {
	ERROR0(ERR_SYNTAX,"Missing command.\n");
	hint_exit(ERR_SYNTAX);
    }

    int cmd_stat;
    const CommandTab_t * cmd_ct = ScanCommand(&cmd_stat,argv[optind],CommandTab);
    if (!cmd_ct)
    {
	PrintCommandError(CommandTab,argv[optind],cmd_stat,0);
	hint_exit(ERR_SYNTAX);
    }

    TRACE("COMMAND FOUND: #%lld = %s\n",(u64)cmd_ct->id,cmd_ct->name1);
    current_command = cmd_ct;

    enumError err = VerifySpecificOptions(&InfoUI,cmd_ct);
    if (err)
	hint_exit(err);

    argc -= optind+1;
    argv += optind+1;

    if ( cmd_ct->id == CMD_MIX || cmd_ct->id == CMD_TEST )
	while ( argc-- > 0 )
	    AddParam(*argv++,false);
    else
	while ( argc-- > 0 )
	    AtFileHelper(*argv++,false,true,AddParam);

    switch ((enumCommands)cmd_ct->id)
    {
	case CMD_VERSION:	version_exit();
	case CMD_HELP:		PrintHelp(&InfoUI,stdout,0,"HELP",0); break;
	case CMD_INFO:		err = cmd_info(); break;
	case CMD_TEST:		err = cmd_test(); break;
	case CMD_ERROR:		err = cmd_error(); break;
	case CMD_COMPR:		err = cmd_compr(); break;
	case CMD_ANAID:		err = cmd_anaid(); break;
	case CMD_EXCLUDE:	err = cmd_exclude(); break;
	case CMD_TITLES:	err = cmd_titles(); break;
	case CMD_GETTITLES:	err = cmd_gettitles(); break;
	case CMD_CERT:		err = cmd_cert(); break;
	case CMD_CREATE:	err = cmd_create(); break;

	case CMD_FILELIST:	err = cmd_filelist(); break;
	case CMD_FILETYPE:	err = cmd_filetype(); break;
	case CMD_ISOSIZE:	err = cmd_isosize(); break;

	case CMD_DUMP:		err = cmd_dump(); break;
	case CMD_ID6:		err = cmd_id6(); break;
	case CMD_FRAGMENTS:	err = cmd_fragments(); break;
	case CMD_LIST:		err = cmd_list(0); break;
	case CMD_LIST_L:	err = cmd_list(1); break;
	case CMD_LIST_LL:	err = cmd_list(2); break;
	case CMD_LIST_LLL:	err = cmd_list(3); break;

	case CMD_FILES:		err = cmd_files(0); break;
	case CMD_FILES_L:	err = cmd_files(1); break;
	case CMD_FILES_LL:	err = cmd_files(2); break;

	case CMD_DIFF:		err = cmd_diff(false); break;
	case CMD_FDIFF:		err = cmd_diff(true); break;
	case CMD_EXTRACT:	err = cmd_extract(); break;
	case CMD_COPY:		err = cmd_copy(); break;
	case CMD_CONVERT:	err = cmd_convert(); break;
	case CMD_EDIT:		err = cmd_edit(); break;
	case CMD_IMGFILES:	err = cmd_imgfiles(); break;
	case CMD_REMOVE:	err = cmd_remove(); break;
	case CMD_MOVE:		err = cmd_move(); break;
	case CMD_RENAME:	err = cmd_rename(true); break;
	case CMD_SETTITLE:	err = cmd_rename(false); break;

	case CMD_VERIFY:	err = cmd_verify(); break;
	case CMD_SKELETON:	err = cmd_skeletonize(); break;
	case CMD_MIX:		err = cmd_mix(); break;

	// no default case defined
	//	=> compiler checks the existence of all enum values

	case CMD__NONE:
	case CMD__N:
	    help_exit(false);
    }

    return PrintErrorStat(err,cmd_ct->name1);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                       main()                    ///////////////
///////////////////////////////////////////////////////////////////////////////

int main ( int argc, char ** argv )
{
    SetupLib(argc,argv,WIT_SHORT,PROG_WIT);
    allow_fst = true;

    InitializeStringField(&source_list);
    InitializeStringField(&recurse_list);


    //----- process arguments

    if ( argc < 2 )
    {
	printf("\n%s\n%s\nVisit %s%s for more info.\n\n",
		text_logo, TITLE, URI_HOME, WIT_SHORT );
	hint_exit(ERR_OK);
    }

    enumError err = CheckEnvOptions("WIT_OPT",CheckOptions);
    if (err)
	hint_exit(err);

    err = CheckOptions(argc,argv,false);
    if (err)
	hint_exit(err);

    SetupFilePattern(file_pattern+PAT_FILES);
    err = CheckCommand(argc,argv);
    CloseAll();

    if (SIGINT_level)
	err = ERROR0(ERR_INTERRUPT,"Program interrupted by user.");
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                     END                         ///////////////
///////////////////////////////////////////////////////////////////////////////

