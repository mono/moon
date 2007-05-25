/*
 * runtime.cpp: Core surface and canvas definitions.
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#if AGG
#    include <agg_rendering_buffer.h>
#    include "Agg2D.h"
#else
#    define CAIRO 1
#endif

#include "runtime.h"

#if AGG
struct _SurfacePrivate {
	agg::rendering_buffer rbuf;
	Agg2D *graphics;
};
#endif

ObjectVtable object_vtable = { NULL } ;

void
object_init (Object *obj)
{
	obj->vtable = &object_vtable;
}

void
object_class_init ()
{
	// Nothing.
}

static void
vt_item_render (Item *item, Surface *s, double *affine, int x, int y, int width, int height)
{
	fprintf (stderr, "vt_item_render called, you must implement this method");
}

static void
vt_item_getbounds (Item *item)
{
	fprintf (stderr, "vt_item_getbounds called, you must implement this method");
}

ItemVtable item_vtable;

item_class_init ()
{
	item_vtable.object_vtable = object_vtable;
	item_vtable.render = vt_item_render;
	item_vtable.getbounds = vt_item_getbounds;
}

/**
 * item_getbounds:
 * @item: the item to update the bounds of
 *
 * Does this by requesting bounds update to all of its parents. 
 */
void
item_update_bounds (Item *item)
{
	double cx1 = item->x1;
	double cy1 = item->y1;
	double cx2 = item->x2;
	double cy2 = item->y2;
	
	ItemVtable *vt = (ItemVtable *) item->object.vtable;
	vt->getbounds (item);

	//
	// If we changed, notify the parent to recompute its bounds
	//
	if (item->x1 != cx1 || item->y1 != cy1 || item->y2 != cy2 || item->x2 != cx2){
		if (item->parent != NULL)
			item_update_bounds (item->parent);
	}
}

void
item_init (Item *item)
{
	object_init (&item->object);
	item->object.vtable = &item_vtable;
}

/*
 * item_get_affine:
 * @container: the affine transform from the container
 * @affine: the affine transform for this item
 * @result: a place that can contain 6 doubles where the data is saved
 * 
 * Composites the container affine transformation with the item affine
 * transformation.
 *
 * Returns the pointer to result if the caller must apply affine
 * transformations, or NULL if the affine should not be used.
 */
double *
item_get_affine (double *container, double *affine, double *result)
{
	int i;

	if (container == NULL){
		if (affine == NULL)
			return FALSE;
		for (i = 0; i < 6; i++)
			result [i] = affine [i];
		return result;
	} 
	if (affine == NULL){
		for (i = 0; i < 6; i++)
			result [i] = container [i];
		return result;
	}
	
	result [0] = container [0] * affine [0] + container [1] * affine [2];
	result [1] = container [0] * affine [1] + container [1] * affine [3];
	result [2] = container [2] * affine [0] + container [3] * affine [2];
	result [3] = container [2] * affine [1] + container [3] * affine [3];
	result [4] = container [4] * affine [0] + container [5] * affine [2] + affine [4];
	result [5] = container [4] * affine [1] + container [5] * affine [3] + affine [5];
	
	return result;
}

void 
item_destroy (Item *item)
{
	if (item->xform)
		free (item->xform);
}

//
// item_affine_get_absolute:
//
//   Returns the absolute affine transformation for this given item
//   (composing the full chain from the top to this item)
//
double *
item_affine_get_absolute (Item *item, double *result)
{
	double res [6];
	double *ret;

	if (item->parent == NULL){
		return item_get_affine (NULL, item->xform, result);
	}

	ret = item_affine_get_absolute (item->parent, res);
	return item_get_affine (ret, item->xform, result);
}

void 
item_invalidate (Item *item)
{
	double res [6];
	double *affine = item_affine_get_absolute (item, res);
	Surface *s = item_surface_get (item);

	if (s == NULL)
		return;

#ifdef DEBUG_INVALIDATE
	printf ("Requesting invalidate for %d %d %d %d\n", 
				    (int) item->x1, (int)item->y1, 
				    (int)(item->x2-item->x1), (int)(item->y2-item->y1));
#endif
	gtk_widget_queue_draw_area ((GtkWidget *)s->data, 
				    (int) item->x1, (int)item->y1, 
				    (int)(item->x2-item->x1), (int)(item->y2-item->y1));
}

