/*
#
# Vector-based class to define video and audio bounds for a system packet
#
# $Id: Pack.cpp 347 2009-03-02 23:27:14Z keescook $
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
#include "Pack.h"

Pack::Pack(off_t start, size_t len,
           int ves_first, int ves_max,
           int aes_first, int aes_max, int error):Vector(start, len)
{
    video_first = ves_first;
    video_max = ves_max;
    audio_first = aes_first;
    audio_max = aes_max;
    found_error = error;
}

int Pack::getVideoFirst()
{
    return video_first;
}

int Pack::getVideoMax()
{
    return video_max;
}

int Pack::getAudioFirst()
{
    return audio_first;
}

int Pack::getAudioMax()
{
    return audio_max;
}

int Pack::getFoundError()
{
    return found_error;
}


/* vi:set ai ts=4 sw=4 expandtab: */
