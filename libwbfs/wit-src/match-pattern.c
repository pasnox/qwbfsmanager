
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

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "lib-std.h"
#include "match-pattern.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////                    variables                    ///////////////
///////////////////////////////////////////////////////////////////////////////

FilePattern_t file_pattern[PAT__N];

//
///////////////////////////////////////////////////////////////////////////////
///////////////                    pattern db                   ///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeFilePattern ( FilePattern_t * pat )
{
    DASSERT(pat);
    memset(pat,0,sizeof(*pat));
    InitializeStringField(&pat->rules);
    pat->match_all	= true;
}

///////////////////////////////////////////////////////////////////////////////

void ResetFilePattern ( FilePattern_t * pat )
{
    DASSERT(pat);
    ResetStringField(&pat->rules);
    InitializeFilePattern(pat);
}

///////////////////////////////////////////////////////////////////////////////

void InitializeAllFilePattern()
{
    memset(file_pattern,0,sizeof(file_pattern));

    FilePattern_t * pat = file_pattern;
    FilePattern_t * end = pat + PAT__N;
    for ( ; pat < end; pat++ )
	InitializeFilePattern(pat);
}

///////////////////////////////////////////////////////////////////////////////

struct macro_tab_t
{
    int len;
    ccp name;
    ccp expand;
};

static const struct macro_tab_t macro_tab[] =
{
    { 4, "base",	"+/*$" },
    { 6, "nobase",	"-/*$" },
    { 4, "disc",	"+/disc/" },
    { 6, "nodisc",	"-/disc/" },
    { 3, "sys",		"+/sys/" },
    { 5, "nosys",	"-/sys/" },
    { 5, "files",	"+/files/" },
    { 7, "nofiles",	"-/files/" },
    { 3, "wit",		"2+/h3.bin;1+/sys/fst.bin;+" },
    { 3, "wwt",		"2+/h3.bin;1+/sys/fst.bin;+" },
    { 7, "compose",	"+/cert.bin;3+/disc/;2+/*$;1+/sys/fst.bin;+" },
    { 4, "neek",	"3+/setup.txt;2+/h3.bin;1+/disc/;+" },
    { 5, "sneek",	"3+/setup.txt;2+/h3.bin;1+/disc/;+" },

    {0,0,0}
};

///////////////////////////////////////////////////////////////////////////////

