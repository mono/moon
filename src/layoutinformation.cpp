/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * layoutinformation.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#include "frameworkelement.h"
#include "geometry.h"
#include "rect.h"
#include "point.h"

Geometry *
LayoutInformation::GetClip (FrameworkElement *item)
{
	FrameworkElement *element = item;
	Point offset = Point (0,0);
	Rect composite (0,0,INFINITY, INFINITY);

	while (element) {
		Point *local_offset = GetVisualOffset (element);
		
		if (!local_offset)
			break;
		
		Rect relative = *GetLayoutSlot (element);
		offset = offset + *local_offset;
		relative.x -= offset.x;
		relative.y -= offset.y;
		relative = relative.GrowBy (-*element->GetMargin ());
		
		composite = composite.Intersection (relative);
		
		if (element != last)
			break;

		element = (FrameworkElement *)element->GetVisualParent ();

		if (element && (element->Is (Type::CANVAS) || element->Is (Type::USERCONTROL)))
			break;
	}
	
	if (isinf (composite.width))
		return NULL;
	
	RectangleGeometry *geom = new RectangleGeometry ();
	geom->SetRect (&composite);

	return geom;
}
