/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * size.cpp: specialized code for dealing with SizeChangedEventArgs
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include "size.h"
#include "utils.h"

namespace Moonlight {

bool
Size::FromStr (const char *s, Size *size)
{
	GArray *values = double_garray_from_str (s, 2);

	if (!values)
		return false;

	*size = Size (g_array_index (values, double, 0), g_array_index (values, double, 1));

	g_array_free (values, true);

	return true;
}

};
