/*
#
# Tool for displaying SPU graphics from DVD navigation streams
#
# $Id: gtkspu.c 340 2006-10-28 07:41:47Z keescook $
#
# Copyright (C) 2002-2003 Kees Cook
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <gtk/gtk.h>
#include "file_buffer.h"

typedef struct
{
    gint red;
    gint green;
    gint blue;
}
quick_color;

quick_color clut[0xF];

/* since we don't have a valid color map, we're going to just
 * display the 4 possible colors as red, green, blue, or yellow
 */
#define COLOR_BACK      0 /* Green */
#define COLOR_PATTERN   1 /* Red */
#define COLOR_EMP1      2 /* Blue */
#define COLOR_EMP2      3 /* Yellow */
#define COLOR_NOTHING   4

gint width = 0;
gint height = 0;
guchar *rgbbuf = NULL;
guchar *backbuf = NULL;
uint8_t *spu_pixelbuf = NULL;

int clut_index[4];
int alpha[5] = { 0x0, 0x0, 0x0, 0x0, 0x0 };     // all transparent
quick_color cmap[4];

static struct option long_options[] = {
        // name, has_arg, flag, val
        {"help",    0, 0, 'h'},
        {"blind",   0, 0, 'b'},
        {"width",   1, 0, 'x'},
        {"height",  1, 0, 'y'},
        {"skip",    1, 0, 's'},
        {"debug",   0, 0, 'd'},
        {"version", 0, 0, 'V'},
        {NULL,      0, 0, 0},
};
static char *short_options = "h?bdVx:y:s:";
int opt_debug = 0;
int opt_version = 0;
int opt_blind = 0;
// This is used to size the SPU display area, in the case that it
// shouldn't be auto-sized to the spu bounding box
int opt_width = -1;
int opt_height = -1;
off_t opt_skip = 0;

void usage(char *argv[])
{
    printf("Usage: %s [OPTIONS] SPU_FILE [packet-number]\n",argv[0]);
    printf("\n");
    printf("-b,--blind     Blindly read the input file as a SPU stream\n");
    printf("-x,--width=X   Force display area width to at least X\n");
    printf("-y,--height=Y  Force display area height to at least Y\n");
    printf("-s,--skip=B    Skip B bytes before processing SPU stream\n");
    printf("-d,--debug     Display debugging information\n");
    printf("-V,--version   Display version number\n");
    printf("-h,-?,--help   Display this help\n");
    printf("\n");
    printf("To generate the SPU_FILE, you can extract the stream\n");
    printf("from a DVD VOB file with 'mpegcat -w'.  They are usually\n");
    printf("in the DVD audio streams, starting from sub-id's 0x20.\n");
    printf("Try: 'mpegcat -iDaw dump.spu,0xBD,0x20 FILE.VOB'\n");

    exit(1);
}

int handle_options(int argc, char *argv[])
{
    int c;
    int option_index = 0;

    while (1) {
        c = getopt_long(argc, argv, short_options, long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
            case '?':
            case 'h':
                usage(argv);
                break;
            case 'b':
                opt_blind = 1;
                break;
            case 'd':
                opt_debug = 1;
                break;
            case 'V':
                opt_version = 1;
                break;
            case 'x':
                opt_width = atol(optarg);
                break;
            case 'y':
                opt_height = atol(optarg);
                break;
            case 's':
                opt_skip = atol(optarg);
                break;
            default:
                printf("?? getopt return character code 0x%x ??\n", c);
                break;
        }
    }

    return optind;
}

gboolean on_darea_expose(GtkWidget * widget,
                         GdkEventExpose * event, gpointer user_data);

int yank_nibble(guchar * rledata, int *addr, int *halfbyte)
{
    int val = 0;

    if (!*halfbyte)
    {
        // take the top half
        val = (rledata[*addr] & 0xF0) >> 4;

        *halfbyte = 1;
    }
    else
    {
        // take the bottom half
        val = rledata[*addr] & 0x0F;

        *halfbyte = 0;
        *addr += 1;
    }
    return val;
}

void count_colors(uint8_t *spu, int height, int width) {
    int color_count[4];
    int x, y;

    /* clear count */
    for (x=0;x<4;x++) { color_count[x]=0; }
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            if (*spu != COLOR_NOTHING) color_count[*spu++]++;
        }
    }

    printf("Pattern   (R): %d\n",color_count[COLOR_PATTERN]);
    printf("Background(G): %d\n",color_count[COLOR_BACK]);
    printf("Emphasis1 (B): %d\n",color_count[COLOR_EMP1]);
    printf("Emphasis2 (Y): %d\n",color_count[COLOR_EMP2]);

}

