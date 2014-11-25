/*
#
# header for FirstPack class
#
# Copyright (C) 2005 Tiziano Cappellari and Igor Baldachini
# t1z1an0@tele2.it
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


#ifndef _FIRSTPACK_H_
#define _FIRSTPACK_H_

#include <unistd.h>
#include <stdio.h>
#include "mpeg2structs.h"

class FirstPack
{
    public:
    
        FirstPack();
        
        ~FirstPack();
        
        void setPackHeader(pack_header_t *pack_header);
        
        void setSystemHeader(system_header_t *system_header);
        
        void addStreamId(stream_id_t *pstream_id);
        
        off_t saveFirstPack(FILE* mpeg2);
        
    private:
    
        pack_header_t pack_header;
        bool is_pack_header;
        system_header_t system_header;
        stream_id_t *pstream_id;    
        uint16_t num_stream_id;    
        bool is_system_header;
        
        int index_stream_ids;
      
};

#endif // _FIRSTPACK_H_
