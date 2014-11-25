/*
#
# Implements the structure-parsing routines needs to parse an MPEG2 using
# the Parser class memory-management interface.
#
# $Id: MPEG2Parser.cpp 347 2009-03-02 23:27:14Z keescook $
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

#include "MPEG2Parser.h"
#include "ElementStream.h"
#include "Picture.h"
#include "mpeg2consts.h"

#include "FirstPack.h"

// send packet info to stderr
#undef MPEG2_PARSE_TO_STDERR
// write maps to disk
#undef MPEG2_MAP
// write debugging to disk (you don't want this)
#undef MPEG2_DEBUG

#if defined(MPEG2_MAP) || defined(MPEG2_DEBUG)
# define MPEG2_DUMP
#endif

#ifdef MPEG2_DUMP
FILE *report;
#endif

#define WRITE_OUT(t, s...)	fprintf(report, t, ##s)

#ifdef MPEG2_MAP
# define DUMP_MAP(t, s...)	WRITE_OUT(t, ##s)
#else
# define DUMP_MAP(t, s...)      /* */
#endif

#ifdef MPEG2_DEBUG
# define DUMP_DEBUG(t, s...)	WRITE_OUT(t, ##s)
#else
# define DUMP_DEBUG(t, s...)    /* */
#endif

// NOTES

// I'm assuming that GOP and picture start codes are not split across
// packet boundries

// Any edits must set broken_link to "1" if the prior GOP has been
// dropped?


MPEG2Parser::MPEG2Parser():Parser()
{
    picture_list = NULL;
    packet_list = NULL;
    video_list = NULL;
    audio_list = NULL;
    GOP_list = NULL;
    ignore_endcode = false;
    first_pack = new FirstPack();   //TI!
}

MPEG2Parser::~MPEG2Parser()
{
    if (packet_list)
        delete packet_list;
    if (picture_list)
        delete picture_list;
    if (GOP_list)
        delete GOP_list;
    if (video_list)
        delete video_list;
    if (audio_list)
        delete audio_list;
}

void MPEG2Parser::set_ignore_endcode(bool setting)
{
    ignore_endcode = setting;
}

List *MPEG2Parser::getGOPs()
{
    return GOP_list;
}

List *MPEG2Parser::getPackets()
{
    return packet_list;
}

List *MPEG2Parser::getVideo()
{
    return video_list;
}

List *MPEG2Parser::getAudio()
{
    return audio_list;
}

List *MPEG2Parser::getPictures()
{
    return picture_list;
}

uint32_t MPEG2Parser::numGOPs()
{
    return GOP_list ? GOP_list->getNum() : 0;
}

#define SKIP_BYTES(b)		location+=(b)
#define SKIP_SIZEOF(v)		SKIP_BYTES(sizeof(v))

