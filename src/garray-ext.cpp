/*
 * garray-ext.cpp: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include "garray-ext.h"



/**
 * MID:
 * @lo: the low bound
 * @hi: the high bound
 *
 * Finds the midpoint between positive integer values, @lo and @hi.
 *
 * Notes: Typically expressed as '(@lo + @hi) / 2', this is incorrect
 * when @lo and @hi are sufficiently large enough that combining them
 * would overflow their integer type. To work around this, we use the
 * formula, '@lo + ((@hi - @lo) / 2)', thus preventing this problem
 * from occuring.
 *
 * Returns the midpoint between @lo and @hi (rounded down).
 **/
#define MID(lo, hi) (lo + ((hi - lo) >> 1))


static int
bsearch (GPtrArray *array, bool stable, GCompareFunc cmp, void *item)
{
	register guint lo, hi;
	guint m;
	int c;
	
	if (array->len == 0)
		return 0;
	
	lo = 0, hi = array->len;
	
	do {
		m = MID (lo, hi);
		if ((c = cmp (&item, &array->pdata[m])) > 0) {
			lo = m + 1;
			m = lo;
		} else if (c < 0) {
			hi = m;
		} else if (stable) {
			lo = m + 1;
			m = lo;
		} else {
			break;
		}
	} while (lo < hi);
	
	return m;
}

void
g_ptr_array_insert_sorted (GPtrArray *array, GCompareFunc cmp, void *item)
{
	guint ins = bsearch (array, true, cmp, item);
	
	g_ptr_array_set_size (array, array->len + 1);
	
	if (ins < array->len) {
		uint8_t *dest = ((uint8_t *) array->pdata) + (sizeof (void *) * (ins + 1));
		uint8_t *src = ((uint8_t *) array->pdata) + (sizeof (void *) * ins);
		guint n = array->len - ins - 1;
		
		g_memmove (dest, src, (sizeof (void *) * n));
	}
	
	array->pdata[ins] = item;
}