void
item_transform_set (Item *item, double *transform)
{
	double *d;

	item_invalidate (item);

	if (item->xform)
		free (item->xform);

	if (transform == NULL)
		item->xform = transform;
	else {
		d = (double *) malloc (sizeof (double) * 6);
		
		d [0] = transform [0];
		d [1] = transform [1];
		d [2] = transform [2];
		d [3] = transform [3];
		d [4] = transform [4];
		d [5] = transform [5];
		
		item->xform = d;
	}
	ItemVtable *vt = (ItemVtable *) item->object.vtable;
	vt->getbounds (item);
	item_invalidate (item);
}

static void 
vt_group_render (Item *item, Surface *s, double *affine, int x, int y, int width, int height)
{
	Group *group = (Group *) item; 
	GSList *il;
	double actual [6];
	double *use_affine = item_get_affine (affine, item->xform, actual);
	
	for (il = group->items; il != NULL; il = il->next){
		Item *item = (Item *) il->data;
		ItemVtable *vt = (ItemVtable *) item->object.vtable;

		vt->render (item, s, use_affine, x, y, width, height);
	}
}

static void
vt_group_getbounds (Item *item)
{
	
}

static ItemVtable group_vtable;

static void
group_class_init ()
{
	group_vtable.object_vtable = object_vtable;
	group_vtable.render = vt_group_render;
	group_vtable.getbounds = vt_group_getbounds;
}

void 
group_init (Group *group)
{
	item_init (&group->item);
	group->item.object.vtable = &group_vtable;
}

void 
group_item_add (Group *group, Item *item)
{
	group->items = g_slist_append (group->items, item);
	item->parent = (Item *) group;

	ItemVtable *vt = (ItemVtable *) item->object.vtable;
	vt->getbounds (item);
}

void
shape_class_init ()
{
	// Nothing, this is pure abstract
}

void 
shape_init (Shape *shape)
{
	item_init (&shape->item);
}

static void
rectangle_draw (Item *item, Surface *s, double *affine)
{
	Rectangle *r = (Rectangle *) item;
	double result [6];
	double *matrix = item_get_affine (affine, item->xform, result);

	cairo_save (s->cairo);
	if (matrix != NULL)
		cairo_set_matrix (s->cairo, (cairo_matrix_t *) matrix);

	cairo_rectangle (s->cairo, r->x, r->y, r->w, r->h);
	cairo_restore (s->cairo);
}

static ItemVtable surface_vtable;

static void
surface_class_init ()
{
	surface_vtable = group_vtable;
}

Surface *
item_surface_get (Item *item)
{
	if (item->object.vtable == &surface_vtable)
		return (Surface *) item;
	if (item->parent != NULL)
		return item_surface_get (item->parent);

	return NULL;
}

static void 
vt_rectangle_render (Item *item, Surface *s, double *affine, int x, int y, int width, int height)
{
#if CAIRO
	rectangle_draw (item, s, affine);
	cairo_stroke (s->cairo);
#else
	Rectangle *r = (Rectangle *) item;
	Agg2D *g = s->priv->graphics;

	double result[6];
	double *matrix = item_get_affine (affine, item->xform, result);

	if (matrix != NULL){
		Agg2D::Affine a (matrix [0], matrix [1], matrix [2], matrix [3], matrix [4], matrix [5]);
		
		g->affine (a);
	}
        g->lineColor (0, 0, 0);
        g->noFill ();
        g->rectangle (r->x, r->y, r->x + r->w, r->y + r->h);
	if (matrix != NULL)
		g->resetTransformations ();
#endif
}

