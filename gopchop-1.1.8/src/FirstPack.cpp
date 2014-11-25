/*
#
# FirstPack class create a dummy first pack length 2KB with a valid system header.
# A valid system header is obtain by the first system header found in the program stream.
# If no system header is found, nothing is done.
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
    
#include "FirstPack.h"    
#include "mpeg2consts.h"

FirstPack::FirstPack()
{
    memset((void *)&pack_header, 0, sizeof(pack_header));    
    is_pack_header = false;
    memset((void *)&system_header, 0, sizeof(system_header));    
    pstream_id=NULL;
    num_stream_id=0;
    is_system_header=false;
    
    index_stream_ids=0;
}


FirstPack::~FirstPack()
{
	if (pstream_id) delete []pstream_id;
}


void FirstPack::setSystemHeader(system_header_t *header)
{
    if (!is_system_header)      //first system_header found
    {
        system_header=*header;  //save system header
        
        //get the number of stream ids
        uint16_t header_length = system_header.header_length[0] * 256 + system_header.header_length[1];
        num_stream_id = (header_length - sizeof(system_header.bits)) / sizeof(stream_id_t);

        if (num_stream_id)      //there are stream_id to save: alloc memory (addStreamId() do the job)
        {
            pstream_id = new stream_id_t[num_stream_id];
        }

        is_system_header = true; //no more system headers to save
    }
}


void FirstPack::addStreamId(stream_id_t *stream)
{
    if (index_stream_ids < num_stream_id)
    {
        pstream_id[index_stream_ids++] = *stream;  // save stream ids
    }
}


void FirstPack::setPackHeader(pack_header_t *header)
{
    if (!is_pack_header)        //first pack header found
    {
        pack_header=*header;    //save pack header
 
        // force SCR to 0
        pack_header.bits[0] = 0x44;
        pack_header.bits[1] = 0x00;
        pack_header.bits[2] = 0x04;
        pack_header.bits[3] = 0x00;
        pack_header.bits[4] = 0x04;
        pack_header.bits[5] = 0x01;
        
        is_pack_header = true;  //no more pack headers to save
    }
}


/*
 * Save Dummy First Pack with a valid system header
 */
off_t FirstPack::saveFirstPack(FILE *mpeg2)
{
    off_t written_bytes = 0;

    if (is_system_header && is_pack_header)     //enough info to do the job
    {
        fwrite(&pack_header,sizeof(pack_header), 1, mpeg2);     //write pack header
        written_bytes+=sizeof(pack_header);
        
        uint16_t data_length = pack_header.bits[9] & 0x07;      //get the num of stuffing bytes and...
        written_bytes += data_length;
        while (data_length--)                                   //...fill it
        {
            fputc(0xFF, mpeg2);
        }
        
        fwrite(&system_header, sizeof(system_header), 1, mpeg2); //write system header
        written_bytes+=sizeof(system_header);
                    
        if (num_stream_id)  //write (if any) stream ids
        {
            fwrite(pstream_id, sizeof(stream_id_t) * num_stream_id, 1, mpeg2);
            written_bytes+=sizeof(stream_id_t) * num_stream_id;
        }

        /* Fill what leaves to 2 KB */
        /* with packet(s) stream id = 0xBF and with one packet length no more than 1KB */
        if ((0x800 - written_bytes) > 1024) //more than 1 KB to fill, so make 2 packets
        {
            /* first packet: fill what leaves to 1 KB */
            data_length = (0x800 - written_bytes) - 6 - 1024;
            fwrite(private_stream2_buf,sizeof(private_stream2_buf),1,mpeg2);
            fputc(data_length / 256, mpeg2);
            fputc(data_length % 256, mpeg2);
            while (data_length--)
            {
                fputc(0x00, mpeg2);
            }
            
            /* second packet: fill the remaining 1 KB */
            data_length = 1018;
            fwrite(private_stream2_buf,sizeof(private_stream2_buf),1,mpeg2);
            fputc(data_length / 256, mpeg2);
            fputc(data_length % 256, mpeg2);
            while (data_length--)
            {
                fputc(0x00, mpeg2);
            }
        }
        else //less than 1 KB, so make just 1 packet
        {
            data_length = (0x800 - written_bytes) - 6;
            fwrite(private_stream2_buf,sizeof(private_stream2_buf),1,mpeg2);
            fputc(data_length / 256, mpeg2);
            fputc(data_length % 256, mpeg2);
            while (data_length--)
            {
                fputc(0x00, mpeg2);
            }
        }
        
        written_bytes = 0x800;
        }
        return written_bytes;
}

