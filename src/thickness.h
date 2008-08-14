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
};

G_BEGIN_DECLS

Thickness *thickness_from_str (const char *str);

G_END_DECLS

#endif /* __MOON_THICKNESS_H__ */