#define GRAB_NIBBLE             yank_nibble(rledata,&addr,&halfbyte)
// depends on global "height" and "width"
// what I really need here is a struct for the SPU area, and just draw
// it.  I should deal with screen width/height when merging instead.
void draw_spu(uint8_t *drawbuf,
              int row_start, int row_end, int row_step,
              int colstart, int colend,
              uint8_t * rledata, int addr)
{
    int row;
    int col = colstart;

    int halfbyte = 0;
    unsigned int bits;
    unsigned int number;
    uint8_t cindex;

    for (row = row_start; row <= row_end;)
    {
        bits = GRAB_NIBBLE;
        if ((bits & 0xC) != 0x0)
        {
            // have 4-bit code
            number = (bits & 0xC) >> 2;
            cindex = (bits & 0x3);
        }
        else
        {
            bits <<= 4;
            bits |= GRAB_NIBBLE;

            if ((bits & 0xF0) != 0)
            {
                // have 8-bit code
                number = (bits & 0x3C) >> 2;
                cindex = (bits & 0x3);
            }
            else
            {
                bits <<= 4;
                bits |= GRAB_NIBBLE;

                if ((bits & 0xFC0) != 0)
                {
                    // have 12-bit code
                    number = (bits & 0xFC) >> 2;
                    cindex = (bits & 0x3);
                }
                else
                {
                    // have 16-bit code
                    bits <<= 4;
                    bits |= GRAB_NIBBLE;

                    number = (bits & 0x3FC) >> 2;
                    cindex = (bits & 0x3);

                    if (number == 0)
                        number = width; // entire row
                }
            }
        }


        // write "number" many "cindex"s until end of row
        //printf("row: %d col: %d cindex: %d\n", row,col,cindex);
        for (; number > 0 && col <= colend; number--)
        {
            drawbuf[row * width + col++] = cindex;
        }

        if (col > colend)
        {
            row += row_step;
            col = colstart;

            // fill to end of byte on row end
            if (halfbyte)
            {
                halfbyte = 0;
                addr++;
            }
        }
    }

}

#define SPU_CMD_END                     0xFF    // ends one SP_DCSQ
#define SPU_FORCED_START_DISPLAY        0x00    // no arguments
#define SPU_START_DISPLAY               0x01    // no arguments
#define SPU_STOP_DISPLAY                0x02    // no arguments

// provides four indices into the CLUT for the current PGC to associate with
// the four pixel values. One nibble per pixel value for a total of 2 bytes: 
// emphasis2 emphasis1 pattern background
#define SPU_SET_COLOR                   0x03

// directly provides the four contrast (alpha blend) values to associate with
// the four pixel values. One nibble per pixel value for a total of 2 bytes.
// 0x0 = transparent, 0xF = opaque
// e2 e1   p b 
#define SPU_SET_CONTRAST                0x04

// defines the display area, each pair (X and Y) of values is 3 bytes wide,
// for a total of 6 bytes, and has the form:
// sx sx   sx ex   ex ex   sy sy   sy ey   ey ey
// sx = starting X coordinate
// ex = ending X coordinate
// sy = starting Y coordinate
// ey = ending Y coordinate 
#define SPU_DEFINE_AREA                 0x05

// defines the pixel data addresses. First a 2-byte offset to the top field
// data, followed by a 2-byte offset to the bottom field data, for a total of
// 4 bytes.
#define SPU_DEFINE_PIXEL_ADDRESS        0x06

// allows for changing the COLor and CONtrast within one or more areas of the
// display. This command contains a series of parameters, arranged in a
// hierarchy.
// Following the command byte is a 2-byte value for the total size of the
// parameter area, including the size word.
//
// The parameter sequence begins with a LN_CTLI, which defines a vertically
// bounded area of the display. The LN_CTLI may include from one to fifteen
// PX_CTLI parameters, which define a starting horizontal position and new
// color and contrast value to apply from that column on towards the right to
// the next PX_CTLI or the right side of the display. 
//
// LN_CTLI, 4 bytes, special value of 0f ff ff ff signifies the end of the
// parameter area (this termination code MUST be present as the last parameter)
// 0 s   s s   n t   t t
// sss = csln, the starting (top-most) line number for this area
// n = number of PX_CTLI to follow
// ttt = ctln, the terminating (bottom-most) line number for this area
//
// PX_CTLI, 6 bytes, defines a starting column and new color and contrast values
// bytes 0 and 1 - starting column number
// bytes 2 and 3 - new color values, as per SET_COLOR
// bytes 4 and 5 - new contrast values, as per SET_CONTR 
#define SPU_CHANGE_COLOR_CONTRAST       0x07

