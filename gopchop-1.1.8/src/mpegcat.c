/*
#
# MPEG2Parser class testing tool
#
# $Id: mpegcat.c 255 2005-04-28 04:27:08Z keescook $
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

/*
 * buffer management inspired by mplayer
 */

#include "GOPchop.h"

#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <string.h>
#include <getopt.h>
#include "mpeg2structs.h"
#include "mpeg2consts.h"
#include "file_buffer.h"

// general stuff
int debug = 0;
int dvd_names = 0;

// what packets to process
int enter_video = 0;

// top-level packets to display
int no_pack = 0;
int no_audio = 0;
int no_video = 0;
int no_other_streams = 0;
// top-level headers to display
int no_system_headers = 0;
// top-level other things...
int no_errors = 0;

// video packet display toggles
int no_slices = 1;
int no_sequence = 0;
int no_GOP_headers = 0;
int no_picture_headers = 0;

// file position options
int walk_offsets = 1;
off_t begin_at = 0;
off_t num_bytes = 0;

static struct option long_options[] = {
    {"help", 0, 0, 'h'},
    {"debug", 0, 0, 'd'},
    {"dvd-format", 0, 0, 'D'},
    {"errors", 0, 0, 'e'},
    {"enter-video", 0, 0, 'V'},
    {"slices", 0, 0, 's'},
    {"audio", 0, 0, 'a'},
    {"video", 0, 0, 'v'},
    {"pack", 0, 0, 'p'},
    {"sequence", 0, 0, 'q'},
    {"system", 0, 0, 'y'},
    {"other", 0, 0, 'o'},
    {"just-frames", 0, 0, 'F'},
    {"GOPs", 0, 0, 'g'},
    {"frames", 0, 0, 'f'},
    {"invert", 0, 0, 'i'},
    {"scan-stream", 0, 0, 't'},
    {"begin", 0, 0, 'b'},
    {"num", 0, 0, 'n'},
    {"write", 0, 0, 'w'},
    {NULL, 0, 0, 0}
};
static char *short_options = "h?dDeVsavpqyoFgfitb:n:w:";

void usage(char *argv[])
{
    printf("Usage: %s [options ...] MPEG2-PS ...\n", argv[0]);
    printf("\n");

    printf("Processing Toggles\n");
    printf("-V/--enter-video  Process video packets (forced by -t option)\n");
    //printf("-A/--enter-audio  Process audio packets\n");

    printf("\n");
    printf("General Display Toggles\n");
    printf("-p/--pack         Toggle Pack packet start display\n");
    printf("-v/--video        Toggle video ES packet start display\n");
    printf("-a/--audio        Toggle audio ES packet start display\n");
    printf
        ("-o/--other        Toggle other stream display (only show video & audio)\n");
    printf("-y/--system       Toggle system header display\n");
    printf("-e/--errors       Toggle packet errors\n");
    printf("-i/--invert       Invert all display toggles\n");

    printf("\n");
    printf("Video Display Toggles\n");
    printf("-s/--slices       Toggle slice starts (default off)\n");
    printf("-q/--sequence     Toggle sequence header display\n");
    printf("-g/--GOPs         Toggle GOP header display\n");
    printf("-f/--frames       Toggle Picture header display\n");

    //printf("Audio Display Toggles\n");

    printf("\n");
    printf("Short Cuts\n");
    printf
        ("-F/--just-frames  Show only video frames (same as '-Vepvaoyq')\n");

    printf("\n");
    printf("File Offset Options\n");
    printf("-b/--begin=NUM    Start reading at offset NUM\n");
    printf("-n/--num=NUM      Stop after reading NUM bytes\n");
    printf
        ("-t/--scan-stream  Scan every byte of the stream instead of using PES offsets\n");
    printf
        ("                  (this can show erroneous 'start code emulation' codes)\n");
    printf("\n");
    printf("General Options\n");
    printf("-D/--dvd-format               Display DVD packet extensions\n");
    printf("-d/--debug                    Report debug info\n");
    printf
        ("-w/--write FILE,0xID[,0xSID]  Write PES with id ID (and subid SID) to FILE\n");
    printf("-h/-?/--help                  Display this help\n");

    exit(1);
}

/* utilities for dealing with timecodes */
struct time_code_t
{
    unsigned char drop;     // 0-1
    unsigned char hours;    // 0-23
    unsigned char minutes;  // 0-59
    unsigned char seconds;  // 0-59
    unsigned char pictures; // 0-59
};

