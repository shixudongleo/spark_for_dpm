/*
#
# This class implements a container to hole bounding information
# for use in loops like "for (i=min;i<toofar;i++)".
#
# $Id: Bounds.cpp 347 2009-03-02 23:27:14Z keescook $
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

#include "Bounds.h"
#include <stdio.h>
#include <unistd.h>

#define BOUNDS_DEBUG
#ifdef BOUNDS_DEBUG
# define REPORT(f, s...)	printf(f, ##s)
#else
# define REPORT(f, s...)        /* */
#endif

Bounds::Bounds()
{
    defined = 0;
    first = 0;
    max = 0;
}

uint32_t Bounds::getFirst()
{
    if (!defined)
    {
        printf("%s", _("Undefined first requested?!\n"));
    }
    return first;
}

uint32_t Bounds::getMax()
{
    if (!defined)
    {
        printf("%s", _("Undefined max requested?!\n"));
    }
    return max;
}

int Bounds::getDefined()
{
    return defined;
}

void Bounds::defineBounds(uint32_t start, uint32_t maxi)
{
    first = start;
    max = maxi;
    defined = 1;
}

void Bounds::extendTo(uint32_t where)
{
    if (!defined)
        defineBounds(where ? where - 1 : 0, where);

    max = where;
}

/* vi:set ai ts=4 sw=4 expandtab: */
