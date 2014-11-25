/*
#
# Header for Vector class
#
# $Id: Vector.h 347 2009-03-02 23:27:14Z keescook $
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

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "config.h"
// FIXME: do config.h magic...
#include <sys/types.h>
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif

class Vector
{
  public:
    Vector(off_t start, size_t len);
    off_t getStart();
    size_t getLen();
  protected:
    off_t location;
    size_t length;
};

#endif // _VECTOR_H_

/* vi:set ai ts=4 sw=4 expandtab: */