/*
void time_code_to_string(struct time_code_t * time_code, char * buf, int len)
{
    if (!time_code) return;
    snprintf(buf,len,"%02d:%02d:%02d.%02d",
             time_code->hours,
             time_code->minutes,
             time_code->seconds,
             time_code->pictures);
}

void time_code_plus_pictures(struct time_code_t * time_code,
                             unsigned char fps,
                             unsigned char pictures)
{
    switch (fps)
    {
        case FPS_23_9:

#define FPS_23_9    1   // 0001 -- 24000/1001
#define FPS_24      2   // 0010 -- 24
#define FPS_25      3   // 0011 -- 25
#define FPS_29_9    4   // 0100 -- 30000/1001
#define FPS_30      5   // 0101 -- 30
#define FPS_50      6   // 0110 -- 50
#define FPS_59_9    7   // 0111 -- 60000/1001
#define FPS_60      8   // 1000 -- 60

    
}
*/

file_buf * fb=NULL;

typedef struct
{
    int id;                     // PES id
    int sid;                    // PES sub-id
    char *filename;             // copy of filename
    FILE *fp;                   // file handle to write it to
}
writer_t;

int max_pes_writers = 0;
writer_t *pes_writers = NULL;

FILE *writer_match(int id, int sid)
{
    int i;

    //fprintf(stderr,"Looking for 0x%02X:0x%02X\n",id,sid);

    for (i = 0; i < max_pes_writers; i++)
    {
        /*
           fprintf(stderr,"Trying %d: 0x%02X:0x%02X\n",i,
           pes_writers[i].id,
           pes_writers[i].sid);
         */
        if (pes_writers[i].id == id && pes_writers[i].sid == sid)
        {
            //fprintf(stderr,"  got it: %d\n",i);
            return pes_writers[i].fp;
        }
    }
    return NULL;
}

int writer_add(int id, int sid, char *filename)
{
    FILE *fp;
    int index;

    if (writer_match(id, sid))
        return -1;

    if (!(fp = fopen(filename, "w")))
    {
        perror(filename);
        return -1;
    }

    index = max_pes_writers++;
    if (!(pes_writers = (writer_t *) realloc(pes_writers,
                                             sizeof(writer_t) *
                                             max_pes_writers)))
    {
        perror("realloc");
        return -1;
    }

    pes_writers[index].id = id;
    pes_writers[index].sid = sid;
    pes_writers[index].fp = fp;
    pes_writers[index].filename = strdup(filename);

    //fprintf(stderr,"Storing 0x%02X:0x%02X to '%s'\n",id,sid,filename);

    return 0;
}

void writer_done()
{
    int i;

    for (i = 0; i < max_pes_writers; i++)
    {
        fclose(pes_writers[i].fp);
        free(pes_writers[i].filename);
    }
    free(pes_writers);
    pes_writers = NULL;
}

typedef enum
{
    PACK_NONE,
    PACK_SPECIAL,
    PACK_PES_SIMPLE,            // packet length == data length
    PACK_PES_COMPLEX,           // crazy headers need skipping
}
packet_type;

typedef enum
{
    SUBID_NONE,                 // no sub id (sid=0)
    SUBID_DATA,                 // first PES data byte
}
subid_type;


// FIXME: 
//   - add "valid scope" flags (eg some markers only valid in Video stream)
//   - add "struct size" field for auto-forwarding past each marker
typedef struct
{
    // the byte value match for the packet tags
    uint8_t code_match_lo;      // low end of the range of matches
    uint8_t code_match_hi;      // high end of the range of matches

    // what kind of PES is it?
    packet_type packet;

    // how do we find the stream sub id
    subid_type subid_method;

    // misc stuff... sorting types?
}
packet_tag_info;

packet_tag_info packet_tags[] = {
    {0x00, 0x00, PACK_SPECIAL, SUBID_NONE},     // pic start
    {0x01, 0xAF, PACK_SPECIAL, SUBID_NONE},     // video slices
    {0xB0, 0xB1, PACK_SPECIAL, SUBID_NONE},     // reserved
    {0xB2, 0xB5, PACK_SPECIAL, SUBID_NONE},     // user data, sequences
    {0xB6, 0xB6, PACK_SPECIAL, SUBID_NONE},     // reserved
    {0xB7, 0xB9, PACK_SPECIAL, SUBID_NONE},     // sequence, gop, end
    {0xBA, 0xBA, PACK_SPECIAL, SUBID_NONE},     // pack
    {0xBB, 0xBB, PACK_PES_SIMPLE, SUBID_NONE},  // system: same len as PES
    {0xBC, 0xBC, PACK_PES_SIMPLE, SUBID_NONE},  // PES: prog stream map     *
    {0xBD, 0xBD, PACK_PES_COMPLEX, SUBID_DATA}, // PES: priv 1
    {0xBE, 0xBF, PACK_PES_SIMPLE, SUBID_NONE},  // PES: padding, priv 2     *
    {0xC0, 0xDF, PACK_PES_COMPLEX, SUBID_NONE}, // PES: Audio
    {0xE0, 0xEF, PACK_PES_COMPLEX, SUBID_NONE}, // PES: Video
    {0xF0, 0xF2, PACK_PES_SIMPLE, SUBID_NONE},  // PES: ecm, emm, dsmcc     *
    {0xF3, 0xF7, PACK_PES_COMPLEX, SUBID_NONE}, // PES: iso 13522/h2221a-d
    {0xF8, 0xF8, PACK_PES_SIMPLE, SUBID_NONE},  // PES: h2221e              *
    {0xF9, 0xF9, PACK_PES_COMPLEX, SUBID_NONE}, // PES: ancillary
    {0xFA, 0xFE, PACK_PES_SIMPLE, SUBID_NONE},  // PES: reserved
    {0xFF, 0xFF, PACK_PES_SIMPLE, SUBID_NONE},  // PES: prog stream dir     *
    {0, 0, PACK_NONE, SUBID_NONE}       // end of list
};

