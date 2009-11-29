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
	mouse_over = NULL;

	SetSubtreeObject (GetChildren());
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
Panel::Render (cairo_t *cr, Region *region, bool path_only)
{
	Brush *background = GetBackground ();
	
	cairo_set_matrix (cr, &absolute_xform);
	
	Size framework (GetActualWidth (), GetActualHeight ());
	framework = ApplySizeConstraints (framework);
	Rect area = Rect (0.0, 0.0, framework.width, framework.height);
	
	cairo_save (cr);
	if (!path_only) 
		RenderLayoutClip (cr);

	cairo_new_path (cr);
	area.Draw (cr);

	if (background && area.width > 0 && area.height > 0 && !path_only) {
		background->SetupBrush (cr, area);
		background->Fill (cr);
	}
	cairo_restore (cr);
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

void
Panel::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::PANEL) {
		FrameworkElement::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == Panel::BackgroundProperty) {
		/* several of the bounds values are conditional on having a brush */
		UpdateBounds ();
		Invalidate ();
	} else if (args->GetId () == Panel::ChildrenProperty) {
		Collection *collection;

		SetSubtreeObject (args->GetNewValue() ? args->GetNewValue()->AsCollection() : NULL);
		
		if (args->GetOldValue()) {
			collection = args->GetOldValue()->AsCollection ();
			for (int i = 0; i < collection->GetCount (); i++)
				ElementRemoved (collection->GetValueAt (i)->AsUIElement());
		}
		
		if (args->GetNewValue()) {
			collection = args->GetNewValue()->AsCollection ();
			for (int i = 0; i < collection->GetCount (); i++)
				ElementAdded (collection->GetValueAt (i)->AsUIElement ());
		}

		UpdateBounds();
	}

	NotifyListenersOfPropertyChange (args, error);
}

void
Panel::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop && prop->GetId () == Panel::BackgroundProperty) {
		Invalidate ();
	} else {
		FrameworkElement::OnSubPropertyChanged (prop, obj, subobj_args);
	}
}

void
Panel::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col == GetChildren ()) {
		MoonError error;

		switch (args->GetChangedAction()) {
		case CollectionChangedActionReplace:
			if (args->GetOldItem()->Is(GetDeployment (), Type::FRAMEWORKELEMENT))
				args->GetOldItem()->AsFrameworkElement()->SetLogicalParent (NULL, &error /* XXX unused */);
			ElementRemoved (args->GetOldItem()->AsUIElement ());
			// now fall thru to Add
		case CollectionChangedActionAdd:
			if (args->GetNewItem()->Is(GetDeployment (), Type::FRAMEWORKELEMENT))
				args->GetNewItem()->AsFrameworkElement()->SetLogicalParent (this, &error /* XXX unused */);
			ElementAdded (args->GetNewItem()->AsUIElement ());
			break;
		case CollectionChangedActionRemove:
			if (args->GetOldItem()->Is(GetDeployment (), Type::FRAMEWORKELEMENT))
				args->GetOldItem()->AsFrameworkElement()->SetLogicalParent (NULL, &error /* XXX unused */);
			ElementRemoved (args->GetOldItem()->AsUIElement ());
			break;
		case CollectionChangedActionClearing:
			for (int i = 0; i < col->GetCount (); i++) {
				UIElement *ui = col->GetValueAt (i)->AsUIElement ();
				if (ui->Is(Type::FRAMEWORKELEMENT))
					((FrameworkElement*)ui)->SetLogicalParent (NULL, &error /* XXX unused */);
				ElementRemoved (ui);
			}
			break;
		case CollectionChangedActionCleared:
			// nothing needed here.
			break;
		}
	} else {
		FrameworkElement::OnCollectionChanged (col, args);
	}
}


void
Panel::OnLoaded ()
{
	FrameworkElement::OnLoaded ();

	if (GetSurface ()) {
		// queue a resort based on ZIndex
		GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
	}
}

void
Panel::ElementAdded (UIElement *item)
{
	FrameworkElement::ElementAdded (item);
	
	if (GetSurface ()) {
		// queue a resort based on ZIndex
		GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
	}
}

void
Panel::ElementRemoved (UIElement *item)
{
	FrameworkElement::ElementRemoved (item);
	
	if (GetSurface ()) {
		// queue a resort based on ZIndex
		GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
	}
}


void
Panel::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (col == GetChildren()) {
		// if a child changes its ZIndex property we need to resort our Children
		if (args->GetId () == Canvas::ZIndexProperty) {
			((UIElement *) obj)->Invalidate ();
			if (GetSurface ()) {
				// queue a resort based on ZIndex
				GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
			}
			return;
		}
	}

	FrameworkElement::OnCollectionItemChanged (col, obj, args);
}

