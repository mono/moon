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
#include "deployment.h"

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
Canvas::MeasureOverride (Size availableSize)
{
	Size childSize = Size (INFINITY, INFINITY); 

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (child->GetVisibility () != VisibilityVisible)
			continue;
		
		if (child->IsContainer ())
			child->Measure (childSize);
		else {
			if (child->dirty_flags & DirtyMeasure) {
				child->dirty_flags &= ~DirtyMeasure;
				child->InvalidateArrange ();
			}
		}
	}

	Size desired = Size (0,0);
	

	return desired;
}

Size 
Canvas::ArrangeOverride (Size finalSize)
{
	// XXX ugly hack to maintain compat
	//if (!GetVisualParent() && !GetSurface ())
	//	return arranged;

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (FrameworkElement *child = (FrameworkElement *)walker.Step ()) {
		if (child->GetVisibility () != VisibilityVisible)
			continue;

		if (child->IsContainer ()) {
			Size desired = child->GetDesiredSize ();
			Rect child_final = Rect (GetLeft (child), GetTop (child), desired.width, desired.height);
			child->Arrange (child_final);
		} else {
			//g_warning ("arranging %s %g,%g", child->GetTypeName (),child->GetActualWidth (), child->GetActualHeight ());
			Rect child_final = Rect (GetLeft (child), GetTop (child), 
						 child->GetActualWidth (),
						 child->GetActualHeight ());
			child->Arrange (child_final);
		}
	}

	return finalSize;
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
			
			ui->InvalidateSubtreePaint ();
			ui->InvalidateArrange ();
			Rect *last = LayoutInformation::GetLayoutSlot (ui);
			if (last) {
				Rect updated = Rect (GetLeft (ui), 
						     GetTop (ui), 
						     last->width, last->height);

				if (args->GetId () == Canvas::TopProperty)
					updated.y = args->GetNewValue()->AsDouble ();
				else
					updated.x = args->GetNewValue()->AsDouble ();

				LayoutInformation::SetLayoutSlot (ui, &updated);
			} else {	
				InvalidateArrange ();
			}
			UpdateBounds ();
			return;
		}
	}

	Panel::OnCollectionItemChanged (col, obj, args);
}
