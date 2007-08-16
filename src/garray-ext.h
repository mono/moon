/*
 * garray-ext.h: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifndef __MOON_GARRAY_EXT_H__
#define __MOON_GARRAY_EXT_H__

#include <glib.h>

void g_ptr_array_insert_sorted (GPtrArray *array, GCompareFunc cmp, void *item);

#endif /* __MOON_GARRAY_EXT_H__ */
