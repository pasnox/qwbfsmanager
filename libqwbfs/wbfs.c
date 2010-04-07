// Copyright 2009 Kwiirk
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */

#ifdef WIN32
#include <windows.h>
#include "win32/xgetopt.h"
#include <direct.h>
#include <io.h>
#define snprintf _snprintf
#else
#include <getopt.h>
#include <unistd.h>
#endif
#include <errno.h>
#include <ctype.h>

#include <sys/stat.h>
#include "tools.h"
#include "libwbfs.h"


/* remove unused warnings */
#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(WIN32)
#pragma warning(disable : 4786)
# define UNUSED(x)  x
#else
# define UNUSED(x) x
#endif


#define GB (1024 * 1024 * 1024.)
#define MB (1024 * 1024)

int verbose;


/* helpers */
static void _spinner(int x, int y){ spinner(x, y); }
static void _progress(int x, int y){ progress(x, y); }

static int _wbfs_disc_read(void *fp, u32 lba, u32 count, void *iobuf)
{
        int ret = wbfs_disc_read(fp, lba, iobuf, count);
        static int num_fail=0;
        if (ret)
        {
                if(num_fail==0)
                        wbfs_error("error reading lba probably the two wbfs don't have the same granularity. Ignoring...\n");
                if(num_fail++ > 0x100)
                {
                        wbfs_fatal( "too many error giving up...\n");
                        return 1;
                }
        }
        else
                num_fail = 0;
	return 0;
}

/* Applets */

void wbfs_applet_ls(wbfs_t *p, int UNUSED(argc), char** UNUSED(argv))
{
	int count = wbfs_count_discs(p);

        int i;
        u32 size;
        u8 *b = wbfs_ioalloc(0x100);
        for (i = 0; i < count; i++)
        {
                if (!wbfs_get_disc_info(p, i, b, 0x100, &size))
                        fprintf(stderr, "%c%c%c%c%c%c|@|%-55s|@|%.2fG\n",b[0], b[1], b[2], b[3], b[4], b[5],
                                b + 0x20,size*4ULL/(GB));
        }

        wbfs_iofree(b);

	if(verbose)
	{
		u32 blcount = wbfs_count_usedblocks(p);
		fprintf(stderr, "------------\n %u Files - Total: %.2fG, Used: %.2fG, Free: %.2fG\n",
				count,
				(float)p->n_wbfs_sec * p->wbfs_sec_sz / GB, 
				(float)(p->n_wbfs_sec-blcount) * p->wbfs_sec_sz / GB,
				(float)(blcount) * p->wbfs_sec_sz / GB);
	}
}

void wbfs_applet_df(wbfs_t *p, int UNUSED(argc), char** UNUSED(argv))
{
	u32 count = wbfs_count_usedblocks(p);
	
	fprintf(stderr, "wbfs\n");
	fprintf(stderr, "  blocks : %u\n", count);
	fprintf(stderr, "  total  : %.2fG\n", (float)p->n_wbfs_sec * p->wbfs_sec_sz / GB);
	fprintf(stderr, "  used   : %.2fG\n", (float)(p->n_wbfs_sec-count) * p->wbfs_sec_sz / GB);
	fprintf(stderr, "  free   : %.2fG\n", (float)(count) * p->wbfs_sec_sz / GB);

}

