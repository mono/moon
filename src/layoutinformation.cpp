/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * layoutinformation.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include "frameworkelement.h"
#include "geometry.h"
#include "rect.h"
#include "point.h"
#include "factory.h"

namespace Moonlight {

Geometry *
LayoutInformation::GetCompositeClip (FrameworkElement *item)
{
	FrameworkElement *element = item;
	Point offset = Point (0,0);
	Rect composite (0,0,INFINITY, INFINITY);

	do {
		Geometry *clip = GetLayoutClip (element);
		
		if (clip && clip->Is (Type::RECTANGLEGEOMETRY)) {
			Rect relative = *((RectangleGeometry *)clip)->GetRect ();
			relative.x -= offset.x;
			relative.y -= offset.y;
			
			composite = composite.Intersection (relative);
		}

		if (element->Is (Type::CANVAS) || element->Is (Type::USERCONTROL))
			break;

		Point *local_offset = GetVisualOffset (element);
		if (local_offset) {
			offset.x += local_offset->x;
			offset.y += local_offset->y;
		}
	} while ((element = (FrameworkElement *)element->GetVisualParent ()));

	if (isinf (composite.width) || isinf (composite.height))
		return NULL;

	RectangleGeometry *geom = MoonUnmanagedFactory::CreateRectangleGeometry ();
	geom->SetRect (&composite);

	return geom;
}

};
