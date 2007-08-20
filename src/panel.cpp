/*
 * panel.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <gtk/gtk.h>

#include "geometry.h"
#include "panel.h"
#include "brush.h"
#include "collection.h"

VisualCollection *
Panel::GetChildren ()
{
	return GetValue (Panel::ChildrenProperty)->AsVisualCollection();
}

void
Panel::SetChildren (VisualCollection *col)
{
	SetValue (Panel::ChildrenProperty, col);
}

void 
panel_child_add (Panel *panel, UIElement *child)
{
	panel->GetChildren()->Add (child);
}

Brush *
panel_get_background (Panel *panel)
{
	Value *value = panel->GetValue (Panel::BackgroundProperty);
	
	return value ? (Brush *) value->AsBrush () : NULL;
}

Panel *
panel_new (void)
{
	return new Panel ();
}

Panel::Panel ()
{
	this->SetValue (Panel::ChildrenProperty, Value (new VisualCollection ()));
	background = NULL;
	mouse_over = NULL;
	ignore_invalidates = false;
}

Panel::~Panel ()
{
	if (background != NULL) {
		background->Detach (NULL, this);
		background->unref ();
	}
}

void
Panel::UpdateTotalOpacity ()
{
	VisualCollection *children = GetChildren ();
	FrameworkElement::UpdateTotalOpacity ();
	Collection::Node *n;

	ignore_invalidates = true;

	//printf ("Am the canvas, and the xform is: %g %g\n", absolute_xform.x0, absolute_xform.y0);
	n = (Collection::Node *) children->list->First ();
	while (n != NULL) {
		UIElement *item = (UIElement *) n->obj;
		
		item->UpdateTotalOpacity ();
		
		n = (Collection::Node *) n->Next ();
	}

	ignore_invalidates = false;
}

void
Panel::UpdateTransform ()
{
	VisualCollection *children = GetChildren ();
	FrameworkElement::UpdateTransform ();
	Collection::Node *n;

	ignore_invalidates = true;

	//printf ("Am the canvas, and the xform is: %g %g\n", absolute_xform.x0, absolute_xform.y0);
	n = (Collection::Node *) children->list->First ();
	while (n != NULL) {
		UIElement *item = (UIElement *) n->obj;
		
		item->UpdateTransform ();
		
		n = (Collection::Node *) n->Next ();
	}

	ignore_invalidates = false;
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
Panel::ComputeBounds ()
{
	VisualCollection *children = GetChildren ();
	Collection::Node *cn;
	bool first = true;

#if DEBUG_BOUNDS
	levelb += 4;
	space (levelb);
	printf ("Panel: Enter ComputeBounds\n");
#endif
	cn = (Collection::Node *) children->list->First ();
	for ( ; cn != NULL; cn = (Collection::Node *) cn->Next ()) {
		UIElement *item = (UIElement *) cn->obj;

		// if the item doesn't take part in layout
		// calculations, skip it
		if (!item->GetVisible ())
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

		Rect fw_rect = bounding_rect_for_transformed_rect (&absolute_xform,
								   Rect (x1,y1,x2,y2));

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

#if DEBUG_BOUNDS
	space (levelb);
	printf ("Panel: Leave ComputeBounds (%g %g %g %g)\n", bounds.x, bounds.y, bounds.w, bounds.h);
	levelb -= 4;
#endif
}


void
Panel::ChildInvalidated (UIElement *child, Rect r)
{
  	if (ignore_invalidates)
  		return;

	FrameworkElement::Invalidate (r);
}

static int level = 0;

//#define DEBUG_INVALIDATE 1

void
Panel::Render (cairo_t *cr, int x, int y, int width, int height)
{
	VisualCollection *children = GetChildren ();
	
	cairo_save (cr);  // for UIElement::ClipProperty

	cairo_set_matrix (cr, &absolute_xform);

	Value *value = GetValue (UIElement::ClipProperty);
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
	for (guint i = 0; i < children->z_sorted->len; i++) {
		UIElement *item = (UIElement *) children->z_sorted->pdata[i];
		
		if (!item->GetVisible()) {
#ifdef DEBUG_INVALIDATE
			printf ("skipping invisible object %s: %p (%s)\n", item->GetName (), item, item->GetTypeName());
#endif
			continue;
		}

		//space (level);
		//printf ("%s %g %g %g %g\n", dependency_object_get_name (item), item->x1, item->y1, item->x2, item->y2);
		
		if (render_rect.IntersectsWith (item->GetBounds())) {
			Rect inter = render_rect.Intersection(item->GetBounds());
#if CAIRO_CLIP
#if TIME_CLIP
			STARTTIMER(clip, "cairo clip setup");
#endif
			cairo_save (cr);

			//printf ("Clipping to %g %g %g %g\n", inter.x, inter.y, inter.w, inter.h);
			// at the very least we need to clip based on the expose area.
			// there's also a UIElement::ClipProperty
			cairo_rectangle (cr, inter.x-1, inter.y-1, inter.w + 2, inter.h + 2);
			cairo_clip (cr);
#if TIME_CLIP
			ENDTIMER(clip, "cairo clip setup");
#endif
#endif
			item->DoRender (cr, (int)inter.x-1, (int)inter.y-1, (int)inter.w + 2, (int)inter.h + 2);

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
			printf ("skipping object %s: %p (%s)\n", item->GetName(), item, item->GetTypeName());
		}
#endif

	}
	//printf ("RENDER: LEAVE\n");
	//draw_grid (cr);

	level -= 4;
	cairo_restore (cr);
}

bool
Panel::InsideObject (cairo_t *cr, double x, double y)
{
	/* if we have explicitly set width/height, we check them */
	if (FrameworkElement::InsideObject (cr, x, y)) {
		return true;
	}

	/* otherwise we try to figure out if we're inside one of our child elements */
	UIElement *mouseover = FindMouseOver (cr, x, y);

	return mouseover != NULL;
}

