/*
#
# Creates an in-memory interface to a file, with largefile support.
# Uses mmap to make data available in a user-space memory window.
#
# $Id: Parser.cpp 347 2009-03-02 23:27:14Z keescook $
#
# Copyright (C) 2001-2009 Kees Cook
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
#include "Parser.h"

// FIXME: do config.h magic...
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#undef MEMORY_UPDATE
#ifdef MEMORY_UPDATE
# define MMAP_PROT	(PROT_READ|PROT_WRITE)
# define OPEN_MODE	(O_RDWR)
#else
# define MMAP_PROT	(PROT_READ)
# define OPEN_MODE	(O_RDONLY)
#endif

Parser::Parser(size_t window)
{
    // set passed-in values
    mmap_max = window;

    // set detected values
    pagesize = getpagesize();

    // set values that "reset" depends on
    filename = NULL;
    fd = -1;
    mmap_ptr = NULL;
    mmap_len = 0;
    mmap_offset = 0;
    error = NULL;

    reset();
}

void Parser::reset()
{
    if (filename)
    {
        free(filename);
        filename = NULL;
    }

    if (fd > -1)
    {
        close(fd);
        fd = -1;
    }
    filesize = 0;

    if (mmap_ptr)
    {
        if (munmap(mmap_ptr, mmap_len))
            perror("munmap");
        mmap_ptr = NULL;
        mmap_len = 0;
        mmap_offset = 0;
    }

    location = 0;
    start = NULL;
    report_errors = TRUE;
    parsing_state=PARSER_STATE_UNLOADED;

    clearError();
}

Parser::~Parser()
{
    reset();
}

int Parser::getParsingState()
{
    return parsing_state;
}

#define MIN(a,b)	((a)<(b) ? (a) : (b))
int Parser::init(char *pathname, void (*ticker) (char *, float))
{
    struct stat st;

    tick = ticker;

    // make sure we're not already running
    if (filename || fd > -1 || mmap_ptr)
    {
        reset();
        addError("%s", _("already initialized\n"));
        return FALSE;
    }

    // open file
    if ((fd = open(pathname, OPEN_MODE)) < 0)
    {
        reset();
        addError(_("cannot open '%s': %s\n"), pathname, strerror(errno));
#ifdef MEMORY_UPDATE
        addError("%s",
                 _
                 (" (write permissions are needed, but the original file won't be changed.)\n"));
#endif
        return FALSE;
    }

    // get file size
    if (fstat(fd, &st))
    {
        reset();
        addError(_("cannot get file size: %s\n"), strerror(errno));
        return FALSE;
    }

    if (!S_ISREG(st.st_mode))
    {
        reset();
        addError(_("'%s' is not a file!\n"), pathname);
        return FALSE;
    }

    filesize = st.st_size;

    mmap_len = MIN(mmap_max, filesize);

    if ((mmap_ptr = (uint8_t *) mmap(NULL, mmap_len, MMAP_PROT,
                                     MAP_PRIVATE, fd, 0)) == MAP_FAILED)
    {
        reset();
        addError("%s", _("cannot memory map file\n"));
        return FALSE;
    }

    filename = strdup(pathname);
    start = mmap_ptr;
    location = 0;

    parsing_state=PARSER_STATE_READY;

    return TRUE;
}

char *Parser::getFilename()
{
    return filename;
}

char *Parser::getError()
{
    return error;
}

off_t Parser::getSize()
{
    return filesize;
}

#define MAX(a,b)	((a) > (b) ? (a) : (b))

uint8_t *Parser::bytesAvail(off_t location, size_t bytes)
{
    off_t pagedloc;
    size_t pagedbytes;

    // make sure we're in bounds
    if (!mmap_ptr || location > filesize || location + bytes > filesize)
    {
        /*
           printf("%s",_("out of bounds\n"));
           printf(_("filesize: %llu\n"),filesize);
           printf(_("location: %llu\n"),location);
           printf(_("bytes: %d\n"),bytes);
         */
        return NULL;
    }

    // are we inbounds in current mmap region?
    if (mmap_offset <= location && location + bytes <= mmap_offset + mmap_len)
    {
        start = mmap_ptr + (location - mmap_offset);
        return start;
    }

    // we're not inbounds, let's realign on page boundry, near "location"

    // get page-aligned location
    pagedloc = location / pagesize;
    pagedloc *= pagesize;

    // figure out our expected mmap window size
    pagedbytes = MAX(mmap_max, (location - pagedloc) + bytes);
    // mmap area too large
    if (pagedbytes > mmap_max)
    {
        printf(_("mmap area too large (must increase window size!)\n"
                 "pagedbytes: %d mmap_max: %d\n"
                 "location: %llu pagedloc: %llu\n"
                 "bytes: %d\n"),
               pagedbytes, mmap_max, location, pagedloc, bytes);
        return NULL;
    }

    // unmmmap or fail
    if (munmap(mmap_ptr, mmap_len))
    {
        printf("%s", _("munmap failed\n"));
        reset();
        addError("%s", _("munmap\n"));
        return NULL;
    }

    mmap_len = pagedbytes;
    mmap_offset = pagedloc;

    // remmap or fail
    if ((mmap_ptr = (uint8_t *) mmap(NULL, mmap_len, MMAP_PROT,
                                     MAP_PRIVATE, fd,
                                     mmap_offset)) == MAP_FAILED)
    {
        printf("%s", _("mmap failed\n"));
        reset();
        addError("%s", _("cannot memory map file\n"));
        return NULL;
    }
    /*
       fprintf(stderr,_("remapped %d from %llu @ 0x%08x\n"),
       mmap_len, mmap_offset, (unsigned int)mmap_ptr);
     */

    start = mmap_ptr + (location - mmap_offset);
    return start;
}

