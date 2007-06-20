/*
 * runtime.cpp: Core surface and canvas definitions.
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
n *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <gtk/gtk.h>
#define Visual _XVisual
#include <gdk/gdkx.h>
#if AGG
#    include <agg_rendering_buffer.h>
#    include "Agg2D.h"
#else
#    define CAIRO 1
#endif

#include <cairo-xlib.h>
#undef Visual
#include "runtime.h"
#include "shape.h"
#include "transform.h"
#include "animation.h"
#include "text.h"
#include "value.h"

#if AGG
struct _SurfacePrivate {
	agg::rendering_buffer rbuf;
	Agg2D *graphics;
};
#endif

//#define DEBUG_INVALIDATE 1
#define DEBUG_REFCNT 0

#define CAIRO_CLIP 0
#define TIME_CLIP 0
#define TIME_REDRAW 1

static callback_mouse_event cb_motion, cb_down, cb_up, cb_enter;
static callback_plain_event cb_got_focus, cb_focus, cb_loaded, cb_mouse_leave;
static callback_keyboard_event cb_keydown, cb_keyup;

void 
Base::ref ()
{
	if (refcount & BASE_FLOATS)
		refcount = 1;
	else
		refcount++;

#if DEBUG_REFCNT
	printf ("refcount++ %p (%s), refcnt = %d\n", this,
		Type::Find (((DependencyObject *) this)->GetObjectType ())->name, refcount);
#endif
}

void
Base::unref ()
{
	if (refcount == BASE_FLOATS || refcount == 1) {
#if DEBUG_REFCNT
		printf ("destroying object %p (%s)\n", this,
			Type::Find (((DependencyObject *) this)->GetObjectType ())->name);
#endif
		delete this;
	} else {
		refcount--;
#if DEBUG_REFCNT
		printf ("refcount-- %p (%s), refcnt = %d\n", this,
			Type::Find (((DependencyObject *) this)->GetObjectType ())->name,
			refcount);
#endif
	}
}

void 
base_ref (Base *base)
{
	if (base != NULL)
		base->ref ();
}
 
void
base_unref (Base *base)
{
	if (base != NULL)
		base->unref ();
}


Collection::Node::Node (DependencyObject *dob, DependencyObject *parent)
{
	dob->Attach (NULL, parent);
	dob->SetParent (parent);
	dob->ref ();
	obj = dob;
}

Collection::Node::~Node ()
{
	obj->Detach (NULL, obj->GetParent ());
	obj->SetParent (NULL);
	obj->unref ();
}

bool
CollectionNodeFinder (List::Node *n, void *data)
{
	Collection::Node *cn = (Collection::Node *) n;
	
	return cn->obj == (DependencyObject *) data;
}


Collection::Collection ()
{
	list = new List ();
	closure = NULL;
}

Collection::~Collection ()
{
	list->Clear (true);
	delete list;
}

void
Collection::Add (DependencyObject *data)
{
	g_return_if_fail (Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType()));
	
	list->Append (new Collection::Node (data, this));
}

void
Collection::Insert (int index, DependencyObject *data)
{
	g_return_if_fail (Type::Find(data->GetObjectType())->IsSubclassOf(GetElementType()));
	
	list->Insert (new Collection::Node (data, this), index);
}

void
Collection::Remove (DependencyObject *data)
{
	Collection::Node *n;
	
	if (!(n = (Collection::Node *) list->Find (CollectionNodeFinder, data)))
		return;
	
	n->Unlink ();
	
	delete n;
}

void 
collection_add (Collection *collection, DependencyObject *data)
{
	collection->Add (data);
}

void 
collection_remove (Collection *collection, DependencyObject *data)
{
	collection->Remove (data);
}

void
collection_insert (Collection *collection, int index, DependencyObject *data)
{
	collection->Insert (index, data);
}

//
// Very trivial implementation for now
//
void 
collection_clear (Collection *collection)
{
	collection->list->Clear (true);
}

Type::Kind
collection_get_element_type (Collection *collection)
{
	return collection->GetElementType ();
}

CollectionIterator *
collection_get_iterator (Collection *collection)
{
	return new CollectionIterator (collection);
}

bool
collection_iterator_move_next (CollectionIterator *iterator)
{
	if (!iterator->current)
		return false;
	
	iterator->current = iterator->current->Next ();
	
	return iterator->current != NULL;
}

void 
collection_iterator_reset (CollectionIterator *iterator)
{
	iterator->current = iterator->collection->list->First ();
}

DependencyObject *
collection_iterator_get_current (CollectionIterator *iterator)
{
	if (iterator->current == NULL)
		return NULL;
	
	return ((Collection::Node *) iterator->current)->obj;
}

void
collection_iterator_destroy (CollectionIterator *iterator)
{
	delete iterator;
}

static char**
split_str (const char* s, int *count)
{
	int n;
	// FIXME - what are all the valid separators ? I've seen ',' and ' '
	char** values = g_strsplit_set (s, ", ", 0);
	if (count) {
		// count non-NULL entries (which means we must skip NULLs later too)
		for (n = 0; values[n]; n++);
		*count = n;
	}
	return values;
}

Point
point_from_str (const char *s)
{
	// FIXME - not robust enough for production
	char *next = NULL;
	double x = strtod (s, &next);
	double y = 0.0;
	if (next)
		y = strtod (++next, NULL);
	return Point (x, y);
}

Point*
point_array_from_str (const char *s, int* count)
{
	int i, j, n = 0;
	bool x = true;
	char** values = split_str (s, &n);

	*count = (n >> 1); // 2 doubles for each point
	Point *points = new Point [*count];
	for (i = 0, j = 0; i < n; i++) {
		char *value = values[i];
		if (value) {
			if (x) {
				points[j].x = strtod (value, NULL);
				x = false;
			} else {
				points[j++].y = strtod (value, NULL);
				x = true;
			}
		}
	}

	g_strfreev (values);
	return points;
}

DoubleArray *
double_array_new (int count, double *values)
{
	DoubleArray *p = (DoubleArray *) g_malloc0 (sizeof (DoubleArray) + count * sizeof (double));
	p->basic.count = count;
	p->basic.refcount = 1;
	memcpy (p->values, values, sizeof (double) * count);
	return p;
}

PointArray *
point_array_new (int count, Point *points)
{
	PointArray *p = (PointArray *) g_malloc0 (sizeof (PointArray) + count * sizeof (Point));
	p->basic.count = count;
	p->basic.refcount = 1;
	memcpy (p->points, points, sizeof (Point) * count);
	return p;
};


Rect
rect_from_str (const char *s)
{
	// FIXME - not robust enough for production
	char *next = NULL;
	double x = strtod (s, &next);
	double y = 0.0;
	if (next)
		y = strtod (++next, &next);
	double w = 0.0;
	if (next)
		w = strtod (++next, &next);
	double h = 0.0;
	if (next)
		h = strtod (++next, &next);
	return Rect (x, y, w, h);
}

double*
double_array_from_str (const char *s, int* count)
{
	int i, n;
	char** values = split_str (s, count);

	double *doubles = new double [*count];
	for (i = 0, n = 0; i < *count; i++) {
		char *value = values[i];
		if (value)
			doubles[n++] = strtod (value, NULL);
	}

	g_strfreev (values);
	return doubles;
}

/**
 * item_getbounds:
 * @item: the item to update the bounds of
 *
 * Does this by requesting bounds update to all of its parents. 
 */
void
item_update_bounds (UIElement *item)
{
	double cx1 = item->x1;
	double cy1 = item->y1;
	double cx2 = item->x2;
	double cy2 = item->y2;
	
	item->getbounds ();
	
	//
	// If we changed, notify the parent to recompute its bounds
	//
	if (item->x1 != cx1 || item->y1 != cy1 || item->y2 != cy2 || item->x2 != cx2){
		if (item->parent != NULL)
			item_update_bounds (item->parent);
	}
}

void
UIElement::get_xform_for (UIElement *item, cairo_matrix_t *result)
{
	printf ("get_xform_for called on a non-container, you must implement this in your container\n");
	exit (1);
}

