/*
 * geometry.cpp: Geometry classes
 *
 * Author:
 *	Sebastien Pouliot  <sebastien@ximian.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>

#include "geometry.h"

//
// GeometryGroup
//

GeometryGroup*
geometry_group_new ()
{
	return new GeometryGroup ();
}

//
// EllipseGeometry
//

EllipseGeometry*
ellipse_geometry_new ()
{
	return new EllipseGeometry ();
}

//
// LineGeometry
//

LineGeometry*
line_geometry_new ()
{
	return new LineGeometry ();
}

//
// PathGeometry
//

PathGeometry*
path_geometry_new ()
{
	return new PathGeometry ();
}

//
// RectangleGeometry
//

RectangleGeometry*
rectangle_geometry_new ()
{
	return new RectangleGeometry ();
}

//
// 
//

void
geometry_init ()
{
}
