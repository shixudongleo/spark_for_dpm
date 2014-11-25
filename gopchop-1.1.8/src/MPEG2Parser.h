/*
#
# header for MPEG2Parser class
#
# $Id: MPEG2Parser.h 347 2009-03-02 23:27:14Z keescook $
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

#ifndef _MPEG2PARSER_H_
#define _MPEG2PARSER_H_

#include "GOPchop.h"
// FIXME: shouldn't I use config.h macros somewhere?
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

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif

#include "Parser.h"
#include "List.h"

#include "mpeg2structs.h"

#include "Pack.h"
#include "GroupOfPictures.h"
#include "ElementStream.h"
#include "FirstPack.h"  //TI!


class MPEG2Parser:public Parser
{
  public:
    MPEG2Parser();
    virtual ~ MPEG2Parser();

    // bind to parent
    int parse(char ignore_errors = FALSE);

    // MPEG2
    List *getGOPs();
    List *getPackets();
    List *getVideo();
    List *getAudio();
    List *getPictures();
    
    FirstPack *getFirstPack();  //TI!
    
    uint32_t numGOPs();
    
    // searches a vector for a certain byte string
    Vector *findCode(Vector * area, uint8_t * code, size_t size);

    // behavior adjustment
    void set_ignore_endcode(bool setting);

  protected:
    // MPEG2
    List * GOP_list;          // all GOPs in MPEG2

    // functional parsing globals
    off_t pack_start;           // where packet started
    off_t GOP_start;            // where GOP started
    off_t picture_start;        // where picture started

    List *packet_list;          // All packets in MPEG2
    List *picture_list;         // All pictures in MPEG2
    List *video_list;           // All video ES fragments
    List *audio_list;           // All audio ES fragments

    FirstPack* first_pack;      // TI! first dummy pack
    
    // section parsers
    void parse_PES_packet(void);
    void parse_system_header(void);
    void parse_pack_header(void);
    void parse_pack(void);
    void parse_MPEG2_program_stream(void);

    // behavior settings
    bool ignore_endcode;

    void buildGOPs(void);
};

#endif // _MPEG2PARSER_H_

/* vi:set ai ts=4 sw=4 expandtab: */
