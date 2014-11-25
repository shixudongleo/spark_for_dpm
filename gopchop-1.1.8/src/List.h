/*
#
# header for List class
#
# $Id: List.h 347 2009-03-02 23:27:14Z keescook $
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

#ifndef _LIST_H_
#define _LIST_H_

#include "GOPchop.h"
#include "Vector.h"

class List
{
  public:
    List();
    ~List();
    void add(Vector * vector);
    uint32_t getNum();
    Vector *vector(uint32_t id);
    void reset();
  protected:
    Vector ** map;
    uint32_t num;
    uint32_t max;
};

#endif // _LIST_H_

/* vi:set ai ts=4 sw=4 expandtab: */
