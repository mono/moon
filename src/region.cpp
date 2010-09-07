/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * region.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <stdlib.h>
#include <cairo.h>

#include "region.h"


namespace Moonlight {


Region::Region ()
{ 
	cairo_region = cairo_region_create ();
}

Region::Region (double x, double y, double width, double height)
{
	cairo_region = cairo_region_create ();
	Union (Rect (x, y, width, height));
}

Region::Region (Rect rect)
{
	cairo_region = cairo_region_create ();
	Union (rect);
}

Region::Region (Region *region)
{
	cairo_region = cairo_region_create ();
	Union (region);
}

Region::~Region ()
{

	cairo_region_destroy (cairo_region);
	cairo_region = NULL;
}

bool
Region::IsEmpty ()
{
	return cairo_region_is_empty (cairo_region);
}

void 
Region::Union (Rect rect)
{
	cairo_rectangle_int_t cairo_rect = rect.ToCairoRectangleInt ();
	cairo_region_union_rectangle (cairo_region, &cairo_rect);
}

void 
Region::Union (Region *region)
{
	cairo_region_union (cairo_region, region->cairo_region);
}

cairo_region_overlap_t
Region::RectIn (Rect rect)
{
	cairo_rectangle_int_t cairo_rect = rect.ToCairoRectangleInt ();
	return cairo_region_contains_rectangle (cairo_region, &cairo_rect);
}

void
Region::Intersect (Region *region)
{
	status = cairo_region_intersect (cairo_region, region->cairo_region);
}

void
Region::Intersect (Rect rect)
{
	Region tmp = Region (rect);
	Intersect (&tmp);
}


void
Region::Subtract (Region *region)
{
	status = cairo_region_subtract (cairo_region, region->cairo_region);
}

void
Region::Subtract (Rect rect)
{
	Region tmp = Region (rect);
	Subtract (&tmp);
}

void
Region::Offset (int dx, int dy)
{
	cairo_region_translate (cairo_region, -dx, dy);
}

int
Region::GetRectangleCount ()
{
	return cairo_region_num_rectangles (cairo_region);
}

Rect
Region::GetRectangle (int index)
{
	cairo_rectangle_int_t cairo_rect;

	cairo_region_get_rectangle (cairo_region, index, &cairo_rect);
	Rect rect (cairo_rect.x, cairo_rect.y, cairo_rect.width, cairo_rect.height);

	return rect;
}

Rect 
Region::GetExtents ()
{
	cairo_rectangle_int_t extents;
	cairo_region_get_extents (cairo_region, &extents);
	return Rect (extents.x, extents.y, extents.width, extents.height);
}

void 
Region::Draw (cairo_t *cr)
{
	int count = GetRectangleCount ();

	for (int i = 0; i < count; i++)
		GetRectangle (i).Draw (cr);
}


};
