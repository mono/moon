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

Canvas::Canvas () : surface (NULL), mouse_over (NULL)
{
}

Surface*
Canvas::GetSurface ()
{
	if (surface)
		return surface;
	else
		return UIElement::GetSurface ();
}

void
Canvas::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	*result = absolute_xform;

	// Compute left/top if its attached to the item
	Value *val_top = item->GetValue (Canvas::TopProperty);
	double top = val_top == NULL ? 0.0 : val_top->AsDouble();

	Value *val_left = item->GetValue (Canvas::LeftProperty);
	double left = val_left == NULL ? 0.0 : val_left->AsDouble();
		
	cairo_matrix_translate (result, left, top);
}

void
Canvas::UpdateTransform ()
{
	VisualCollection *children = GetChildren ();
	UIElement::UpdateTransform ();
	Collection::Node *n;

	//printf ("Am the canvas, and the xform is: %g %g\n", absolute_xform.x0, absolute_xform.y0);
	n = (Collection::Node *) children->list->First ();
	while (n != NULL) {
		UIElement *item = (UIElement *) n->obj;
		
		item->UpdateTransform ();
		
		n = (Collection::Node *) n->Next ();
	}
}

#define DEBUG_BOUNDS 0

#if DEBUG_BOUNDS
void space (int n)
{
	for (int i = 0; i < n; i++)
		putchar (' ');
}
static int levelb = 0;
#endif

void
Canvas::ComputeBounds ()
{
	if (surface) {
		// toplevel canvas don't subscribe to the same bounds computation as others
		bounds = Rect (0, 0, surface->GetWidth(), surface->GetHeight());
	}
	else {
		VisualCollection *children = GetChildren ();
		Collection::Node *cn;
		bool first = true;

#if DEBUG_BOUNDS
		levelb += 4;
		space (levelb);
		printf ("Canvas: Enter ComputeBounds\n");
#endif
		cn = (Collection::Node *) children->list->First ();
		for ( ; cn != NULL; cn = (Collection::Node *) cn->Next ()) {
			UIElement *item = (UIElement *) cn->obj;

			/* if the element is collapsed, it doesn't figure into
			   layout, and therefore shouldn't figure into our
			   bounding box computation.  Hidden means it still
			   takes up space, just isn't drawn and doesn't take
			   part in HitTest stuff. */
			if (item->GetValue (UIElement::VisibilityProperty)->AsInt32() == VisibilityCollapsed)
				continue;

			Rect r = item->GetBounds ();

#if DEBUG_BOUNDS
			space (levelb + 4);
			printf ("Item (%s) bounds %g %g %g %g\n", 
				dependency_object_get_name (item),r.x, r.y, r.w, r.h);
#endif
			if (first) {
				bounds = r;
				first = false;
			}
			else {
				bounds = bounds.Union (r);
			}
		}

		// If we found nothing.
		double x1, x2, y1, y2;
		
		x1 = y1 = 0.0;
		x2 = framework_element_get_width (this);
		y2 = framework_element_get_height (this);

		if (x2 != 0.0 && y2 != 0.0) {
			cairo_matrix_transform_point (&absolute_xform, &x1, &y1);
			cairo_matrix_transform_point (&absolute_xform, &x2, &y2);

			Rect fw_rect = Rect (x1, y1, x2 - x1, y2 - y1);

			if (first)
				bounds = fw_rect;
			else
				bounds = bounds.Union (fw_rect);
		}

		/* standard "grow the rectangle by enough to cover our
		   asses because of cairo's floating point rendering"
		   thing */
		bounds.x -= 1;
		bounds.y -= 1;
		bounds.w += 2;
		bounds.h += 2;
	}
#if DEBUG_BOUNDS
	space (levelb);
	printf ("Canvas: Leave ComputeBounds (%g %g %g %g)\n", bounds.x, bounds.y, bounds.w, bounds.h);
	levelb -= 4;
#endif
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
	}
	
	return false;
}

bool
Canvas::InsideObject (cairo_t *cr, double x, double y)
{
	/* if we have explicitly set width/height, we check them */
	if (FrameworkElement::InsideObject (cr, x, y))
		return true;

	/* otherwise we try to figure out if we're inside one of our child elements */
	UIElement *mouseover = FindMouseOver (cr, x, y);

	return mouseover != NULL;
}

bool
Canvas::CheckOver (cairo_t *cr, UIElement *item, double x, double y)
{
	// if the item isn't visible, it's really easy
	if (!item->GetVisible ())
		return false;

	// if the item doesn't take part in hit testing, it's also easy
	if (!item->GetHitTestVisible ())
		return false;

	// first a quick bounds check
	if (!item->GetBounds().PointInside (x, y))
		return false;

	// then, if that passes, a more tailored shape check
	if (!item->InsideObject (cr, x, y))
		return false;

	return true;
}

