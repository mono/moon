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


DependencyProperty *Canvas::TopProperty;
DependencyProperty *Canvas::LeftProperty;

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
	if (args->property->GetPropertyType() != Type::CANVAS) {
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
			printf ("Child %s is not a UIELEMENT\n", dependency_object_get_name (obj));
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

Point
Canvas::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	return Point (GetWidth () * user_xform_origin.x, 
		      GetHeight () * user_xform_origin.y);
}

void
Canvas::SetLeft (UIElement *item, double left)
{
	item->SetValue (Canvas::LeftProperty, Value (left));
}

double
Canvas::GetLeft (UIElement *item)
{
	Value *value = item->GetValue (Canvas::LeftProperty);
	
	return value ? value->AsDouble () : 0.0;
}

void
Canvas::SetTop (UIElement *item, double top)
{
	item->SetValue (Canvas::TopProperty, Value (top));
}

double
Canvas::GetTop (UIElement *item)
{
	Value *value = item->GetValue (Canvas::TopProperty);
	
	return value ? value->AsDouble () : 0.0;
}

void
Canvas::SetLeft (double left)
{
	SetValue (Canvas::LeftProperty, Value (left));
}

double
Canvas::GetLeft ()
{
	return GetValue (Canvas::LeftProperty)->AsDouble ();
}

void
Canvas::SetTop (double top)
{
	SetValue (Canvas::TopProperty, Value (top));
}

double
Canvas::GetTop ()
{
	return GetValue (Canvas::TopProperty)->AsDouble ();
}


Canvas *
canvas_new (void)
{
	return new Canvas ();
}

void 
canvas_init (void)
{
	Canvas::TopProperty = DependencyProperty::RegisterFull (Type::CANVAS, "Top", new Value (0.0), Type::DOUBLE, true, false);
	Canvas::LeftProperty = DependencyProperty::RegisterFull (Type::CANVAS, "Left", new Value (0.0), Type::DOUBLE, true, false);
}


