/*
#
# Session loading and saving routines
#
# $Id: sessions.cpp 247 2004-03-25 07:11:39Z schmidtw $
#
# Copyright (C) 2003 Weston Schmidt
# weston_schmidt@yahoo.com http://flanders.hopto.org
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

#include "sessions.h"
#include <stdio.h>
#include <libxml/parser.h>
#include "main.h"
#include "GOPutils.h"
#include "widgets.h"

static int handle_clips( xmlNodePtr node )
{
    int rValue;
    uint32_t start, end;
    xmlChar*   value;

    rValue = 0;
    start  = 0;
    end    = 0;

    while( NULL != node ) {

        if( 0 == xmlStrcmp(node->name, (const xmlChar *)"clip") ) {

            value = xmlGetProp( node, (const xmlChar *)"start_gop" );
            start = atoi( (const char*)value );
            xmlFree( value );

            value = xmlGetProp( node, (const xmlChar *)"end_gop" );
            end = atoi( (const char*)value );
            xmlFree( value );

            add_GOP_slice( start, end );
        }
        node = node->next;
    }

    return rValue;
}

int load_session( char *filename )
{
    int rValue;
    xmlDocPtr  doc;
    xmlNodePtr node;
    xmlChar*   value;

    doc   = NULL;
    node  = NULL;
    value = NULL;
    rValue = SESSION_LOADED_OK;

    doc = xmlReadFile( filename, NULL, 0 );
    if( NULL == doc ) {
        rValue = SESSION_ERR_FILEREAD;
        goto done;
    }

    /* get the root node & check for <gopchop> */
    node = xmlDocGetRootElement( doc );
    if( xmlStrcmp(node->name, (const xmlChar *) "gopchop") ) {
        rValue = SESSION_ERR_NOT_GOPCHOP_FILE;
        goto done;
    }

    /* parse the children of the root node */
    node = node->xmlChildrenNode;

    /* harvest the parameters */
    while( NULL != node ) {
        if( 0 == xmlStrcmp(node->name, (const xmlChar *)"filename") ) {
            value = xmlNodeListGetString( doc, node->xmlChildrenNode, 1 );
            open_file( (char*) value);
            xmlFree(value);
            value = NULL;
        }

        if( 0 == xmlStrcmp(node->name, (const xmlChar *)"clips") ) {
            handle_clips( node->xmlChildrenNode );
        }

        node = node->next;
    }

    /* perform clean-up and exit */
done:
    if( NULL != doc   ) xmlFreeDoc( doc );
    if( NULL != value ) xmlFree( value );

    xmlCleanupParser();
    xmlMemoryDump();
    return rValue;
}

gboolean foreach_save_GOPs(GtkTreeModel *model,
                           GtkTreePath  *path,
                           GtkTreeIter  *iter,
                           gpointer      data)
{
    FILE* file = (FILE*)data;
    struct clip_list_t clips;

    get_clips_from_list( model, iter, &clips );

    fprintf( file, "        <clip start_gop=\"%d\" end_gop=\"%d\"/>\n", clips.GOP_first, clips.GOP_last );

    return FALSE;
}

int save_session( char *filename )
{
    FILE *file;
    int   rValue;
    int   numClips;
    int   i;

    numClips = 0;

    rValue = SESSION_SAVE_FILE_OK;

    file = fopen( filename, "w" );

    if( NULL == file ) {
        rValue = SESSION_SAVE_FILE_OPEN_FAILURE;
        goto done;
    }

    fprintf( file, "<gopchop>\n" );
    fprintf( file, "    <filename><![CDATA[%s]]></filename>\n", (char*) mpeg2parser->getFilename() );
    fprintf( file, "    <clips>\n" );

    /* save the clips */
    gtk_tree_model_foreach(GTK_TREE_MODEL(main_list_store),
                           foreach_save_GOPs,
                           file);

    fprintf( file, "    </clips>\n" );
    fprintf( file, "</gopchop>\n" );
done:
    if( NULL != file ) {
        fclose( file );
    }

    return rValue;
}

const gchar* error_to_string( const int error )
{
    const gchar* rValue;

    switch( error ) {
        case SESSION_ERR_FILEREAD:
            rValue = (const gchar*)_("File Read Error.\n");
            break;
        case SESSION_ERR_NOT_GOPCHOP_FILE:
            rValue = (const gchar*)_("Not a GOPchop file.\n");
            break;
        case SESSION_ERR_CLIP_PARSE:
            rValue = (const gchar*)_("Clip parsing error.\n");
            break;
        default:
            rValue = (const gchar*)_("Unknown Error.\n");
    }
    return rValue;
}
