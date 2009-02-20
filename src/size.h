/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * size.h
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_SIZE_H__
#define __MOON_SIZE_H__

#include <math.h>
#include <glib.h>
#include "eventargs.h"
#include "thickness.h"

/* @SilverlightVersion="2" */
/* @IncludeInKinds */
struct Size {
	double width, height;

	Size () : width(0), height(0) {}
	
	Size (int zero) : width(0), height(0) {}
	
	Size (double w, double h)
	{
		this->width = w;
		this->height = h;
	}

	bool IsEmpty () const
	{
		return (isinf (width) == -1 && isinf (height) == -1);
	}

	Size Max (double w, double h) const
	{
		return Size (width < w ? w : width, height < h ? h : height);
	}

	Size Max (const Size &s) const
	{
		return Max (s.width, s.height);
	}

	Size Min (double w, double h) const
	{
		return Size (width > w ? w : width, height > h ? h : height);
	}

	Size Min (const Size &s) const
	{
		return Min (s.width, s.height);
	}

	Size GrowBy (const double w, const double h) const
	{
		return Size (w + width, h + height);
	}

	Size GrowBy (const Thickness &t) 
	{
		const double w = width + t.left + t.right;
		const double h = height + t.top + t.bottom;
		return Size (w > 0 ? w : 0, h > 0 ? h : 0); 
	}

	bool operator == (const Size &size)
	{
		return fabs (size.width-width) < DBL_EPSILON && fabs (size.height-height) < DBL_EPSILON;
	}

	bool operator != (const Size &size)
	{
		return !(*this == size);
	}

	//
	// FromStr
	//   Parses @s and return a new size in @size. Returns true if
	//   this was successful, false otherwise.
	//
	static bool FromStr (const char *s, Size *size);
};

/* @SilverlightVersion="2" */
/* @Namespace=None */
class SizeChangedEventArgs : public RoutedEventArgs {
	Size prev_size;
	Size new_size;
	
public:
	/* @GenerateCBinding,GeneratePInvoke */
	SizeChangedEventArgs ();
	
	SizeChangedEventArgs (Size prev_size, Size new_size);
	
	Size GetPrevSize () { return prev_size; }
	Size GetNewSize () { return new_size; }
};

G_BEGIN_DECLS

void                  size_changed_event_args_get_prev_size (SizeChangedEventArgs *args, Size *prev_size);
void                  size_changed_event_args_get_new_size  (SizeChangedEventArgs *args, Size *new_size);

G_END_DECLS

#endif /* __MOON_POINT_H__ */
