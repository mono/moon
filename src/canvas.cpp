/*
 * canvas.cpp: canvas definitions.
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <gtk/gtk.h>

#include "brush.h"
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
	// Compute left/top if its attached to the item
	Value *val_top = item->GetValue (Canvas::TopProperty);
	double top = val_top == NULL ? 0.0 : val_top->AsDouble();

	Value *val_left = item->GetValue (Canvas::LeftProperty);
	double left = val_left == NULL ? 0.0 : val_left->AsDouble();
		
	cairo_matrix_init_translate (result, left, top);
}

void
Canvas::ComputeBounds ()
{
	Surface *surface = GetSurface ();
	if (surface && surface->IsTopLevel (this)) {
		// toplevel canvas don't subscribe to the same bounds computation as others
		bounds = Rect (0, 0, surface->GetWidth(), surface->GetHeight());
	}
	else {
		Panel::ComputeBounds ();
	}
}

bool
Canvas::OnChildPropertyChanged (DependencyProperty *prop, DependencyObject *child)
{
	if (prop == TopProperty || prop == LeftProperty) {
		//
		// Technically the canvas cares about Visuals, but we cant do much
		// with them, all the logic to relayout is in UIElement
		//
		if (!Type::Find (child->GetObjectType ())->IsSubclassOf (Type::UIELEMENT)){
			printf ("Child %s is not a UIELEMENT\n", dependency_object_get_name (child));
			return false;
		}
		UIElement *ui = (UIElement *) child;

		ui->UpdateTransform ();

		return true;
	}
	
	return false;
}

Point
Canvas::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	return Point (framework_element_get_width (this) * user_xform_origin.x, 
		      framework_element_get_height (this) * user_xform_origin.y);
}

Canvas *
canvas_new (void)
{
	return new Canvas ();
}

DependencyProperty* Canvas::TopProperty;
DependencyProperty* Canvas::LeftProperty;

void 
canvas_init (void)
{
	Canvas::TopProperty = DependencyObject::RegisterFull (Type::CANVAS, "Top", new Value (0.0), Type::DOUBLE, true, false);
	Canvas::LeftProperty = DependencyObject::RegisterFull (Type::CANVAS, "Left", new Value (0.0), Type::DOUBLE, true, false);
}


