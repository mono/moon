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

SizeChangedEventArgs::SizeChangedEventArgs()
{
	SetObjectType(Type::SIZECHANGEDEVENTARGS);
	prev_size = Size (); new_size = Size ();
}

SizeChangedEventArgs::SizeChangedEventArgs (Size prev_size, Size new_size)
{
	SetObjectType(Type::SIZECHANGEDEVENTARGS);
	this->prev_size = prev_size;
	this->new_size = new_size;
}

void
size_changed_event_args_get_prev_size (SizeChangedEventArgs *args, Size *prev_size)
{
	*prev_size = args->GetPrevSize ();
}

void
size_changed_event_args_get_new_size  (SizeChangedEventArgs *args, Size *new_size)
{
	*new_size = args->GetNewSize ();
}
