/*
 * utils.h: 
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

#include "zip/unzip.h"

G_BEGIN_DECLS

void g_ptr_array_insert_sorted (GPtrArray *array, GCompareFunc cmp, void *item);

bool ExtractFile (unzFile zip, int fd);

char *make_tmpdir (char *tmpdir);

int moon_rmdir (const char *dir);

G_END_DECLS

#endif /* __MOON_GARRAY_EXT_H__ */
