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
	
	item->getbounds ();

	//
	// If we changed, notify the parent to recompute its bounds
	//
	if (item->x1 != cx1 || item->y1 != cy1 || item->y2 != cy2 || item->x2 != cx2){
		if (item->parent != NULL)
			item_update_bounds (item->parent);
	}
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
	item->getbounds ();
	item_invalidate (item);
}

Surface *
item_surface_get (Item *item)
{
	if (item->flags & Video::IS_SURFACE)
		return (Surface *) item;

	if (item->parent != NULL)
		return item_surface_get (item->parent);

	return NULL;
}

void
Group::render (Surface *s, double *affine, int x, int y, int width, int height)
{
	GSList *il;
	double actual [6];
	double *use_affine = item_get_affine (affine, xform, actual);
	
	for (il = items; il != NULL; il = il->next){
		Item *item = (Item *) il->data;

		item->render (s, use_affine, x, y, width, height);
	}
}

void
Group::getbounds ()
{
	bool first = TRUE;
	GSList *il;

	for (il = items; il != NULL; il = il->next){
		Item *item = (Item *) il->data;

		item->getbounds ();
		if (first){
			x1 = item->x1;
			x2 = item->x2;
			y1 = item->y1;
			y2 = item->y2;
			first = FALSE;
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

	// If we found nothing.
	if (first){
		x1 = y1 = x2 = y2 = 0;
	}
}

void 
group_item_add (Group *group, Item *item)
{
	group->items = g_slist_append (group->items, item);
	item->parent = (Item *) group;

	item->getbounds ();
}

void 
Shape::DoDraw (Surface *s, bool do_op)
{
	if (fill){
		fill->SetupBrush (s->cairo);
		Draw (s);
		if (do_op)
			cairo_fill (s->cairo);
	}

	if (stroke){
		stroke->SetupBrush (s->cairo);
		Draw (s);
		if (do_op)
			cairo_stroke (s->cairo);
	}

}

void
Shape::render (Surface *s, double *affine, int x, int y, int width, int height)
{
	double result [6];
	double *matrix = item_get_affine (affine, xform, result);

	cairo_save (s->cairo);
	if (matrix != NULL)
		cairo_set_matrix (s->cairo, (cairo_matrix_t *) matrix);

	DoDraw (s, TRUE);

	cairo_restore (s->cairo);
}

void 
Shape::getbounds ()
{
	double res [6];
	double *affine = item_affine_get_absolute (this, res);

	Surface *s = item_surface_get (this);
	if (s == NULL){
		// not yet attached
		return;
	}

	DoDraw (s, FALSE);
	cairo_stroke_extents (s->cairo, &x1, &y1, &x2, &y2);
	cairo_new_path (s->cairo);
}

void 
shape_set_fill (Shape *shape, Brush *fill)
{
	if (shape->fill != NULL)
		brush_unref (shape->fill);

	shape->fill = brush_ref (fill);
}

void 
shape_set_stroke (Shape *shape, Brush *stroke)
{
	if (shape->stroke != NULL)
		brush_unref (shape->stroke);

	shape->stroke = brush_ref (stroke);
}

void
Rectangle::Draw (Surface *s)
{
	printf ("here\n");
	cairo_rectangle (s->cairo, x, y, w, h);
}

Rectangle *
rectangle_new (double x, double y, double w, double h)
{
	Rectangle *rect = new Rectangle (x, y, w, h);

	return rect;
}

void
Line::Draw (Surface *s)
{
	cairo_move_to (s->cairo, line_x1, line_x2);
	cairo_line_to (s->cairo, line_x2, line_y2);
}

Line *
line_new (double x1, double y1, double x2, double y2)
{
	return new Line (x1, y1, x2, y2);
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
       
	s->pixbuf = gdk_pixbuf_new_from_data (s->buffer, GDK_COLORSPACE_RGB, TRUE, 8, 
					      s->width, s->height, s->width * 4, NULL, NULL);

	s->cairo_surface = cairo_image_surface_create_for_data (
		s->buffer, CAIRO_FORMAT_ARGB32, s->width, s->height, s->width * 4);

	s->cairo = cairo_create (s->cairo_surface);
}

void 
surface_destroy (Surface *s)
{
	cairo_surface_destroy (s->cairo_surface);
	delete s;
}

Surface *
surface_new (int width, int height)
{
	Surface *s = new Surface ();

	s->buffer = NULL;
	s->flags |= Item::IS_SURFACE;
	s->width = width;
	s->height = height;
	surface_realloc (s);

	return s;
}

void
surface_repaint (Surface *s, int x, int y, int width, int height)
{
	surface_clear (s, x, y, width, height);
	s->render (s, NULL, x, y, width, height);
}

