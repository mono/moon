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

/* @SilverlightVersion="2" */
/* @IncludeInKinds */
struct CornerRadius {
	double topLeft;
	double topRight;
	double bottomLeft;
	double bottomRight;

	CornerRadius ()
	  : topLeft (0), topRight (0),
	    bottomLeft (0), bottomRight (0)
	{
	}

	CornerRadius (double uniformRadius)
	  : topLeft (uniformRadius), topRight (uniformRadius),
	    bottomLeft (uniformRadius), bottomRight (uniformRadius)
	{
	}

	CornerRadius (double topLeft, double topRight,
		      double bottomLeft, double bottomRight)
	  : topLeft (topLeft), topRight (topRight),
	    bottomLeft (bottomLeft), bottomRight (bottomRight)
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
