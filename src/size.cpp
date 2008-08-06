/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * eventargs.cpp: specialized code for dealing with mouse/stylus/keyboard event args.
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "size.h"

SizeChangedEventArgs::SizeChangedEventArgs (Size prev_size, Size new_size)
{
	this->prev_size = prev_size;
	this->new_size = new_size;
}


SizeChangedEventArgs*
size_changed_event_args_new (Size prev_size, Size new_size)
{
	return new SizeChangedEventArgs (prev_size, new_size);
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
