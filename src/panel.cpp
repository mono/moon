/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * panel.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "canvas.h"
#include "geometry.h"
#include "panel.h"
#include "brush.h"
#include "math.h"
#include "collection.h"
#include "runtime.h"

Panel::Panel ()
{
	SetObjectType (Type::PANEL);
	SetValue (Panel::ChildrenProperty, Value::CreateUnref (new UIElementCollection ()));
	SetSubtreeObject (GetChildren());
	mouse_over = NULL;
}

Panel::~Panel()
{
}

#define DEBUG_BOUNDS 0
#define CAIRO_CLIP 0

#if DEBUG_BOUNDS
static void space (int n)
{
	for (int i = 0; i < n; i++)
		putchar (' ');
}
static int levelb = 0;
#endif

void
Panel::ComputeBounds ()
{
#if DEBUG_BOUNDS
	levelb += 4;
	space (levelb);
	printf ("Panel: Enter ComputeBounds (%s)\n", GetName());
#endif

	// Clear the previous values
	extents = bounds = bounds_with_children = Rect ();

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *item = walker.Step ()) {
		
		// if the item isn't drawn, skip it
		if (!item->GetRenderVisible ())
			continue;

		Rect r = item->GetSubtreeBounds ();
		r = IntersectBoundsWithClipPath (r, true);

#if DEBUG_BOUNDS
		space (levelb + 4);
		printf ("Item (%s, %s) bounds %g %g %g %g\n", item->GetName (), item->GetTypeName (),
			r.x, r.y, r.width, r.height);
#endif
		bounds_with_children = bounds_with_children.Union (r);
	}

	Brush *bg = GetBackground();
	if (bg) {
		extents = Rect (0,0,GetActualWidth (),GetActualHeight ());
		bounds = IntersectBoundsWithClipPath (extents, false).Transform (&absolute_xform);
		bounds_with_children = bounds_with_children.Union (bounds);
	}

#if DEBUG_BOUNDS
	space (levelb);
	printf ("Panel: Leave ComputeBounds (%g %g %g %g)\n",
		bounds.x, bounds.y, bounds.width, bounds.height);
	space (levelb);
	printf ("Panel: Leave ComputeBounds (%g %g %g %g)\n",
		bounds_with_children.x, bounds_with_children.y, bounds_with_children.width, bounds_with_children.height);
	levelb -= 4;
#endif
}

void
Panel::ShiftPosition (Point p)
{
	double dx = p.x - bounds.x;
	double dy = p.y - bounds.y;

	// need to do this after computing the delta
	FrameworkElement::ShiftPosition (p);

	bounds_with_children.x += dx;
	bounds_with_children.y += dy;
}

//#define DEBUG_INVALIDATE 1

void
Panel::Render (cairo_t *cr, Region *region)
{
	Brush *background;
	
	cairo_set_matrix (cr, &absolute_xform);
	
	if ((background = GetBackground ())) {
		Rect area = Rect (0.0, 0.0, GetActualWidth (), GetActualHeight ());
		area = area.GrowBy (-*GetMargin ());
		if (area.width > 0 && area.height > 0) {
			background->SetupBrush (cr, area);
			
			cairo_new_path (cr);
			area.Draw (cr);
			background->Fill (cr);
		}
	}
}

Rect
Panel::GetCoverageBounds ()
{
	Brush *background = GetBackground ();
	
	if (background && background->IsOpaque ())
		return bounds;

	return Rect ();
}

bool
Panel::InsideObject (cairo_t *cr, double x, double y)
{
	if (GetBackground ())
		return FrameworkElement::InsideObject (cr, x, y);

	return false;
}

void
Panel::HitTest (cairo_t *cr, Rect r, List *uielement_list)
{
}

bool
Panel::EmptyBackground ()
{
	return GetBackground () == NULL;
}

Size
Panel::MeasureOverride (Size availableSize)
{
	Size result = Size (0,0);
	return result;
}

//
// Intercept any changes to the children property and mirror that into our
// own variable
//
void
Panel::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::PANEL) {
		FrameworkElement::OnPropertyChanged (args);
		return;
	}

	if (args->property == Panel::BackgroundProperty)
		Invalidate ();

	if (args->property == Panel::ChildrenProperty) {
		Collection *collection;
		
		if (args->old_value) {
			collection = args->old_value->AsCollection ();
			for (int i = 0; i < collection->GetCount (); i++)
				ElementRemoved (collection->GetValueAt (i)->AsUIElement());
		}
		
		if (args->new_value) {
			collection = args->new_value->AsCollection ();
			for (int i = 0; i < collection->GetCount (); i++)
				ElementAdded (collection->GetValueAt (i)->AsUIElement ());
		}

		SetSubtreeObject (args->new_value ? args->new_value->AsCollection() : NULL);

		UpdateBounds();
	}

	NotifyListenersOfPropertyChange (args);
}

void
Panel::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == Panel::BackgroundProperty) {
		Invalidate ();
	} else {
		FrameworkElement::OnSubPropertyChanged (prop, obj, subobj_args);
	}
}

void
Panel::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col == GetChildren ()) {
		switch (args->action) {
		case CollectionChangedActionReplace:
			ElementRemoved (args->old_value->AsUIElement ());
			// now fall thru to Add
		case CollectionChangedActionAdd:
			ElementAdded (args->new_value->AsUIElement ());
			break;
		case CollectionChangedActionRemove:
			ElementRemoved (args->old_value->AsUIElement ());
			break;
		case CollectionChangedActionClearing:
			for (int i = 0; i < col->GetCount (); i++)
				ElementRemoved (col->GetValueAt (i)->AsUIElement ());
			break;
		case CollectionChangedActionCleared:
			// nothing needed here.
			break;
		}
	} else {
		FrameworkElement::OnCollectionChanged (col, args);
	}
}