int handle_options(int argc, char *argv[])
{
    int c;
    int option_index = 0;

    while (1)
    {
        c = getopt_long(argc, argv, short_options, long_options,
                        &option_index);
        if (c == -1)
            break;

        switch (c)
        {
            case '?':
            case 'h':
                usage(argv);
                break;

            case 'd':
                debug = !debug;
                break;

            case 'D':
                dvd_names = !dvd_names;
                break;

            case 'e':
                no_errors = !no_errors;
                break;

            case 's':
                no_slices = !no_slices;
                break;

            case 'a':
                no_audio = !no_audio;
                break;

            case 'v':
                no_video = !no_video;
                break;

            case 'p':
                no_pack = !no_pack;
                break;

            case 'q':
                no_sequence = !no_sequence;
                break;

            case 'o':
                no_other_streams = !no_other_streams;
                break;

            case 'y':
                no_system_headers = !no_system_headers;
                break;

            case 'g':
                no_GOP_headers = !no_GOP_headers;
                break;

            case 'f':
                no_picture_headers = !no_picture_headers;
                break;

            case 'F':
                no_pack = !no_pack;
                no_video = !no_video;
                no_audio = !no_audio;
                no_other_streams = !no_other_streams;
                no_system_headers = !no_system_headers;
                no_errors = !no_errors;

                no_sequence = !no_sequence;

                enter_video = !enter_video;
                break;

            case 'i':
                no_pack = !no_pack;
                no_audio = !no_audio;
                no_video = !no_video;
                no_other_streams = !no_other_streams;
                no_system_headers = !no_system_headers;

                no_slices = !no_slices;
                no_sequence = !no_sequence;
                no_GOP_headers = !no_GOP_headers;
                no_picture_headers = !no_picture_headers;
                break;

            case 'V':
                enter_video = !enter_video;
                break;

            case 't':
                walk_offsets = !walk_offsets;
                break;

            case 'n':
                num_bytes = atoll(optarg);
                break;

            case 'b':
                begin_at = atoll(optarg);
                break;

            case 'w':
                {
                    char *filename;
                    char *id_str;
                    char *sid_str;
                    int id;
                    int sid;

                    filename = strtok(optarg, ",");
                    id_str = strtok(NULL, ",");
                    sid_str = strtok(NULL, ",");

                    if (!filename || !id_str ||
                        sscanf(id_str, "%x", &id) != 1 ||
                        (sid_str && sscanf(sid_str, "%x", &sid) != 1))
                        usage(argv);
                    if (!sid_str)
                        sid = 0;

                    if (writer_add(id, sid, filename) < 0)
                        exit(1);
                    break;
                }

            default:
                printf("?? getopt returned character code 0x%x ??\n", c);
                break;
        }
    }


    return optind;
}

static char *length_format;

uint8_t packet_buffer[65536];   // max size of a double byte
uint8_t *packet_start;
uint16_t packet_size;

