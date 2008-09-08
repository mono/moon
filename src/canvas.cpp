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
	NameScope *ns = new NameScope ();
	ns->SetTemporary (true);
	NameScope::SetNameScope (this, ns);
	ns->unref ();
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
		extents = Rect (0, 0, GetWidth (), GetHeight ());
		bounds = Rect (0, 0, surface->GetWindow()->GetWidth(), surface->GetWindow()->GetHeight());
		bounds_with_children = Rect (0, 0, surface->GetWindow()->GetWidth(), surface->GetWindow()->GetHeight());
	}
	else {
		Panel::ComputeBounds ();
	}
}

void
Canvas::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::CANVAS) {
		Panel::OnPropertyChanged (args);
		return;
	}

	if (args->property == Canvas::TopProperty || args->property == Canvas::LeftProperty) {
		if (GetVisualParent () == NULL)
			UpdateTransform ();
	}
	NotifyListenersOfPropertyChange (args);
}

void
Canvas::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (subobj_args->property == Canvas::TopProperty || subobj_args->property == Canvas::LeftProperty) {
		//
		// Technically the canvas cares about Visuals, but we cant do much
		// with them, all the logic to relayout is in UIElement
		//
		if (!Type::Find (obj->GetObjectType ())->IsSubclassOf (Type::UIELEMENT)){
			printf ("Child %s is not a UIELEMENT\n", obj ? obj->GetName () : NULL);
			return;
		}
		UIElement *ui = (UIElement *) obj;

		if (moonlight_flags & RUNTIME_INIT_USE_UPDATE_POSITION)
			ui->UpdatePosition ();
		else
			ui->UpdateTransform ();
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

Point
Canvas::GetTransformOrigin ()
{
	Point *user_xform_origin = GetRenderTransformOrigin ();
	
	return Point (GetWidth () * user_xform_origin->x, 
		      GetHeight () * user_xform_origin->y);
}

Size
Canvas::MeasureOverride (Size availableSize)
{
	Size result = FrameworkElement::MeasureOverride (availableSize);

	// XXX ugly hack to maintain compat
	if (!GetVisualParent ())
		return result;

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ())
		child->Measure (Size (INFINITY, INFINITY));
	
	return result;
}

Size 
Canvas::ArrangeOverride (Size finalSize)
{
	Size result = FrameworkElement::ArrangeOverride (finalSize);

	// XXX ugly hack to maintain compat
	if (!GetVisualParent ())
		return result;

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		Size child_desired = child->GetDesiredSize ();
		Rect child_final = Rect (GetLeft (child), GetTop (child),
					 child_desired.width, child_desired.height);

		child->Arrange (child_final);
		// XXX fill layout slot?
	}

	return result;
}