UIElement *
Canvas::FindMouseOver (cairo_t *cr, double x, double y)
{
	VisualCollection *children = GetChildren ();
	Collection::Node *cn;

	// if we have a previous mouse_over, see if we're still in it
	// first, to make things a little quicker in the case where we
	// have a lot of children
	//
	if (mouse_over) {
		if (CheckOver (cr, mouse_over, x, y))
			return mouse_over;
	}

	// Walk the list in reverse order, since it's sorted in ascending z-index order
	//
	for (cn = (Collection::Node *) children->z_sorted_list->Last (); cn != NULL; cn = (Collection::Node *) cn->Prev ()) {
		UIElement *item = (UIElement *) cn->obj;

		// skip the mouse_over, since we already checked it above
		if (item == mouse_over)
			continue;

		if (CheckOver (cr, item, x, y))
			return item;
	}

	return NULL;
}

void
Canvas::HandleMotion (Surface *s, cairo_t *cr, int state, double x, double y, MouseCursor *cursor)
{
	UIElement *new_over = FindMouseOver (cr, x, y);

	if (new_over != mouse_over) {
		if (mouse_over)
			mouse_over->Leave (s);

		mouse_over = new_over;

		if (mouse_over)
			mouse_over->Enter (s, cr, state, x, y);
	}

	if (cursor && *cursor == MouseCursorDefault)
		*cursor = (MouseCursor)GetValue (UIElement::CursorProperty)->AsInt32();

	if (mouse_over)
		mouse_over->HandleMotion (s, cr, state, x, y, cursor);

	if (mouse_over || InsideObject (cr, x, y))
		UIElement::HandleMotion (s, cr, state, x, y, NULL);
}

void
Canvas::HandleButton (Surface *s, cairo_t *cr, callback_mouse_event cb, int state, double x, double y)
{
	// not sure if this is correct, but we don't bother updating
	// the current mouse_over here (and along with that, emitting
	// enter/leave events).
	if (mouse_over) {
		if (mouse_over->GetBounds().PointInside (x, y)
		    && mouse_over->InsideObject (cr, x, y)) {

			mouse_over->HandleButton (s, cr, cb, state, x, y);
		}
	}

	UIElement::HandleButton (s, cr, cb, state, x, y);
}

void
Canvas::Enter (Surface *s, cairo_t *cr, int state, double x, double y)
{
	mouse_over = FindMouseOver (cr, x, y);

	if (mouse_over)
		mouse_over->Enter (s, cr, state, x, y);
	  
	UIElement::Enter (s, cr, state, x, y);
}

void
Canvas::Leave (Surface *s)
{
	if (mouse_over) {
	       mouse_over->Leave (s);
	       mouse_over = NULL;
	}

	UIElement::Leave (s);
}

static int level = 0;

void
Canvas::Render (cairo_t *cr, int x, int y, int width, int height)
{
	VisualCollection *children = GetChildren ();
	Collection::Node *cn;

	cairo_save (cr);  // for UIElement::ClipProperty

	cairo_set_matrix (cr, &absolute_xform);

	Value *value = GetValue (Canvas::ClipProperty);
	if (value) {
		Geometry *geometry = value->AsGeometry ();
		geometry->Draw (cr);
		cairo_clip (cr);
	}

	value = GetValue (Panel::BackgroundProperty);
	if (value) {
		double fwidth = framework_element_get_width (this);
		double fheight = framework_element_get_height (this);

		if (fwidth > 0 && fheight > 0){
			Brush *background = value->AsBrush ();
			background->SetupBrush (cr, this);

			// FIXME - UIElement::Opacity may play a role here
			cairo_rectangle (cr, 0, 0, fwidth, fheight);
			cairo_fill (cr);
		}
	}

	Rect render_rect (x, y, width, height);

	level += 4;

	//
	// from this point on, we use the identity matrix to set the clipping
	// path for the children
	//
	cairo_identity_matrix (cr);
	cn = (Collection::Node *) children->z_sorted_list->First ();
	for ( ; cn != NULL; cn = (Collection::Node *) cn->Next ()) {
		UIElement *item = (UIElement *) cn->obj;

		if (!item->GetVisible())
			continue;
		
		//space (level);
		//printf ("%s %g %g %g %g\n", dependency_object_get_name (item), item->x1, item->y1, item->x2, item->y2);
		
		if (true || render_rect.IntersectsWith (item->GetBounds())) {
			Rect inter = render_rect.Intersection(item->GetBounds());
#if CAIRO_CLIP
#if TIME_CLIP
			STARTTIMER(clip, "cairo clip setup");
#endif
			cairo_save (cr);

			//printf ("Clipping to %g %g %g %g\n", inter.x, inter.y, inter.w, inter.h);
			// at the very least we need to clip based on the expose area.
			// there's also a UIElement::ClipProperty
			cairo_rectangle (cr, inter.x, inter.y, inter.w + 2, inter.h + 2);
			cairo_clip (cr);
#if TIME_CLIP
			ENDTIMER(clip, "cairo clip setup");
#endif
#endif
			item->DoRender (cr, (int)inter.x, (int)inter.y, (int)inter.w + 2, (int)inter.h + 2);

#if CAIRO_CLIP
#if TIME_CLIP
			STARTTIMER(endclip, "cairo clip teardown");
#endif			
			cairo_restore (cr);

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
	//draw_grid (cr);

 leave:
	level -= 4;
	cairo_restore (cr);
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


