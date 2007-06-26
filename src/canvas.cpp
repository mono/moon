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

#include "geometry.h"
#include "brush.h"
#include "rect.h"
#include "canvas.h"
#include "runtime.h"
#include "collection.h"

Canvas::Canvas () : surface (NULL), current_item (NULL)
{
}

void
Canvas::get_xform_for (UIElement *item, cairo_matrix_t *result)
{
	*result = absolute_xform;

	// This is a hack to pick up the position allcation offset of the gtk widget
	if (surface != NULL && surface->drawing_area != NULL && GTK_WIDGET_NO_WINDOW (surface->drawing_area))
	        cairo_matrix_translate (result, surface->drawing_area->allocation.x, surface->drawing_area->allocation.y);

	// Compute left/top if its attached to the item
	Value *val_top = item->GetValue (Canvas::TopProperty);
	double top = val_top == NULL ? 0.0 : val_top->AsDouble();

	Value *val_left = item->GetValue (Canvas::LeftProperty);
	double left = val_left == NULL ? 0.0 : val_left->AsDouble();
		
	cairo_matrix_translate (result, left, top);
}

void
Canvas::update_xform ()
{
	VisualCollection *children = GetChildren ();
	UIElement::update_xform ();
	Collection::Node *n;

	//printf ("Am the canvas, and the xform is: %g %g\n", absolute_xform.x0, absolute_xform.y0);
	n = (Collection::Node *) children->list->First ();
	while (n != NULL) {
		UIElement *item = (UIElement *) n->obj;
		
		item->update_xform ();
		
		n = (Collection::Node *) n->Next ();
	}
}

void space (int n)
{
	for (int i = 0; i < n; i++)
		putchar (' ');
}
static int levelb = 0;

void
Canvas::getbounds ()
{
	VisualCollection *children = GetChildren ();
	Collection::Node *cn;
	bool first = true;
	GList *il;

	//levelb += 4;
	//space (levelb);
	//printf ("Canvas: Enter GetBounds\n");
	cn = (Collection::Node *) children->list->First ();
	for ( ; cn != NULL; cn = (Collection::Node *) cn->Next ()) {
		UIElement *item = (UIElement *) cn->obj;
		
		item->getbounds ();

		//space (levelb + 4);
		//printf ("Item (%s) bounds %g %g %g %g\n", 
		//dependency_object_get_name (item),item->x1, item->y1, item->x2, item->y2);

		//
		// The topmost canvas does not use the children
		// boundaries, it only triggers their computation
		//
		if (surface)
			continue;

		if (first) {
			x1 = item->x1;
			x2 = item->x2;
			y1 = item->y1;
			y2 = item->y2;
			first = false;
			continue;
		} 

		if (item->x1 < x1)
			x1 = item->x1;
		if (item->x2 > x2)
			x2 = item->x2;
		if (item->y1 < y1)
			y1 = item->y1;
		if (item->y2 > y2)
			y2 = item->y2;
	}

	if (surface) {
		x1 = y1 = 0;
		x2 = surface->width;
		y2 = surface->height;
		//printf ("Canvas: Leave GetBounds (%g %g %g %g)\n", x1, y1, x2, y2);
	} else {
		// If we found nothing.
		if (first)
			x1 = y1 = x2 = y2 = 0;
	}
	//space (levelb);
	//printf ("Canvas: Leave GetBounds (%g %g %g %g)\n", x1, y1, x2, y2);
	//levelb -= 4;
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
			printf ("Child %d is not a UIELEMENT\n");
			return false;
		}
		UIElement *ui = (UIElement *) child;
		ui->FullInvalidate (true);
	}
	
	return false;
}

bool
Canvas::handle_motion (Surface *s, int state, double x, double y)
{
	VisualCollection *children = GetChildren ();
	bool handled = false;
	Collection::Node *cn;

	// 
	// Walk the list in reverse
	//
	if (!(cn = (Collection::Node *) children->list->Last ()))
		goto leave;
	
	for ( ; cn != NULL; cn = (Collection::Node *) cn->Prev ()) {
		UIElement *item = (UIElement *) cn->obj;
		
		// Bounds check:
		if (x < item->x1 || x > item->x2 || y < item->y1 || y > item->y2)
			continue;
		
		handled = item->handle_motion (s, state, x, y);
		if (handled){
			if (item != current_item){
				if (current_item != NULL)
					current_item->leave (s);

				current_item = item;
				current_item->enter (s, state, x, y);
			}
			goto leave;
		} 
	}

	if (current_item != NULL){
		current_item->leave (s);
		current_item = NULL;
	}

 leave:
	if (handled || inside_object (s, x, y)){
		s->cb_motion (this, state, x, y);
		return true;
	}
	
	return handled;
}

