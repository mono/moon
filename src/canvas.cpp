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
	SetObjectType (Type::CANVAS);
}

void
Canvas::ComputeBounds ()
{
	Surface *surface = GetSurface ();
	Panel::ComputeBounds ();
	if (surface && surface->IsTopLevel (this)) {
		// toplevel canvas don't subscribe to the same bounds computation as others
		bounds = Rect (0, 0, surface->GetWindow()->GetWidth(), surface->GetWindow()->GetHeight());
		bounds_with_children = Rect (0, 0, surface->GetWindow()->GetWidth(), surface->GetWindow()->GetHeight());
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
Canvas::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::CANVAS) {
		Panel::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == Canvas::TopProperty 
	    || args->GetId () == Canvas::LeftProperty) {
		if (GetVisualParent () == NULL)
			UpdateTransform ();
	}
	NotifyListenersOfPropertyChange (args);
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
	Size childSize = Size (INFINITY, INFINITY); 

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (child->GetVisibility () != VisibilityVisible)
			continue;
		
		child->Measure (childSize);
	}

	return FrameworkElement::MeasureOverride (availableSize);
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
		if (child->GetVisibility () != VisibilityVisible)
			continue;

		Size desired = child->GetDesiredSize ();
		Rect child_final = Rect (GetLeft (child), GetTop (child), desired.width, desired.height);

		child->Arrange (child_final);
	}

	return result;
}

void
Canvas::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
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
		else if (args->GetId () == Canvas::TopProperty ||
			 args->GetId () == Canvas::LeftProperty) {

			UIElement *ui = (UIElement *) obj;

			ui->UpdateTransform ();
			ui->InvalidateArrange ();
			return;
		}
	}

	Panel::OnCollectionItemChanged (col, obj, args);
}
