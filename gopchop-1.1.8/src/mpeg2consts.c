/*
#
# MPEG2 constant string values
#
# $Id: mpeg2consts.c 283 2005-04-30 20:10:01Z keescook $
#
# Copyright (C) 2001-2003 Kees Cook
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

#include <stdio.h>
#include "mpeg2structs.h"
#include "mpeg2consts.h"

const char *aspect_str[] = {
    "forbidden",                // 0000
    "1:1",                      // 0001
    "4:3",                      // 0010
    "16:9",                     // 0011
    "2.21:1",                   // 0100
    "reserved",                 // 0101
    "reserved",                 // 0110
    "reserved",                 // 0111
    "reserved",                 // 1000
    "reserved",                 // 1001
    "reserved",                 // 1010
    "reserved",                 // 1011
    "reserved",                 // 1100
    "reserved",                 // 1101
    "reserved",                 // 1110
    "reserved",                 // 1111
};

const char *frame_str[] = {
    "forbidden",                // 0000
    "23.9",                     // 0001
    "24",                       // 0010
    "25",                       // 0011
    "29.9",                     // 0100
    "30",                       // 0101
    "50",                       // 0110
    "59.9",                     // 0111
    "60",                       // 1000
    "reserved",                 // 1001
    "reserved",                 // 1010
    "reserved",                 // 1011
    "reserved",                 // 1100
    "reserved",                 // 1101
    "reserved",                 // 1110
    "reserved",                 // 1111
};

const struct rate_fractions_t rate_frac[] = {
    { 1, 1 },        // forbidden
    { 24000, 1001 }, // 23.97
    { 24, 1 },       // 24
    { 25, 1 },       // 25
    { 30000, 1001 }, // 29.97
    { 30, 1 },       // 30
    { 50, 1 },       // 50
    { 60000, 1001 }, // 59.97
    { 60, 1 },       // 60
    { 1, 1 },        // reserved
    { 1, 1 },        // reserved
    { 1, 1 },        // reserved
    { 1, 1 },        // reserved
    { 1, 1 },        // reserved
    { 1, 1 },        // reserved
    { 1, 1 },        // reserved
};

const char *pic_str[] = {
    "forbidden",                // 000
    "I-Frame",                  // 001
    "P-Frame",                  // 010
    "B-Frame",                  // 011
    "invalid",                  // 100
    "reserved",                 // 101
    "reserved",                 // 110
    "reserved",                 // 111
};

const uint8_t seq_err_buf[4]     = { 0x0, 0x0, 0x1, PES_TYPE_sequence_error };
const uint8_t seq_end_buf[4]     = { 0x0, 0x0, 0x1, PES_TYPE_sequence_end };
const uint8_t eos_buf[4]         = { 0x0, 0x0, 0x1, PES_TYPE_program_end };
const uint8_t slice_start_buf[4] = { 0x0, 0x0, 0x1, 0xAF };

const uint8_t stuffing_byte[1] =            { 0xFF };
const uint8_t sequence_start_code[4] =      { 0x0, 0x0, 0x1,
                                              PES_TYPE_sequence_header };
const uint8_t picture_start_code[4] =       { 0x0, 0x0, 0x1,
                                              PES_TYPE_picture_start };
const uint8_t group_start_code[4] =         { 0x0, 0x0, 0x1,
                                              PES_TYPE_group_start };
const uint8_t MPEG_program_end_code[4] =    { 0x0, 0x0, 0x1,
                                              PES_TYPE_program_end };
const uint8_t pack_start_code[4] =          { 0x0, 0x0, 0x1,
                                              PES_TYPE_pack_start };
const uint8_t system_header_start_code[4] = { 0x0, 0x0, 0x1,
                                              PES_TYPE_system_header };
const uint8_t private_stream2_buf[4]      = { 0x0, 0x0, 0x1,
                                              PES_TYPE_private_stream_2 };
const uint8_t packet_start_code_prefix[3] = { 0x0, 0x0, 0x1 };


char *speed_str(unsigned int speed)
{
    static char answer[64];

    if (speed > 1000000) {
        sprintf(answer, "%0.2fMbs", (float) speed / 1000000.0);
    }
    else if (speed > 1000) {
        sprintf(answer, "%0.2fKbs", (float) speed / 1000.0);
    }
    else
        sprintf(answer, "%dbs", speed);

    return answer;
}



/* vi:set ai ts=4 sw=4 expandtab: */