struct spu_cmds_t {
    uint8_t cmd;
    int     arg_bytes;
    char *  name;
};

// -1 arg_bytes means two-byte arg length
struct spu_cmds_t spu_cmds[]={
    { SPU_FORCED_START_DISPLAY,     0,  "Forced Start Display" },
    { SPU_START_DISPLAY,            0,  "Start Display" },
    { SPU_STOP_DISPLAY,             0,  "Stop Display" },
    { SPU_SET_COLOR,                2,  "Set Color" },
    { SPU_SET_CONTRAST,             2,  "Set Contrast" },
    { SPU_DEFINE_AREA,              6,  "Define Area" },
    { SPU_DEFINE_PIXEL_ADDRESS,     4,  "Define Pixel Address" },
    { SPU_CHANGE_COLOR_CONTRAST,   -1,  "Change Color and Contrast" },
    { SPU_CMD_END,                  0,  "Command End" },
    { 0,                            0,  NULL },
};

#define ADDR_OKAY               (location<spu_len)
#define GRAB_BYTE               (buf[location])
#define GRAB_WORD(buf,loc)      (((buf)[(loc)]<<8)|(buf)[(loc)+1])
#define GRAB_HIGH_NIBBLE        ((buf[location]&0xF0)>>4)
#define GRAB_LOW_NIBBLE         ((buf[location]&0x0F))

// ran past the end of the buffer
#define SPU_ERR_OUT_OF_BOUNDS   -1
// no valid command found
#define SPU_ERR_NO_CMD          -2
// null pointer argument
#define SPU_ERR_NULL_ARG        -3

/* this expects to find a SPU command at the current location */
// the returned spu_cmd must be free'd by the caller
struct spu_cmds_t * spu_cmd_lookup(file_buf * fb,
                                   struct spu_cmds_t * spu_cmd_copy)
{
    uint8_t             cmd_info[3];
    unsigned int        cmd_len=0;
    struct spu_cmds_t * spu_cmd;

    if (!fb) return NULL;

    if (buffer_look_ahead(fb, cmd_info, 1) != FILE_BUF_OKAY) {
        if (opt_debug)
            printf("\t\tFile ends before next command\n");
        return NULL;
    }

    for (spu_cmd=spu_cmds; spu_cmd->name; spu_cmd++) {
        if (spu_cmd->cmd == cmd_info[0]) {
            if (opt_debug)
                printf("\t\tChecking command 0x%02X (%s)\n",
                       cmd_info[0], spu_cmd->name);
            break;
        }
    }
    if (!spu_cmd->name) {
        if (opt_debug)
            printf("\t\tNo such command 0x%02X\n",cmd_info[0]);
        return NULL;
    }
    if (spu_cmd->arg_bytes!=0) {
        // variable length arguments
        if (spu_cmd->arg_bytes==-1) {
            if (buffer_look_ahead(fb, cmd_info, 3) != FILE_BUF_OKAY) {
                if (opt_debug)
                    printf("\t\tFile ends before variable command length\n");
                return NULL;
            }
            cmd_len=GRAB_WORD(cmd_info,1);
        }
        else {
            cmd_len=spu_cmd->arg_bytes;
        }
    }
    if (!spu_cmd_copy) {
        if (!(spu_cmd_copy=(struct spu_cmds_t *)
                            malloc(sizeof(*spu_cmd_copy)))) {
            perror("malloc");
            exit(1);
        }
    }
    *spu_cmd_copy=*spu_cmd;
    spu_cmd_copy->arg_bytes=cmd_len;

    if (opt_debug)
        printf("\t\tCommand length 0x%04X valid\n",cmd_len);

    return spu_cmd_copy;
}


