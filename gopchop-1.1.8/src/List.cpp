/*
#
# Implements a class to hold an ordered array of Vectors
#
# $Id: List.cpp 347 2009-03-02 23:27:14Z keescook $
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
#include "List.h"
// FIXME: why are these here?
#include <stdio.h>
#include <stdlib.h>

List::List()
{
    max = num = 0;
    map = NULL;
}

void List::reset()
{
    uint32_t i;

    if (max > 0)
    {
        for (i = 0; i < num; i++)
            delete map[i];
        free(map);
        map = NULL;
        max = num = 0;
    }
}

List::~List()
{
    reset();
}

#define STEP_SIZE	4096
void List::add(Vector * vector)
{
    if (num == max)
    {
        max += STEP_SIZE;
        if (!(map = (Vector **) realloc(map, max * sizeof(Vector *))))
        {
            perror("realloc");
            exit(1);
        }
    }
    map[num++] = vector;
}

uint32_t List::getNum()
{
    return num;
}

Vector *List::vector(uint32_t id)
{
    if (id >= num)
        return NULL;
    return map[id];
}

/* vi:set ai ts=4 sw=4 expandtab: */