void Parser::clearError()
{
    if (error)
    {
        free(error);
        error = NULL;
    }
}

void Parser::addError(const char *fmt, ...)
{
    char buf[1024];
    va_list vaptr;
    int len;
    int leftoff = 0;

    if (!report_errors)
        return;

    buf[0] = '\0';              // preterminate for fun

    va_start(vaptr, fmt);
    vsnprintf(buf, 1024, fmt, vaptr);
    va_end(vaptr);

    len = strlen(buf) + 1;      // one for NULL

    if (error)
    {
        leftoff = strlen(error);
        len += leftoff;
    }

    if (!(error = (char *) realloc(error, len)))
    {
        // if we can't get memory for an error, we probably
        // can't do much of anything anyway.
        perror("realloc");
        exit(1);
    }
    error[leftoff] = '\0';      // terminate our (possibly) new memory

    // this is safe now, since we used add's len to allocate it
    strcat(error, buf);

    // send out the stderr too
    fprintf(stderr, buf);
}

int Parser::consume(size_t bytes, uint8_t * match)
{
    // no bytes?  SURE!
    if (!bytes)
        return TRUE;

    if (!bytesAvail(location, bytes) || memcmp(start, match, bytes))
        return FALSE;

    start += bytes;
    location += bytes;

    return TRUE;
}

int Parser::verify(uint8_t * source, size_t bytes, uint8_t * match)
{
    // no bytes?  SURE!
    if (!bytes)
        return TRUE;

    if (memcmp(source, match, bytes) == 0)
        return TRUE;
    return FALSE;
}

// Returns "start" before incrementing by "bytes"
uint8_t *Parser::attach(size_t bytes)
{
    uint8_t *ptr;

    if ((ptr = examine(bytes)))
    {
        start += bytes;
        location += bytes;
    }

    return ptr;
}

// returns points to memory but does not increment
uint8_t *Parser::examine(size_t bytes)
{
    uint8_t *ptr = NULL;

    // no bytes? bad...
    if (bytes && bytesAvail(location, bytes))
    {
        ptr = start;
    }

    return ptr;
}

// Checks for "bytes" many bytes into the future to match "match"
int Parser::forwardMatch(size_t bytes, uint8_t * match)
{
    // no bytes?  SURE!
    if (!bytes)
        return TRUE;

    if (bytesAvail(location, bytes) && !memcmp(start, match, bytes))
    {
        return TRUE;
    }
    else
        return FALSE;
}

// Checks for up to 8 bits in the next byte, matching "value"
int Parser::forwardBitMatch(int bits, uint8_t value)
{
    uint8_t match;
    uint8_t mask;

    // 0 bits?  SURE!
    if (!bits)
        return TRUE;

    if (!bytesAvail(location, 1))
        return FALSE;

    match = value << (8 - bits);
    mask = (0xFF >> (8 - bits)) << (8 - bits);

    /*
       int result=((*start & mask) == match);
       printf(_("*start: %d mask: %d match: %d result: %d\n"),
       *start, mask, match, result);
     */

    return ((*start & mask) == match);
}

// external access to the parsing state.  This can only be set if 
// Parser is "READY" or "PARSING".  Once set to "FINISHED", it must
// be ->reset to clear it.
void Parser::setParsingState(int state)
{
    if ((parsing_state==PARSER_STATE_READY &&
         state==PARSER_STATE_PARSING) ||
        (parsing_state==PARSER_STATE_PARSING &&
         state==PARSER_STATE_FINISHED))
    {
        parsing_state=state;
    }
}


/* vi:set ai ts=4 sw=4 expandtab: */
