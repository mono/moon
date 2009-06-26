/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * cornerradius.h
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_CORNERRADIUS_H__
#define __MOON_CORNERRADIUS_H__

#include <math.h>

/* @IncludeInKinds */
struct CornerRadius {
	double topLeft;
	double topRight;
	double bottomRight;
	double bottomLeft;

	CornerRadius ()
	  : topLeft (0), topRight (0),
	    bottomRight (0), bottomLeft (0)
	{
	}

	CornerRadius (double uniformRadius)
	  : topLeft (uniformRadius), topRight (uniformRadius),
	    bottomRight (uniformRadius), bottomLeft (uniformRadius)
	{
	}

	CornerRadius (double topLeft, double topRight,
		      double bottomRight, double bottomLeft)
	  : topLeft (topLeft), topRight (topRight),
	    bottomRight (bottomRight), bottomLeft (bottomLeft)
	{
	}

	bool operator == (const CornerRadius &corner)
	{
		return fabs (topLeft-corner.topLeft) < DBL_EPSILON && fabs (bottomLeft-corner.bottomLeft) < DBL_EPSILON && fabs(topRight-corner.topRight) < DBL_EPSILON && fabs(bottomRight-corner.bottomRight) < DBL_EPSILON; 
	}

	bool operator != (const CornerRadius &corner)
	{
		return !(*this == corner);
	}

	//
	// FromStr
	//   Parses @s and return a new CornerRadius in @p.  Returns
	//   true if this was successful, false otherwise.
	//
	static bool FromStr (const char *s, CornerRadius *p);
};

#endif /* __MOON_POINT_H__ */
