/*
#
# Routines for handling loading/saving an runtime commands file.
#
# $Id: rc.c 273 2005-04-30 03:48:09Z keescook $
#
# Copyright (C) 2003 Kees Cook
# kees@outflux.net, http://outflux.net/
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# http://www.gnu.org/copyleft/gpl.html
#
*/

#include "GOPchop.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "rc.h"

#define LINE_LENGTH    1024

/*
 * Figure out the rc file's path name.
 * Caller must free the allocated filename.
 */
char * rc_get_path(char * package)
{
    const char * rc_name = "options";
    char * path;
    char * dirname = NULL;
    char * filename = NULL;
    int length = 0;

    /* find the base path */
    if (!(path=getenv("HOME"))) {
        fprintf(stderr,_("Cannot find your 'HOME' environment variable.\n"));
        path="."; /* default to current directory */
    }

    /* create directory name */
    length=strlen(path)+strlen(package)+3; /* "/", ".", and NULL */
    if (!(dirname=(char*)malloc(length))) {
            perror("malloc");
            return NULL;
    }
    sprintf(dirname,"%s/.%s",path,package);

    /* check for the package directory */
    if (access(dirname,W_OK)) {
        if (errno==ENOENT) {
            /* doesn't exist: make it */
            if (mkdir(dirname,0700)) {
                perror(dirname);
                goto need_free;
            }
        }
        else {
            perror(dirname);
            goto need_free;
        }
    }

    length=strlen(dirname)+strlen(rc_name)+2; /* need "/" and NULL */
    if (!(filename=(char*)malloc(length))) {
        perror("malloc");
        goto need_free;
    }
    sprintf(filename,"%s/%s",dirname,rc_name);

need_free:
    free(dirname);
    return filename;
}

/*
 * Take a list of options, save them to the rc file
 * Returns 0 on success.
 */
int rc_save(char * package, rc_parse_item * items)
{
    char * filename;
    FILE * rc;
    rc_parse_item * match;
    int result = -1;

    if (!(filename = rc_get_path(package))) return result;

    /* open rc file */
    if (!(rc=fopen(filename,"w"))) {
        perror(filename);
        goto need_free;
    }

    /* write file */
    for (match=items; match && match->option; match++) {
        switch (match->vartype) {
        case RC_TYPE_STRING:
            if (fprintf(rc,"%s=%s\n",match->option,
                        *((char**)match->variable))<0)
                perror("fprintf");
            break;
        case RC_TYPE_BOOLEAN:
        case RC_TYPE_INTEGER:
            if (fprintf(rc,"%s=%d\n",match->option,*((int*)match->variable))<0)
                perror("fprintf");
            break;
        default:
            /* unknown option type */
            fprintf(stderr,
                    _("Ignoring unknown type(%d) for option '%s' in '%s'.\n"),
                    match->vartype,match->option,filename);
            break;
        }
    }

    /* close and quit */
    if (fclose(rc)) perror(filename);
    result = 0;

need_free:
    free(filename);
    return result;
}

/*
 * Load options from the rc file
 */
int rc_load(char * package, rc_parse_item * items)
{
    char * filename;
    FILE * rc;
    char line[LINE_LENGTH];
    int linenum;
    int result=-1;

    if (!(filename = rc_get_path(package))) return -1;

    /* open rc file */
    if (!(rc=fopen(filename,"r"))) {
        // if it's there but we can't read it, complain
        if (access(filename,F_OK)==0) perror(filename);
        else result=0; /* don't complain if unreadable */

        goto need_free;
    }    

    /* read file */
    linenum=0;
    while (fgets(line,LINE_LENGTH,rc)) {
        char * option;
        char * value;
        rc_parse_item * match;

        linenum++;

        /* skip white space or comments(?!) */
        if (line[0] == '#' || line[0] == '\n') {
            //printf("%d: comment\n",linenum);
            continue;
        }

        option=strtok(line,"=\n");
        value =strtok(NULL,"\n");

        if (!option || !value) {
            fprintf(stderr,_("Error parsing '%s' line %d.\n"),
                filename,linenum);
        }

        //printf("%d: option:'%s' value:'%s'\n",linenum,option,value);

        /* look up the option in the option list */
        for (match=items; match && match->option; match++)
            if (strcmp(match->option,option)==0) break;

        /* did we find a matching option */
        if (match->option) {
            if (!match->variable) {
                /* unknown option type */
                fprintf(stderr,
                        _("Ignoring missing variable for option '%s' "
                          "in '%s' line %d.\n"),
                    option,filename,linenum);
                continue; /* skip to the next rc option */
            }

            switch (match->vartype) {
            case RC_TYPE_STRING:
                /* free any previous default */
                if (*(char**)match->variable) free(*(char**)match->variable);
                *((char**)match->variable)=strdup(value);
                if (!*(char**)match->variable)
                {
                    perror("strdup");
                    continue;
                }
                break;
            case RC_TYPE_BOOLEAN:
                *((int*)match->variable)=(atoi(value)!=0);
                break;
            case RC_TYPE_INTEGER:
                *((int*)match->variable)=atoi(value);
                break;
            default:
                /* unknown option type */
                fprintf(stderr,_("Ignoring unknown type(%d) for option '%s' in '%s' line %d.\n"),
                    match->vartype,option,filename,linenum);
                break;
            }
        }
        else {
            /* unknown option name */
            fprintf(stderr,_("Ignoring unknown option '%s' in '%s' line %d.\n"),
                option,filename,linenum);
            continue; /* skip to the next rc option */
        }
    }

    /* close and quit */
    if (fclose(rc)) perror(filename);

    result=0;

need_free:
    free(filename);
    return result;
}

/* vi:set ai ts=4 sw=4 expandtab: */
