/*
#
# header for Parser class
#
# $Id: Parser.h 347 2009-03-02 23:27:14Z keescook $
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

#ifndef _PARSER_H_
#define _PARSER_H_

#include "GOPchop.h"
// FIXME: do config.h magic...
// open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// everything
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
// mmap
#include <sys/mman.h>
// strlen
#include <string.h>

// What state is the parser in?
#define PARSER_STATE_UNLOADED 0
#define PARSER_STATE_READY    1
#define PARSER_STATE_PARSING  2
#define PARSER_STATE_FINISHED 3

class Parser
{
  public:
    Parser(size_t window = 10485760);
    virtual ~ Parser();
    // take a pathname and a function for loading percent
    int init(char *pathname, void (*ticker) (char *, float));
    //virtual int parse(void);

    // return collected error strings
    char *getError();

    // return mmap'd filename
    char *getFilename();

    // return file size
    off_t getSize();

    // reset all state
    void reset(void);

    // see if we have access to bytes
    uint8_t *bytesAvail(off_t location, size_t bytes);

    // has everything been parsed?
    int getParsingState();

  protected:
    char *filename;
    int fd;
    off_t filesize;

    size_t pagesize;
    size_t mmap_max;            // our max mmap memory size
    uint8_t *mmap_ptr;
    size_t mmap_len;
    off_t mmap_offset;

    off_t location;
    uint8_t *start;
    int parsing_state;

    char report_errors;

    // loading call-back
    void (*tick) (char *task, float done);

    // Error messages
    void clearError();
    void addError(const char *fmt, ...);
    char *error;

    // Parsing utilities
    int consume(size_t bytes, uint8_t * match);
    int verify(uint8_t * source, size_t bytes, uint8_t * match);
    uint8_t *examine(size_t bytes);
    uint8_t *attach(size_t bytes);
    int forwardMatch(size_t bytes, uint8_t * match);
    int forwardBitMatch(int bits, uint8_t value);

    void setParsingState(int state);
};

#endif // _PARSER_H_

/* vi:set ai ts=4 sw=4 expandtab: */
