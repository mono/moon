/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * canvas.cpp: canvas definitions.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <math.h>

#include "rect.h"
#include "canvas.h"
#include "runtime.h"
#include "namescope.h"
#include "collection.h"
#include "window.h"
#include "deployment.h"

namespace Moonlight {

Canvas::Canvas ()
{
	SetObjectType (Type::CANVAS);
	bounds = Rect (0,0,0,0);
}

void
Canvas::ComputeBounds ()
{
	Surface *surface = GetDeployment ()->GetSurface ();
	Panel::ComputeBounds ();
	coverage_bounds = bounds;
	if (surface && IsAttached () && surface->IsTopLevel (this)) {
		// toplevel canvas don't subscribe to the same bounds computation as others
		bounds = extents = Rect (0, 0, surface->GetWindow()->GetWidth(), surface->GetWindow()->GetHeight());
		bounds_with_children = extents_with_children = Rect (0, 0, surface->GetWindow()->GetWidth(), surface->GetWindow()->GetHeight());

		ComputeGlobalBounds ();
		ComputeSurfaceBounds ();
	}
}

Rect
Canvas::GetCoverageBounds ()
{
	Brush *background = GetBackground ();
	
	if (background && background->IsOpaque ())
		return coverage_bounds;

	return Rect ();
}

void
Canvas::ShiftPosition (Point p)
{
	Surface *surface = GetDeployment ()->GetSurface ();
	if (surface && IsAttached () && surface->IsTopLevel (this)) {
		ComputeBounds ();
	} else {
		Panel::ShiftPosition (p);
	}
} 

void
Canvas::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::CANVAS) {
		Panel::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == Canvas::TopProperty 
	    || args->GetId () == Canvas::LeftProperty) {
		if (GetVisualParent () == NULL) {
			UpdateTransform ();
			InvalidateArrange ();
		}
	}
	NotifyListenersOfPropertyChange (args, error);
}

bool
Canvas::IsLayoutContainer ()
{
	Types *types = Deployment::GetCurrent ()->GetTypes ();

	DeepTreeWalker walker = DeepTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (!types->IsSubclassOf (child->GetObjectType (), Type::CANVAS) && child->IsLayoutContainer ())
			return true;
	}

	return false;
}

Size
Canvas::MeasureOverrideWithError (Size availableSize, MoonError *error)
{
	Size childSize = Size (INFINITY, INFINITY); 

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		child->MeasureWithError (childSize, error);
	}

	Size desired = Size (0,0);

	return desired;
}

Size 
Canvas::ArrangeOverrideWithError (Size finalSize, MoonError *error)
{
	VisualTreeWalker walker = VisualTreeWalker (this);
	while (FrameworkElement *child = (FrameworkElement *)walker.Step ()) {
		Size desired = child->GetDesiredSize ();
		Rect child_final = Rect (GetLeft (child), GetTop (child),
					 desired.width, desired.height);
		child->ArrangeWithError (child_final, error);
		//child->ClearValue (LayoutInformation::LayoutClipProperty);
	}

	return finalSize;
}

void
Canvas::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (PropertyHasValueNoAutoCreate (Panel::ChildrenProperty, col)) {
		// this used to contain ZIndex property checking, but
		// it has been moved to Panel's implementation, since
		// all panels allow ZIndex sorting of children.
		if (args->GetId () == Canvas::TopProperty ||
		    args->GetId () == Canvas::LeftProperty) {
			FrameworkElement *child = (FrameworkElement *) obj;
			
			Size desired = child->GetDesiredSize ();
			Rect child_final = Rect (GetLeft (child), GetTop (child),
						 desired.width, desired.height);

			if (child->GetUseLayoutRounding ()) {
				child_final.x = round (child_final.x);
				child_final.y = round (child_final.y);
				child_final.width = round (child_final.width);
				child_final.height = round (child_final.height);
			}

			LayoutInformation::SetLayoutSlot (child, &child_final);
			child->InvalidateArrange ();
			return;
		}
	}

	Panel::OnCollectionItemChanged (col, obj, args);
}

};