#define ADDERROR(f, s...)	\
{	\
	addError("%s @ %" OFF_T_FORMAT "(%.2f%%): ",	\
		__FUNCTION__, location,	\
	       	100.0*(float)((float)location/(float)filesize));	\
	addError(f, ##s);	\
}

#define VERIFY_FAILED(s,l)	\
{	\
	ADDERROR(_("header %s (length %d) not found\n"),	# s, (l));	\
}
#define ATTACH_FAILED(s,l)	\
{	\
	ADDERROR(_("%s (length %d) structure attach failed\n"), (s), (l));	\
}
#define	FIXME_FAIL(s)	\
{	\
	ADDERROR(_("stream has %s marker! (Need to finish parsing routine...)\n"),(s));\
}
#define CONSUME_FAILED(s, l)	\
{	\
	ADDERROR(_("expecting %s marker (length %d)\n"),	# s, (l) );	\
}

#define CheckMaskValue(byte, mask, value)       (((byte) & (mask))==(value))

void MPEG2Parser::parse_PES_packet(void)
{
    PES_packet_header_t *header;
    PES_packet_internals_t *internals_area;
    PES_packet_internals_t internals;
    PES_packet_additional_copy_info_t *additional_copy_info;
    uint8_t *PES_private_data;  // 16 bytes
    uint8_t *STD_data;          // 16 bits
    uint8_t *PTS_data;          // 40 bits
    uint8_t *DTS_data;          // 40 bits
    uint8_t *data;              // ES data


    // for ES extraction
    int type = ES_TYPE_WEIRD;
    uint8_t id = 0;

    unsigned int length;

    off_t marked_offset;
    off_t data_loc;
    off_t pes_loc;

    pes_loc = location;
    if (!(header = (PES_packet_header_t *)attach(sizeof(*header))))
    {
        ATTACH_FAILED(_("header"), sizeof(*header));
        return;
    }
    // verify start code
    if (!verify(header->start_code, 3, (uint8_t*)packet_start_code_prefix))
    {
        VERIFY_FAILED(packet_start_code_prefix, 3);
        return;
    }

    length = (header->PES_packet_length[0] << 8) +
        header->PES_packet_length[1];

#ifdef MPEG2_PARSE_TO_STDERR
    fprintf(stderr, _("found PES 0x%02X @ %llu (%d bytes)\n"),
            header->stream_id, pes_loc, length);
#endif

    switch (header->stream_id)
    {
        // "simple" length info
        case PES_TYPE_private_stream_2:
        case PES_TYPE_ECM_stream:
        case PES_TYPE_EMM_stream:
        case PES_TYPE_program_stream_directory:
        case PES_TYPE_DSMCC_stream:
        case PES_TYPE_H2221E:
        case PES_TYPE_program_stream_map:
            SKIP_BYTES(length);
            break;

            if (!(data = (uint8_t *)attach(length)))
            {
                ATTACH_FAILED(_("stream data"), length);
                return;
            }
            /*
               if (verbose)
               Report("%d byte%s of odd stream data",
               length,
               length==1?"":"s");
             */
            break;

        case PES_TYPE_padding_stream:
            SKIP_BYTES(length);
            break;

            if (!(data = (uint8_t *)attach(length)))
            {
                ATTACH_FAILED(_("padding data"), length);
                return;
            }
            /*
               if (verbose)
               Report("%d byte%s of padding",
               length,
               length==1?"":"s");
             */
            break;

        default:               // total insano flag freakies
            // Report!
            if ((header->stream_id & PES_TYPE_MASK_audio) == PES_TYPE_audio)
            {
                id = header->stream_id & ~PES_TYPE_MASK_audio;
                type = ES_TYPE_AUDIO;

                /*
                   if (verbose)
                   Report("found header for audio stream 0x%02X",id);
                 */
            }
            else if (header->stream_id == PES_TYPE_private_stream_1)
            {
                // assume private stream 1 is going to be DVD audio
                id = PES_TYPE_private_stream_1;
                type = ES_TYPE_AUDIO;

                /*
                   if (verbose)
                   Report("found header for DVD audio stream");
                 */
            }
            else if ((header->stream_id & PES_TYPE_MASK_video) ==
                     PES_TYPE_video)
            {
                id = header->stream_id & ~PES_TYPE_MASK_video;
                type = ES_TYPE_VIDEO;

                /*
                   if (verbose)
                   Report("found header for video stream 0x%02X", id);
                 */
            }
            else                // if (type==ES_TYPE_WEIRD)
            {
                ADDERROR(_("Whoa: got PES id 0x%02X.  Skipping...\n"),
                         header->stream_id);
                SKIP_BYTES(length);
                break;
            }

	    // MPEG-1 allows stuffing bytes here

	    while (forwardMatch(1, (uint8_t*)stuffing_byte))
            {
                if (!consume(1, (uint8_t*)stuffing_byte))
                {
                    CONSUME_FAILED(stuffing_byte, 1);
                    return;
                }
		length --;
	    }

            // verify marker bits

	    if (forwardBitMatch(2,0x1)) {

		// MPEG 1 STD buffer
		if (!(STD_data = attach(2)))
		{
		    ATTACH_FAILED(_("STD data"), 5);
		    return;
		}
		length -= 2;

	    }

	    if (forwardBitMatch(4,0x2)) {

		// MPEG 1 type 2 PES
		if (!(PTS_data = attach(5)))
		{
		    ATTACH_FAILED(_("PTS data"), 5);
		    return;
		}
		length -= 5;

	    } else if (forwardBitMatch(4,0x3)) {

		// MPEG 1 type 3 PES
		if (!(PTS_data = attach(5)))
		{
		    ATTACH_FAILED(_("PTS data"), 5);
		    return;
		}
		if (!(DTS_data = attach(5)))
		{
		    ATTACH_FAILED(_("DTS data"), 5);
		    return;
		}
		length -= 10;

	    } else if (forwardBitMatch(8,0x15)) {

		// MPEG 1 - no header extensions
		if (! attach(1))
		{
		    ATTACH_FAILED(_("MPEG 1 flag byte"), 1);
		    return;
		}
		length --;

	    } else if (forwardBitMatch(2,0x2)) {

		// MPEG 2 header
		if (!(internals_area = (PES_packet_internals_t*)attach(sizeof(internals))))
		{
		    ATTACH_FAILED(_("flag data"), sizeof(internals));
		    return;
		}
		// we need to keep a copy of this in case we flip 
		// across mmap boundries.  tricky!
		memcpy(&internals, internals_area, sizeof(internals));

		marked_offset = location;

		/*
		  if (debug) Report("PES_header_data_length: %d",
		  internals.PES_header_data_length);
		*/

		if (CheckMaskValue(internals.bits[1], 0xC0, 0x80))
		{
		    if (!(PTS_data = attach(5)))
		    {
			ATTACH_FAILED(_("PTS data"), 5);
			return;
		    }
		    // FIXME: verify marker bits
		    /*
		      if (debug) Report("has PTS");
		    */
		}
		if (CheckMaskValue(internals.bits[1], 0xC0, 0xC0))
		{
		    if (!(PTS_data = attach(5)))
		    {
			ATTACH_FAILED(_("PTS data"), 5);
			return;
		    }
		    // FIXME: verify marker bits
		    /*
		      if (debug) Report("has PTS");
		    */
		    if (!(DTS_data = attach(5)))
		    {
			ATTACH_FAILED(_("DTS data"), 5);
			return;
		    }
		    // FIXME: verify marker bits
		    /*
		      if (debug) Report("has DTS");
		    */
		}
		if (CheckMaskValue(internals.bits[1], 0x20, 0x20))
		{
		    FIXME_FAIL("ESCR");
		    return;
		}
		if (CheckMaskValue(internals.bits[1], 0x10, 0x10))
		{
		    FIXME_FAIL("ES rate");
		    return;
		}
		if (CheckMaskValue(internals.bits[1], 0x8, 0x8))
		{
		    FIXME_FAIL("DSM trick mode");
		    return;
		}
		if (CheckMaskValue(internals.bits[1], 0x4, 0x4))
		{
		    if (!
			(additional_copy_info =
			 (PES_packet_additional_copy_info_t*)attach(sizeof(*additional_copy_info))))
		    {
			ATTACH_FAILED(_("additional copy info"),
				      sizeof(*additional_copy_info));
			return;
		    }
		    // FIXME: verify marker bit
		    /*
		      if (debug)
		      Report("has additional copy info 0x%02X",
		      additional_copy_info->data);
		    */
		}
		if (CheckMaskValue(internals.bits[1], 0x2, 0x2))
		{
		    // isn't this only used in Transport Streams?
		    FIXME_FAIL("PES CRC");
		    return;
		}
		if (CheckMaskValue(internals.bits[1], 0x1, 0x1))
		{
		    PES_packet_extension_t *extension;

		    if (!(extension = (PES_packet_extension_t*)attach(sizeof(*extension))))
		    {
			ATTACH_FAILED(_("extension data"), sizeof(*extension));
			return;
		    }

		    if (PPE_PES_private_data_flag(extension->bits[0]))
		    {
			if (!(PES_private_data = attach(16)))
			{
			    ATTACH_FAILED(_("PES private data"), 16);
			    return;
			}
		    }
		    if (PPE_pack_header_field_flag(extension->bits[0]))
		    {
			uint8_t *pack_field_length;
			if (!(pack_field_length = attach(1)))
			{
			    ATTACH_FAILED(_("PES extension pack field length"),
					  1);
			    return;
			}
			if (!attach(*pack_field_length))
			{
			    ATTACH_FAILED(_("PES extension pack field header"),
					  *pack_field_length);
			    return;

			}
		    }
		    if (PPE_program_packet_sequence_counter_flag
			(extension->bits[0]))
		    {
			FIXME_FAIL
			    ("PES extension program packet sequence  counter");
			return;
		    }
		    if (PPE_P_STD_buffer_flag(extension->bits[0]))
		    {
			FIXME_FAIL("PES extension P-STD buffer");
                    return;
		    }
		    if (PPE_PES_extension_flag_2(extension->bits[0]))
		    {
			FIXME_FAIL("second PES extension");
			return;
		    }
		}

		DUMP_DEBUG("ES packet analysis @ %llu\n", pes_loc);

		DUMP_DEBUG("\tPES packet length: %d\n", length);

		DUMP_DEBUG("\tmarked_offset (flag start?) @ %llu\n",
			   marked_offset);

		DUMP_DEBUG("\tPES_header_data_length: %d\n",
			   internals.PES_header_data_length);

		DUMP_DEBUG("\tstart of non-flags @ %llu\n", location);

		off_t newloc = (marked_offset + internals.PES_header_data_length);
		if (location != newloc)
		{
		    DUMP_DEBUG
			("\t\twas at %llu, should be at %llu -- adjusting\n",
			 location, newloc);
		    location = newloc;
		}

		// FIXME: no more than 32 of these
		// FIXME: with above adjustment, I'll never seen stuffing bytes
		while (location - marked_offset < internals.PES_header_data_length
		       && forwardMatch(1, (uint8_t*)stuffing_byte))
		{
		    if (!consume(1, (uint8_t*)stuffing_byte))
		    {
			CONSUME_FAILED(stuffing_byte, 1);
			return;
		    }
		    DUMP_MAP("\t\tstuffing found @ %llu\n", location - 1);
		}

		DUMP_DEBUG("\tstart of non-stuffing @ %llu\n", location);

		// adjust down for extra headers (where does this 3 come from?)
		length -= (internals.PES_header_data_length + 3);

	    } else {
		ADDERROR(_("unknown marker bits (%02x)\n"), internals.bits[0]);
	    }

            data_loc = location;
            if (!(data = (uint8_t*)attach(length)))
            {
                ATTACH_FAILED(_("ES data"), length);
                return;
            }

            DUMP_DEBUG("\tdata length calculated to be %d\n", length);

            DUMP_DEBUG("\tnext location @ %llu\n", location);

            if (type != ES_TYPE_WEIRD)
            {
                ElementStream *es;
                es = new ElementStream(data_loc, length, type, id);

                if (type == ES_TYPE_VIDEO)
                {
                    video_list->add(es);

                    /*
                     * Really should do GOP processing here
                     */
                }
                else
                {
                    audio_list->add(es);
                }
            }

            break;
    }
}

void MPEG2Parser::parse_system_header(void)
{
    system_header_t *header;
    stream_id_t *stream;

#ifdef MPEG2_PARSE_TO_STDERR
    fprintf(stderr, "System @ %llu?\n", location);
#endif

    if (!(header = (system_header_t *)attach(sizeof(*header))))
    {
        ATTACH_FAILED(_("header"), sizeof(*header));
        return;
    }

    // verify start code
    if (!verify(header->start_code, 4, (uint8_t*)system_header_start_code))
    {
        VERIFY_FAILED(system_header_start_code, 4);
        return;
    }

#ifdef MPEG2_PARSE_TO_STDERR
    fprintf(stderr, "\tsystem header: 0x%02X\n", header->start_code[3]);
#endif

    
    first_pack->setSystemHeader(header);    //TI! save system header info
    
    // FIXME: verify marker bits
        
    // report!
    /*
    if (verbose) Report("found system_header");
    */
    
    uint16_t index = 0;
    
    while (forwardBitMatch(1, 0x1))
    {
        if (!(stream = (stream_id_t *)attach(sizeof(*stream))))
        {
            ATTACH_FAILED(_("stream header"), sizeof(*stream));
            return;
            }
            // verify stream id
        if ((stream->stream_id & 0xF8) < 0xB8)
        {
            ADDERROR(_("invalid stream_id 0x%02X\n"), stream->stream_id);
            return;
        }
        
        first_pack->addStreamId(stream);    //TI! save stream id info
           
        // report!
        /*
        if (verbose) Report("found stream_id 0x%02X",stream->stream_id);
        */
    }   
}

void MPEG2Parser::parse_pack_header(void)
{
    int i;
    pack_header_t *header;
    pack_header_MPEG1_t *MPEG1_header;
    int pack_stuffing_length;

    pack_start = location;

    // verify start code
    if (!consume(4, (uint8_t*)pack_start_code))
    {
        CONSUME_FAILED(pack_start_code, 4);
        return;
    }

    // verify marker bits
    if (forwardBitMatch(4,0x2)) {
        // MPEG 1
        if (!(MPEG1_header = (pack_header_MPEG1_t*)attach(sizeof(*MPEG1_header))))
        {
               ATTACH_FAILED(_("MPEG1_header"), sizeof(*MPEG1_header));
               return;
        }

        printf("Detected MPEG1 stream!  This is experimental code.\n",
               pack_start);

#ifdef MPEG2_PARSE_TO_STDERR
        fprintf(stderr, "found MPEG-1 Pack @ %llu?\n", pack_start);
#endif
    } else if (forwardBitMatch(2,0x1)) {
        // MPEG 2
        if (!(header = (pack_header_t*)attach(sizeof(*header))))
        {
               ATTACH_FAILED(_("header"), sizeof(*header));
               return;
        }

#ifdef MPEG2_PARSE_TO_STDERR
        fprintf(stderr, "found MPEG-2 Pack @ %llu?\n", pack_start);
#endif

        first_pack->setPackHeader(header);  //TI! save pack header info

        pack_stuffing_length = header->bits[9] & 0x7;

        // report!
        /*
           if (verbose)
           Report("pack_header found");
         */

        for (i = 0; i < pack_stuffing_length; i++)
        {
            if (!consume(1, (uint8_t*)stuffing_byte))
            {
                CONSUME_FAILED(stuffing_byte, 1);
                return;
            }
        }
    } 
    else {
        ADDERROR(_("Neither MPEG1 nor MPEG2 Pack header - 0x%02x!\n"),
                MPEG1_header->bits[0]);
        consume(4, (uint8_t*)pack_start_code);
    }

    if (forwardMatch(4, (uint8_t*)system_header_start_code))
        parse_system_header();
}


void MPEG2Parser::parse_pack(void)
{
    parse_pack_header();
    while (location + sizeof(PES_packet_header_t) <= filesize &&
           forwardMatch(3, (uint8_t*)packet_start_code_prefix) &&
           !forwardMatch(4, (uint8_t*)pack_start_code) &&
           (ignore_endcode || !forwardMatch(4, (uint8_t*)MPEG_program_end_code)))
        parse_PES_packet();
}

/*
 * This function steps through the stream, looking for "Pack" boundries.
 * For each pack boundry, all the PES packets within it are examined and
 * added to the overall index of packets.  We track the video and audio
 * packets by simply saying "here where the next _will_ go" (first_video),
 * before we've actually seen it.  Once we're done processing a pack, we
 * store our current video position as the 'max'.
 *
 */
void MPEG2Parser::parse_MPEG2_program_stream(void)
{
    Pack *packet;

    int first_video;
    int first_audio;
    int error_found = 0;

    char *task = _("Reading MPEG2");

    setParsingState(PARSER_STATE_PARSING);

    while ((location < filesize)
              && (ignore_endcode ||
                  !forwardMatch(4,(uint8_t*)MPEG_program_end_code)))
    {
        // callback our progress
        if (tick)
            tick(task, (float) ((float) location / (float) filesize));

        if (forwardMatch(4, (uint8_t*)pack_start_code))
        {

            first_video = video_list->getNum();
            first_audio = audio_list->getNum();

            parse_pack();

            // did we see a packet?
            if (pack_start != location)
            {
                packet = new Pack(pack_start, location - pack_start,
                                  first_video, video_list->getNum(),
                                  first_audio, audio_list->getNum(),
                                  error_found);
                packet_list->add(packet);
                error_found = 0;
            }
        }
        else
        {
            // abort if we're out of data (any code is at least 6)
            if (location + sizeof(PES_packet_header_t) >= filesize)
                break;

            // if we didn't match a pack_start_code
            // then we should see an MPEG_program_end_code
            // if not, we hit garbage
            if (!forwardMatch(4, (uint8_t*)MPEG_program_end_code))
            {
                uint8_t *garbage;
                int garbage_count;
                off_t garbage_start;
                char report[REPORT_LENGTH];

                garbage = examine(4);

                ADDERROR(_
                         ("expecting pack_start_code, got 0x%02X 0x%02X 0x%02X 0x%02X\n"),
                         garbage[0], garbage[1], garbage[2], garbage[3]);

                garbage_start = location;

                error_found = 1;

                garbage_count = 0;
                while (location < filesize &&
                       garbage_count < 102400 &&
                       !forwardMatch(4, (uint8_t*)pack_start_code))
                {
                    SKIP_SIZEOF(uint8_t);
                    garbage_count++;
                }
                snprintf(report,REPORT_LENGTH,
                         _("garbage data seen from %%%s to %%%s.\n"),
                         OFF_T_FORMAT,OFF_T_FORMAT);
                ADDERROR(report, garbage_start, location);
                if (garbage_count == 102400)
                {
                    ADDERROR(_
                             ("More than 100K of garbage -- aborting parse.\n"));
                    break;
                }
                else if (location < filesize)
                {
                    ADDERROR(_("found long-lost pack_start_code\n"));
                }
            }
        }
    }
    // They *should* have end code, but nothing ever does
    // Therefore, we'll skip this test if we're EOF
    if (location != filesize && !consume(4, (uint8_t*)MPEG_program_end_code))
    {
        CONSUME_FAILED(MPEG_program_end_code, 4);
    }

    if (tick)
        tick(task, 1.0);

    setParsingState(PARSER_STATE_FINISHED);
}

// FIXME: Assumes headers will be fully contained by single video ES packet
Vector *MPEG2Parser::findCode(Vector * area, uint8_t * code, size_t size)
{
    uint8_t *buf;
    uint8_t *head;
    uint8_t *ptr;
    size_t tosearch;
    uint8_t *header = NULL;

    tosearch = area->getLen();

    // can't search what we don't have
    if (size > tosearch)
        return NULL;

    if (!(buf = bytesAvail(area->getStart(), tosearch)))
        return NULL;
    head = buf;

    // Can't scan x-1 few bytes for an x-byte marker
    tosearch -= (size - 1);

    while (tosearch > 0)
    {
        if ((ptr = (uint8_t*)memchr(buf, 0x00, tosearch)))
        {
            if (verify(ptr, 4, code))
            {
                header = ptr;
                break;
            }
            // advance searcher and buffer head
            tosearch -= (ptr - buf) + 1;
            buf = ptr + 1;
        }
        else
            tosearch = 0;
    }


    if (header)
        return new Vector(area->getStart() + (header - head), size);

    return NULL;
}

#define GOPADDERROR(f, s...)	\
{	\
	addError("%s @ %" OFF_T_FORMAT "(%.2f%%): ",	\
		__FUNCTION__, header->getStart(),	\
	       	100.0*(float)((float)header->getStart()/(float)filesize));	\
	addError(f, ##s);	\
}

/*
 * This function examines the list of Packs and creates the GOP table.
 * Packs track video and audio packet vectors.
 * GOPs track Packs and Pictures
 * Pictures track video vectors.
 */
void MPEG2Parser::buildGOPs(void)
{
    GroupOfPictures *GOP = NULL;
    Picture *picture = NULL;

    int packet_index;
    int packet_max;

    char *task = _("Building GOP table");

    int GOP_count = 0;
    int picture_count = 0;

    int fake_GOPs = 0;
    int I_frame_count = 0;

    // verify MPEG2's internals
    if (!packet_list)
    {
        printf("%s", _("NULL packet list?!\n"));
        return;
    }
    if (!video_list)
    {
        printf("%s", _("NULL video list?!\n"));
        return;
    }
    if (!audio_list)
    {
        printf("%s", _("NULL audio list?!\n"));
        return;
    }
    if (!picture_list)
    {
        printf("%s", _("NULL picture list?!\n"));
        return;
    }
    if (!GOP_list)
    {
        printf("%s", _("NULL GOP list?!\n"));
        return;
    }

    packet_max = packet_list->getNum();
    for (packet_index = 0; packet_index < packet_max; packet_index++)
    {
        Pack *packet;
        int pack_ves_first, pack_aes_first;
        int pack_ves_index, pack_aes_index;
        int pack_ves_max, pack_aes_max;

        // update status bar
        if (tick)
            tick(task, (float) ((float) packet_index / (float) packet_max));

        if (!(packet = (Pack *) packet_list->vector(packet_index)))
        {
            printf("%s", _("NULL packet?!\n"));
            return;
        }

        if (packet->getFoundError())
        {
            ADDERROR(_("Garbage was detected at GOP %d\n"), GOP_count - 1);
        }


        DUMP_MAP("Pack: %d @ %llu-%llu\n",
                 packet_index,
                 packet->getStart(),
                 packet->getStart() + packet->getLen() - 1);

        pack_ves_first = packet->getVideoFirst();
        pack_ves_max = packet->getVideoMax();

        for (pack_ves_index = pack_ves_first;
             pack_ves_index < pack_ves_max; pack_ves_index++)
        {
            ElementStream *ves;
            Vector *header;

            if (!(ves = (ElementStream *) video_list->vector(pack_ves_index)))
            {
                printf("%s", _("NULL VES?!\n"));
                return;
            }

            DUMP_MAP("\tVES: %d @ %llu-%llu\n",
                     pack_ves_index,
                     ves->getStart(), ves->getStart() + ves->getLen() - 1);

            // first VES packet in system packet will
            // be checked for GOP... (not really "proper")
            // but is the only sane way to handle GOP editing
            // without a full stream demuxer
            if (pack_ves_index == pack_ves_first) {
                bool saw_sequence = false;
                sequence_header seq_header;
                struct sequence_info info;

                memset(&info,0,sizeof(info));

                // look for sequence header
                if ((header = findCode(ves, (uint8_t*)sequence_start_code,
                                       sizeof(sequence_header)))) {
                    uint8_t *bytes;

                    if (!(bytes = bytesAvail(header->getStart(), sizeof(sequence_header))))
                    {
                        printf("%s", _("Sequence header not available?!\n"));
                    }
                    else
                    {
                        memcpy(&seq_header,bytes,sizeof(sequence_header));
                        bytes+=4; // skip the header codes

                        // see mpegcat.c for formula details
                        info.horizontal_size = (bytes[0] << 4) |
                                                ((bytes[1] & 0xF0) >> 4);
                        info.vertical_size = ((bytes[1] & 0x0F) << 8) |
                                                bytes[2];
                        info.aspect_ratio = (bytes[3] & 0xF0) >> 4;
                        info.frame_rate = (bytes[3] & 0x0F);
                        info.bit_rate =
                            (bytes[4] << 10) | (bytes[5] << 2) |
                            ((bytes[6] & 0xC0) >> 6);
                        info.constrained = (bytes[7] & 0x04) >> 2;

                        saw_sequence = true;

                        /*
                        printf("Sequence Header (%dx%d: %s, %sfps, %s%s)\n",
                            info.horizontal_size, info.vertical_size,
                            aspect_str[info.aspect_ratio],
                            frame_str[info.frame_rate],
                            speed_str(info.bit_rate * 400),
                            info.constrained ? " constrained" : "");
                        */
                    }
                }

                // look for GOP
                if ((header = findCode(ves, (uint8_t*)group_start_code, 8))) {
                    GOP = new GroupOfPictures(header);
                    if (saw_sequence) {
                        GOP->set_sequence_info(&info);
                        GOP->set_sequence_header(&seq_header);
                    }
                    GOP_list->add(GOP);

                    GOP_count++;
                    DUMP_MAP("\t\tGOP: %d @ %llu\n", GOP_count,
                         header->getStart());

                    // we found a real GOP, so no more fake ones
                    fake_GOPs = 0;
                    I_frame_count = 0;
                }
            }

            if ((header = findCode(ves, (uint8_t*)picture_start_code, 6)))
            {
                uint8_t *area;
                int pic_type;
                int pic_time;

                area = bytesAvail(header->getStart(), header->getLen());
                pic_type = (area[5] & PICTURE_CODING_MASK) >> 3;
                pic_time = area[4];
                pic_time <<= 2;
                pic_time |= ((area[5] & 0xC0) >> 6);

                picture = new Picture(header->getStart(), header->getLen(),
                                      pic_type, pic_time);
                picture_list->add(picture);

                DUMP_MAP("\t\t\tPicture %d (%s) @ %llu\n",
                         picture_count++,
                         pic_str[pic_type], header->getStart());

                if (pic_type == PICTURE_CODING_INTRA)
                {
                    I_frame_count++;

                    if (!GOP || (fake_GOPs))
                    {
                        GOPADDERROR(_
                                    ("Whoa: no GOP seen before an I-Frame!?\n (You might want to re-multiplex your MPEG2-PS file first.)\n"));
#if 0
                        // in the situation where we don't find a
                        // GOP, we can play along and try to trick
                        // ourself into thinking this MPEG2-PS file
                        // actually has GOP headers.

                        if (!fake_GOPs)
                            ADDERROR(_
                                     ("No GOP seen before first Picture header.  Attempting to do I-frame splitting...\n"));
                        fake_GOPs = 1;

                        // FIXME: build a better GOP header???!

                        GOP = new GroupOfPictures(header);
                        GOP_list->add(GOP);

                        GOP_count++;
                        DUMP_MAP("\t\tPretend GOP: %d @ %llu\n", GOP_count,
                                 header->getStart());
#endif
                    }

                    if (I_frame_count > 1)
                    {
                        GOPADDERROR(_
                                    ("Whoa: I-Frame number %d seen in the same GOP!?\n (You might want to re-multiplex your MPEG2-PS file first.)\n"),
                                    I_frame_count);

                    }
                }

                if (GOP)
                    GOP->getPictureBounds()->extendTo(picture_list->getNum());
            }

            if (picture)
                picture->getVideoBounds()->extendTo(pack_ves_index + 1);


        }

        // Don't forget trailing packets...
        if (picture)
            picture->getVideoBounds()->extendTo(pack_ves_index);

        // handle imbedded audio packets too!
        pack_aes_first = packet->getAudioFirst();
        pack_aes_max = packet->getAudioMax();

        for (pack_aes_index = pack_aes_first;
             pack_aes_index < pack_aes_max; pack_aes_index++)
        {
            ElementStream *aes;
            Vector *header;

            if (!(aes = (ElementStream *) audio_list->vector(pack_aes_index)))
            {
                printf("%s", _("NULL AES?!\n"));
                return;
            }

            DUMP_MAP("\tAES: %d @ %llu-%llu\n",
                     pack_aes_index,
                     aes->getStart(), aes->getStart() + aes->getLen() - 1);

        }

        // we want to INCLUDE this packet, so packet_index+1
        if (GOP)
            GOP->getPacketBounds()->extendTo(packet_index + 1);

    }
    // Don't forget trailing packets...
    if (GOP)
        GOP->getPacketBounds()->extendTo(packet_index);
}

int MPEG2Parser::parse(char ignore_errors)
{
    clearError();
    report_errors = !ignore_errors;

    // reset and clear lists
    if (GOP_list)
        GOP_list->reset();
    else
    {
        if (!(GOP_list = new List))
        {
            reset();
            addError("cannot allocate GOP list\n");
            return FALSE;
        }
    }
    if (packet_list)
        packet_list->reset();
    else
    {
        if (!(packet_list = new List))
        {
            reset();
            addError("cannot allocate packet list\n");
            return FALSE;
        }
    }

    if (video_list)
        video_list->reset();
    else
    {
        if (!(video_list = new List))
        {
            reset();
            addError("cannot allocate video ES list\n");
            return FALSE;
        }
    }

    if (audio_list)
        audio_list->reset();
    else
    {
        if (!(audio_list = new List))
        {
            reset();
            addError("cannot allocate audio ES list\n");
            return FALSE;
        }
    }

    if (picture_list)
        picture_list->reset();
    else
    {
        if (!(picture_list = new List))
        {
            reset();
            addError("cannot allocate Picture list\n");
            return FALSE;
        }
    }

    picture_start = 0;
    GOP_start = 0;

#ifdef MPEG2_DUMP
    report = fopen("mpeg2parsing.output", "w");
#endif

    parse_MPEG2_program_stream();

    buildGOPs();

    //printf(_("GOPs: %d\n"),numGOPs());

#ifdef MPEG2_DUMP
    fclose(report);
#endif

    if (!GOP_list || !GOP_list->getNum())
    {
        //reset();
        addError("No Groups of Pictures found!\n");
        return FALSE;
    }

    return TRUE;
}

/* TI! *
 * Return first dummy pack
 */
FirstPack *MPEG2Parser::getFirstPack(){
    return first_pack;
}
    
    

/* vi:set ai ts=4 sw=4 expandtab: */
