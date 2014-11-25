/*
#
# This is to handle writing file info about the GOP objects
#
# $Id: GOPutils.h 347 2009-03-02 23:27:14Z keescook $
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

#ifndef _GOPUTILS_H_
#define _GOPUTILS_H_

#include "MPEG2Parser.h"

#ifdef __cplusplus
extern "C" {
#endif



struct clip_list_t {
    uint32_t    GOP_first;
    uint32_t    GOP_last;
};


int  write_gop_cache(char * filename, MPEG2Parser * mpeg2parser);
void write_clip_list(char * filename, struct clip_list_t * clips);

off_t write_GOP(FILE * file, MPEG2Parser * mpeg2parser,
                uint32_t num, int continuous,
                int drop_orphaned_frames,
                int adjust_timestamps,
                uint32_t *frames_written,
                bool is_last_gop,
                bool drop_trailing_pack_with_system_header
                );

#ifdef __cplusplus
}
#endif

#endif /* _GOPUTILS_H_ */

/* vi:set ai ts=4 sw=4 expandtab: */
