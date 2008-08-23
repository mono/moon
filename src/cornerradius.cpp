/*
 * cornerradius.cpp: CornerRadius parsing
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "utils.h"
#include "cornerradius.h"

bool
CornerRadius::FromStr (const char *str, CornerRadius *corner)
{
	GArray *values = double_garray_from_str (str, 4);
	bool rv = true;

	*corner = NULL;

	switch (values->len) {
	case 1:
		*corner = CornerRadius (g_array_index (values, double, 0));
		break;
	case 2:
		g_warning ("CornerRadius specified with 2 values, '%s'.", str);
		rv = false;
		break;
	case 3:
		g_warning ("CornerRadius specified with 3 values, '%s'.", str);
		rv = false;
		break;
	case 4:
		*corner = CornerRadius (g_array_index (values, double, 0),
					g_array_index (values, double, 1),
					g_array_index (values, double, 2),
					g_array_index (values, double, 3));
		break;
	}

	if (values)
		g_array_free (values, TRUE);

	return rv;
}