#define SPU_PACKET_VALID         0 
#define SPU_PACKET_NOT_VALID    -1
#define SPU_PACKET_EOF          -2
#define SPU_PACKET_NULL_BUFFER  -3
int spu_packet_valid(file_buf * fb, off_t file_loc, uint8_t ** buf) {
    uint8_t words[4];
    size_t  packet_len;
    size_t  dscq_table;
    size_t  dscq;
    size_t  next_dscq;
    size_t  prev_dscq;
    size_t  loc; // location within the packet

    // sanity-check the data area between the spu header and the dscq table
    int has_pixel_data=0;
    int has_pixel_cmd=0;
    int commands=0;

    if (!buf) return SPU_PACKET_NULL_BUFFER;

    if (opt_debug)
        printf("%06"OFF_T_FORMAT" ...\n",file_loc);
    if (buffer_look_ahead(fb, words, 4) != FILE_BUF_OKAY) {
        if (opt_debug)
            printf("\tFile ends within 4 bytes "
                   "(no room for SPU packet header)\n");
        return SPU_PACKET_EOF;
    }
    packet_len=GRAB_WORD(words,0);
    dscq_table=GRAB_WORD(words,2);

    // packet_len must be at least 4+4+1 long (spu header, dscq,
    // and a single "end" command
    if (packet_len<9) {
        if (opt_debug)
            printf("\tPacket length < 9 bytes\n");
        return SPU_PACKET_NOT_VALID;
    }

    // is there a gap between the header and the dscq table?
    if (dscq_table>4)
        has_pixel_data=1;

    // if the table is beyond the packet len, it's not a SPU
    if (dscq_table>=packet_len) {
        if (opt_debug)
            printf("\tdscq table can't be past end of packet\n");
        return SPU_PACKET_NOT_VALID;
    }

    if (!(*buf=(uint8_t*)realloc(*buf,packet_len))) {
        perror("realloc");
        exit(1);
    }
    // if we can't read in the remaining packet, it's not a SPU
    if (buffer_look_ahead(fb, *buf, packet_len) != FILE_BUF_OKAY) {
        if (opt_debug)
            printf("\tFile ends before end of packet\n");
        return SPU_PACKET_NOT_VALID;
    }

    // FIXME: when decoding dscq starts, each should start right after the
    // previous one
    prev_dscq=0;
    loc=dscq_table;
    for (dscq=dscq_table;
         dscq<packet_len && prev_dscq!=dscq;
         dscq=next_dscq) {
        struct spu_cmds_t spu_cmd;
        struct spu_cmds_t * spu_cmd_valid;
        unsigned int clocks;

        // notice when next == current
        prev_dscq=dscq;

        // freak out if dscq!=loc
        if (dscq != loc) {
            if (opt_debug)
                printf("\tNext dscq is at 0x%04zX instead of loc 0x%04zX\n",
                       dscq,loc);
            return SPU_PACKET_NOT_VALID;
        }

        clocks   =GRAB_WORD(*buf,dscq);
        next_dscq=GRAB_WORD(*buf,dscq+2);

        if (opt_debug)
            printf("\tDSCQ 0x%04zX (Next 0x%04zX)\n",dscq,next_dscq);

        // spu's don't go backwards
        if (next_dscq<dscq) {
            if (opt_debug)
                printf("\tNext dscq can't be before current dscq\n");
            return SPU_PACKET_NOT_VALID;
        }

        // move to cmd area
        loc+=4;

        // I can leave the command loop when I either
        // a) see an "END" command
        // b) exactly hit the end-of-packet boundry?
        // c) exactly hit the end-of-dscq boundry?
        do {
            // seek
            if (buffer_seek(fb,file_loc+loc) != FILE_BUF_OKAY) {
                if (opt_debug)
                    printf("\tFile ends before next command area\n");
                return SPU_PACKET_NOT_VALID;
            }
            // lookup & check
            if (!(spu_cmd_valid=spu_cmd_lookup(fb,&spu_cmd))) {
                if (opt_debug)
                    printf("\tNo valid SPU command not found\n");
                return SPU_PACKET_NOT_VALID;
            }
            // verify packet contains command
            loc+=spu_cmd.arg_bytes+1; // total length of command
            if (loc>packet_len) {
                if (opt_debug)
                    printf("\tCommand can't be outside current packet\n");
                return SPU_PACKET_NOT_VALID;
            }
            // handle command?
            commands++;
            if (spu_cmd.cmd==SPU_DEFINE_PIXEL_ADDRESS) {
                has_pixel_cmd=1;
            }
            // leave on "last" cmd
        } while (spu_cmd.cmd!=SPU_CMD_END);
    }

    if (prev_dscq!=dscq) {
        // end without correct end-of-dscq's
        if (opt_debug)
            printf("\tPacket finished without finishing last dscq\n");
        return SPU_PACKET_NOT_VALID;
    }

    // at least one command in a SPU packet
    if (commands<1) {
        if (opt_debug)
            printf("\tSPU packet has no commands\n");
        return SPU_PACKET_NOT_VALID;
    }
    // check pixel data space
    if (has_pixel_data != has_pixel_cmd) {
        if (opt_debug) {
            if (has_pixel_data)
                printf("\tSPU has pixel area used but no cmd addressing it\n");
            if (has_pixel_cmd)
                printf("\tSPU has pixel cmd but no area defined for use\n");
        }
        return SPU_PACKET_NOT_VALID;
    }

    // It sure LOOKS like a SPU...
    if (opt_debug)
        printf("\tLooks good, with length %zu\n",packet_len);
    return SPU_PACKET_VALID;
}