void 
item_invalidate (UIElement *item)
{
	Surface *s = item_get_surface (item);
	double res [6];
	
	if (s == NULL)
		return;

#ifdef DEBUG_INVALIDATE
	printf ("Requesting invalidate for object %p (%s) at %d %d - %d %d\n", 
		item, Type::Find(item->GetObjectType())->name,
		(int) item->x1, (int)item->y1, 
		(int)(item->x2-item->x1+1), (int)(item->y2-item->y1+1));
#endif
	// 
	// Note: this is buggy: why do we need to queue the redraw on the toplevel
	// widget (s->data) and does not work with the drawing area?
	//
	gtk_widget_queue_draw_area ((GtkWidget *)s->drawing_area, 
				    (int) item->x1, (int)item->y1, 
				    (int)(item->x2-item->x1+2), (int)(item->y2-item->y1+2));
}

void 
item_set_transform_origin (UIElement *item, Point p)
{
	item->SetValue (UIElement::RenderTransformOriginProperty, p);
}

void
item_get_render_affine (UIElement *item, cairo_matrix_t *result)
{
	Value* v = item->GetValue (UIElement::RenderTransformProperty);
	if (v == NULL)
		cairo_matrix_init_identity (result);
	else {
		Transform *t = v->AsTransform();
		t->GetTransform (result);
	}
}

UIElement::UIElement () : opacityMask(NULL), parent(NULL), flags (0), x1 (0), y1(0), x2(0), y2(0)
{
	cairo_matrix_init_identity (&absolute_xform);

	this->SetValue (UIElement::TriggersProperty, Value (new TriggerCollection ()));
	this->SetValue (UIElement::ResourcesProperty, Value (new ResourceCollection ()));
}

UIElement::~UIElement ()
{
	if (opacityMask != NULL) {
		opacityMask->Detach (NULL, this);
		opacityMask->unref ();
	}
}

//
// Intercept any changes to the triggers property and mirror that into our
// own variable
//
void
UIElement::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == UIElement::OpacityProperty) {
		item_invalidate (this);
	} else if (prop == UIElement::VisibilityProperty) {
		item_invalidate (this);
	} else if (prop == UIElement::ClipProperty) {
		FullInvalidate (false);
	} else if (prop == UIElement::OpacityMaskProperty) {
		if (opacityMask != NULL) {
			opacityMask->Detach (NULL, this);
			opacityMask->unref ();
		}
		
		if ((opacityMask = uielement_get_opacity_mask (this)) != NULL) {
			opacityMask->Attach (NULL, this);
			opacityMask->ref ();
		}
		
		FullInvalidate (false);
	} else if (prop == RenderTransformProperty || prop == RenderTransformOriginProperty) {
		FullInvalidate (true);
	} else if (prop == TriggersProperty){
		Value *v = GetValue (prop);
		TriggerCollection *newcol = v ?  v->AsTriggerCollection() : NULL;

		if (newcol) {
			if  (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
				
			newcol->closure = this;
		}
	} else if (prop == ResourcesProperty) {
		Value *v = GetValue (prop);
		ResourceCollection *newcol = v ? v->AsResourceCollection() : NULL;

		if (newcol) {
			if  (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
				
			newcol->closure = this;
		}
	} else {
		Visual::OnPropertyChanged (prop);
	}
}

#if true
int
UIElement::dump_hierarchy (UIElement *obj)
{
	if (obj == NULL)
		return 0;
	
	int n = dump_hierarchy (obj->parent);
	for (int i = 0; i < n; i++)
		putchar (' ');
	printf ("%s (%p)\n", dependency_object_get_name (obj), obj);
	return n + 4;
}
#endif

void
UIElement::update_xform ()
{
	cairo_matrix_t user_transform;
	
	//
	// What is more important, the space used by 6 doubles,
	// or the time spent calling the parent (that will call
	// DependencyObject->GetProperty to get the positions?
	//
	// Currently we go for thiner, but if we decide to go
	// for reduced computation, we should introduce the 
	// base transform in UIElement that will be updated by the
	// container on demand
	//
	item_get_render_affine (this, &user_transform);

	if (parent != NULL)
		parent->get_xform_for (this, &absolute_xform);
	else
		cairo_matrix_init_identity (&absolute_xform);
	
	Point p = getxformorigin ();
	cairo_matrix_translate (&absolute_xform, p.x, p.y);
	cairo_matrix_multiply (&absolute_xform, &user_transform, &absolute_xform);
	cairo_matrix_translate (&absolute_xform, -p.x, -p.y);
	//printf ("      Final position for %s x=%g y=%g\n", dependency_object_get_name (this), absolute_xform.x0, absolute_xform.y0);
}

void
UIElement::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (prop == UIElement::RenderTransformProperty ||
	    prop == UIElement::RenderTransformOriginProperty)
		FullInvalidate (true);
	else if (prop == UIElement::ClipProperty ||
		 prop == UIElement::OpacityMaskProperty) {

		// Maybe this could also be just item_invalidate?
		FullInvalidate (false);
	}  else if (prop == UIElement::OpacityProperty ||
		    prop == UIElement::VisibilityProperty){
		item_invalidate (this);
	} else if (Type::Find (subprop->type)->IsSubclassOf (Type::BRUSH)) {
		item_invalidate (this);
	}
	
	Visual::OnSubPropertyChanged (prop, subprop);
}

//
// Queues the invalidate for the current region, performs any 
// updates to the RenderTransform (optional) and queues a 
// new redraw with the new bounding box
//
void
UIElement::FullInvalidate (bool rendertransform)
{
	item_invalidate (this);
	if (rendertransform)
		update_xform ();
	item_update_bounds (this);
	item_invalidate (this);
}

void
item_set_render_transform (UIElement *item, Transform *transform)
{
	item->SetValue (UIElement::RenderTransformProperty, Value(transform));
}

double
uielement_get_opacity (UIElement *item)
{
	return item->GetValue (UIElement::OpacityProperty)->AsDouble();
}

void
uielement_set_opacity (UIElement *item, double opacity)
{
	item->SetValue (UIElement::OpacityProperty, Value (opacity));
}

Brush *
uielement_get_opacity_mask (UIElement *item)
{
	Value *value = item->GetValue (UIElement::OpacityMaskProperty);
	
	return value ? (Brush *) value->AsBrush () : NULL;
}

//
// Maps the x, y coordinate to the space of the given item
//
void
uielement_transform_point (UIElement *item, double *x, double *y)
{
	cairo_matrix_t inverse = item->absolute_xform;
	cairo_matrix_invert (&inverse);

	cairo_matrix_transform_point (&inverse, x, y);
}

bool
UIElement::inside_object (Surface *s, double x, double y)
{
	printf ("UIElement derivatives should implement inside object\n");
}

bool
UIElement::handle_motion (Surface *s, int state, double x, double y)
{
	if (inside_object (s, x, y)){
		s->cb_motion (this, state, x, y);
		return true;
	} 
	return false;
}

bool
UIElement::handle_button (Surface *s, callback_mouse_event cb, int state, double x, double y)
{
	if (inside_object (s, x, y)){
		cb (this, state, x, y);
		return true;
	}
	return false;
}

void
UIElement::enter (Surface *s, int state, double x, double y)
{
	s->cb_enter (this, state, x, y);
}

void
UIElement::leave (Surface *s)
{
	s->cb_mouse_leave (this);
}

void
UIElement::getbounds ()
{
	g_warning ("UIElement:getbounds has been called. The derived class %s should have overridden it.",
		   dependency_object_get_name (this));
}

void
UIElement::dorender (Surface *surface, int x, int y, int width, int height)
{
	STARTTIMER (UIElement_render, Type::Find (GetObjectType())->name);
	render (surface, x, y, width, height);
	ENDTIMER (UIElement_render, Type::Find (GetObjectType())->name);
}

void
UIElement::render (Surface *s, int x, int y, int width, int height)
{
	g_warning ("UIElement:render has been called. The derived class should have overridden it.");
}

void
UIElement::get_size_for_brush (cairo_t *cr, double *width, double *height)
{
	double x1, y1, x2, y2;
	
	cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
	
	*height = fabs (y2 - y1);
	*width = fabs (x2 - x1);
}

