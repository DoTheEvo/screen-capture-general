/* desktop session recorder
 * Copyright (C) 2005 Benjamin Otte <otte@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <gtk/gtk.h>

#ifndef __HAVE_BYZANZ_SELECT_H__
#define __HAVE_BYZANZ_SELECT_H__

guint			byzanz_select_get_method_count	        (void);
const char *		byzanz_select_method_describe	        (guint		        method);
const char *		byzanz_select_method_get_mnemonic       (guint	                method);
const char *		byzanz_select_method_get_icon_name      (guint	                method);
const char *		byzanz_select_method_get_name	        (guint		        method);
int			byzanz_select_method_lookup	        (const char *	        name);

typedef void (* ByzanzSelectFunc) (GdkWindow *window, const cairo_rectangle_int_t *area, gpointer data);
void    		byzanz_select_method_select	        (guint		        method,
                                                                 ByzanzSelectFunc       func,
                                                                 gpointer               data);
					

#endif /* __HAVE_BYZANZ_SELECT_H__ */