/* sub-picture unit header (spuh):
 * offset  size      description
 * 0x0:    0x2       total length of all packets in bytes
 * 0x2:    0x2       offset to first sp_dcsqt
 * 0x4:    variable  pixel data
 *
 * sub-picture display control sequence table (sp_dcsqt):
 * offset  size      description
 * 0x0:    0x2       90KHz/1024 clock units to wait until running
 * 0x2:    0x2       next sp_dcsq (or self if end)
 *
 * One way to scan for a SPU packet would be to look for
 * DWORD-aligned pairs of WORDs where WORD1 > WORD2
 * and offset+WORD2 is a valid SPU command (0-7, 0xFF).
 * Or rather, a series of valid commands.
 *
 * Additionally, you could check for 0xFF near the end?
 *
 */
int seek_to_next_valid_spu(file_buf * fb) {
    off_t        file_loc;
    uint8_t *    buf=NULL;
    uint8_t      skip_byte;
    int          valid=0;
    int          percent, prev_percent=-1;
    double       calc;
    off_t        file_size;

    if (!fb) return -1;

    file_loc = buffer_tell(fb);
    file_size = buffer_file_size(fb);
    do {
        switch (spu_packet_valid(fb,file_loc,&buf)) {
        case SPU_PACKET_VALID:
                // it's valid
                valid=1;
                break;
        case SPU_PACKET_NOT_VALID:
                // it's not valid
                break;
        default:
                // eof or other error
                goto eof;
        }

        // reset file location
        if (buffer_seek(fb,file_loc) != FILE_BUF_OKAY)
            goto eof;

        if (valid) break;

        // advance the buffer to the next byte
        if (buffer_get_byte(fb,&skip_byte) != FILE_BUF_OKAY)
            goto eof;
        file_loc = buffer_tell(fb);

        // update a little counter
        calc=(double)file_loc;
        calc/=(double)file_size;
        calc*=(double)10000;
        percent=(int)(calc);
        if (percent!=prev_percent) {
            printf("%"OFF_T_FORMAT"/%"OFF_T_FORMAT" (%0.2f%%)\r",
                   file_loc, file_size, (double)percent/(double)100);
            fflush(NULL);
        }
        prev_percent=percent;
    } while (1);

    // good packet!
    if (buf) free(buf);
    return 0;

eof:
    if (buf) free(buf);
    return -1;
}

unsigned int odd_pixel_rows;
unsigned int even_pixel_rows;
gint sx, sy, ex, ey, i;

