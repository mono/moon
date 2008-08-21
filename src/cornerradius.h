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
	  : bottomLeft (uniformRadius), bottomRight (uniformRadius),
	    topLeft (uniformRadius), topRight (uniformRadius)
	{
	}

	CornerRadius (double topLeft, double topRight,
		      double bottomLeft, double bottomRight)
	  : bottomLeft (bottomLeft), bottomRight (bottomRight),
	    topLeft (topLeft), topRight (topRight)
	{
	}
};

#endif /* __MOON_POINT_H__ */
