/*
#
# Header for Session loading and saving
#
# $Id: sessions.h 247 2004-03-25 07:11:39Z schmidtw $
#
# Copyright (C) 2004 Weston Schmidt
# weston_schmidt@yahoo.com, http://flanders.hopto.org
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

#ifndef _SESSIONS_H_
#define _SESSIONS_H_

#include "config.h"
#include <gtk/gtk.h>
#include <glib.h>

#include <inttypes.h>

#define SESSION_LOADED_OK                0
#define SESSION_ERR_FILEREAD            -1
#define SESSION_ERR_NOT_GOPCHOP_FILE    -2
#define SESSION_ERR_CLIP_PARSE          -3

#define SESSION_SAVE_FILE_OK             0
#define SESSION_SAVE_FILE_OPEN_FAILURE  -1

int save_session( char *filename );
int load_session( char *filename );
const gchar* error_to_string( int error );

#endif /* _SESSIONS_H_ */
/* vi:set ai ts=4 sw=4 expandtab: */