int AddFilePattern ( ccp arg, int pattern_index )
{
    TRACE("AddFilePattern(%s,%d)\n",arg,pattern_index);
    DASSERT( pattern_index >= 0 );
    DASSERT( pattern_index < PAT__N );
    
    if ( !arg || (u32)pattern_index >= PAT__N )
	return 0;

    FilePattern_t * pat = file_pattern + pattern_index;
   
    pat->is_active = true;

    while (*arg)
    {
	ccp start = arg;
	bool ok = false;
	if ( *arg >= '1' && *arg <= '9' )
	{
	    while ( *arg >= '0' && *arg <= '9' )
		arg++;
	    if ( *arg == '+' || *arg == '-' )
		ok = true;
	    else
		arg = start;
	}

	// hint: '=' is obsolete and compatible to ':'

	if ( !ok && *arg != '+' && *arg != '-' && *arg != ':' && *arg != '=' )
	    return ERROR0(ERR_SYNTAX,
		"File pattern rule must begin with '+', '-' or ':' => %.20s\n",arg);

	while ( *arg && *arg != ';' )
	    arg++;

	if ( *start == ':' || *start == '=' )
	{
	    const int len = arg - ++start;
	    const struct macro_tab_t *tab;
	    for ( tab = macro_tab; tab->len; tab++ )
		if ( tab->len == len && !memcmp(start,tab->name,len) )
		{
		    AddFilePattern(tab->expand,pattern_index);
		    break;
		}
	    if (!tab->len)
	    {
		if (!strcmp(start,"negate"))
		{
		    pat->macro_negate = true;
		    pat->active_negate = pat->macro_negate != pat->user_negate;
		}
		else
		    return ERROR0(ERR_SYNTAX,
			"Macro '%.*s' not found: :%.20s\n",len,start,start);
	    }
	}
	else
	{
	    const size_t len = arg - start;
	    char * pattern = MALLOC(len+1);
	    memcpy(pattern,start,len);
	    pattern[len] = 0;
	    TRACE(" - ADD |%s|\n",pattern);
	    AppendStringField(&pat->rules,pattern,true);
	    pat->is_dirty = true;
	}
	
	while ( *arg == ';' )
	    arg++;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanRule ( ccp arg, enumPattern pattern_index )
{
    return AtFileHelper(arg,pattern_index,pattern_index,AddFilePattern) != 0;
}

///////////////////////////////////////////////////////////////////////////////

FilePattern_t * GetDFilePattern ( enumPattern pattern_index )
{
    DASSERT( (u32)pattern_index < PAT__N );
    FilePattern_t * pat = file_pattern + pattern_index;
    if (!pat->rules.used)
	pat = file_pattern + PAT_DEFAULT;
    return pat;
}

///////////////////////////////////////////////////////////////////////////////

FilePattern_t * GetDefaultFilePattern()
{
    FilePattern_t * pat = file_pattern + PAT_FILES;
    if (!pat->rules.used)
	pat = file_pattern + PAT_DEFAULT;
    return pat;
}

///////////////////////////////////////////////////////////////////////////////

void DefineNegatePattern ( FilePattern_t * pat, bool negate )
{
    DASSERT(pat);
    pat->user_negate = negate;
    pat->active_negate = pat->macro_negate != pat->user_negate;
}

///////////////////////////////////////////////////////////////////////////////

void MoveParamPattern ( FilePattern_t * dest_pat )
{
    DASSERT(dest_pat);
    FilePattern_t * src = file_pattern + PAT_PARAM;
    SetupFilePattern(src);
    memcpy( dest_pat, src, sizeof(*dest_pat) );
    InitializeFilePattern(src);
}

///////////////////////////////////////////////////////////////////////////////

bool SetupFilePattern ( FilePattern_t * pat )
{
    ASSERT(pat);
    if (pat->is_dirty)
    {
	pat->is_active	= true;
	pat->is_dirty	= false;
	pat->match_all	= false;
	pat->match_none	= false;

	if (!pat->rules.used)
	    pat->match_all = true;
	else
	{
	    ccp first = *pat->rules.field;
	    ASSERT(first);
	    if (   !strcmp(first,"+")
		|| !strcmp(first,"+*")
		|| !strcmp(first,"+**") )
	    {
		pat->match_all = true;
	    }
	    else if (   !strcmp(first,"-")
		     || !strcmp(first,"-*")
		     || !strcmp(first,"-**") )
	    {
		pat->match_none = true;
	    }
	}
     #ifdef DEBUG
	TRACE("FILE PATTERN: N=%u, all=%d, none=%d\n",
		pat->rules.used, pat->match_all, pat->match_none );
	
	ccp * ptr = pat->rules.field;
	ccp * end = ptr +  pat->rules.used;
	while ( ptr < end )
	    TRACE("  |%s|\n",*ptr++);
     #endif
    }

    pat->active_negate = pat->macro_negate != pat->user_negate;
    return pat->is_active && !pat->match_none;
}

///////////////////////////////////////////////////////////////////////////////

bool MatchFilePattern
(
    FilePattern_t	* pat,		// filter rules
    ccp			text,		// text to check
    char		path_sep	// path separator character, standard is '/'
)
{
    if (!pat)
	pat = GetDefaultFilePattern(); // use default pattern if not set
    DASSERT(pat);

    if (pat->is_dirty)
	SetupFilePattern(pat);
    if (pat->match_all)
	return !pat->active_negate;
    if (pat->match_none)
	return pat->active_negate;

    bool default_result = !pat->active_negate;
    int skip = 0;

    ccp * ptr = pat->rules.field;
    ccp * end = ptr + pat->rules.used;
    while ( ptr < end )
    {
	char * pattern = (char*)(*ptr++); // non const because of strtoul()
	DASSERT(pattern);
	switch (*pattern++)
	{
	    case '-':
		if ( skip-- <= 0 && MatchPattern(pattern,text,path_sep) )
		    return pat->active_negate;
		default_result = !pat->active_negate;
		break;

	    case '+':
		if ( skip-- <= 0 && MatchPattern(pattern,text,path_sep) )
		    return !pat->active_negate;
		default_result = pat->active_negate;
		break;

	    default:
		if ( skip-- <= 0 )
		{
		    pattern--;
		    ulong num = strtoul(pattern,&pattern,10);
		    switch (*pattern++)
		    {
			case '-':
			    if (!MatchPattern(pattern,text,path_sep))
				skip = num;
			    break;

			case '+':
			    if (MatchPattern(pattern,text,path_sep))
				skip = num;
			    break;
		    }
		}
		break;
	}
    }

    return default_result;
}

///////////////////////////////////////////////////////////////////////////////

int MatchFilePatternFST
(
	struct wd_iterator_t *it	// iterator struct with all infos
)
{
    DASSERT(it);
    // result>0: ignore this file
    return it->icm >= WD_ICM_DIRECTORY
	&& !MatchFilePattern(it->param,it->fst_name,'/');
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                 MatchPattern()                  ///////////////
///////////////////////////////////////////////////////////////////////////////

static ccp AnalyseBrackets
(
    ccp		pattern,
    ccp		* p_start,
    bool	* p_negate,
    int		* p_multiple
)
{
    ASSERT(pattern);

    bool negate = false;
    if ( *pattern == '^' )
    {
	pattern++;
	negate = true;
    }
    if (p_negate)
	*p_negate = negate;

    int multiple = 0;
    if ( *pattern == '+' )
    {
	pattern++;
	multiple = 1;
    }
    else if ( *pattern == '*' )
    {
	pattern++;
	multiple = 2;
    }
    if (p_multiple)
	*p_multiple = multiple;

    if (p_start)
	*p_start = pattern;

    if (*pattern) // ']' allowed in first position
	pattern++;
    while ( *pattern && *pattern++ != ']' ) // find end
	;

    return pattern;
}

//-----------------------------------------------------------------------------

static bool MatchBracktes
(
    char	ch,
    ccp		pattern,
    bool	negate
)
{
    if (!ch)
	return false;

    bool ok = false;
    ccp p = pattern;
    for (; !ok && *p && ( p == pattern || *p != ']' ); p++ )
    {
	if ( *p == '-' )
	{
	    if ( ch <= *++p && ch >= p[-2] )
	    {
		if (negate)
		    return false;
		ok = true;
	    }
	}
	else
	{
	    if ( *p == '\\' )
		p++;

	    if ( *p == ch )
	    {
		if (negate)
		    return false;
		ok = true;
	    }
	}
    }
    return ok || negate;
}

//-----------------------------------------------------------------------------

static bool MatchPatternHelper
(
    ccp		pattern,
    ccp		text,
    bool	skip_end,
    int		alt_depth,
    char	path_sep	// path separator character, standard is '/'
)
{
    ASSERT(pattern);
    ASSERT(text);
    noTRACE(" - %d,%d |%s|%s|\n",skip_end,alt_depth,pattern,text);

    char ch;
    while ( ( ch = *pattern++ ) != 0 )
    {
	switch (ch)
	{
	   case '*':
		if ( *pattern == '*' )
		{
		    pattern++;
		    if (*pattern)
			while (!MatchPatternHelper(pattern,text,skip_end,alt_depth,path_sep))
			    if (!*text++)
				return false;
		}
		else
		{
		    while (!MatchPatternHelper(pattern,text,skip_end,alt_depth,path_sep))
			if ( *text == path_sep || !*text++ )
			    return false;
		}
		return true;

	    case '#':
	 	if ( *text < '0' || *text > '9' )
		    return false;
		while ( *text >= '0' && *text <= '9' )
			if (MatchPatternHelper(pattern,++text,skip_end,alt_depth,path_sep))
			    return true;
		return false;

	    case ' ':
		if ( *text < 1 || * text > ' ' )
		    return false;
		text++;
		break;

	    case '?':
		if ( !*text || *text == path_sep )
		    return false;
		text++;
		break;

	    case '[':
		{
		    ccp start;
		    bool negate;
		    int multiple;
		    TRACELINE;
		    pattern = AnalyseBrackets(pattern,&start,&negate,&multiple);
		    TRACELINE;

		    if ( multiple < 2 && !MatchBracktes(*text++,start,negate) )
			return false;

		    if (multiple)
		    {
			while (!MatchPatternHelper(pattern,text,skip_end,alt_depth,path_sep))
			    if (!MatchBracktes(*text++,start,negate))
				return false;
			return true;
		    }
		}
		break;

	   case '{':
		for (;;)
		{
		    if (MatchPatternHelper(pattern,text,skip_end,alt_depth+1,path_sep))
			return true;
		    // skip until next ',' || '}'
		    int skip_depth = 1;
		    while ( skip_depth > 0 )
		    {
			ch = *pattern++;
			switch(ch)
			{
			    case 0:
				return false;

			    case '\\':
				if (!*pattern)
				    return false;
				pattern++;
				break;

			    case '{':
				skip_depth++;
				break;

			    case ',':
				if ( skip_depth == 1 )
				    skip_depth--;
				break;

			    case '}':
				if (!--skip_depth)
				    return false;
				break;

			    case '[': // [[2do]] forgotten marker?
				pattern = AnalyseBrackets(pattern,0,0,0);
				break;
			}
		    }
		}

	   case ',':
		if (alt_depth)
		{
		    alt_depth--;
		    int skip_depth = 1;
		    while ( skip_depth > 0 )
		    {
			ch = *pattern++;
			switch(ch)
			{
			    case 0:
				return false;

			    case '\\':
				if (!*pattern)
				    return false;
				pattern++;
				break;

			    case '{':
				skip_depth++;
				break;

			    case '}':
				skip_depth--;
				break;

			    case '[': // [[2do]] forgotten marker?
				pattern = AnalyseBrackets(pattern,0,0,0);
				break;
			}
		    }
		}
		else if ( *text++ != ch )
		    return false;
		break;

	   case '}':
		if ( !alt_depth && *text++ != ch )
		    return false;
		break;

	   case '$':
		if ( !*pattern && !*text )
		    return true;
		if ( *text++ != ch )
		    return false;
		break;

	   case '\\':
		ch = *pattern++;
		// fall through

	   default:
		if ( *text++ != ch )
		    return false;
		break;
	}
    }
    return skip_end || *text == 0;
}

//-----------------------------------------------------------------------------

bool MatchPattern
(
    ccp		pattern,	// pattern text
    ccp		text,		// raw text
    char	path_sep	// path separator character, standard is '/'
)
{
    TRACE("MatchPattern(|%s|%s|%c|)\n",pattern,text,path_sep);
    if ( !pattern || !*pattern )
	return true;

    if (!text)
	text = "";

    const size_t plen = strlen(pattern);
    ccp last = pattern + plen - 1;
    char last_ch = *last;
    int count = 0;
    while ( last > pattern && *--last == '\\' )
	count++;
    if ( count & 1 )
	last_ch = 0; // no special char!
	
    if ( *pattern == path_sep )
    {
	pattern++;
	return MatchPatternHelper(pattern,text++,last_ch!='$',0,path_sep);
    }

    while (*text)
	if (MatchPatternHelper(pattern,text++,0,0,path_sep))
	    return true;

    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                     END                         ///////////////
///////////////////////////////////////////////////////////////////////////////