// returns -1 if done, or offset of next sequence
int spu_run(uint8_t * buf, off_t file_offset, int spu_seq_loc, size_t spu_len)
{
    int location = spu_seq_loc;
    int my_location = spu_seq_loc;
    unsigned int stall_time;
    unsigned int next_sequence;
    unsigned int size;
    uint8_t cmd;

    // adjust our buffer & max_addr
    if (!buf) return -1;

    printf("0x%05X: ", location);
    stall_time = GRAB_WORD(buf,location);
    location += 2;
    next_sequence = GRAB_WORD(buf,location);
    printf("SEQ: Stall time: %u, Next sequence: 0x%X\n",
           stall_time, next_sequence);
    location += 2;

    for (cmd = buf[location]; ADDR_OKAY; cmd = buf[location])
    {
        printf("0x%05X: ", location);
        location++;

        switch (cmd)
        {
            case SPU_FORCED_START_DISPLAY:
                printf("Forced Start Display\n");
                break;
            case SPU_START_DISPLAY:
                printf("Start Display\n");
                break;
            case SPU_STOP_DISPLAY:
                printf("Stop Display\n");
                break;

            case SPU_SET_COLOR:
                clut_index[COLOR_EMP2] = GRAB_HIGH_NIBBLE;
                clut_index[COLOR_EMP1] = GRAB_LOW_NIBBLE;
                location++;
                clut_index[COLOR_PATTERN] = GRAB_HIGH_NIBBLE;
                clut_index[COLOR_BACK] = GRAB_LOW_NIBBLE;
                location++;
                printf
                    ("Set Color (pat(R): 0x%X back(G): 0x%X emp1(B): 0x%X emp2(Y): 0x%X)\n",
                     clut_index[COLOR_PATTERN], clut_index[COLOR_BACK],
                     clut_index[COLOR_EMP1], clut_index[COLOR_EMP2]);

                cmap[0] = clut[clut_index[0]];
                cmap[1] = clut[clut_index[1]];
                cmap[2] = clut[clut_index[2]];
                cmap[3] = clut[clut_index[3]];

                /* over-ride the colors, since we don't have a color map */
                cmap[COLOR_PATTERN].red=0xFF;
                cmap[COLOR_PATTERN].green=0x00;
                cmap[COLOR_PATTERN].blue=0x00;

                cmap[COLOR_BACK].red=0x00;
                cmap[COLOR_BACK].green=0xFF;
                cmap[COLOR_BACK].blue=0x00;

                cmap[COLOR_EMP1].red=0x00;
                cmap[COLOR_EMP1].green=0x00;
                cmap[COLOR_EMP1].blue=0xFF;

                cmap[COLOR_EMP2].red=0xFF;
                cmap[COLOR_EMP2].green=0xFF;
                cmap[COLOR_EMP2].blue=0x80;

                break;

            case SPU_SET_CONTRAST:
                alpha[COLOR_EMP2] = GRAB_HIGH_NIBBLE;
                alpha[COLOR_EMP1] = GRAB_LOW_NIBBLE;
                location++;
                alpha[COLOR_PATTERN] = GRAB_HIGH_NIBBLE;
                alpha[COLOR_BACK] = GRAB_LOW_NIBBLE;
                location++;
                printf
                    ("Set Alpha (pat(R): 0x%X back(G): 0x%X emp1(B): 0x%X emp2(Y): 0x%X)\n",
                     alpha[COLOR_PATTERN], alpha[COLOR_BACK],
                     alpha[COLOR_EMP1], alpha[COLOR_EMP2]);

                // force visible bitmap if they're all transparent
                if (alpha[COLOR_PATTERN] == 0x0 &&
                    alpha[COLOR_BACK]    == 0x0 &&
                    alpha[COLOR_EMP1]    == 0x0 &&
                    alpha[COLOR_EMP2]    == 0x0)
                {
                    printf("All transparent, forcing full visibility...\n");
                    alpha[COLOR_PATTERN] = 0xF;
                    alpha[COLOR_BACK]    = 0xF;
                    alpha[COLOR_EMP1]    = 0xF;
                    alpha[COLOR_EMP2]    = 0xF;
                }
                break;

            case SPU_DEFINE_AREA:
                sx = GRAB_BYTE;
                location++;
                sx <<= 4;
                sx |= GRAB_HIGH_NIBBLE;
                ex = GRAB_LOW_NIBBLE;
                location++;
                ex <<= 8;
                ex |= GRAB_BYTE;
                location++;

                sy = GRAB_BYTE;
                location++;
                sy <<= 4;
                sy |= GRAB_HIGH_NIBBLE;
                ey = GRAB_LOW_NIBBLE;
                location++;
                ey <<= 8;
                ey |= GRAB_BYTE;
                location++;

                printf("Define Area (%d,%d)-(%d,%d) %dx%d\n",
                       sx, sy, ex, ey, (ex-sx)+1, (ey-sy)+1);
                if (opt_width != -1 || opt_height != -1) {
                    // this version draws from the screen's top left
                    // plus any extra space created by the cmdline opts
                    width  = MAX(opt_width,ex + 1);
                    height = MAX(opt_height,ey + 1);
                }
                else {
                    // this version draws only the defined SPU area
                    width  = (ex - sx) + 1;
                    height = (ey - sy) + 1;
                }

                break;

            case SPU_DEFINE_PIXEL_ADDRESS:
                odd_pixel_rows = GRAB_WORD(buf,location);
                location += 2;
                even_pixel_rows = GRAB_WORD(buf,location);
                location += 2;
                printf("Define Pixels (odd: 0x%X even: 0x%X)\n",
                       odd_pixel_rows, even_pixel_rows);
                // FIXME: validate pixel location
                break;

            case SPU_CHANGE_COLOR_CONTRAST:
                size = GRAB_WORD(buf,location);
                location += size;
                printf("Change Color & Contrast (%d)\n", size);
                break;

            case SPU_CMD_END:
                printf("End of Commands\n");

                // handle drawing now that we got everything
                if (spu_pixelbuf)
                    free(spu_pixelbuf);
                if (!(spu_pixelbuf =
                      (uint8_t *) malloc(sizeof(*spu_pixelbuf) * width * height)))
                {
                    perror("malloc");
                    exit(1);
                }
                // clear the pixels
                memset(spu_pixelbuf, COLOR_NOTHING, width * height);

                if (opt_width != -1 || opt_height != -1) {
                    // this version draws from the screen's top left corner
                    draw_spu(spu_pixelbuf, sy,     ey, 2, sx, ex,
                             buf, odd_pixel_rows);
                    draw_spu(spu_pixelbuf, sy + 1, ey, 2, sx, ex,
                             buf, even_pixel_rows);
                }
                else {
                    // this version draws only the defined SPU area
                    draw_spu(spu_pixelbuf, 0, height-1, 2, 0, width-1,
                             buf, odd_pixel_rows);
                    draw_spu(spu_pixelbuf, 1, height-1, 2, 0, width-1,
                             buf, even_pixel_rows);
                }

                if (my_location == next_sequence)
                    return -1;

                return next_sequence;

            default:
                printf("Can't handle SPU command 0x%02X\n", cmd);
                return -1;
        }
    }
    printf("SPU packet ran past end of buffer\n");
    return -1;
}