double
UIElement::GetTotalOpacity ()
{
	double opacity = uielement_get_opacity (this);
	// this is recursive to parents
	UIElement *uielement = this->parent;
	while (uielement) {
		double parent_opacity = uielement_get_opacity (uielement);
		if (parent_opacity < 1.0)
			opacity *= parent_opacity;
		// FIXME: we should be calling FrameworkElement::Parent
		uielement = uielement->parent;
	}

	return opacity;
}

FrameworkElement::FrameworkElement ()
{
}

void
FrameworkElement::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == FrameworkElement::WidthProperty ||
	    prop == FrameworkElement::HeightProperty) {
		FullInvalidate (false);
		return;
	}

	UIElement::OnPropertyChanged (prop);
}

bool
FrameworkElement::inside_object (Surface *s, double x, double y)
{
	// Quick bounding box check.
	if (x < x1 || x > x2 || y < y1 || y > y2)
		return FALSE;

	bool ret = FALSE;
	double nx = x, ny = y;

	uielement_transform_point (this, &nx, &ny);
	if (nx < 0 || ny < 0 || nx > framework_element_get_width (this) || ny > framework_element_get_height (this))
		return false;

	return true;

#if false
	// 
	// The following code is slower and does about the same as the
	// one above, but I believe it might be buggy.   it is here
	// as a reference for Shapes that need to ues 
	// "cairo_in_stroke" and "cairo_in_fill" (like circles) to
	// detect in/out 
	//
	// When I tried this with scaled/rotated objects it was incorrect
	// for rectangles, but this is roughly the "spirit".
	//
	cairo_save (s->cairo);
	cairo_set_matrix (s->cairo, &absolute_xform);
	cairo_rectangle (s->cairo, 0, 0, framework_element_get_width (this), framework_element_get_height (this));
	cairo_matrix_transform_point (&absolute_xform, &nx, &ny);
	if (cairo_in_stroke (s->cairo, nx, ny) || cairo_in_fill (s->cairo, nx, ny))
		ret = TRUE;

	cairo_restore (s->cairo);
	return ret;
#endif
}

double
framework_element_get_height (FrameworkElement *framework_element)
{
	return framework_element->GetValue (FrameworkElement::HeightProperty)->AsDouble();
}

void
framework_element_set_height (FrameworkElement *framework_element, double height)
{
	framework_element->SetValue (FrameworkElement::HeightProperty, Value (height));
}

double
framework_element_get_width (FrameworkElement *framework_element)
{
	return framework_element->GetValue (FrameworkElement::WidthProperty)->AsDouble();
}

void
framework_element_set_width (FrameworkElement *framework_element, double width)
{
	framework_element->SetValue (FrameworkElement::WidthProperty, Value (width));
}

Surface *
item_get_surface (UIElement *item)
{
	if (item->flags & UIElement::IS_CANVAS){
		Canvas *canvas = (Canvas *) item;
		if (canvas->surface)
			return canvas->surface;
	}

	if (item->parent != NULL)
		return item_get_surface (item->parent);

	return NULL;
}

void
VisualCollection::VisualUpdate (DependencyObject *data)
{
	Panel *panel = (Panel *) closure;
	UIElement *item = (UIElement *) data;
	
	item->parent = panel;
	item->update_xform ();
	item_update_bounds (panel);
	item_invalidate (panel);
}

void
VisualCollection::Add (DependencyObject *data)
{
	UIElement *item = (UIElement *) data;
	Collection::Add (item);
	VisualUpdate (data);
}

void
VisualCollection::Insert (int index, DependencyObject *data)
{
	UIElement *item = (UIElement *) data;

	Collection::Insert (index, item);

	VisualUpdate (data);
}

void
VisualCollection::Remove (DependencyObject *data)
{
	Panel *panel = (Panel *) closure;
	UIElement *item = (UIElement *) data;
	
	item_invalidate (item);
	Collection::Remove (item);
	item_update_bounds (panel);
}

VisualCollection*
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

Panel*
panel_new (void)
{
	return new Panel ();
}

Panel::Panel ()
{
	this->SetValue (Panel::ChildrenProperty, Value (new VisualCollection ()));
	background = NULL;
}

Panel::~Panel ()
{
	if (background != NULL) {
		background->Detach (NULL, this);
		background->unref ();
	}
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
		
		item_invalidate (this);
	}
}

Canvas::Canvas () : surface (NULL), current_item (NULL)
{
	flags |= IS_CANVAS;
}

void
Canvas::get_xform_for (UIElement *item, cairo_matrix_t *result)
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

	if (surface){
		x1 = y1 = 0;
		x2 = surface->width;
		y2 = surface->height;
	} else {
		// If we found nothing.
		if (first)
			x1 = y1 = x2 = y2 = 0;
	}
	//space (levelb);
	//printf ("Canvas: Leave GetBounds (%g %g %g %g)\n", x1, y1,x2, y2);
	//levelb -= 4;
}

