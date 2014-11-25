/*
#
# Routines for handling loading/saving an runtime commands file.
#
# $Id: rc.h 347 2009-03-02 23:27:14Z keescook $
#
# Copyright (C) 2003-2009 Kees Cook
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
#ifndef _RC_H_
#define _RC_H_

#ifdef __cplusplus
extern "C" {
#endif


                            /*
                             * When an RC_TYPE_STRING is found, the variable
                             * will be free()d, and the loaded value will
                             * be strdup()d into place.  It is the caller's
                             * responsibility to free() this as needed.
                             * (And to only pass in variables that have been
                             * malloc()d.)
                             */
#define RC_TYPE_STRING	0	/* char*                    */ 
#define RC_TYPE_BOOLEAN 1	/* int (enforced to 0 or 1) */
#define RC_TYPE_INTEGER 2	/* int                      */

typedef struct rc_parse_item_t {
	char * option;	 /* name of the rc option */
	void * variable; /* variable to load into */
	int    vartype;  /* type of option to load */
} rc_parse_item;

/*
 * takes
 *  package name (used as "$HOME/.[PACKAGE]/options")
 *  a list of rc_parse_items (last item must have a NULL 'option')
 * returns
 *   0 on success
 *  -1 on failure
 */
int rc_save(char * package, rc_parse_item * items);
int rc_load(char * package, rc_parse_item * items);



#ifdef __cplusplus
}
#endif

#endif /* _RC_H_ */
/* vi:set ai ts=4 sw=4 expandtab: */