bool
Canvas::handle_button (Surface *s, callback_mouse_event cb, int state, double x, double y)
{
	VisualCollection *children = GetChildren ();
	bool handled = false;
	Collection::Node *cn;
	
	// 
	// Walk the list in reverse
	//
	if (!(cn = (Collection::Node *) children->list->Last ()))
		goto leave;
	
	for ( ; cn != NULL; cn = (Collection::Node *) cn->Prev ()) {
		UIElement *item = (UIElement *) cn->obj;
		
		// Quick bound check:
		if (x < item->x1 || x > item->x2 || y < item->y1 || y > item->y2)
			continue;
		
		handled = item->handle_button (s, cb, state, x, y);
		if (handled)
			break;
	}
	
 leave:
	if (handled || inside_object (s, x, y)){
		cb (this, state, x, y);
		return true;
	}
	
	return handled;
}

void
Canvas::leave (Surface *s)
{
	if (current_item != NULL){
	       current_item->leave (s);
	       current_item = NULL;
	}
	s->cb_mouse_leave (this);
}

static int level = 0;

void
Canvas::render (Surface *s, int x, int y, int width, int height)
{
	VisualCollection *children = GetChildren ();
	Collection::Node *cn;
	double actual [6];

	cairo_save (s->cairo);  // for UIElement::ClipProperty

	cairo_set_matrix (s->cairo, &absolute_xform);

	Value *value = GetValue (Canvas::ClipProperty);
	if (value) {
		Geometry *geometry = value->AsGeometry ();
		geometry->Draw (s);
		cairo_clip (s->cairo);
	}

	value = GetValue (Panel::BackgroundProperty);
	if (value) {
		double fwidth = framework_element_get_width (this);
		double fheight = framework_element_get_height (this);

		if (fwidth > 0 && fheight > 0){
			Brush *background = value->AsBrush ();
			background->SetupBrush (s->cairo, this);

			// FIXME - UIElement::Opacity may play a role here
			cairo_rectangle (s->cairo, 0, 0, fwidth, fheight);
			cairo_fill (s->cairo);
		}
	}

	Rect render_rect (x, y, width, height);

	level += 4;

	//
	// from this point on, we use the identity matrix to set the clipping
	// path for the children
	//
	cairo_identity_matrix (s->cairo);
	cn = (Collection::Node *) children->z_sorted_list->First ();
	for ( ; cn != NULL; cn = (Collection::Node *) cn->Next ()) {
		UIElement *item = (UIElement *) cn->obj;

		if (item->GetValue (UIElement::VisibilityProperty)->AsInt32() != VisibilityVisible)
			goto leave;
		
		Rect item_rect (item->x1, item->y1, item->x2 - item->x1, item->y2 - item->y1);
		
		//space (level);
		//printf ("%s %g %g %g %g\n", dependency_object_get_name (item), item->x1, item->y1, item->x2, item->y2);
		
		if (true || render_rect.IntersectsWith (item_rect)) {
			Rect inter = render_rect.Intersection(item_rect);
#if CAIRO_CLIP
#if TIME_CLIP
			STARTTIMER(clip, "cairo clip setup");
#endif
			cairo_save (s->cairo);

			//printf ("Clipping to %g %g %g %g\n", inter.x, inter.y, inter.w, inter.h);
			// at the very least we need to clip based on the expose area.
			// there's also a UIElement::ClipProperty
			cairo_rectangle (s->cairo, inter.x, inter.y, inter.w + 2, inter.h + 2);
			cairo_clip (s->cairo);
#if TIME_CLIP
			ENDTIMER(clip, "cairo clip setup");
#endif
#endif
			item->dorender (s, (int)inter.x, (int)inter.y, (int)inter.w + 2, (int)inter.h + 2);

#if CAIRO_CLIP
#if TIME_CLIP
			STARTTIMER(endclip, "cairo clip teardown");
#endif			
			cairo_restore (s->cairo);

#if TIME_CLIP
			ENDTIMER(endclip, "cairo clip teardown");
#endif
#endif
		}
#ifdef DEBUG_INVALIDATE
		else {
			printf ("skipping object %p (%s)\n", item, Type::Find(item->GetObjectType())->name);
		}
#endif

	}
	//printf ("RENDER: LEAVE\n");
	//draw_grid (s->cairo);

 leave:
	level -= 4;
	cairo_restore (s->cairo);
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
	Canvas::TopProperty = DependencyObject::RegisterFull (Type::CANVAS, "Top", new Value (0.0), Type::DOUBLE, true);
	Canvas::LeftProperty = DependencyObject::RegisterFull (Type::CANVAS, "Left", new Value (0.0), Type::DOUBLE, true);
}


