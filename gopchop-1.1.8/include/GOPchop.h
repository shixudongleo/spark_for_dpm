/*
#
# Common definitions for the entire project.
#
# $Id: GOPchop.h 340 2006-10-28 07:41:47Z keescook $
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
#ifndef _GOPCHOP_H_
#define _GOPCHOP_H_

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* get the cool numbers */
#include <stdint.h>

/* import locale functions */
#include "gettext.h"

/* figure out which fseek/ftell we need */
#ifdef HAVE_FSEEKO
# define FSEEK      fseeko
# define FSEEK_NAME "fseeko"
# define FTELL      ftello
# define FTELL_NAME "ftello"
#else
# define FSEEK      fseek
# define FSEEK_NAME "fseek"
# define FTELL      ftell
# define FTELL_NAME "ftell"
#endif

/* figure out off_t formatting */
#if _FILE_OFFSET_BITS==64 || defined(__NetBSD__)
# define OFF_T_FORMAT  "llu"
#else
# define OFF_T_FORMAT  "lu"
#endif

/* define a string length used for short reports */
#define REPORT_LENGTH	128

/* standard boolean */
#ifndef  TRUE
# define TRUE 1
#endif
#ifndef  FALSE
# define FALSE 0
#endif

#endif /* _GOPCHOP_H_ */