void init_colors()
{
    int x;

    gdk_rgb_init();

    // make up a color map with shades
    for (x = 0; x < 0xF; x++)
    {
        clut[x].red = (x+1)*0x11;
        clut[x].green = (x+1)*0x11;
        clut[x].blue = (x+1)*0x11;
    }
}

// if packet is -1, do an auto-find
void prepare_packet(int packet, file_buf * fb) {
}

int main(int argc, char *argv[])
{
    GtkWidget *window, *darea;
    gint x, y;
    guchar *pos;
    guchar info[4];

    char *filename;
    file_buf * fb=NULL;
    int packet;
    int packet_wanted=0;
    off_t spu_packet_start;
    uint8_t * buf=NULL;
    int spu_seq_loc;
    size_t spu_len;


    gtk_init(&argc, &argv);
    optind = handle_options(argc,argv);

    if (opt_version) {
        printf("$Id: gtkspu.c 340 2006-10-28 07:41:47Z keescook $\n");
        exit(0);
    }

    if (argc - optind < 1)
        usage(argv);

    // start up program initialization
    init_colors();

    filename = argv[optind];
    if (argc - optind > 1)
        packet_wanted = atoi(argv[optind+1]);

    if (!(fb=buffer_start(fb,filename,DEFAULT_FILE_BUFFER_SIZE))) {
        perror(filename);
        exit(1);
    }

    spu_packet_start = opt_skip;
    if (buffer_seek(fb,spu_packet_start) != FILE_BUF_OKAY) {
            perror("seek");
            exit(1);
    }
    for (packet = 0; packet <= packet_wanted; packet++) {
        if (!opt_blind) {
            if (seek_to_next_valid_spu(fb)<0) {
                printf("No more SPU packets can be found\n");
                exit(1);
            }
        }
        spu_packet_start=buffer_tell(fb);
        if (buffer_look_ahead(fb, info, 4) == FILE_BUF_OKAY) {
            // execute every sequence in this SPU packet
            spu_len     = GRAB_WORD(info,0);
            spu_seq_loc = GRAB_WORD(info,2);
        }
        else {
            printf("No more packets in file\n");
            exit(1);
        }
                       
        if (packet == packet_wanted) {
            if (!(buf=(uint8_t *)realloc(buf,spu_len))) {
                perror("malloc");
                exit(1);
            }
            if (buffer_look_ahead(fb,buf,spu_len) != FILE_BUF_OKAY) {
                printf("Tried to read SPU packet, but read off end of file\n");
                free(buf);
                exit(1);
            }

            printf("Packet %d @ offset %"OFF_T_FORMAT" length %zu\n",
                   packet,spu_packet_start,spu_len);
            while (spu_seq_loc != -1) {
                spu_seq_loc = spu_run(buf, spu_packet_start,
                                      spu_seq_loc, spu_len);
            }
            break; // we processed the packet we wanted
        }
        else {
            if (buffer_seek(fb,spu_packet_start+spu_len) != FILE_BUF_OKAY) {
                printf("Ran off the end of file\n");
                exit(2);
            }
        }
    }
    if (buf)
        free(buf);

    if (width == 0 || height == 0)
    {
        printf("SPU didn't have enough info about window size\n");
        exit(1);
    }

    if (!(backbuf = (guchar *) malloc(sizeof(*backbuf) * 3 * width * height)))
    {
        perror("malloc");
        exit(1);
    }
    memset(backbuf, 0xFF, 3 * width * height);

    if (!(rgbbuf = (guchar *) malloc(sizeof(*rgbbuf) * 3 * width * height)))
    {
        perror("malloc");
        exit(1);
    }
    memset(rgbbuf, 0xFF, 3 * width * height);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    darea = gtk_drawing_area_new();
    gtk_drawing_area_size(GTK_DRAWING_AREA(darea), width, height);
    gtk_container_add(GTK_CONTAINER(window), darea);
    gtk_signal_connect(GTK_OBJECT(darea), "expose-event",
                       GTK_SIGNAL_FUNC(on_darea_expose), NULL);
    gtk_widget_show_all(window);

    /* Set up the background RGB buffer. */
    pos = backbuf;
    for (y = 0; y < height; y++)
    {
        int on = 0;
        for (x = 0; x < width; x++)
        {
            // checker board a la Gimp
            on = ((x >> 4) & 1);
            if ((y >> 4 & 1)) on = !on;
            *pos++ = 0xFF - (on ? 0x80 : 0);      /* Red. */
            *pos++ = 0xFF - (on ? 0x80 : 0);      /* Green. */
            *pos++ = 0xFF - (on ? 0x80 : 0);      /* Blue. */
        }
    }

    /* count colors */
    count_colors(spu_pixelbuf,height,width);

    gtk_main();
    return 0;
}

