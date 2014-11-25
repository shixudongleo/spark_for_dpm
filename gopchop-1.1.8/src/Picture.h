/*
#
# header for Picture class
#
# $Id: Picture.h 115 2003-04-27 07:21:57Z nemies $
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

#ifndef _PICTURE_H_
#define _PICTURE_H_

#include "config.h"
#include "Bounds.h"
#include "Vector.h"

class Picture:public Vector
{
  public:
    Picture(off_t offset, size_t size, int picType, int time);
     ~Picture();

    int getType();
    int getTime();
    Bounds *getVideoBounds();
  private:
    int type;
    int time_ref;
    Bounds *videobounds;
};

#endif // _PICTURE_H_

/* vi:set ai ts=4 sw=4 expandtab: */
