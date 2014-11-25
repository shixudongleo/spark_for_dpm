/*
#
# Header for GOP class
#
# $Id: GroupOfPictures.h 347 2009-03-02 23:27:14Z keescook $
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

#ifndef _GROUPOFPICTURES_H_
#define _GROUPOFPICTURES_H_

#include "config.h"
#include "mpeg2structs.h"
#include "Vector.h"
#include "Bounds.h"

struct sequence_info {
    uint32_t    horizontal_size;
    uint32_t    vertical_size;
    uint8_t     aspect_ratio;
    uint8_t     frame_rate;
    uint32_t    bit_rate;
    uint8_t     constrained;
};

class GroupOfPictures:public Vector
{
  public:
    GroupOfPictures(Vector * head);
    ~GroupOfPictures();

    // lists
    Bounds *getPacketBounds();
    Bounds *getPictureBounds();

    Vector *getHeader();

    // information
    char *strTimeCode();

    // dump info
    void get_sequence_info(struct sequence_info * output);
    void set_sequence_info(struct sequence_info * input);
    bool has_sequence_info();
    void get_sequence_header(sequence_header * output);
    void set_sequence_header(sequence_header * input);

  private:
    // GOP header itself
    Vector * header;

    Bounds *packets;            // high-level pack's
    Bounds *pictures;           // picture groups

    void updateTimeCode();
    uint32_t hours, mins, secs, pics;

    struct sequence_info info;  // Where to store the sequence header info
    bool contains_sequence_info; // Does this GOP have a sequence header?
    sequence_header sequence_head; // Actual packet bytes
};

#endif // _GROUPOFPICTURES_H_

/* vi:set ai ts=4 sw=4 expandtab: */
