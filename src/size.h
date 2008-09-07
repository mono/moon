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

	Size Max (const double w, const double h) const
	{
		return Size (width < w ? w : width, height < h ? h : height);
	}

	Size Max (const Size s) const
	{
		return Max (s.width, s.height);
	}

	Size Min (const double w, const double h) const
	{
		return Size (width > w ? w : width, height > h ? h : height);
	}

	Size Min (const Size s) const
	{
		return Min (s.width, s.height);
	}

	Size GrowBy (const double w, const double h) const
	{
		return Size (w + width, h + height);
	}

	Size GrowBy (const Thickness *t) 
	{
		return Size (width + t->left + t->right, height + t->top + t->bottom); 
	}
};

/* @SilverlightVersion="2" */
/* @Namespace=None */
class SizeChangedEventArgs : public RoutedEventArgs {
	Size prev_size;
	Size new_size;
	
public:
	/* @GenerateCBinding,GeneratePInvoke */
	SizeChangedEventArgs () { prev_size = Size (); new_size = Size (); }
	
	SizeChangedEventArgs (Size prev_size, Size new_size);
	
	virtual Type::Kind GetObjectType () { return Type::SIZECHANGEDEVENTARGS; }
	
	Size GetPrevSize () { return prev_size; }
	Size GetNewSize () { return new_size; }
};

G_BEGIN_DECLS

void                  size_changed_event_args_get_prev_size (SizeChangedEventArgs *args, Size *prev_size);
void                  size_changed_event_args_get_new_size  (SizeChangedEventArgs *args, Size *new_size);

G_END_DECLS

#endif /* __MOON_POINT_H__ */
