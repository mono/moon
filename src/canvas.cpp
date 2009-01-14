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
#include "rect.h"
#include "canvas.h"
#include "runtime.h"
#include "namescope.h"
#include "collection.h"

Canvas::Canvas ()
{
}

void
Canvas::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	double left = Canvas::GetLeft (item);
	double top = Canvas::GetTop (item);
	
	cairo_matrix_init_translate (result, left, top);
}

void
Canvas::ComputeBounds ()
{
	Surface *surface = GetSurface ();
	if (surface && surface->IsTopLevel (this)) {
		// toplevel canvas don't subscribe to the same bounds computation as others
		extents = Rect (0, 0, 
				isnan (GetWidth ()) ? 0.0 : GetWidth (), 
				isnan (GetHeight ()) ? 0.0 : GetHeight ());

		bounds = Rect (0, 0, surface->GetWindow()->GetWidth(), surface->GetWindow()->GetHeight());
		bounds_with_children = Rect (0, 0, surface->GetWindow()->GetWidth(), surface->GetWindow()->GetHeight());
	}
	else {
		Panel::ComputeBounds ();
	}
}

void
Canvas::ShiftPosition (Point p)
{
	Surface *surface = GetSurface ();
	if (surface && surface->IsTopLevel (this)) {
		ComputeBounds ();
	} else {
		Panel::ShiftPosition (p);
	}
} 

void
Canvas::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::CANVAS) {
		Panel::OnPropertyChanged (args);
		return;
	}

	if (args->property == Canvas::TopProperty 
	    || args->property == Canvas::LeftProperty) {
		if (GetVisualParent () == NULL)
			UpdateTransform ();
	}
	NotifyListenersOfPropertyChange (args);
}

void
Canvas::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (subobj_args->property == Canvas::TopProperty ||
	    subobj_args->property == Canvas::LeftProperty) {
		//
		// Technically the canvas cares about Visuals, but we cant do much
		// with them, all the logic to relayout is in UIElement
		//
		if (!Type::Find (obj->GetObjectType ())->IsSubclassOf (Type::UIELEMENT)){
			printf ("Child %s is not a UIELEMENT\n", obj ? obj->GetName () : NULL);
			return;
		}
		UIElement *ui = (UIElement *) obj;

		ui->UpdateTransform ();
		ui->InvalidateMeasure ();
	}
	else
		Panel::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
Canvas::OnLoaded ()
{
	UIElement::OnLoaded ();

	if (GetSurface ()) {
		// queue a resort based on ZIndex
		GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
	}
}

void
Canvas::ElementAdded (UIElement *item)
{
	Panel::ElementAdded (item);
	
	if (GetSurface ()) {
		// queue a resort based on ZIndex
		GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
	}
}

void
Canvas::ElementRemoved (UIElement *item)
{
	Panel::ElementRemoved (item);
	
	if (GetSurface ()) {
		// queue a resort based on ZIndex
		GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
	}
}

Size
Canvas::MeasureOverride (Size availableSize)
{
	Size result = FrameworkElement::MeasureOverride (availableSize);
	Size childSize = Size (INFINITY, INFINITY); 

	// XXX ugly hack to maintain compat
	if (result.IsEmpty ())
		return result;

	//if (availableSize.width <= 0.0 && availableSize.height <= 0.0)
	//	childSize = Size (0.0, 0.0);

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ())
		child->Measure (childSize);
	
	return result;
}

Size 
Canvas::ArrangeOverride (Size finalSize)
{
	Size result = FrameworkElement::ArrangeOverride (finalSize);

	// XXX ugly hack to maintain compat
	if (!GetVisualParent() && !GetSurface ())
		return result;

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		Size arranged = finalSize.Max (child->GetDesiredSize ());
		Rect child_final = Rect (GetLeft (child), GetTop (child),
					 arranged.width, arranged.height);
		child->Arrange (child_final);
		// XXX fill layout slot?
	}

	return result;
}

void
Canvas::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (col == GetChildren()) {
		// if a child changes its ZIndex property we need to resort our Children
		if (args->property == Canvas::ZIndexProperty) {
			((UIElement *) obj)->Invalidate ();
			if (GetSurface ()) {
				// queue a resort based on ZIndex
				GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
			}
		}
	} else {
		FrameworkElement::OnCollectionItemChanged (col, obj, args);
	}
}
