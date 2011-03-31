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
#include "deployment.h"
#include "effect.h"
#include "projection.h"
#include "factory.h"

namespace Moonlight {

Panel::Panel ()
{
	SetObjectType (Type::PANEL);
	mouse_over = NULL;
}

Panel::~Panel()
{
}

Value *
Panel::CreateChildren (Type::Kind kind, DependencyProperty *property, DependencyObject *forObj)
{
	UIElementCollection *col = MoonUnmanagedFactory::CreateUIElementCollection ();
	col->SetIsSecondaryParent (true);
	if (forObj)
		((Panel*)forObj)->SetSubtreeObject (col);
	return Value::CreateUnrefPtr (col);
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
	extents = extents_with_children = bounds = bounds_with_children = Rect ();

	VisualTreeWalker walker = VisualTreeWalker (this, Logical, false);
	while (UIElement *item = walker.Step ()) {
		
		// if the item isn't drawn, skip it
		if (!item->GetRenderVisible ())
			continue;

		extents_with_children = extents_with_children.Union (item->GetGlobalBounds ());
	}

	Brush *bg = GetBackground();
	if (bg) {
		extents = Rect (0,0,GetActualWidth (),GetActualHeight ());
		extents_with_children = extents_with_children.Union (extents);
	}

	bounds = IntersectBoundsWithClipPath (extents.GrowBy (effect_padding), false).Transform (&absolute_xform);
	bounds_with_children = IntersectBoundsWithClipPath (extents_with_children.GrowBy (effect_padding).Transform (&absolute_xform), true);

	ComputeGlobalBounds ();
	ComputeSurfaceBounds ();

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
Panel::Render (Context *ctx, Region *region)
{
	cairo_t *cr = ctx->Push (Context::Cairo ());
	Render (cr, region);
	ctx->Pop ();
}

void
Panel::Render (cairo_t *cr, Region *region, bool path_only)
{
	Brush *background = GetBackground ();
	
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

	// make sure we don't leave a stray path in the context
	if (!path_only)
		cairo_new_path (cr);

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
Panel::MeasureOverrideWithError (Size availableSize, MoonError *error)
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
			int children_count = collection->GetCount ();
			for (int i = 0; i < children_count; i++)
				ElementRemoved (collection->GetValueAt (i)->AsUIElement());
		}
		
		if (args->GetNewValue()) {
			collection = args->GetNewValue()->AsCollection ();
			int children_count = collection->GetCount ();
			for (int i = 0; i < children_count; i++)
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
	if (PropertyHasValueNoAutoCreate (Panel::ChildrenProperty, col)) {
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
		case CollectionChangedActionClearing: {
			int children_count = col->GetCount ();
			for (int i = 0; i < children_count; i++) {
				UIElement *ui = col->GetValueAt (i)->AsUIElement ();
				if (ui->Is(Type::FRAMEWORKELEMENT))
					((FrameworkElement*)ui)->SetLogicalParent (NULL, &error /* XXX unused */);
				ElementRemoved (ui);
			}
			break;
		}
		case CollectionChangedActionCleared:
			// nothing needed here.
			break;
		}
	} else {
		FrameworkElement::OnCollectionChanged (col, args);
	}
}


void
Panel::OnIsAttachedChanged (bool attached)
{
	FrameworkElement::OnIsAttachedChanged (attached);

	if (attached) {
		// queue a resort based on ZIndex
		GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
	}
}

void
Panel::ElementAdded (UIElement *item)
{
	FrameworkElement::ElementAdded (item);
	
	if (IsAttached ()) {
		// queue a resort based on ZIndex
		GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
	}
}

void
Panel::ElementRemoved (UIElement *item)
{
	FrameworkElement::ElementRemoved (item);
	
	if (IsAttached ()) {
		// queue a resort based on ZIndex
		GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
	}
}


void
Panel::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (PropertyHasValueNoAutoCreate (Panel::ChildrenProperty, col)) {
		// if a child changes its ZIndex or Z property we need to resort our Children
		if (args->GetId () == Canvas::ZIndexProperty || args->GetId () == Canvas::ZProperty) {
			((UIElement *) obj)->Invalidate ();
			if (IsAttached ()) {
				// queue a resort based on ZIndex
				GetDeployment ()->GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
			}
			return;
		}
	}

	FrameworkElement::OnCollectionItemChanged (col, obj, args);
}


};