bool
Canvas::OnChildPropertyChanged (DependencyProperty *prop, DependencyObject *child)
{
	if (prop == TopProperty || prop == LeftProperty){
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

Control *
control_new (void)
{
	return new Control ();
}

void
draw_grid (cairo_t *cairo)
{
	cairo_set_line_width (cairo, 1.0);
	cairo_set_source_rgba (cairo, 0, 1, 0, 1.0);
	for (int col = 100; col < 1024; col += 100){
		cairo_move_to (cairo, col, 0);
		cairo_line_to (cairo, col, 1024);
		cairo_stroke (cairo);
	}

	for (int row = 0; row < 1024; row += 100){
		cairo_move_to (cairo, 0, row);
		cairo_line_to (cairo, 1024, row);
		cairo_stroke (cairo);
	}
}

void
create_similar (Surface *s, GtkWidget *widget)
{
    cairo_t *ctx = gdk_cairo_create (widget->window);

    if (s->cairo_xlib){
	cairo_destroy (s->cairo_xlib);
    }

    cairo_surface_t *xlib_surface = cairo_surface_create_similar (
	   cairo_get_target (ctx), 
	   CAIRO_CONTENT_COLOR_ALPHA,
	   s->width, s->height);

    cairo_destroy (ctx);

    s->cairo_xlib = cairo_create (xlib_surface);
    cairo_surface_destroy (xlib_surface);
}



static void
surface_realloc (Surface *s)
{
	if (s->buffer)
		free (s->buffer);

	int size = s->width * s->height * 4;
	s->buffer = (unsigned char *) malloc (size);

	s->cairo_buffer_surface = cairo_image_surface_create_for_data (
		s->buffer, CAIRO_FORMAT_ARGB32, s->width, s->height, s->width * 4);

	s->cairo_buffer = cairo_create (s->cairo_buffer_surface);

	if (s->cairo_xlib == NULL) {
		s->cairo = s->cairo_buffer;
	}
	else {
		create_similar (s, s->drawing_area);
		s->cairo = s->cairo_xlib;
	}
}

void 
surface_destroy (Surface *s)
{
	delete s;
}

static void
render_surface (gpointer data)
{
	Surface *s = (Surface*)data;
	gdk_window_process_updates (GTK_WIDGET (s->drawing_area)->window, FALSE);
}

gboolean
realized_callback (GtkWidget *widget, gpointer data)
{
	Surface *s = (Surface *) data;

	create_similar (s, widget);
	s->cairo = s->cairo_xlib;

	TimeManager::Instance()->AddHandler ("render", render_surface, s);
}

gboolean
unrealized_callback (GtkWidget *widget, gpointer data)
{
	Surface *s = (Surface *) data;

	if (s->cairo_xlib) {
		cairo_destroy (s->cairo_xlib);
		s->cairo_xlib = NULL;
	}

	s->cairo = s->cairo_buffer;

	TimeManager::Instance()->RemoveHandler ("render", render_surface, s);
}

gboolean
expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	Surface *s = (Surface *) data;

	s->frames++;

	if (event->area.x > s->width || event->area.y > s->height)
		return TRUE;

#if TIME_REDRAW
	STARTTIMER (expose, "redraw");
#endif
	s->cairo = s->cairo_xlib;

	//
	// BIG DEBUG BLOB
	// 
	if (cairo_status (s->cairo) != CAIRO_STATUS_SUCCESS){
		printf ("expose event: the cairo context has an error condition and refuses to paint: %s\n", 
			cairo_status_to_string (cairo_status (s->cairo)));
	}

#ifdef DEBUG_INVALIDATE
	printf ("Got a request to repaint at %d %d %d %d\n", event->area.x, event->area.y, event->area.width, event->area.height);
#endif

	cairo_reset_clip (s->cairo);
	gdk_cairo_region (s->cairo, event->region);
	cairo_clip (s->cairo);
	surface_repaint (s, event->area.x, event->area.y, event->area.width, event->area.height);
	cairo_t *ctx = gdk_cairo_create (widget->window);
	cairo_surface_flush (cairo_get_target (s->cairo));
	cairo_set_source_surface (ctx, cairo_get_target (s->cairo), 0, 0);
	gdk_cairo_region (ctx, event->region);
	cairo_clip (ctx);
	cairo_paint (ctx);
	cairo_destroy (ctx);

/*
	gdk_draw_drawable (
		widget->window, gtk_widget_get_style (widget)->white_gc, s->pixmap, 
		event->area.x, event->area.y, // gint src_x, gint src_y,
		event->area.x, event->area.y, // gint dest_x, gint dest_y,
		MIN (event->area.width, s->width),
		MIN (event->area.height, s->height));
*/
#if TIME_REDRAW
	ENDTIMER (expose, "redraw");
#endif

	return TRUE;
}

static gboolean
motion_notify_callback (GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	Surface *s = (Surface *) data;
	GdkModifierType state;
	int x, y;

	if (!s->cb_motion)
		return FALSE;

	if (event->is_hint) {
		gdk_window_get_pointer (event->window, &x, &y, &state);
	} else {
		x = (int)event->x;
		y = (int)event->y;
		state = (GdkModifierType)event->state;
	}
	s->toplevel->handle_motion (s, state, x, y);
	return TRUE;
}

static gboolean
crossing_notify_callback (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	Surface *s = (Surface *) data;

	if (s->cb_enter == NULL || s->cb_mouse_leave == NULL)
		return FALSE;

	if (event->type == GDK_ENTER_NOTIFY){
		s->toplevel->handle_motion (s, event->state, event->x, event->y);
		s->toplevel->enter (s, event->state, event->x, event->y);
	} else {
		s->toplevel->leave (s);
	}
	return TRUE;
}

static gboolean 
key_press_callback (GtkWidget *widget, GdkEventKey *key, gpointer data)
{
	Surface *s = (Surface *) data;

	if (!s->cb_keydown)
		return FALSE;

	// 
	// I could not write a test that would send the output elsewhere, for now
	// just send to the toplevel
	//
	return s->cb_keydown (s->toplevel, key->state, key->keyval, key->hardware_keycode);
}

static gboolean 
key_release_callback (GtkWidget *widget, GdkEventKey *key, gpointer data)
{
	Surface *s = (Surface *) data;

	if (!s->cb_keyup)
		return FALSE;
	
	// 
	// I could not write a test that would send the output elsewhere, for now
	// just send to the toplevel
	//
	return s->cb_keyup (s->toplevel, key->state, key->keyval, key->hardware_keycode);

}

static gboolean
button_release_callback (GtkWidget *widget, GdkEventButton *button, gpointer data)
{
	Surface *s = (Surface *) data;

	if (!s->cb_up)
		return FALSE;
	
	if (button->button != 1)
		return FALSE;

	s->toplevel->handle_button (s, s->cb_up, button->state, button->x, button->y);
	return TRUE;
}

static gboolean
button_press_callback (GtkWidget *widget, GdkEventButton *button, gpointer data)
{
	Surface *s = (Surface *) data;

	gtk_widget_grab_focus (widget);

	if (!s->cb_down)
		return FALSE;

	if (button->button != 1)
		return FALSE;

	s->toplevel->handle_button (s, s->cb_down, button->state, button->x, button->y);
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
	cn = (Collection::Node *) children->list->First ();
	for ( ; cn != NULL; cn = (Collection::Node *) cn->Next ()) {
		UIElement *item = (UIElement *) cn->obj;
		
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

		//cairo_set_source_rgb (s->cairo, 1.0, 0, 1.0);
		//cairo_set_line_width (s->cairo, 10);
		//cairo_rectangle (s->cairo, item->x1, item->y1, item->x2 - item->x1, item->y2 - item->y1);
		//cairo_stroke (s->cairo);
	}
//	printf ("RENDER: LEAVE\n");

	level -= 4;
	//draw_grid (s->cairo);

	cairo_restore (s->cairo); // for UIElement::ClipProperty
}

Canvas *
canvas_new (void)
{
	return new Canvas ();
}

void 
clear_drawing_area (GtkObject *obj, gpointer data)
{
	Surface *s = (Surface *) data;

	s->drawing_area = NULL;
}

void
surface_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
	Surface *s = (Surface *) user_data;

	if (s->width != allocation->width || s->height != allocation->height){
		s->width = allocation->width;
		s->height = allocation->height;

		if (s->cb_surface_resize != NULL)
			s->cb_surface_resize (s->toplevel);

		surface_realloc (s);
		gtk_widget_queue_draw_area ((GtkWidget *) s->drawing_area,
					    0, 0, allocation->width, allocation->height);
	}
}

Surface *
surface_new (int width, int height)
{
	Surface *s = new Surface ();

	s->drawing_area = gtk_drawing_area_new ();
	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "size_allocate",
			    G_CALLBACK(surface_size_allocate), s);
			    
	gtk_widget_add_events (s->drawing_area, 
			       GDK_POINTER_MOTION_MASK |
			       GDK_POINTER_MOTION_HINT_MASK |
			       GDK_KEY_PRESS_MASK |
			       GDK_KEY_RELEASE_MASK |
			       GDK_BUTTON_PRESS_MASK |
			       GDK_BUTTON_RELEASE_MASK);
	GTK_WIDGET_SET_FLAGS (s->drawing_area, GTK_CAN_FOCUS);
	gtk_widget_set_double_buffered (s->drawing_area, FALSE);

	gtk_widget_show (s->drawing_area);

	gtk_widget_set_usize (s->drawing_area, width, height);
	s->buffer = NULL;

	s->width = width;
	s->height = height;
	s->toplevel = NULL;

	surface_realloc (s);

	return s;
}

//
// This will resize the surface (merely a convenience function for
// resizing the widget area that we have.
//
// This will not change the Width and Height properties of the 
// toplevel canvas, if you want that, you must do that yourself
//
void
surface_resize (Surface *s, int width, int height)
{
	gtk_widget_set_usize (s->drawing_area, width, height);
}

Surface::~Surface ()
{
	if (toplevel) {
		toplevel->unref ();
		toplevel = NULL;
	}

	if (buffer) {
		free (buffer);
		buffer = NULL;
	}

	cairo_destroy (cairo_buffer);
	cairo_buffer = NULL;

	cairo_surface_destroy (cairo_buffer_surface);
	cairo_buffer_surface = NULL;

	if (drawing_area != NULL){
		g_signal_handlers_disconnect_matched (drawing_area,
						      (GSignalMatchType) G_SIGNAL_MATCH_DATA,
						      0, 0, NULL, NULL, this);

		gtk_widget_destroy (drawing_area);
		drawing_area = NULL;
	}
}

