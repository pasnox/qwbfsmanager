
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

#include "lib-bzip2.h"
#include "lib-lzma.h"

///////////////////////////////////////////////////////////////////////////////
//   This file is included by wwt.c and wit.c and contains common commands.  //
///////////////////////////////////////////////////////////////////////////////

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    cmd_error()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_error()
{
    if (!n_param)
    {
	if ( print_sections )
	{
	    int i;
	    for ( i=0; i<ERR__N; i++ )
		printf("\n[error-%02u]\ncode=%u\nname=%s\ntext=%s\n",
			i, i, GetErrorName(i), GetErrorText(i));
	}
	else
	{
	    const bool print_header = !OptionUsed[OPT_NO_HEADER];

	    if (print_header)
	    {
		print_title(stdout);
		printf(" List of error codes\n\n");
	    }
	    int i;

	    // calc max_wd
	    int max_wd = 0;
	    for ( i=0; i<ERR__N; i++ )
	    {
		const int len = strlen(GetErrorName(i));
		if ( max_wd < len )
		    max_wd = len;
	    }

	    // print table
	    for ( i=0; i<ERR__N; i++ )
		printf("%3d : %-*s : %s\n",i,max_wd,GetErrorName(i),GetErrorText(i));

	    if (print_header)
		printf("\n");
	}

	return ERR_OK;
    }

    int stat;
    long num = ERR__N;
    if ( n_param != 1 )
	stat = ERR_SYNTAX;
    else
    {
	char * end;
	num = strtoul(first_param->arg,&end,10);
	stat = *end ? ERR_SYNTAX : num < 0 || num >= ERR__N ? ERR_SEMANTIC : ERR_OK;
    }

    if (print_sections)
	printf("\n[error]\ncode=%lu\nname=%s\ntext=%s\n",
		num, GetErrorName(num), GetErrorText(num));
    else if (long_count)
	printf("%s\n",GetErrorText(num));
    else
	printf("%s\n",GetErrorName(num));
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    cmd_gettitles()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_gettitles()
{
    if (n_param)
	return ERROR0(ERR_SYNTAX,"No parameters needed: %s%s\n",
		first_param->arg, n_param > 1 ? " ..." : "" );

 #ifdef __CYGWIN__

    ccp progs = getenv("ProgramFiles");
    if ( !progs || !*progs )
	progs = "C:/Program Files";
    char dirbuf[PATH_MAX], normbuf[PATH_MAX];
    snprintf(dirbuf,sizeof(dirbuf),"%s/%s",progs,WIN_INSTALL_PATH);
    NormalizeFilenameCygwin(normbuf,sizeof(normbuf),dirbuf);
    PRINT("-> %s\n",dirbuf);
    PRINT("-> %s\n",normbuf);
    
    if (chdir(normbuf))
	return ERROR0(ERR_CANT_OPEN,"Can't change directory: %s\n",normbuf);

    system("./load-titles.sh --cygwin");

 #else

    if (chdir(SHARE_PATH))
	return ERROR0(ERR_CANT_OPEN,"Can't change directory: %s\n",SHARE_PATH);

    system("./load-titles.sh");

 #endif

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    cmd_compr()			///////////////
///////////////////////////////////////////////////////////////////////////////

void print_default_compr ( ccp mode )
{
    int level = 0;
    u32 csize = 0;
    wd_compression_t compr = ScanCompression(mode,true,&level,&csize);
    CalcDefaultSettingsWIA(&compr,&level,&csize);
    printf("mode-%s=%s\n",
		mode,
		wd_print_compression(0,0,compr,level,csize,2));
};

///////////////////////////////////////////////////////////////////////////////

enumError cmd_compr()
{
    const bool have_param = n_param > 0;
    if (!n_param)
    {
	int i;
	for ( i = 0; i < WD_COMPR__N; i++ )
	    AddParam(wd_get_compression_name(i,0),false);
	if (long_count)
	{
	    AddParam(" DEFAULT",false);
	    AddParam(" FAST",false);
	    AddParam(" GOOD",false);
	    AddParam(" BEST",false);
	    AddParam(" MEM",false);
	    
	    char buf[2] = {0,0};
	    for ( i = '0'; i <= '9'; i++ )
	    {
		buf[0] = i;
		AddParam(buf,true);
	    }
	}
    }

    int err_count = 0;
    if (print_sections)
    {
	if (!have_param)
	{
	    printf( "\n[compression-modes]\n"
		    "n-methods=%u\n", WD_COMPR__N );
	    print_default_compr("default");
	    print_default_compr("fast");
	    print_default_compr("good");
	    print_default_compr("best");
	    print_default_compr("mem");
	}

	int index = 0;
	ParamList_t * param;
	for ( param = first_param; param; param = param->next, index++ )
	{
	    int level;
	    u32 csize;
	    wd_compression_t compr = ScanCompression(param->arg,true,&level,&csize);
	    printf( "\n[compression-mode-%u]\n"
		"input=%s\n"
		"num=%d\n"
		"name=%s\n",
		index, param->arg,
		compr, wd_get_compression_name(compr,"-") );
	    if ( compr == (wd_compression_t)-1 )
		err_count++;
	    else
	    {
		CalcDefaultSettingsWIA(&compr,&level,&csize);
		if ( level > 0 )
		    printf("level=%u\n",level);
		if ( csize > 0 )
		    printf("chunk-factor=%u\nchunk-size=%u\n",
				csize/WIA_BASE_CHUNK_SIZE, csize );
	     #ifdef NO_BZIP2
		if ( compr == WD_COMPR_BZIP2 )
		    fputs("not-supported=1\n",stdout);
	     #endif
	    }
	}
	putchar('\n');
    }
    else if (long_count)
    {
	const bool print_header = !OptionUsed[OPT_NO_HEADER];
	if (print_header)
	    printf( "\n"
		    " mode           memory usage\n"
		      " %s         reading   writing   input mode\n"
		    "---------------------------------------------\n",
		    OptionUsed[OPT_NUMERIC] ? "num " : "name" );

	const int mode = OptionUsed[OPT_NUMERIC] ? 1 : 2;
	ParamList_t * param;
	for ( param = first_param; param; param = param->next )
	{
	    int level;
	    u32 csize;
	    wd_compression_t compr = ScanCompression(param->arg,true,&level,&csize);
	    if ( verbose > 0 )
	    {
		wd_compression_t compr2 = compr;
		CalcDefaultSettingsWIA(&compr2,&level,&csize);
	    }

	    if ( compr == (wd_compression_t)-1 )
	    {
		err_count++;
		printf(" -                -         -     %s\n",param->arg);
	    }
	    else
	    {
		u32 read_size  = CalcMemoryUsageWIA(compr,level,csize,false);
		u32 write_size = CalcMemoryUsageWIA(compr,level,csize,true);
		printf(" %-11s %s  %s   %.30s\n",
			wd_print_compression(0,0,compr,level,csize,mode),
			wd_print_size_1024(0,0,read_size,true),
			wd_print_size_1024(0,0,write_size,true),
			param->arg );
	    }
	}

	if (print_header)
	    putchar('\n');
    }
    else
    {
	const int mode = OptionUsed[OPT_NUMERIC] ? 1 : 2;
	ParamList_t * param;
	for ( param = first_param; param; param = param->next )
	{
	    int level;
	    u32 csize;
	    wd_compression_t compr = ScanCompression(param->arg,true,&level,&csize);
	    if ( verbose > 0 )
	    {
		wd_compression_t compr2 = compr;
		CalcDefaultSettingsWIA(&compr2,&level,&csize);
	    }

	 #ifdef NO_BZIP2
	    if ( !have_param && compr == WD_COMPR_BZIP2 )
		continue; // ignore it
	 #endif
	    if ( compr == (wd_compression_t)-1 )
		err_count++;
	    printf("%s\n",wd_print_compression(0,0,compr,level,csize,mode));
	}
    }

    return err_count ? ERR_WARNING : ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    cmd_exclude()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_exclude()
{
    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AtFileHelper(param->arg,0,1,AddExcludeID);

    DumpExcludeDB();    
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    cmd_titles()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_titles()
{
    ParamList_t * param;
    for ( param = first_param; param; param = param->next )
	AtFileHelper(param->arg,0,0,AddTitleFile);

    InitializeTDB();
    DumpIDDB(&title_db,stdout);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			cmd_test_options()		///////////////
///////////////////////////////////////////////////////////////////////////////

static void print_val( ccp title, u64 val, ccp comment )
{
    ccp t1000 = wd_print_size_1000(0,0,val,true);
    ccp t1024 = wd_print_size_1024(0,0,val,true);

    if (strcmp(t1000,t1024))
	printf("  %-13s%16llx = %12lld = %s = %s%s\n",
			title, val, val, t1024, t1000, comment ? comment : "" );
    else
	printf("  %-13s%16llx = %12lld%s\n",
			title, val, val, comment ? comment : "" );
}

//-----------------------------------------------------------------------------

enumError cmd_test_options()
{
    printf("Options (hex=dec):\n");

    printf("  test:        %16d\n",testmode);
    printf("  verbose:     %16d\n",verbose);
    printf("  width:       %16d\n",opt_width);

 #if IS_WWT
    print_val( "size:",		opt_size, 0);
    print_val( "hd sec-size:",	opt_hss, 0);
    print_val( "wb sec-size:",	opt_wss, 0);
    print_val( "repair-mode:",	repair_mode, 0);
 #else
    u64 opt_size = 0;
 #endif

    *iobuf = 0;
    if (output_file_type)
	snprintf(iobuf,sizeof(iobuf)," = %s",oft_info[output_file_type].name);
    print_val( "output-mode:",	output_file_type, iobuf );

 #if WDF2_ENABLED
    print_val( "wdf-version:",	opt_wdf_version, 0 );
    print_val( "wdf-align:",	opt_wdf_align, 0 );
 #endif

    print_val( "chunk-mode:",	opt_chunk_mode,	0 );
    print_val( "chunk-size:",	opt_chunk_size,	force_chunk_size ? " FORCE!" : "" );
    print_val( "max-chunks:",	opt_max_chunks,	0 );
    {
	u64 filesize[] = { 100ull*MiB, 1ull*GiB, 10ull*GiB, opt_size, 0 };
	u64 *fs_ptr = filesize;	for (;;)
	{
	    u32 n_blocks;
	    u32 block_size = CalcBlockSizeCISO(&n_blocks,*fs_ptr);
	    printf("    size->CISO %16llx = %12lld -> %5u * %8x/hex == %12llx/hex\n",
			*fs_ptr, *fs_ptr, n_blocks, block_size,
			(u64)block_size * n_blocks );
	    if (!*fs_ptr++)
		break;
	}
    }

    print_val( "split-size:",	opt_split_size, 0 );
    printf("  compression: %16x = %12d = %s (level=%d)\n",
		opt_compr_method, opt_compr_method,
		wd_get_compression_name(opt_compr_method,"?"), opt_compr_level );
    printf("    level:     %16x = %12d\n",opt_compr_level,opt_compr_level);
    print_val( "  chunk-size:",	opt_compr_chunk_size, 0 );

    print_val( "mem:",		opt_mem, 0 );
    GetMemLimit();
    print_val( "mem limit:",	opt_mem, 0 );

    printf("  escape-char: %16x = %12d\n",escape_char,escape_char);
    printf("  print-time:  %16x = %12d\n",opt_print_time,opt_print_time);
    printf("  sort-mode:   %16x = %12d\n",sort_mode,sort_mode);
    printf("  show-mode:   %16x = %12d\n",opt_show_mode,opt_show_mode);
    printf("  unit:        %16x = %12d, unit=%s\n",
			opt_unit, opt_unit, wd_get_size_unit(opt_unit,"?") );
    printf("  limit:       %16x = %12d\n",opt_limit,opt_limit);
 #if IS_WIT
    printf("  file-limit:  %16x = %12d\n",opt_file_limit,opt_file_limit);
    printf("  block-size:  %16x = %12d\n",opt_block_size,opt_block_size);
 #endif
    printf("  rdepth:      %16x = %12d\n",opt_recurse_depth,opt_recurse_depth);
    printf("  enc:         %16x = %12d\n",encoding,encoding);
    printf("  region:      %16x = %12d\n",opt_region,opt_region);
    printf("  prealloc:    %16x = %12d\n",prealloc_mode,prealloc_mode);

    if (opt_ios_valid)
    {
	const u32 hi = opt_ios >> 32;
	const u32 lo = (u32)opt_ios;
	if ( hi == 1 && lo < 0x100 )
	    printf("  ios:        %08x-%08x = IOS %u\n", hi, lo, lo );
	else
	    printf("  ios:        %08x-%08x\n", hi, lo );
    }

    printf("  modify:      %16x = %12d\n",opt_modify,opt_modify);
    if (modify_name)
	printf("  modify name: '%s'\n",modify_name);
    if (modify_id)
	printf("  modify id:        '%s'\n",modify_id);
    if (modify_disc_id)
	printf("  modify disc id:   '%s'\n",modify_disc_id);
    if (modify_boot_id)
	printf("  modify boot id:   '%s'\n",modify_boot_id);
    if (modify_ticket_id)
	printf("  modify ticket id: '%s'\n",modify_ticket_id);
    if (modify_tmd_id)
	printf("  modify tmd id:    '%s'\n",modify_tmd_id);
    if (modify_wbfs_id)
	printf("  modify wbfs id:   '%s'\n",modify_wbfs_id);

 #if IS_WWT
    char buf_set_time[20];
    if (opt_set_time)
    {
	struct tm * tm = localtime(&opt_set_time);
	strftime(buf_set_time,sizeof(buf_set_time),"%F %T",tm);
    }
    else
	strcpy(buf_set_time,"NULL");
    printf("  set-time:    %16llx = %12lld = %s\n",
		(u64)opt_set_time, (u64)opt_set_time,buf_set_time );
 #endif

    printf("  trim:        %16x = %12u\n",opt_trim,opt_trim);
    print_val( "align #1:",	opt_align1, 0 );
    print_val( "align #2:",	opt_align2, 0 );
    print_val( "align #3:",	opt_align3, 0 );
    print_val( "align-part:",	opt_align_part, 0 );
    print_val( "disc-size:",	opt_disc_size, 0 );
    printf("  partition selector:\n");
    wd_print_select(stdout,6,&part_selector);

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PrintErrorStat()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError PrintErrorStat ( enumError err, ccp cmdname )
{
    if ( print_sections )
    {
	putchar('\n');
	if ( err )
	    printf("[error]\nprogram=%s\ncommand=%s\ncode=%u\nname=%s\ntext=%s\n\n",
			progname, cmdname, err, GetErrorName(err), GetErrorText(err) );
    }

    if ( err && verbose > 0 || err == ERR_NOT_IMPLEMENTED )
	fprintf(stderr,"%s: Command '%s' returns with status #%d [%s]\n",
			progname, cmdname, err, GetErrorName(err) );

    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  cmd_info()			///////////////
///////////////////////////////////////////////////////////////////////////////

static void info_file_formats()
{
    if (print_sections)
    {
	printf(	"\n"
		"[FILE-FORMAT]\n"
		"n=%u\n",
		OFT__N - 1);

	ccp text = "list=";
	enumOFT oft;
	for ( oft = 1; oft < OFT__N; oft++ )
	{
	    printf("%s%s",text,oft_info[oft].name);
	    text = " ";
	}
	putchar('\n');

	for ( oft = 1; oft < OFT__N; oft++ )
	{
	    const OFT_info_t * info = oft_info + oft;
	    printf(	"\n"
			"[FILE-FORMAT:%s]\n"
			"name=%s\n"
			"info=%s\n"
			"option=%s\n"
			"extensions=%s %s\n"
			"attributes=%s%s%s%s%s%s%s\n"
			,info->name
			,info->name
			,info->info
			,info->option
			,info->ext1 ? info->ext1 : ""
			,info->ext2 ? info->ext2 : ""
			,info->attrib & OFT_A_READ	? "read "   : ""
			,info->attrib & OFT_A_WRITE	? "write "  : ""
			,info->attrib & OFT_A_MODIFY	? "modify " : ""
			,info->attrib & OFT_A_EXTEND	? "extend " : ""
			,info->attrib & OFT_A_FST	? "fst "    : ""
			,info->attrib & OFT_A_COMPR	? "compr "  : ""
			,info->attrib & OFT_A_NOSIZE	? "nosize "  : ""
			);
	}
	return;
    }

    //----- table output

    enumOFT oft;
    int info_fw = 0;
    for ( oft = 1; oft < OFT__N; oft++ )
    {
	const int len = strlen(oft_info[oft].info);
	if ( info_fw < len )
	     info_fw = len;
    }

    printf("\n"
	   "File formats:\n\n"
	   "  name  %-*s  option  extensions  attributes\n"
	   " %.*s\n",
	   info_fw, "description",
	   info_fw+61, wd_sep_200 );

    for ( oft = 1; oft < OFT__N; oft++ )
    {
	const OFT_info_t * i = oft_info + oft;
	printf("  %-4s  %-*s  %-7s %-5s %-5s %s %s %s %s %s\n",
		i->name, info_fw, i->info,
		i->option ? i->option : "  -",
		i->ext1 && *i->ext1 ? i->ext1 : " -",
		i->ext2 && *i->ext2 ? i->ext2 : " -",
		i->attrib & OFT_A_READ		? "read"   : "-  ",
		i->attrib & OFT_A_WRITE		? "write"  : "-    ",
		i->attrib & OFT_A_MODIFY	? "modify" : "-     ",
		i->attrib & OFT_A_EXTEND	? "extend" : "-     ",
		i->attrib & OFT_A_FST		? "fst"
		: i->attrib & OFT_A_COMPR	? "compr"
		: i->attrib & OFT_A_NOSIZE	? "nosize" : "-" );
    }
}

///////////////////////////////////////////////////////////////////////////////

enum
{
    INFO_FILE_FORMAT	= 0x0001,

    INFO__ALL		= 0x0001
};

///////////////////////////////////////////////////////////////////////////////

enumError cmd_info()
{
    static const CommandTab_t cmdtab[] =
    {
	{ INFO__ALL,		"ALL",		0,		0 },
	{ INFO_FILE_FORMAT,	"FILE-FORMATS",	"FORMATS",	0 },

	{ 0,0,0,0 }
    };

    u32 keys = 0;
    ParamList_t * param;
    for (  param = first_param; param; param = param->next )
    {
	ccp arg = param->arg;
	if ( !arg || !*arg )
	    continue;

	const CommandTab_t * cmd = ScanCommand(0,arg,cmdtab);
	if (!cmd)
	    return ERROR0(ERR_SYNTAX,"Unknown keyword: %s\n",arg);

	keys |= cmd->id;
    }

    if (!keys)
	keys = INFO__ALL;

    if (print_sections)
    {
	printf("\n[INFO]\n");

	ccp text = "infos-avail=";
	const CommandTab_t * cptr;
	for ( cptr = cmdtab + 1; cptr->name1; cptr++ )
	{
	    printf("%s%s",text,cptr->name1);
	    text = " ";
	}
	putchar('\n');

	text = "infos=";
	for ( cptr = cmdtab + 1; cptr->name1; cptr++ )
	{
	    if ( cptr->id & keys )
	    {
		printf("%s%s",text,cptr->name1);
		text = " ";
	    }
	}
	putchar('\n');
    }

    if ( keys & INFO_FILE_FORMAT )
	info_file_formats();

    putchar('\n');
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                     END                         ///////////////
///////////////////////////////////////////////////////////////////////////////

