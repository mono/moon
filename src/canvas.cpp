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

	Size desired = Size (0,0);
	Size specified = Size (GetWidth (), GetHeight ());
	
	desired = desired.Max (specified);
	desired = desired.Min (specified);

	return desired.Min (availableSize);
}

Size 
Canvas::ArrangeOverride (Size finalSize)
{
	Size specified = Size (GetWidth (), GetHeight ());
	Size arranged = finalSize;

	arranged = arranged.Max (specified);
	arranged = arranged.Min (specified);

	// XXX ugly hack to maintain compat
	if (!GetVisualParent() && !GetSurface ())
		return arranged;

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (child->GetVisibility () != VisibilityVisible)
			continue;

		Size desired = child->GetDesiredSize ();
		Rect child_final = Rect (GetLeft (child), GetTop (child), desired.width, desired.height);

		child->Arrange (child_final);
	}

	return arranged;
}

void
Canvas::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (col == GetChildren()) {
		// this used to contain ZIndex property checking, but
		// it has been moved to Panel's implementation, since
		// all panels allow ZIndex sorting of children.
		if (args->GetId () == Canvas::TopProperty ||
			 args->GetId () == Canvas::LeftProperty) {

			UIElement *ui = (UIElement *) obj;

			ui->UpdateTransform ();
			ui->InvalidateArrange ();
			return;
		}
	}

	Panel::OnCollectionItemChanged (col, obj, args);
}