void
surface_connect_events (Surface *s)
{
	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "expose_event",
			    G_CALLBACK (expose_event_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "motion_notify_event",
			    G_CALLBACK (motion_notify_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "enter_notify_event",
			    G_CALLBACK (crossing_notify_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "leave_notify_event",
			    G_CALLBACK (crossing_notify_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "key_press_event",
			    G_CALLBACK (key_press_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "key_release_event",
			    G_CALLBACK (key_release_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "button_press_event",
			    G_CALLBACK (button_press_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "button_release_event",
			    G_CALLBACK (button_release_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "realize",
			    G_CALLBACK (realized_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "unrealize",
			    G_CALLBACK (unrealized_callback), s);

	gtk_signal_connect (GTK_OBJECT (s->drawing_area), "destroy",
			    G_CALLBACK (clear_drawing_area), s);

	if (GTK_WIDGET_REALIZED (s->drawing_area)){
		realized_callback (s->drawing_area, s);
	}
}

static void
emit_loaded_events (UIElement *ui)
{
	if ((ui->flags & UIElement::IS_CANVAS)) {
		Canvas *c = (Canvas*)ui;
		VisualCollection *children = c->GetChildren ();
		Collection::Node *cn;

		cn = (Collection::Node *) children->list->First ();
		for ( ; cn != NULL; cn = (Collection::Node *) cn->Next ()) {
			UIElement *item = (UIElement *) cn->obj;

			emit_loaded_events (item);
		}
	}

	if (!(ui->flags & UIElement::IS_LOADED)) {
		ui->flags |= UIElement::IS_LOADED;
		ui->events->Emit ("Loaded");
	}
}

void
surface_attach (Surface *surface, UIElement *toplevel)
{
	bool first = FALSE;

	if (!(toplevel->flags & UIElement::IS_CANVAS)) {
		printf ("Unsupported toplevel\n");
		return;
	}
	
	if (surface->toplevel) {
		item_invalidate (surface->toplevel);
		surface->toplevel->unref ();
	} else 
		first = TRUE;

	Canvas *canvas = (Canvas *) toplevel;
	canvas->ref ();

	canvas->surface = surface;
	surface->toplevel = canvas;

	// First time we connect the surface, start responding to events
	if (first)
		surface_connect_events (surface);

	emit_loaded_events (canvas);

	bool change_size = false;
	//
	// If the did not get a size specified
	//
	if (surface->width == 0){
		Value *v = toplevel->GetValue (FrameworkElement::WidthProperty);

		if (v){
			surface->width = (int) v->AsDouble ();
			if (surface->width < 0)
				surface->width = 0;
			change_size = true;
		}
	}

	if (surface->height == 0){
		Value *v = toplevel->GetValue (FrameworkElement::HeightProperty);

		if (v){
			surface->height = (int) v->AsDouble ();
			if (surface->height < 0)
				surface->height = 0;
			change_size = true;
		}
	}

	if (change_size)
		surface_realloc (surface);

	canvas->FullInvalidate (true);
}

void
surface_repaint (Surface *s, int x, int y, int width, int height)
{
	cairo_set_source_rgb (s->cairo, 1, 1, 1);
	cairo_rectangle (s->cairo, x, y, width, height);
	cairo_fill (s->cairo);
	s->toplevel->dorender (s, x, y, width, height);
}

void *
surface_get_drawing_area (Surface *s)
{
	return s->drawing_area;
}

/*
	DependencyObject
*/

GHashTable *DependencyObject::properties = NULL;

typedef struct {
	DependencyObject *dob;
	DependencyProperty *prop;
} Attachee;

//
// Attaches the dependency object to the container on the given property.
//
void
DependencyObject::Attach (DependencyProperty *property, DependencyObject *container)
{
	Attachee *att = new Attachee ();
	att->dob = container;
	att->prop = property;
	attached_list = g_slist_append (attached_list, att);
}

void
DependencyObject::Detach (DependencyProperty *property, DependencyObject *container)
{
	for (GSList *l = attached_list; l; l = l->next) {
		Attachee *att = (Attachee*)l->data;

		if (att->dob == container && att->prop == property) {
			attached_list = g_slist_remove_link (attached_list, l);
			delete att;
			break;
		}
	}
}

void
DependencyObject::NotifyAttacheesOfPropertyChange (DependencyProperty *subproperty)
{
	for (GSList *l = attached_list; l != NULL; l = l->next){
		Attachee *att = (Attachee*)l->data;

		att->dob->OnSubPropertyChanged (att->prop, subproperty);
	}
}

void
DependencyObject::SetValue (DependencyProperty *property, Value *value)
{
	g_return_if_fail (property != NULL);

	if (value != NULL){
		if (!Type::Find (value->GetKind ())->IsSubclassOf (property->value_type)) {
			g_warning ("DependencyObject::SetValue, value cannot be assigned to the property %s::%s (property has type '%s', value has type '%s')\n", GetType ()->name, property->name, Type::Find (property->value_type)->name, Type::Find (value->GetKind ())->name);
			return;
		}
	} else {
		if (!(property->value_type >= Type::DEPENDENCY_OBJECT) && !property->IsNullable ()) {
			g_warning ("Can not set a non-nullable scalar type to NULL");
			return;
		}
	}

	Value *current_value = (Value*)g_hash_table_lookup (current_values, property->name);

	if (current_value != NULL && current_value->GetKind () >= Type::DEPENDENCY_OBJECT) {
		DependencyObject *current_as_dep = current_value->AsDependencyObject ();
		if (current_as_dep)
			current_as_dep->SetParent (NULL);
	}
	if (value != NULL && value->GetKind () >= Type::DEPENDENCY_OBJECT) {
		DependencyObject *new_as_dep = value->AsDependencyObject ();
		
		new_as_dep->SetParent (this);
	}

	if ((current_value == NULL && value != NULL) ||
	    (current_value != NULL && value == NULL) ||
	    (current_value != NULL && value != NULL && *current_value != *value)) {

		if (current_value != NULL && current_value->GetKind () >= Type::DEPENDENCY_OBJECT){
			DependencyObject *dob = current_value->AsDependencyObject();

			if (dob != NULL)
				dob->Detach (property, this);
		}

		Value *store;
		if (value == NULL) {
			store = NULL;
		} else {
			store = new Value (*value);
		}

		g_hash_table_insert (current_values, property->name, store);

		if (value) {
			if (value->GetKind () >= Type::DEPENDENCY_OBJECT){
				DependencyObject *dob = value->AsDependencyObject();
				
				if (dob != NULL)
					dob->Attach (property, this);
			}

			// 
			//NotifyAttacheesOfPropertyChange (property);
		}

		OnPropertyChanged (property);
		if (property->is_attached_property)
			NotifyParentOfPropertyChange (property, true);
	}
}

void
DependencyObject::SetValue (DependencyProperty *property, Value value)
{
	SetValue (property, &value);
}

Value *
DependencyObject::GetValue (DependencyProperty *property)
{
	Value *value = NULL;

	bool found;
	found = g_hash_table_lookup_extended (current_values, property->name, NULL, (void**)&value);

	if (found)
		return value;

	return property->default_value;
}

Value *
DependencyObject::GetValueNoDefault (DependencyProperty *property)
{
	Value *value = NULL;

	value = (Value *) g_hash_table_lookup (current_values, property->name);

	if (value != NULL)
		return value;

	return NULL;
}

Value *
DependencyObject::GetValue (const char *name)
{
	DependencyProperty *property;
	property = GetDependencyProperty (name);

	if (property == NULL) {
		g_warning ("This object (of type '%s') doesn't have a property called '%s'\n", GetType ()->name, name);
		return NULL;
	}

	return GetValue (property);
}

void 
DependencyObject::SetValue (const char *name, Value value)
{
	SetValue (name, &value);
}

void
DependencyObject::SetValue (const char *name, Value *value)
{
	DependencyProperty *property;
	property = GetDependencyProperty (name);

	if (property == NULL) {
		g_warning ("This object (of type '%s') doesn't have a property called '%s'\n", GetType ()->name, name);
		return;
	}

	SetValue (property, value);
}

static void
free_value (void *v)
{
	delete (Value*)v;
}

DependencyObject::DependencyObject ()
{
	current_values = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, free_value);
	events = new EventObject ();
	this->attached_list = NULL;
	this->parent = NULL;
}

static void
dump (gpointer key, gpointer value, gpointer data)
{
	printf ("%s\n", key);
}

Type::Kind
DependencyObject::GetObjectType ()
{
	g_warning ("%p This class is missing an override of GetObjectType ()", this);
	g_hash_table_foreach (current_values, dump, NULL);
	return Type::DEPENDENCY_OBJECT; 
}

DependencyObject::~DependencyObject ()
{
	g_hash_table_destroy (current_values);
	delete events;
}

DependencyProperty *
DependencyObject::GetDependencyProperty (const char *name)
{
	return DependencyObject::GetDependencyProperty (GetObjectType (), name);
}

DependencyProperty *
DependencyObject::GetDependencyProperty (Type::Kind type, const char *name)
{
	return GetDependencyProperty (type, name, true);
}

DependencyProperty *
DependencyObject::GetDependencyProperty (Type::Kind type, const char *name, bool inherits)
{
	GHashTable *table;
	DependencyProperty *property = NULL;

	if (properties == NULL)
		return NULL;

	table = (GHashTable*) g_hash_table_lookup (properties, &type);

	if (table)
		property = (DependencyProperty*) g_hash_table_lookup (table, name);

	if (property != NULL)
		return property;

	if (!inherits)
		return NULL;	

	// Look in the parent type
	Type *current_type;
	current_type = Type::Find (type);
	
	if (current_type == NULL)
		return NULL;
	
	if (current_type->parent == Type::INVALID)
		return NULL;

	return GetDependencyProperty (current_type->parent, name);
}

bool
DependencyObject::HasProperty (const char *name, bool inherits)
{
	return GetDependencyProperty (GetObjectType (), name, inherits) != NULL;
}

DependencyObject*
DependencyObject::FindName (const char *name)
{
	NameScope *scope = NameScope::GetNameScope (this);
	DependencyObject *rv = NULL;

	if (scope)
		rv = scope->FindName (name);

	if (rv)
		return rv;
	else if (parent)
		return parent->FindName (name);
	else
		return NULL;
}

DependencyObject *
dependency_object_find_name (DependencyObject *obj, const char *name, Type::Kind *element_kind)
{
	//printf ("Looking up in %p the string %p\n", obj, name);
	//printf ("        String: %s\n", name);
	DependencyObject *ret = obj->FindName (name);

	if (ret == NULL)
		return NULL;

	*element_kind = ret->GetObjectType ();

	return ret;
}

//
//  A helper debugging routine for C#
//
const char *
dependency_object_get_name (DependencyObject *obj)
{
	return Type::Find (obj->GetObjectType ())->name;
}

void
dependency_object_add_event_handler (DependencyObject *o, char *event, EventHandler handler, gpointer closure)
{
	o->events->AddHandler (event, handler, closure);
}

void
dependency_object_remove_event_handler (DependencyObject *o, char *event, EventHandler handler, gpointer closure)
{
	o->events->RemoveHandler (event, handler, closure);
}


//
// Use this for values that can be null
//
DependencyProperty *
DependencyObject::Register (Type::Kind type, const char *name, Type::Kind vtype)
{
	g_return_val_if_fail (name != NULL, NULL);

	return RegisterFull (type, name, NULL, vtype, false);
}

//
// DependencyObject takes ownership of the Value * for default_value
//
DependencyProperty *
DependencyObject::Register (Type::Kind type, const char *name, Value *default_value)
{
	g_return_val_if_fail (default_value != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	return RegisterFull (type, name, default_value, default_value->GetKind (), false);
}

static void
free_property_hash (gpointer v)
{
	g_hash_table_destroy ((GHashTable*)v);
}

static void
free_property (gpointer v)
{
	delete (DependencyProperty*)v;
}

//
// DependencyObject takes ownership of the Value * for default_value
// This overload can be used to set the type of the property to a different type
// than the default value (the default value can for instance be a SolidColorBrush
// while the property type can be a Brush).
//
DependencyProperty *
DependencyObject::Register (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype)
{
	g_return_val_if_fail (default_value != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	return RegisterFull (type, name, default_value, vtype, false);
}

DependencyProperty *
DependencyObject::RegisterNullable (Type::Kind type, const char *name, Type::Kind vtype)
{
	DependencyProperty *property;
	property = Register (type, name, vtype);
	property->is_nullable = true;
	return property;
}

//
// Register the dependency property that belongs to @type with the name @name
// The default value is @default_value (if provided) and the type that can be
// stored in the dependency property is of type @vtype
//
DependencyProperty *
DependencyObject::RegisterFull (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached)
{
	GHashTable *table;

	DependencyProperty *property = new DependencyProperty (type, name, default_value, vtype, attached);
	
	/* first add the property to the global 2 level property hash */
	if (properties == NULL)
		properties = g_hash_table_new_full (g_int_hash, g_int_equal,
						    NULL, free_property_hash);

	table = (GHashTable*) g_hash_table_lookup (properties, &property->type);

	if (table == NULL) {
		table = g_hash_table_new_full (g_str_hash, g_str_equal,
					       NULL, free_property);
		g_hash_table_insert (properties, &property->type, table);
	}

	g_hash_table_insert (table, property->name, property);

	return property;
}

void
DependencyObject::Shutdown ()
{
	g_hash_table_destroy (DependencyObject::properties);
	DependencyObject::properties = NULL;
}

DependencyObject *
DependencyObject::GetParent ()
{
	return parent;
}

void
DependencyObject::SetParent (DependencyObject *parent)
{
#if DEBUG
	// Check for circular families
	DependencyObject *current = parent;
	do while (current != NULL) {
		g_assert (current != this);
		current = current->GetParent ();
	} 
#endif
	this->parent = parent;
}

void
DependencyObject::NotifyParentOfPropertyChange (DependencyProperty *property, bool only_exact_type)
{
	DependencyObject *current = GetParent ();
	while (current != NULL) {
		if (!only_exact_type || property->type == current->GetObjectType ()) {	

			// Only handle up to the first one that catches the attached change
			if (only_exact_type && current->OnChildPropertyChanged (property, this))
				return;
		}
		current = current->GetParent ();
	}
}

Value *
dependency_object_get_value (DependencyObject *object, DependencyProperty *prop)
{
	if (object == NULL)
		return NULL;

	return object->GetValue (prop);
}

void
dependency_object_set_value (DependencyObject *object, DependencyProperty *prop, Value *val)
{
	if (object == NULL)
		return;

	object->SetValue (prop, val);
}

/*
 *	DependencyProperty
 */
DependencyProperty::DependencyProperty (Type::Kind type, const char *name, Value *default_value, Type::Kind value_type, bool attached)
{
	this->type = type;
	this->name = g_strdup (name);
	this->default_value = default_value;
	this->value_type = value_type;
	this->is_attached_property = attached;
}

DependencyProperty::~DependencyProperty ()
{
	g_free (name);
	if (default_value != NULL)
		delete default_value;
}

DependencyProperty *dependency_property_lookup (Type::Kind type, char *name)
{
	return DependencyObject::GetDependencyProperty (type, name);
}

char*
dependency_property_get_name (DependencyProperty *property)
{
	return property->name;
}

Type::Kind
dependency_property_get_value_type (DependencyProperty *property)
{
	return property->value_type;
}

bool
dependency_property_is_nullable (DependencyProperty *property)
{
	return property->IsNullable ();
}


//
// Everything inside of a ( ) resolves to a DependencyProperty, if there is a
// '.' after the property, we get the object, and continue resolving from there
// if there is a [n] after the property, we convert the property to a collection
// and grab the nth item.
//
// Dependency properties can be specified as (PropertyName) of the current object
// or they can be specified as (DependencyObject.PropertyName).
//
// Returns NULL on any error
//
DependencyProperty *
resolve_property_path (DependencyObject **o, const char *path)
{
	g_assert (o);
	g_assert (path);

	int c;
	int len = strlen (path);
	char *typen = NULL;
	char *propn = NULL;
	bool expression_found = false;
	DependencyProperty *res = NULL;
	DependencyObject *lu = *o;

	for (int i = 0; i < len; i++) {
		switch (path [i]) {
		case '(':
		{
			expression_found = true;

			typen = NULL;
			propn = NULL;
			int estart = i + 1;
			for (c = estart; c < len; c++) {
				if (path [c] == '.') {
					typen = strndup (path + estart, c - estart);
					estart = c + 1;
					continue;
				}
				if (path [c] == ')') {
					propn = strndup (path + estart, c - estart);
					break;
				}
			}

			i = c;
			

			Type *t = NULL;
			if (typen) {
				t = Type::Find (typen);
				g_free (typen);
			} else
				t = Type::Find (lu->GetObjectType ());

			res = DependencyObject::GetDependencyProperty (t->type, propn);
			g_free (propn);
			break;
		}
		case '.':
			lu = lu->GetValue (res)->AsDependencyObject ();
			// we can ignore this, since we pull the lookup object when we finish a ( ) block
			break;
		case '[':
		{
			int indexer = 0;

			// Need to be a little more loving
			g_assert (path [i + 1]);
			g_assert (path [i + 2] == ']');
			g_assert (path [i + 3] == '.');

			indexer = strtol (path + i + 1, NULL, 10);

			Collection *col = lu->GetValue (res)->AsCollection ();
			List::Node *n = col->list->Index (indexer);
			lu = n ? ((Collection::Node *) n)->obj : NULL;
			i += 3;
			break;
		}
		}
	}

	if (!expression_found)
		res = DependencyObject::GetDependencyProperty (lu->GetObjectType (), path);

	*o = lu;
	return res;
}

// event handlers for c++
typedef struct {
	EventHandler func;
	gpointer data;
} EventClosure;

EventObject::EventObject ()
{
	event_hash = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
deleter (gpointer data)
{
	delete (EventClosure *) data;
}

static void
free_closure_list (gpointer key, gpointer data, gpointer userdata)
{
	GList *l = (GList*)data;
	g_free (key);
	g_list_foreach (l, (GFunc)deleter, NULL);
	g_list_free (l);
}

EventObject::~EventObject ()
{
	g_hash_table_foreach (event_hash, free_closure_list, NULL);
	g_hash_table_destroy (event_hash);
}

void
EventObject::AddHandler (char *event_name, EventHandler handler, gpointer data)
{
	GList *events = (GList*)g_hash_table_lookup (event_hash, event_name);

	EventClosure *closure = new EventClosure ();
	closure->func = handler;
	closure->data = data;

	if (events == NULL) {
		g_hash_table_insert (event_hash, g_strdup (event_name), g_list_prepend (NULL, closure));
	}
	else {
		events = g_list_append (events, closure); // not prepending means we don't need to g_hash_table_replace
	}
}

void
EventObject::RemoveHandler (char *event_name, EventHandler handler, gpointer data)
{
	gpointer key, value;
	if (!g_hash_table_lookup_extended (event_hash, event_name,
					   &key, &value)) {
		return;
	}

	GList *events = (GList*)value;

	GList *l;
	for (l = events; l; l = l->next) {
		EventClosure *closure = (EventClosure*)l->data;
		if (closure->func == handler && closure->data == data)
			break;
	}

	if (l == NULL) /* we didn't find it */
		return;

	delete (EventClosure*)l->data;
	events = g_list_delete_link (events, l);

	if (events == NULL) {
		/* delete the event */
		g_hash_table_remove (event_hash, key);
		g_free (key);
	}
	else {
		g_hash_table_replace (event_hash, key, events);
	}
}

void
EventObject::Emit (char *event_name)
{
	GList *events = (GList*)g_hash_table_lookup (event_hash, event_name);

	if (events == NULL)
		return;
	
	GList *copy = g_list_copy (events);

	for (GList *l = copy; l; l = l->next) {
		EventClosure *closure = (EventClosure*)l->data;
		closure->func (closure->data);
	}

	g_list_free (copy);
}

NameScope::NameScope ()
{
	names = g_hash_table_new (g_str_hash, g_str_equal);
}

NameScope::~NameScope ()
{
  //	g_hash_table_foreach (/* XXX */);
}

void
NameScope::RegisterName (const char *name, DependencyObject *object)
{
	g_hash_table_insert (names, g_strdup (name) ,object);
}

void
NameScope::UnregisterName (const char *name)
{
}

DependencyObject*
NameScope::FindName (const char *name)
{
	return (DependencyObject*)g_hash_table_lookup (names, name);
}

NameScope*
NameScope::GetNameScope (DependencyObject *obj)
{
	Value *v = obj->GetValue (NameScope::NameScopeProperty);
	return v == NULL ? NULL : v->AsNameScope();
}

void
NameScope::SetNameScope (DependencyObject *obj, NameScope *scope)
{
	obj->SetValue (NameScope::NameScopeProperty, scope);
}

void
TriggerCollection::Add (DependencyObject *data)
{
	FrameworkElement *fwe = (FrameworkElement *) closure;
	
	printf ("Adding %p\n", data);
	EventTrigger *trigger = (EventTrigger *) data;

	Collection::Add (trigger);

	trigger->SetTarget (fwe);
}

void
TriggerCollection::Insert (int index, DependencyObject *data)
{
	FrameworkElement *fwe = (FrameworkElement *) closure;
	
	printf ("Inserting %p\n", data);
	EventTrigger *trigger = (EventTrigger *) data;

	Collection::Insert (index, trigger);

	trigger->SetTarget (fwe);
}

void
TriggerCollection::Remove (DependencyObject *data)
{
	FrameworkElement *fwe = (FrameworkElement *) closure;
	
	EventTrigger *trigger = (EventTrigger *) data;

	Collection::Remove (trigger);

	trigger->RemoveTarget (fwe);
}

void
ResourceCollection::Add (DependencyObject *data)
{
	Collection::Add (data);
}

void
ResourceCollection::Remove (DependencyObject *data)
{
	Collection::Remove (data);
}

void
StrokeCollection::Add (DependencyObject *data)
{
	Collection::Add (data);
}

void
StrokeCollection::Remove (DependencyObject *data)
{
	Collection::Remove (data);
}

void
MediaAttributeCollection::Add (DependencyObject *data)
{
	Collection::Add (data);
}

void
MediaAttributeCollection::Remove (DependencyObject *data)
{
	Collection::Remove (data);
}

void
StylusPointCollection::Add (DependencyObject *data)
{
	Collection::Add (data);
}

void
StylusPointCollection::Remove (DependencyObject *data)
{
	Collection::Remove (data);
}

void
TimelineMarkerCollection::Add (DependencyObject *data)
{
	Collection::Add (data);
}

void
TimelineMarkerCollection::Remove (DependencyObject *data)
{
	Collection::Remove (data);
}

void
Inlines::Add (DependencyObject *data)
{
	Collection::Add (data);
}

void
Inlines::Remove (DependencyObject *data)
{
	Collection::Remove (data);
}


Collection *
collection_new (Type::Kind kind)
{
	switch (kind) {
	case Type::GEOMETRY_COLLECTION:
		return new GeometryCollection ();
	case Type::GRADIENTSTOP_COLLECTION:
		return new GradientStopCollection ();
	case Type::INLINES:
		return new Inlines ();
	case Type::KEYFRAME_COLLECTION:
		return new KeyFrameCollection ();
	case Type::MEDIAATTRIBUTE_COLLECTION:
		return new MediaAttributeCollection ();
	case Type::PATHFIGURE_COLLECTION:
		return new PathFigureCollection ();
	case Type::PATHSEGMENT_COLLECTION:
		return new PathSegmentCollection ();
	case Type::RESOURCE_COLLECTION:
		return new ResourceCollection ();
	case Type::STROKE_COLLECTION:
		return new StrokeCollection ();
	case Type::STYLUSPOINT_COLLECTION:
		return new StylusPointCollection ();
	case Type::TIMELINE_COLLECTION:
		return new TimelineCollection ();
	case Type::TIMELINEMARKER_COLLECTION:
		return new TimelineMarkerCollection ();
	case Type::TRANSFORM_COLLECTION:
		return new TransformCollection ();
	case Type::TRIGGER_COLLECTION:
		return new TriggerCollection ();
	case Type::TRIGGERACTION_COLLECTION:
		return new TriggerActionCollection ();
	case Type::VISUAL_COLLECTION:
		return new VisualCollection ();
	default:
		return NULL;
	}
}

VisualCollection *
visual_collection_new (void)
{
	return new VisualCollection ();
}

TriggerCollection *
trigger_collection_new (void)
{
	return new TriggerCollection ();
}

TriggerActionCollection *
trigger_action_collection_new (void)
{
	return new TriggerActionCollection ();
}

ResourceCollection *
resource_collection_new (void)
{
	return new ResourceCollection ();
}

StrokeCollection *
stroke_collection_new (void)
{
	return new StrokeCollection ();
}

StylusPointCollection *
stylus_point_collection_new (void)
{
	return new StylusPointCollection ();
}

TimelineMarkerCollection *
timeline_marker_collection_new (void)
{
	return new TimelineMarkerCollection ();
}

MediaAttributeCollection *
media_attribute_collection_new (void)
{
	return new MediaAttributeCollection ();
}

Inlines *
inlines_new (void)
{
	return new Inlines ();
}


EventTrigger::EventTrigger ()
{
	this->SetValue (EventTrigger::ActionsProperty, Value (new TriggerActionCollection ()));
}

//
// Intercept any changes to the actions property and mirror that into our
// own variable
//
void
EventTrigger::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == ActionsProperty){
		TriggerActionCollection *newcol = GetValue (prop)->AsTriggerActionCollection();

		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	}
}

void
EventTrigger::SetTarget (DependencyObject *target)
{
	g_assert (target);

	// Despite the name, it can only be loaded (according to the docs)
	target->events->AddHandler ("Loaded", (EventHandler) event_trigger_fire_actions, this);
}

void
EventTrigger::RemoveTarget (DependencyObject *target)
{
	g_assert (target);

	target->events->RemoveHandler ("Loaded", (EventHandler) event_trigger_fire_actions, this);
}

EventTrigger::~EventTrigger ()
{
}

EventTrigger *
event_trigger_new (void)
{
	return new EventTrigger ();
}

void
event_trigger_action_add (EventTrigger *trigger, TriggerAction *action)
{
	printf ("Adding action\n");
	trigger->GetValue (EventTrigger::ActionsProperty)->AsTriggerActionCollection()->Add (action);
}

void
event_trigger_fire_actions (EventTrigger *trigger)
{
	g_assert (trigger);
	TriggerActionCollection *actions = trigger->GetValue (EventTrigger::ActionsProperty)->AsTriggerActionCollection();
	Collection::Node *n = (Collection::Node *) actions->list->First ();
	
	for ( ; n != NULL; n = (Collection::Node *) n->Next ()) {
		TriggerAction *action = (TriggerAction *) n->obj;
		action->Fire ();
	}
}

//
// UIElement
//
DependencyProperty* UIElement::RenderTransformProperty;
DependencyProperty* UIElement::OpacityProperty;
DependencyProperty* UIElement::ClipProperty;
DependencyProperty* UIElement::OpacityMaskProperty;
DependencyProperty* UIElement::TriggersProperty;
DependencyProperty* UIElement::RenderTransformOriginProperty;
DependencyProperty* UIElement::CursorProperty;
DependencyProperty* UIElement::IsHitTestVisibleProperty;
DependencyProperty* UIElement::VisibilityProperty;
DependencyProperty* UIElement::ResourcesProperty;


void
item_init (void)
{
	UIElement::RenderTransformProperty = DependencyObject::Register (Type::UIELEMENT, "RenderTransform", Type::TRANSFORM);
	UIElement::OpacityProperty = DependencyObject::Register (Type::UIELEMENT, "Opacity", new Value(1.0));
	UIElement::ClipProperty = DependencyObject::Register (Type::UIELEMENT, "Clip", Type::GEOMETRY);
	UIElement::OpacityMaskProperty = DependencyObject::Register (Type::UIELEMENT, "OpacityMask", Type::BRUSH);
	UIElement::TriggersProperty = DependencyObject::Register (Type::UIELEMENT, "Triggers", Type::TRIGGER_COLLECTION);
	UIElement::RenderTransformOriginProperty = DependencyObject::Register (Type::UIELEMENT, "RenderTransformOrigin", Type::POINT);
	UIElement::CursorProperty = DependencyObject::Register (Type::UIELEMENT, "Cursor", Type::INT32);
	UIElement::IsHitTestVisibleProperty = DependencyObject::Register (Type::UIELEMENT, "IsHitTestVisible", Type::BOOL);
	UIElement::VisibilityProperty = DependencyObject::Register (Type::UIELEMENT, "Visibility", Type::INT32);
	UIElement::ResourcesProperty = DependencyObject::Register (Type::UIELEMENT, "Resources", Type::RESOURCE_COLLECTION);
}

//
// Namescope
//
DependencyProperty *NameScope::NameScopeProperty;

void
namescope_init (void)
{
	NameScope::NameScopeProperty = DependencyObject::Register (Type::NAMESCOPE, "NameScope", Type::NAMESCOPE);
}

DependencyProperty* FrameworkElement::HeightProperty;
DependencyProperty* FrameworkElement::WidthProperty;

void
framework_element_init (void)
{
	FrameworkElement::HeightProperty = DependencyObject::Register (Type::FRAMEWORKELEMENT, "Height", new Value (0.0));
	FrameworkElement::WidthProperty = DependencyObject::Register (Type::FRAMEWORKELEMENT, "Width", new Value (0.0));
}

DependencyProperty* Panel::ChildrenProperty;
DependencyProperty* Panel::BackgroundProperty;

void 
panel_init (void)
{
	Panel::ChildrenProperty = DependencyObject::Register (Type::PANEL, "Children", Type::VISUAL_COLLECTION);
	Panel::BackgroundProperty = DependencyObject::Register (Type::PANEL, "Background", Type::BRUSH);
}

DependencyProperty* Canvas::TopProperty;
DependencyProperty* Canvas::LeftProperty;

void 
canvas_init (void)
{
	Canvas::TopProperty = DependencyObject::RegisterFull (Type::CANVAS, "Top", new Value (0.0), Type::DOUBLE, true);
	Canvas::LeftProperty = DependencyObject::RegisterFull (Type::CANVAS, "Left", new Value (0.0), Type::DOUBLE, true);
}

DependencyProperty* DependencyObject::NameProperty;

void
dependency_object_init(void)
{
	DependencyObject::NameProperty = DependencyObject::Register (Type::DEPENDENCY_OBJECT, "Name", Type::STRING);
}

DependencyProperty* EventTrigger::RoutedEventProperty;
DependencyProperty* EventTrigger::ActionsProperty;

void
event_trigger_init (void)
{
	EventTrigger::RoutedEventProperty = DependencyObject::Register (Type::EVENTTRIGGER, "RoutedEvent", Type::STRING);
	EventTrigger::ActionsProperty = DependencyObject::Register (Type::EVENTTRIGGER, "Actions", Type::TRIGGERACTION_COLLECTION);
}

static bool inited = false;

void
runtime_init (void)
{
	if (inited)
		return;
	
	if (cairo_version () < CAIRO_VERSION_ENCODE(1,4,0)) {
		printf ("*** WARNING ***\n");
		printf ("*** Cairo versions < 1.4.0 should not be used for Moon.\n");
		printf ("*** Moon was configured to use Cairo version %d.%d.%d, but\n", CAIRO_VERSION_MAJOR, CAIRO_VERSION_MINOR, CAIRO_VERSION_MICRO);
		printf ("*** is being run against version %s.\n", cairo_version_string ());
		printf ("*** Proceed at your own risk\n");
	}

	inited = true;

	g_type_init ();

	TimeManager::Instance()->Start();

	types_init ();
	namescope_init ();
	item_init ();
	framework_element_init ();
	canvas_init ();
	dependency_object_init();
	event_trigger_init ();
	transform_init ();
	animation_init ();
	brush_init ();
	shape_init ();
	geometry_init ();
	xaml_init ();
	clock_init ();
	text_init ();
	downloader_init ();
	media_init ();
	panel_init ();
}

void
runtime_shutdown ()
{
	if (!inited)
		return;

	TimeManager::Instance()->Shutdown ();
	Type::Shutdown ();
	DependencyObject::Shutdown ();
}

void surface_register_events (Surface *s,
			      callback_mouse_event motion, callback_mouse_event down, callback_mouse_event up,
			      callback_mouse_event enter,
			      callback_plain_event got_focus, callback_plain_event lost_focus,
			      callback_plain_event loaded, callback_plain_event mouse_leave, callback_plain_event surface_resize,
			      callback_keyboard_event keydown, callback_keyboard_event keyup)
{
	s->cb_motion = motion;
	s->cb_down = down;
	s->cb_up = up;
	s->cb_enter = enter;
	s->cb_got_focus = got_focus;
	s->cb_lost_focus = lost_focus;
	s->cb_loaded = loaded;
	s->cb_mouse_leave = mouse_leave;
	s->cb_keydown = keydown;
	s->cb_keyup = keyup;
	s->cb_surface_resize = surface_resize;
}

