/*
#
# Defines a Vector-based (so List can list it) way to track the packet
# bounds associated with a GOP (and retain the GOP header)
#
# $Id: GroupOfPictures.cpp 347 2009-03-02 23:27:14Z keescook $
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
#include "GroupOfPictures.h"
#include <stdio.h>
#include <string.h>

GroupOfPictures::GroupOfPictures(Vector * head):Vector(0, 0)
{
    // FIXME: I'd sure like to use the Vector to store the header info...
    header = head;

    packets = new Bounds();
    pictures = new Bounds();

    // set up sequence info as "unknown"
    memset(&info,0,sizeof(info));
    contains_sequence_info = false;
    memset(&sequence_head,0,sizeof(sequence_head));
}

GroupOfPictures::~GroupOfPictures()
{
    delete header;
    delete packets;
    delete pictures;
}

Bounds *GroupOfPictures::getPacketBounds()
{
    return packets;
}

Bounds *GroupOfPictures::getPictureBounds()
{
    return pictures;
}

Vector *GroupOfPictures::getHeader()
{
    return header;
}

bool GroupOfPictures::has_sequence_info()
{
    return contains_sequence_info;
}

void GroupOfPictures::get_sequence_info(struct sequence_info * output)
{
    if (!output) return;

    *output=info;
}

void GroupOfPictures::set_sequence_info(struct sequence_info * input)
{
    if (!input) return;

    info=*input;
    contains_sequence_info = true;
}

void GroupOfPictures::get_sequence_header(sequence_header * output)
{
    if (!output) return;

    *output=sequence_head;
}

void GroupOfPictures::set_sequence_header(sequence_header * input)
{
    if (!input) return;

    sequence_head=*input;
}

char *GroupOfPictures::strTimeCode()
{
    static char buf[32];

    snprintf(buf, 31, "%02d:%02d:%02d.%02d", hours, mins, secs, pics);
    buf[31] = '\0';

    return buf;
}

void GroupOfPictures::updateTimeCode()
{
#if 0
    uint8_t *area;

    if (!(area = bytesAvail(head_vector->getStart(), 8)))
    {
        printf("%s", _("GOP header not available?!\n"));
        goto failure;
    }

    // calculate GOP information
    /*
       4         5         6        7
       |         |        |         |
       7 65432 107654 3 210765 432107 6 543210
       1 11111 111111 1 111111 111111 1 1
       d hour  min    m sec    pic    c b
       r              a               l roken
       o              r               osed
       p              k
     */
    drop = ((head[4] & 0x80) > 0);
    hour = ((head[4] & 0x7C) >> 2);
    min = ((head[4] & 0x3) << 4) | ((head[5] & 0xF0) >> 4);
    sec = ((head[5] & 0x7) << 3) | ((head[6] & 0xE0) >> 6);
    pictures = ((head[6] & 0x3F) << 1) | ((head[7] & 0x80) >> 7);
    closed = ((head[7] & 0x40) > 0);
    broken = ((head[7] & 0x20) > 0);

#endif
}

/* vi:set ai ts=4 sw=4 expandtab: */