void wbfs_applet_mkhbc(wbfs_t *p, int UNUSED(argc), char** UNUSED(argv))
{
	int count = wbfs_count_discs(p);
	FILE *xml;
	
        int i;
        u32 size;
        u8 *b = wbfs_ioalloc(0x100);
		
        for (i = 0; i < count; i++)
        {
#ifdef WIN32
                char dirname[7 + 1];
                char dolname[7 + 1 + 8 + 1];
                char pngname[7 + 1 + 8 + 1];
                char xmlname[7 + 1 + 8 + 1];

                wbfs_get_disc_info(p, i, b, 0x100, &size);

                snprintf(dirname, 7, "%c%c%c%c%c%c", b[0], b[1], b[2], b[3], b[4], b[5]);
                snprintf(dolname, 7 + 1 + 8, "%c%c%c%c%c%c\\boot.dol", b[0], b[1], b[2], b[3], b[4], b[5]);
                snprintf(pngname, 7 + 1 + 8, "%c%c%c%c%c%c\\icon.png", b[0], b[1], b[2], b[3], b[4], b[5]);
                snprintf(xmlname, 7 + 1 + 8, "%c%c%c%c%c%c\\meta.xml", b[0], b[1], b[2], b[3], b[4], b[5]);
			
                CreateDirectory(dirname, NULL);
                printf("%s\n", dirname);
			
                CopyFile("boot.dol", dolname, FALSE);
                CopyFile("icon.png", pngname, FALSE);

                xml = fopen(xmlname, "wb");

#else
                char filename[7];
                wbfs_get_disc_info(p,i,b,0x100,&size);
                snprintf(filename,7,"%c%c%c%c%c%c",b[0], b[1], b[2], b[3], b[4], b[5]);
                mkdir(filename, 0777);
                printf("%s\n",filename);
                if (chdir(filename))
                        wbfs_fatal("chdir");
                system("cp ../boot.dol .");
                system("cp ../icon.png .");

                xml = fopen("meta.xml","w");
                if (chdir(".."))
                        wbfs_fatal("chdir");

#endif
                fprintf(xml,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
                fprintf(xml,"<app>\n\t<name>%s</name>\n", b + 0x20);
                fprintf(xml,"<short_description>%.2fGB on USB HD</short_description>\n", size * 4ULL / GB);
                fprintf(xml,"<long_description>Click the Load Button to play %s from your USB Drive</long_description>\n", b+0x20);
                fprintf(xml,"</app>");
                fclose(xml);
        }
        wbfs_iofree(b);
}

void wbfs_applet_init(wbfs_t *UNUSED(p),int UNUSED(argc),char** UNUSED(argv))
{
        // nothing to do actually..
        // job already done by the reset flag of the wbfs_open_partition
}
void wbfs_applet_estimate(wbfs_t *p, int UNUSED(argc), char **argv)
{
        void *handle = wbfs_open_file_for_read(argv[0]);
        if(handle)
        {
                u32 estimation = wbfs_estimate_disc(p, wbfs_read_wii_file, handle, ONLY_GAME_PARTITION);
                fprintf(stderr, "%.2fG\n", estimation / (GB));
        }
}

void wbfs_applet_add(wbfs_t *p, int UNUSED(argc), char**argv)
{
        void *handle = wbfs_open_file_for_read(argv[0]);
	u8 discinfo[7];
	wbfs_disc_t *d;
        if(!handle)
                wbfs_fatal("unable to open disc file");
        else
        {
                wbfs_read_file(handle,6,discinfo);
                if (memcmp(discinfo, "WBFS", 4) == 0) // add all games in the wbfs partition
                {
                        wbfs_t *src_p;
                        int i, count;
                        u8 *b;
                        wbfs_close_file(handle);
                        src_p = wbfs_try_open_partition(argv[0], 0);
                        if (!src_p)
                        {
                                wbfs_fatal("incorrect wbfs file");
                                return ;
                        }
                        count = wbfs_count_discs(src_p);
                        b = wbfs_ioalloc(0x100);
                        if (count==0)
                          wbfs_fatal("no disc in wbfs");
                        for (i=0;i<count;i++)
                        {
                                if(!wbfs_get_disc_info(src_p, i, b, 0x100, NULL)){
                                        d = wbfs_open_disc(p, b);
                                        if(!d){ // not here already.
                                                d = wbfs_open_disc(src_p, b);
                                                wbfs_add_disc(p, _wbfs_disc_read, d, verbose?_spinner:_progress, ONLY_GAME_PARTITION, 0);
                                        }
                                        wbfs_close_disc(d);
                                }
                        }
                        wbfs_close(src_p);
                }
                else
                {
                        d = wbfs_open_disc(p,discinfo);
                        if(d)
                        {
                                discinfo[6]=0;
                                fprintf(stderr,"%s already in disc..\n",discinfo);
                                wbfs_close_disc(d);
                                return ;
                        } else
                                wbfs_add_disc(p,wbfs_read_wii_file,handle,verbose?_spinner:_progress,ONLY_GAME_PARTITION,0);
                }
        }
}


void wbfs_applet_rm(wbfs_t *p, int UNUSED(argc), char **argv)
{
	if ( wbfs_rm_disc(p, (u8 *)(argv[0])) )
		wbfs_error("Couldn't find %s", argv[0]);
}

void wbfs_applet_extract(wbfs_t *p, int argc, char **argv)
{
	wbfs_disc_t *d;
	d = wbfs_open_disc(p, (u8 *)(argv[0]));
	
	if (d)
	{
		void *handle;
		char isoname[0x100];
		int i,len;
		
		if(argc>1)
		{
			wbfs_memset(isoname, 0, sizeof(isoname));
			strncpy(isoname, argv[1], 0x40);
			len = strlen(isoname);
		}
		else
		{
			/* get the name of the title to find out the name of the iso */
			strncpy(isoname, (char *)d->header->disc_header_copy + 0x20, 0x100);
			len = strlen(isoname);
		
			// replace silly chars by '_'
			for (i = 0; i < len; i++)
			{
				if (isoname[i] == ' ' || isoname[i] == '/' || isoname[i] == ':')
				{
					isoname[i] = '_';
				}
			}
		}
		
		strncpy(isoname + len, ".iso", 0x100 - len);
		
		handle = wbfs_open_file_for_write(isoname);

		if (handle)
		{
                        // write a zero at the end of the iso to ensure the correct size
                        // XXX should check if the game is DVD9..
                        wbfs_file_reserve_space(handle, (d->p->n_wii_sec_per_disc/2)* 0x8000ULL);

			wbfs_extract_disc(d,wbfs_write_wii_sector_file, handle, _spinner);
                        
                        wbfs_close_file(handle);
		}
		
		wbfs_close_disc(d);
	}
	else
	{
		wbfs_error("Couldn't find %s", argv[0]);
	}
}
void wbfs_applet_ren(wbfs_t *p, int UNUSED(argc), char **argv)
{
	if(wbfs_ren_disc(p, (u8*)argv[0], (u8*)argv[1]))
	{
		wbfs_error("error");
	}
}
void wbfs_applet_nid(wbfs_t *p, int UNUSED(argc), char **argv)
{	
	if(wbfs_nid_disc(p,(u8*)argv[0], (u8*)argv[1]))
	{
		wbfs_error("error");
	}
}

void wbfs_applet_create(char**argv)
{
        char buf[1024];
        strncpy(buf,argv[0],1019);
        strcpy(buf+strlen(buf),".wbfs");
        void *handle= wbfs_open_file_for_write(buf);
        if(!handle)
                wbfs_fatal("unable to open dest file");
        else{
                // reserve space for the maximum size.
                wbfs_file_reserve_space(handle, 143432*2*0x8000ULL);
                wbfs_close_file(handle);
                wbfs_t *p = wbfs_try_open_partition(buf,1);
                if(p){
                        wbfs_applet_add(p,1,argv);
                        wbfs_trim(p);
                        wbfs_file_truncate(p->callback_data,p->n_hd_sec*512ULL);
                        wbfs_close(p);
                }
        }
}

struct wbfs_applets
{
	char *command;
	char *alternatecmd;
        char *help;
        int needed_argc;
	void (*function)(wbfs_t *p, int argc, char**argv);
}


#define APPLET(x, y, n, z) { #x, #y, z, n,wbfs_applet_##x }

wbfs_applets[] = 
{
        APPLET(ls,list,0," \n\t\t\t\t list all games in wbfs"),
        APPLET(df,info,0," \n\t\t\t\t show disc usage statistics"),
        APPLET(mkhbc,makehbc,0," \n\t\t\t\t make homebrew channel compatible directories"),
        APPLET(init,init,0," \n\t\t\t\t init a wbfs partition"),
        APPLET(add,add,1,"file.iso|file.wbfs  \n\t\t\t\t add game(s) to wbfs"),
        APPLET(rm,remove,1,"gameid \n\t\t\t\t remove game from wbfs"),
        APPLET(ren,rename,2,"gameid newname \n\t\t\t\t change name of a wbfs game"),
        APPLET(nid,changeid,2,"gameid newgameid \n\t\t\t\t Change id of a wbfs game"),
        APPLET(extract,extract,1,"gameid \n\t\t\t\t extract a game iso from wbfs"),
};

static int num_applets = sizeof(wbfs_applets) / sizeof(wbfs_applets[0]);

void usage(char **argv)
{
	int i;
#ifdef WIN32
	fprintf(stderr, "\nwbfs windows port build 'delta'. Mod v1.0 by Sorg.\n\nUsage:\n");
        fprintf(stderr, "%s [-f] [-v] <drive letter> command args \n",argv[0]);
#else
        fprintf(stderr, 
                "\nUsage: %s [-d disk|-p partition] [-v] [-f] commands args\n",
                argv[0]);
        
#endif
        fprintf(stderr, " [-f] force mode\n [-v] verbose mode\n");
        fprintf(stderr, "create game.iso \n\t\t\t\t Create a wbfs compressed version of the iso\n");
	
	for (i = 0; i < num_applets; i++)
	{
		if(!strcmp(wbfs_applets[i].command, wbfs_applets[i].alternatecmd))
			fprintf(stderr, "%s %s\n", wbfs_applets[i].command, wbfs_applets[i].help);
                else
			fprintf(stderr, "%s|%s %s\n", wbfs_applets[i].command, wbfs_applets[i].alternatecmd, wbfs_applets[i].help);
	}
}

int main(int argc, char *argv[])
{
	int opt;
	int i;
	char *partition = NULL, *disk = NULL;
        char *command;
#ifdef WIN32
	while ((opt = getopt(argc, argv, "hfv")) != -1) {
#else
	while ((opt = getopt(argc, argv, "p:d:hfv")) != -1) {
#endif
			switch (opt) {
#ifndef WIN32
			case 'p':
					partition = optarg;
					break;
			case 'd':
					disk = optarg;
					break;
#endif
			case 'f':
					wbfs_set_force_mode(1);
					break;
			case 'h':
			default: /* '?' */
				usage(argv);
			}
	}

	if (optind >= argc) {
		usage(argv);
		exit(EXIT_FAILURE);
	}
#ifdef WIN32
        partition = argv[optind++];
	if (optind >= argc) {
		usage(argv);
		exit(EXIT_FAILURE);
	}
	if (strlen(partition) != 1)
	{
		fprintf(stderr, "You must supply a valid drive letter.\n");
		return EXIT_FAILURE;
	}
#endif	
        command = argv[optind++];
        // special case for the create command :-/
	if ( strcmp(command, "create") == 0 )
	{
		if (optind+1 >= argc)
				usage(argv);
		else{
			wbfs_applet_create(argv+optind);
                        return 0;
                }
	}
	
	if (strcmp(command, "init") == 0)
	{
		char c;
		fprintf(stderr, "!!! Warning ALL data on drive '%s' will be lost irretrievably !!!\n\n", partition);
		fprintf(stderr, "Are you sure? (y/n): ");

		c = getchar();
		if (toupper(c) != 'Y')
		{
			fprintf(stderr, "Aborted.\n");
			return EXIT_FAILURE;
		}
		
		fprintf(stderr, "\n");
	}

	int command_found = 0;
	
	for (i = 0; i < num_applets; i++)
	{
		struct wbfs_applets *ap = &wbfs_applets[i];
		if ( strcmp( command, ap->command) == 0 || strcmp( command, ap->alternatecmd) == 0)
		{
			command_found = 1;
			wbfs_t *p = wbfs_try_open(disk, partition,
                                                  ap->function== wbfs_applet_init);
			if(!p)
                                break;
                        
                        if (optind + ap->needed_argc > argc)
                        {
                                fprintf(stderr,"too few arguments for command: %s\n", command);
                                usage(argv);
                        }
                        else
                                do {
                                        ap->function(p, ap->needed_argc, argv + optind );
                                        optind += ap->needed_argc;
                                }
                                while (ap->needed_argc && optind + ap->needed_argc <= argc);
                        wbfs_close(p);
			break;
		}
	}

	if (command_found) {
		exit(EXIT_SUCCESS);
	}
	else {
		fprintf(stderr,"Unknown command: %s\n", command);
                usage(argv);
		exit(EXIT_FAILURE);
	}
}