static void
vt_rectangle_getbounds (Item *item)
{
	double res [6];
	double *affine = item_affine_get_absolute (item, res);

	Surface *s = item_surface_get (item);
	if (s == NULL){
		// not yet attached
		return;
	}
#if CAIRO
	rectangle_draw (item, s, affine);
	cairo_stroke_extents (s->cairo, &item->x1, &item->y1, &item->x2, &item->y2);
	cairo_new_path (s->cairo);
#else
#endif
}

static ItemVtable rectangle_vtable;

void
rectangle_class_init ()
{
	rectangle_vtable.object_vtable = object_vtable;
	rectangle_vtable.render = vt_rectangle_render;
	rectangle_vtable.getbounds = vt_rectangle_getbounds;
}

void
rectangle_init (Rectangle *rectangle)
{
	shape_init (&rectangle->shape);
	Item *i = &rectangle->shape.item;
	i->object.vtable = &rectangle_vtable;
}

Item *
rectangle_new (double x, double y, double w, double h)
{
	Rectangle *rect = g_new0 (Rectangle, 1);
	rectangle_init (rect);

	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;

	return (Item*)rect;
}

void 
surface_clear (Surface *s, int x, int y, int width, int height)
{
	if (x < 0){
		width -= x;
		x = 0;
	}

	if (y < 0){
		height -= y;
		y = 0;
	}
	width = MAX (0, MIN (width, s->width));
	height = MAX (0, MIN (height, s->height));

	int stride = s->width * 4;
	unsigned char *dest = s->buffer + (y * stride) + (x * 4);

	while (height--){
		memset (dest, 14, width * 4);
		dest += stride;
	}
}

void
surface_clear_all (Surface *s)
{
	memset (s->buffer, 14, s->width * s->height * 4);
}

static void
surface_realloc (Surface *s)
{
	if (s->buffer){
		free (s->buffer);
		gdk_pixbuf_unref (s->pixbuf);
	}

	int size = s->width * s->height * 4;
	s->buffer = (unsigned char *) malloc (size);
	surface_clear_all (s);
       
	//
	// Hook up our surfaces
	//

	s->pixbuf = gdk_pixbuf_new_from_data (s->buffer, GDK_COLORSPACE_RGB, TRUE, 8, 
					      s->width, s->height, s->width * 4, NULL, NULL);

#if AGG
	s->priv = g_new0 (SurfacePrivate, 1);
	s->priv->rbuf =  agg::rendering_buffer ();
	s->priv->rbuf.attach  (s->buffer, s->width, s->height, s->width * 4);
	s->priv->graphics = new Agg2D();
	s->priv->graphics->attach (s->buffer, s->width, s->height, s->width * 4);
	s->priv->graphics->viewport (0, 0, s->width, s->height, 0, 0, s->width, s->height, Agg2D::XMidYMid);
#else

	s->cairo_surface = cairo_image_surface_create_for_data (s->buffer, CAIRO_FORMAT_ARGB32, s->width, s->height, s->width * 4);
	s->cairo = cairo_create (s->cairo_surface);
	//cairo_set_operator (s->cairo, CAIRO_OPERATOR_SOURCE);
	//cairo_paint (s->cairo);
	//cairo_set_operator (s->cairo, CAIRO_OPERATOR_SOURCE);
	//cairo_set_source_rgb (s->cairo, 0, 1, 0);
#endif
}

void
surface_init (Surface *s, int width, int height)
{
	group_init (&s->group);
	s->group.item.object.vtable = &surface_vtable;
	s->width = width;
	s->height = height;
	surface_realloc (s);
}

void 
surface_destroy (Surface *s)
{
	cairo_surface_destroy (s->cairo_surface);
	free (s);
}

Surface *
surface_new (int width, int height)
{
	Surface *s = g_new0 (Surface, 1);

	surface_init (s, width, height);

	return s;
}

void
surface_repaint (Surface *s, int x, int y, int width, int height)
{
	surface_clear (s, x, y, width, height);
	vt_group_render ((Item *) s, s, NULL, x, y, width, height);
}

void
runtime_init ()
{
	object_class_init ();
	item_class_init ();
	group_class_init ();
	shape_class_init ();
	rectangle_class_init ();
	surface_class_init ();
	video_class_init ();
}