bool
Panel::CheckOver (cairo_t *cr, UIElement *item, double x, double y)
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
Panel::FindMouseOver (cairo_t *cr, double x, double y)
{
	VisualCollection *children = GetChildren ();
	
	// Walk the list in reverse order, since it's sorted in ascending z-index order
	//
	for (guint i = children->z_sorted->len; i > 0; i--) {
		UIElement *item = (UIElement *) children->z_sorted->pdata[i - 1];

		if (CheckOver (cr, item, x, y)) {
			//printf (" mouse_over = %s\n", item->GetName());
			return item;
		}
	}

	return NULL;
}

void
Panel::HandleMotion (cairo_t *cr, int state, double x, double y, MouseCursor *cursor)
{
	UIElement *new_over = FindMouseOver (cr, x, y);

	if (new_over != mouse_over) {
		if (mouse_over)
			mouse_over->Leave ();

		mouse_over = new_over;

		if (mouse_over)
			mouse_over->Enter (cr, state, x, y);
	}

	if (cursor && *cursor == MouseCursorDefault)
		*cursor = (MouseCursor)GetValue (UIElement::CursorProperty)->AsInt32();

	if (mouse_over)
		mouse_over->HandleMotion (cr, state, x, y, cursor);

	if (mouse_over || InsideObject (cr, x, y))
		FrameworkElement::HandleMotion (cr, state, x, y, NULL);
}

void
Panel::HandleButtonPress (cairo_t *cr, int state, double x, double y)
{
	// not sure if this is correct, but we don't bother updating
	// the current mouse_over here (and along with that, emitting
	// enter/leave events).
	if (mouse_over) {
		if (mouse_over->GetBounds().PointInside (x, y)
		    && mouse_over->InsideObject (cr, x, y)) {

			mouse_over->HandleButtonPress (cr, state, x, y);
		}
	}

	FrameworkElement::HandleButtonPress (cr, state, x, y);
}

void
Panel::HandleButtonRelease (cairo_t *cr, int state, double x, double y)
{
	// not sure if this is correct, but we don't bother updating
	// the current mouse_over here (and along with that, emitting
	// enter/leave events).
	if (mouse_over) {
		if (mouse_over->GetBounds().PointInside (x, y)
		    && mouse_over->InsideObject (cr, x, y)) {

			mouse_over->HandleButtonRelease (cr, state, x, y);
		}
	}

	FrameworkElement::HandleButtonRelease (cr, state, x, y);
}

void
Panel::Enter (cairo_t *cr, int state, double x, double y)
{
	mouse_over = FindMouseOver (cr, x, y);

	if (mouse_over)
		mouse_over->Enter (cr, state, x, y);
	  
	FrameworkElement::Enter (cr, state, x, y);
}

void
Panel::Leave ()
{
	if (mouse_over) {
	       mouse_over->Leave ();
	       mouse_over = NULL;
	}

	FrameworkElement::Leave ();
}

//
// Intercept any changes to the children property and mirror that into our
// own variable
//
void
Panel::OnPropertyChanged (DependencyProperty *prop)
{
	FrameworkElement::OnPropertyChanged (prop);

	if (prop == Panel::ChildrenProperty) {
		VisualCollection *newcol = GetValue (prop)->AsVisualCollection();
		
		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	} else if (prop == BackgroundProperty) {
		if (background != NULL) {
			background->Detach (NULL, this);
			background->unref ();
		}
		
		if ((background = panel_get_background (this)) != NULL) {
			background->Attach (NULL, this);
			background->ref ();
		}
		
		Invalidate ();
	}
}

void
Panel::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (prop == Panel::BackgroundProperty) {
		Invalidate ();
	}
	else {
		FrameworkElement::OnSubPropertyChanged (prop, subprop);
	}
}

void
Panel::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	// if a child changes its ZIndex property we need to resort our Children
	if (prop == UIElement::ZIndexProperty) {
		// FIXME: it would probably be faster to remove the
		// changed item and then re-add it using
		// g_ptr_array_insert_sorted() because
		// g_ptr_array_sort() uses QuickSort which has poor
		// performance on nearly-sorted input.
		GetChildren()->ResortByZIndex ();
	}
}


void
Panel::OnLoaded ()
{
	VisualCollection *children = GetChildren ();
	Collection::Node *cn;

	cn = (Collection::Node *) children->list->First ();
	for ( ; cn != NULL; cn = (Collection::Node *) cn->Next ()) {
		UIElement *item = (UIElement *) cn->obj;

		item->OnLoaded ();
	}

	FrameworkElement::OnLoaded ();
}


DependencyProperty* Panel::ChildrenProperty;
DependencyProperty* Panel::BackgroundProperty;

void 
panel_init (void)
{
	Panel::ChildrenProperty = DependencyObject::Register (Type::PANEL, "Children", Type::VISUAL_COLLECTION);
	Panel::BackgroundProperty = DependencyObject::Register (Type::PANEL, "Background", Type::BRUSH);
}

