/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * thickness.h:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_THICKNESS_H__
#define __MOON_THICKNESS_H__

#include <glib.h>

/* @IncludeInKinds */
struct Thickness {
	double left;
	double top;
	double right;
	double bottom;
	
	Thickness ()
	  : left (0), top (0),
	    right (0), bottom (0)
	{
	}

	Thickness (double uniform)
	{
		bottom = uniform;
		right = uniform;
		left = uniform;
		top = uniform;
	}
	
	Thickness (double hori, double vert)
	{
		bottom = vert;
		right = hori;
		left = hori;
		top = vert;
	}
	
	Thickness (double left, double top, double right, double bottom)
	{
		this->bottom = bottom;
		this->right = right;
		this->left = left;
		this->top = top;
	}
	
	Thickness (const Thickness &thickness)
	{
		bottom = thickness.bottom;
		right = thickness.right;
		left = thickness.left;
		top = thickness.top;
	}

	Thickness operator- ()
	{
		return Thickness (-left, -top, -right, -bottom);
	}

	Thickness operator+ (const Thickness &th)
	{
		return Thickness (left + th.left, top + th.top, right + th.right, bottom + th.bottom);
	}

	Thickness operator- (const Thickness &th)
	{
		return Thickness (left - th.left, top - th.top, right - th.right, bottom - th.bottom);
	}

	bool operator == (const Thickness &v)
	{
		return fabs (bottom - v.bottom) < DBL_EPSILON && fabs(right - v.right) < DBL_EPSILON && fabs (left - v.left) < DBL_EPSILON && fabs (top - v.top) < DBL_EPSILON;
	}

	bool operator != (const Thickness &v)
	{
		return !(*this == v);
	}
	//
	// FromStr
	//   Parses @s and return a new Thickness in @t.  Returns
	//   true if this was successful, false otherwise.
	//
	static bool FromStr (const char *s, Thickness *t);
};

#endif /* __MOON_THICKNESS_H__ */