char *code_string(uint8_t code, off_t * position)
{
    static char answer[128];
    uint8_t bytes[32];
    int i;

    // length variables
    // HI LO
    unsigned int length;
    unsigned int header_len;
    static char length_str[64];

    // pack header variables
    // 
    //|   0   |   1   |   2   |   3   |   4   |   5   |   6   |
    // 76543210765432107654321076543210765432107654321076543210
    // xxHHHxMMMMMMMMMMMMMMMxLLLLLLLLLLLLLLLxEEEEEEEEExRRRRRRRR
    //|   7   |   8   |   9   |
    // 765432107654321076543210
    // RRRRRRRRRRRRRRxxrrrrrSSS
    unsigned long long system_clock_ref_base;   // "H"igh, "M"edium, "L"ow bits
    unsigned long system_clock_ref_ext; // "E"
    unsigned int program_mux_rate;      // "R"
    unsigned int pack_stuffing_length;  // "S"

    // sequence header variables
    //|horizontal |vertical   |asp|rat|bit rate         ||vbv_buff...
    //|   0   |   1   |   2   |   3   |   4   |   5   |   6   |
    // 76543210765432107654321076543210765432107654321076543210
    // ... |c
    //|   7   |   8   |   9   |
    // 765432107654321076543210
    int horizontal_size;
    int vertical_size;
    int aspect_ratio;
    int frame_rate;
    int bit_rate;
    int constrained;

    // picture header variables;
    //|temporal |ty|
    //|   0   |   1   |   2   |   3   |   4   |   5   |   6   |
    // 76543210765432107654321076543210765432107654321076543210
    int pic_type;
    int pic_time;

    // GOP header variables
    // |    0    |    1    |    2   |    3    |
    //  7 65432 107654 3 210765 432107 6 543210
    //  1 11111 111111 1 111111 111111 1 1
    //  d hour  min    m sec    pic    c b
    //  r              a               l roken
    //  o              r               osed
    //  p              k
    int drop;
    int hour;
    int min;
    int sec;
    int pictures;
    int closed;
    int broken;

    // PES scramble
    int scramble=0;

    // initialize "unknown" packet length
    header_len = length = 0;
    length_str[0] = '\0';

    // find the packet type
    for (i = 0; packet_tags[i].packet != PACK_NONE; i++)
    {
        if (code >= packet_tags[i].code_match_lo &&
            code <= packet_tags[i].code_match_hi)
        {
            FILE *fp;
            int sid = 0;

            switch (packet_tags[i].packet)
            {
                case PACK_SPECIAL:
                    // actually, nothing special
                    break;
                case PACK_PES_SIMPLE:
                case PACK_PES_COMPLEX:
                    if (packet_tags[i].packet == PACK_PES_COMPLEX)
                    {

                        if (buffer_look_ahead(fb, bytes, 5) != FILE_BUF_OKAY)
                            return NULL;

                        // packet_length is 0 and 1
    // PES header variables
    // |    2        |    3         |   4   |
    //  76 54 3 2 1 0 76 5 4 3 2 1 0 76543210
    //  10 scramble   pts/dts    pes_crc
    //        priority   escr      extension
    //          alignment  es_rate   header_data_length
    //            copyright  dsm_trick
    //              copy       addtl copy

                        if ((bytes[2]>>6) != 0x2) {
                            printf("PES (0x%02X) header mark != 0x2: 0x%x (is this an MPEG2-PS file?)\n",code,(bytes[2]>>6));
                        }
                        scramble=((bytes[2]>>4)&0x3);

                        // skip PES header data
                        header_len = 5 + bytes[4];

                        if (dvd_names &&
                            packet_tags[i].subid_method ==
                                SUBID_DATA &&
                            buffer_look_ahead(fb, bytes, header_len + 1) ==
                                FILE_BUF_OKAY)
                        {
                            sid = bytes[header_len];
                        }
                    }
                    else
                    {
                        if (buffer_look_ahead(fb, bytes, 2) != FILE_BUF_OKAY)
                            return NULL;
                    }

                    length = bytes[1] | (bytes[0] << 8);

                    // "2" comes from the "length" bytes
                    sprintf(length_str, length_format,
                            length, length == 1 ? "" : "s",
                            "marker", buffer_tell(fb) + length + 2);

                    // write out any possible matches
                    if ((fp = writer_match(code, sid))) {
                        if (buffer_look_ahead(fb,packet_buffer, length + 2) ==
                                FILE_BUF_OKAY) {

                            packet_start = packet_buffer;
                            if (header_len) {
                                // skip sub id byte
                                packet_start += header_len;
                                packet_size = length - header_len + 2;
                                if (dvd_names &&
                                    packet_tags[i].subid_method == SUBID_DATA) {
                                    packet_start++;
                                    packet_size--;
                                }
                            }
                            else {
                                // packet length bytes
                                packet_start += 2;
                                packet_size = length;
                            }

                            if (fwrite(packet_start, packet_size, 1, fp) < 1) {
                                perror("fwrite");
                            }
                            else {
                                //fprintf(stderr,"wrote %d\n",packet_size);
                            }
                        }
                    }

                    // get position updated for next packet location
                    if (position && walk_offsets)
                    {
                        if (enter_video &&
                            (code & PES_TYPE_MASK_video) == PES_TYPE_video)
                        {
                            break;      // skip position update
                        }
                        *position = buffer_tell(fb) + length + 2;
                    }

                    break;
                default:
                case PACK_NONE:
                    // FIXME: not possible!
                    break;
            }

            break;              // found a match
        }
    }

    switch (code)
    {
        case PES_TYPE_picture_start:
            if (no_picture_headers)
                return NULL;

            if (scramble) {
                sprintf(answer,"  Picture Start (scrambling: %d)",scramble);
                return answer;
            }

            if (buffer_look_ahead(fb,bytes, 2) == FILE_BUF_OKAY)
            {
                pic_type = (bytes[1] & PICTURE_CODING_MASK) >> 3;
                pic_time = (bytes[0] << 2) | ((bytes[1] & 0xC0) >> 6);

                sprintf(answer, "  Picture (%s) Start (%u)",
                        pic_str[pic_type], pic_time);
                return answer;
            }
            else
                return "  Picture Start (incomplete flags)";

        case PES_TYPE_user_data_start:
            if (no_other_streams)
                return NULL;

            if (scramble) {
                sprintf(answer," User Data Start (scrambling: %d)",scramble);
                return answer;
            }
            return " User Data Start";

        case PES_TYPE_sequence_header:
            if (no_sequence)
                return NULL;

            if (scramble) {
                sprintf(answer,"  Sequence Header (scrambling: %d)",scramble);
                return answer;
            }

            if (buffer_look_ahead(fb,bytes, 8) == FILE_BUF_OKAY)
            {
                horizontal_size = (bytes[0] << 4) | ((bytes[1] & 0xF0) >> 4);
                vertical_size = ((bytes[1] & 0x0F) << 8) | bytes[2];
                aspect_ratio = (bytes[3] & 0xF0) >> 4;
                frame_rate = (bytes[3] & 0x0F);
                bit_rate =
                    (bytes[4] << 10) | (bytes[5] << 2) |
                    ((bytes[6] & 0xC0) >> 6);
                constrained = (bytes[7] & 0x04) >> 2;

                sprintf(answer,
                        "  Sequence Header (%dx%d: %s, %sfps, %s%s)",
                        horizontal_size, vertical_size,
                        aspect_str[aspect_ratio],
                        frame_str[frame_rate],
                        speed_str(bit_rate * 400),
                        constrained ? " constrained" : "");
                return answer;
            }
            else
                return "  Sequence Header (incomplete flags)";

        case PES_TYPE_sequence_error:
            if (no_sequence)
                return NULL;

            if (scramble) {
                sprintf(answer,"  Sequence Error (scrambling: %d)",scramble);
                return answer;
            }
            return "  Sequence Error";

        case PES_TYPE_extension_start:
            if (no_sequence)
                return NULL;
            if (scramble) {
                sprintf(answer,"  Sequence Extension Start (scrambling: %d)",scramble);
                return answer;
            }
            return "  Sequence Extension Start";

        case PES_TYPE_sequence_end:
            if (no_sequence)
                return NULL;
            if (scramble) {
                sprintf(answer,"  Sequence End (scrambling: %d)",scramble);
                return answer;
            }
            return "  Sequence End";

        case PES_TYPE_group_start:
            if (no_GOP_headers)
                return NULL;

            if (scramble) {
                sprintf(answer,"  GOP Start (scrambling: %d)",scramble);
                return answer;
            }
            if (buffer_look_ahead(fb,bytes, 4) == FILE_BUF_OKAY)
            {
    // GOP header variables
    // |    0    |    1    |    2   |    3    |
    //  7 65432 107654 3 210765 432107 6 543210
    //  1 11111 111111 1 111111 111111 1 1
    //  d hour  min    m sec    pic    c b
    //  r              a               l roken
    //  o              r               osed
    //  p              k
                drop = ((bytes[0] & 0x80) > 0);
                hour = ((bytes[0] & 0x7C) >> 2);
                min = ((bytes[0] & 0x3) << 4) | ((bytes[1] & 0xF0) >> 4);
                sec = ((bytes[1] & 0x7) << 3) | ((bytes[2] & 0xE0) >> 5);
                pictures =
                    ((bytes[2] & 0x1F) << 1) | ((bytes[3] & 0x80) >> 7);
                closed = ((bytes[3] & 0x40) > 0);
                broken = ((bytes[3] & 0x20) > 0);

                sprintf(answer,
                        "  GOP Start (%02d:%02d:%02d.%02d%s%s%s)",
                        hour, min, sec, pictures,
                        drop ? " drop" : "",
                        closed ? " closed" : " open",
                        broken ? " broken" : "");
                return answer;
            }
            else
            {
                return "  GOP Start (incomplete flags)";
            }

        case PES_TYPE_program_end:
            if (no_other_streams)
                return NULL;
            if (scramble) {
                sprintf(answer,"Program End (scrambling: %d)",scramble);
                return answer;
            }
            return "Program End";

        case PES_TYPE_pack_start:
            if (no_pack)
                return NULL;

            if (scramble) {
                sprintf(answer,"Pack Start (scrambling: %d)",scramble);
                return answer;
            }
            if (buffer_look_ahead(fb,bytes, 10) == FILE_BUF_OKAY)
            {

                //|   0   |   1   |   2   |   3   |   4   |   5   |   6   |
                // 76543210765432107654321076543210765432107654321076543210
                // xxHHHxMMMMMMMMMMMMMMMxLLLLLLLLLLLLLLLxEEEEEEEEExRRRRRRRR
                //   333 222222222211111 111110000000000 000000000 22111111// bit
                //   210 987654321098765 432109876543210 876543210 10987654//     number
                //     -  -       -    -  -       -    -  -      -        -
                //     2  2       2    1  1       0    -  0      -        1// left
                //     7  8       0    2  3       5    3  7      1        4//      shift
                //
                //|   7   |   8   |   9   |
                // 765432107654321076543210
                // RRRRRRRRRRRRRRxxrrrrrSSS
                // 11110000000000  00000000   // bit
                // 32109876543210  43210210   //     number
                //        -     -      -  -
                //        0     -      -  0   // left
                //        6     2      3  0   //      shift

                system_clock_ref_base = (((bytes[0] & 0x38) << 27) |
                                         ((bytes[0] & 0x03) << 28) |
                                         ((bytes[1] & 0xFF) << 20) |
                                         ((bytes[2] & 0xF8) << 12) |
                                         ((bytes[2] & 0x03) << 13) |
                                         ((bytes[3] & 0xFF) << 5) |
                                         ((bytes[4] & 0xF8) >> 3));
                system_clock_ref_ext = (((bytes[4] & 0x03) << 7) |
                                        ((bytes[5] & 0xFE) >> 1));
                program_mux_rate = (((bytes[6] & 0xFF) << 14) |
                                    ((bytes[7] & 0xFF) << 6) |
                                    ((bytes[8] & 0xFC) >> 2));
                pack_stuffing_length = (bytes[9] & 0x07);

                sprintf(answer, "Pack Start (Mux Rate: %u Stuffing: %u)",
// FIXME: too many damn bits
//                                system_clock_ref_base,
//                                system_clock_ref_ext,
                        program_mux_rate, pack_stuffing_length);
                return answer;
            }
            else
            {
                return "Pack Start (incomplete flags)";
            }

        case PES_TYPE_system_header:
            if (no_system_headers)
                return NULL;

            if (scramble) {
                sprintf(answer," System Header (scrambling: %d)%s",
                        scramble,length_str);
                return answer;
            }
            sprintf(answer, " System Header%s", length_str);
            return answer;
        case PES_TYPE_program_stream_map:
            if (no_other_streams)
                return NULL;
            sprintf(answer, " Program Stream Map%s", length_str);
            return answer;

        case PES_TYPE_private_stream_1:
            if (scramble) {
                sprintf(answer," Private Stream 1 (scrambling: %d)%s",
                        scramble,length_str);
                return answer;
            }
            if (dvd_names)
            {
                char *audio_type;
                int raw_id, id;

                if (no_audio)
                    return NULL;
                // already done in the header checks above
                //buffer_look_ahead(fb,bytes,header_len+1);

                audio_type = "unknown sub stream id";
                raw_id = id = bytes[header_len];

                if ((raw_id & DVD_AUDIO_TYPE_MASK_sub) == DVD_AUDIO_TYPE_sub)
                {
                    audio_type = "SPU";
                    id = (raw_id & (DVD_AUDIO_TYPE_sub - 1));
                }
                else if ((raw_id & DVD_AUDIO_TYPE_MASK_ac3) ==
                         DVD_AUDIO_TYPE_ac3)
                {
                    audio_type = "AC3";
                    id = (raw_id & (DVD_AUDIO_TYPE_ac3 - 1));
                }
                else if ((raw_id & DVD_AUDIO_TYPE_MASK_pcm) ==
                         DVD_AUDIO_TYPE_pcm)
                {
                    audio_type = "PCM";
                    id = (raw_id & (DVD_AUDIO_TYPE_pcm - 1));
                }

                sprintf(answer, " DVD Audio 0x%02X (%s %d)%s",
                        raw_id, audio_type, id, length_str);
            }
            else
            {
                if (no_other_streams)
                    return NULL;
                sprintf(answer, " Private Stream 1%s", length_str);
            }
            return answer;

        case PES_TYPE_padding_stream:
            if (no_other_streams)
                return NULL;
            sprintf(answer, " Padding Stream%s", length_str);
            return answer;
        case PES_TYPE_private_stream_2:
            if (no_other_streams)
                return NULL;
            sprintf(answer, " %s%s",
                    dvd_names ? "DVD Navigation" : "Private Stream 2",
                    length_str);
            return answer;
        case PES_TYPE_ECM_stream:
            if (no_other_streams)
                return NULL;
            sprintf(answer, " ECM Stream%s", length_str);
            return answer;
        case PES_TYPE_EMM_stream:
            if (no_other_streams)
                return NULL;
            sprintf(answer, " EMM Stream%s", length_str);
            return answer;
        case PES_TYPE_DSMCC_stream:
            if (no_other_streams)
                return NULL;
            sprintf(answer, " DSMCC Stream%s", length_str);
            return answer;
        case PES_TYPE_ISO_13522:
            if (no_other_streams)
                return NULL;
            if (scramble) {
                sprintf(answer," ISO13522 Stream (scrambling: %d)%s",
                        scramble,length_str);
                return answer;
            }
            sprintf(answer, " ISO13522 Stream%s", length_str);
            return answer;
        case PES_TYPE_H2221A:
            if (no_other_streams)
                return NULL;
            if (scramble) {
                sprintf(answer," H222.1A (scrambling: %d)%s",
                        scramble,length_str);
                return answer;
            }
            sprintf(answer, " H222.1A%s", length_str);
            return answer;
        case PES_TYPE_H2221B:
            if (no_other_streams)
                return NULL;
            if (scramble) {
                sprintf(answer," H222.1B (scrambling: %d)%s",
                        scramble,length_str);
                return answer;
            }
            sprintf(answer, " H222.1B%s", length_str);
            return answer;
        case PES_TYPE_H2221C:
            if (no_other_streams)
                return NULL;
            if (scramble) {
                sprintf(answer," H222.1C (scrambling: %d)%s",
                        scramble,length_str);
                return answer;
            }
            sprintf(answer, " H222.1C%s", length_str);
            return answer;
        case PES_TYPE_H2221D:
            if (no_other_streams)
                return NULL;
            if (scramble) {
                sprintf(answer," H222.1D (scrambling: %d)%s",
                        scramble,length_str);
                return answer;
            }
            sprintf(answer, " H222.1D%s", length_str);
            return answer;
        case PES_TYPE_H2221E:
            if (no_other_streams)
                return NULL;
            sprintf(answer, " H222.1E Stream%s", length_str);
            return answer;
        case PES_TYPE_ancillary_stream:
            if (no_other_streams)
                return NULL;
            if (scramble) {
                sprintf(answer," Ancillary Stream (scrambling: %d)%s",
                        scramble,length_str);
                return answer;
            }
            sprintf(answer, " Ancillary Stream%s", length_str);
            return answer;
        case PES_TYPE_program_stream_directory:
            if (no_other_streams)
                return NULL;
            sprintf(answer, " Program Stream Directory%s", length_str);
            return answer;
        default:
            if (code >= 0x01 && code <= 0xAF)
            {
                if (no_slices)
                    return NULL;
                if (scramble) {
                    sprintf(answer,"  Slice Start (scrambling: %d)%s",
                            scramble,length_str);
                    return answer;
                }
                sprintf(answer, "  Slice Start%s", length_str);
                return answer;
            }
            if ((code & PES_TYPE_MASK_video) == PES_TYPE_video)
            {
                if (no_video)
                    return NULL;
                if (scramble) {
                    sprintf(answer," Video Stream 0x%02X (scrambling: %d)%s",
                            PES_TYPE_video - (code & PES_TYPE_MASK_video),
                            scramble,length_str);
                    return answer;
                }

                sprintf(answer, " Video Stream 0x%02X%s",
                        PES_TYPE_video -
                        (code & PES_TYPE_MASK_video), length_str);
                return answer;
            }
            if ((code & PES_TYPE_MASK_audio) == PES_TYPE_audio)
            {
                if (no_audio)
                    return NULL;

                if (scramble) {
                    sprintf(answer," Audio Stream 0x%02X (scrambling: %d)%s",
                            PES_TYPE_video - (code & PES_TYPE_MASK_video),
                            scramble,length_str);
                    return answer;
                }
                sprintf(answer, " Audio Stream 0x%02X%s",
                        PES_TYPE_audio -
                        (code & PES_TYPE_MASK_audio), length_str);
                return answer;
            }
            if ((code >= 0xFA && code <= 0xFE) ||
                (code >= 0xB0 && code <= 0xB1) || (code == 0xB6))
            {
                if (no_errors)
                    return NULL;

                if (scramble) {
                    sprintf(answer,"Reserved (scrambling: %d)%s",
                            scramble,length_str);
                    return answer;
                }
                sprintf(answer, "Reserved%s", length_str);
                return answer;
            }

            if (no_errors)
                return NULL;

            if (scramble) {
                sprintf(answer,"Unknown (scrambling: %d)%s",
                        scramble,length_str);
                return answer;
            }
            sprintf(answer, "Unknown%s", length_str);
            return answer;
    }
    return "IMPOSSIBLE ERROR: no match?!";
}


