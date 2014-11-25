/*
#
# Header for mpeg2 constant strings
#
# $Id: mpeg2consts.h 347 2009-03-02 23:27:14Z keescook $
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

#ifndef _MPEG2CONSTS_H_
#define _MPEG2CONSTS_H_

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* bit field meanings */
extern const char *aspect_str[];
extern const char *frame_str[];
extern const char *pic_str[];

/* rate information */
struct rate_fractions_t {
    uint32_t frames;
    uint32_t seconds;
};
extern const struct rate_fractions_t rate_frac[];

char *speed_str(unsigned int speed);

/* standard mpeg2 streams */
extern const uint8_t seq_err_buf[4];     /* Sequence Error marker */
extern const uint8_t seq_end_buf[4];     /* Sequence End marker */
extern const uint8_t eos_buf[4];         /* end of MPEG2-PS stream */
extern const uint8_t slice_start_buf[4]; /* Slice Start marker */
extern const uint8_t stuffing_byte[1];
extern const uint8_t sequence_start_code[4];
extern const uint8_t picture_start_code[4];
extern const uint8_t group_start_code[4];
extern const uint8_t MPEG_program_end_code[4];
extern const uint8_t pack_start_code[4];
extern const uint8_t system_header_start_code[4];
extern const uint8_t private_stream2_buf[4];
extern const uint8_t packet_start_code_prefix[3];


#ifdef __cplusplus
}
#endif

#endif /* _MAIN_H_ */

/* vi:set ai ts=4 sw=4 expandtab: */