// I should deal with the SPU being smaller than the screen area here,
// rather than drawing a giant spu in the draw_spu function
gboolean
on_darea_expose(GtkWidget * widget,
                GdkEventExpose * event, gpointer user_data)
{
    uint8_t *spu;
    guchar *screen;
    guchar *back;
    gint x, y;


    // merge back with SPU onto rgbbuf
    spu = spu_pixelbuf;
    back = backbuf;
    screen = rgbbuf;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            if (*spu > COLOR_NOTHING)
            {
                printf("spu buffer freaked out (%d @ %d,%d)\n", *spu, x, y);
                exit(1);
            }
            /*
             * Optimized alpha blending
             *
             *   x*a + y*(1-a)       =
             *   x*a + y*(1) - y*a   =
             *   x*a - y*a + y       =
             *   (x-y)*a + y
             */

            // Red
            //*screen = alpha[*spu] * cmap[*spu].red + (0xF-alpha[*spu]) * *back;
            *screen = (cmap[*spu].red-*back)*alpha[*spu] + *back;
            screen++;
            back++;
            // Green
            //*screen = alpha[*spu] * cmap[*spu].green + (0xF-alpha[*spu]) * *back;
            *screen = (cmap[*spu].green-*back)*alpha[*spu] + *back;
            screen++;
            back++;
            // Blue
            //*screen = alpha[*spu] * cmap[*spu].blue + (0xF-alpha[*spu]) * *back;
            *screen = (cmap[*spu].blue-*back)*alpha[*spu] + *back;
            screen++;
            back++;
            spu++;
        }
    }

    gdk_draw_rgb_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
                       0, 0, width, height,
                       GDK_RGB_DITHER_MAX, rgbbuf, width * 3);
    return 1;
}

/* vi:set ai ts=4 sw=4 expandtab: */