int main(int argc, char *argv[])
{
    int i, optind;
    uint32_t marker;
    uint8_t byte;
    char *report_format;

    optind = handle_options(argc, argv);

    if (optind >= argc)
        usage(argv);

    report_format = "%10" OFF_T_FORMAT ": 0x%02X: %s\n";
    length_format = ": %u byte%s (next %s @ %" OFF_T_FORMAT ")";

    if (debug)
    {
        int i;

        printf("Debug: on (off_t format is %%" OFF_T_FORMAT ")\n");
        printf("Starting from offset %" OFF_T_FORMAT "\n", begin_at);
        printf("Stopping at offset   %" OFF_T_FORMAT "\n",
               begin_at + num_bytes);
        for (i = 0; i < max_pes_writers; i++)
        {
            printf("Writing PES id 0x%02X:0x%02X to '%s'\n",
                   pes_writers[i].id,
                   pes_writers[i].sid, pes_writers[i].filename);
        }
        if (walk_offsets)
        {
            printf
                ("Examining packet structures for offset to next packet marker.\n");
            printf("%sEntering video packets to find sub packets.\n",
                   enter_video ? "" : "NOT ");
        }
        else
        {
            printf
                ("Blindly scanning every byte for possible 0x00 0x00 0x01 0x?? markers.\n");
            printf("(Entering video packets to find sub packets.)\n");

        }
        if (dvd_names)
        {
            printf("Using DVD packet names.\n");
        }
        else
        {
            printf("Using MPEG2-PS packet names.\n");
        }
        printf("%sDisplaying unexpected PES IDs.\n", no_errors ? "NOT " : "");
        printf("%sDisplaying Pack starts.\n", no_pack ? "NOT " : "");
        printf("%sDisplaying System Headers.\n",
               no_system_headers ? "NOT " : "");
        printf("%sDisplaying Video starts.\n", no_video ? "NOT " : "");
        printf("%sDisplaying Audio starts%s.\n",
               no_audio ? "NOT " : "",
               dvd_names ? " (both MPEG2-PS Audio and Priv 1)" : "");
        printf("%sDisplaying Other starts.\n",
               no_other_streams ? "NOT " : "");
        if (enter_video || !walk_offsets)
        {
            printf("%sDisplaying Video Slice starts.\n",
                   no_slices ? "NOT " : "");
            printf("%sDisplaying Video Sequence starts.\n",
                   no_sequence ? "NOT " : "");
            printf("%sDisplaying Video GOP Headers.\n",
                   no_GOP_headers ? "NOT " : "");
            printf("%sDisplaying Video Picture Headers.\n",
                   no_picture_headers ? "NOT " : "");

        }
        printf("\n");
    }

    for (i = optind; i < argc; i++)
    {
        int running = 1;

        if (!(fb=buffer_start(fb,argv[i],DEFAULT_FILE_BUFFER_SIZE))) {
            perror(argv[i]);
            continue;
        }

        buffer_seek(fb,begin_at);


        /*
           for (running=0;running<8;running++)
           {
           off_t loc;
           uint8_t byte;

           loc=buffer_tell();
           buffer_get_byte(&byte);
           printf("%" OFF_T_FORMAT ": %d\n",loc,byte);
           }
           buffer_seek(32);
           for (running=0;running<8;running++)
           {
           off_t loc;
           uint8_t byte;

           loc=buffer_tell();
           buffer_get_byte(&byte);
           printf("%" OFF_T_FORMAT ": %d\n",loc,byte);
           }
           exit(0);
         */


        marker = 0xFFFFFFFF;
        while (running)
        {
            off_t newpos;

            //fprintf(stderr,"%08X\n",marker);
            if ((marker & 0xFFFFFF00) == 0x100)
            {
                char *str;

                newpos = 0;
                if ((str = code_string(byte, &newpos)))
                {
                    printf(report_format, buffer_tell(fb) - 4, byte, str);
                }
                // we skipped to a new location?
                if (newpos != 0)
                {
                    //fprintf(stdout,"skipping to %" OFF_T_FORMAT "\n",newpos);
                    marker = 0xFFFFFFFF;
                    buffer_seek(fb,newpos);
                }
            }
            marker <<= 8;
            if (buffer_get_byte(fb,&byte) < 0)
            {
                printf(report_format, buffer_tell(fb), 0x00, "End of File");
                running = 0;
            }
            else
                marker |= byte;

            if (num_bytes && buffer_tell(fb) - begin_at > num_bytes)
            {
                printf(report_format, buffer_tell(fb), 0x00, "End of Report");
                running = 0;
            }
        }
    }

    writer_done();

    return 0;
}

/* vi:set ai ts=4 sw=4 expandtab: */
