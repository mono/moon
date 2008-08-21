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

/* @SilverlightVersion="2" */
/* @IncludeInKinds */
struct CornerRadius {
	double topLeft;
	double topRight;
	double bottomLeft;
	double bottomRight;

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
};

#endif /* __MOON_POINT_H__ */
