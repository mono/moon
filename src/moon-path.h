/*
 * moon-path.h: Path-based API, similar to cairo but without requiring a cairo_context_t
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007, 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_PATH_H__
#define __MOON_PATH_H__

#include <string.h>
#include <math.h>
#include <glib.h>
#include <cairo.h>

G_BEGIN_DECLS

// http://graphics.stanford.edu/courses/cs248-98-fall/Final/q1.html
#define ARC_TO_BEZIER	0.55228475

typedef struct {
	cairo_path_t	cairo;
	int allocated;				/* how many cairo_path_data_t were allocated */
} moon_path;

/* the numbers of cairo_path_data_t required to hold a path containing the matching call */
#define MOON_PATH_MOVE_TO_LENGTH		2
#define MOON_PATH_LINE_TO_LENGTH		2
#define MOON_PATH_CURVE_TO_LENGTH		4
#define MOON_PATH_ELLIPSE_LENGTH		18
#define MOON_PATH_RECTANGLE_LENGTH		9
#define MOON_PATH_ROUNDED_RECTANGLE_LENGTH	27
#define MOON_PATH_CLOSE_PATH_LENGTH		1

// is it true only for arcs or for everything ? if so using the same values ?
// note: lupus noted this suggest SL uses floats, not doubles, internally (MIL?)
#define IS_ZERO(x)	(fabs(x) < 0.000019)
#define IS_TOO_SMALL(x)	(fabs(x) < 0.000117)

/*
 * These functions are similar to cairo_* functions (with some extra ones) except that they don't require a cairo_context_t
 * in order to build a cairo_path_t.
 */

moon_path*	moon_path_new (int size);
moon_path*	moon_path_renew (moon_path* path, int size);
void		moon_path_clear (moon_path* path);
void		moon_path_destroy (moon_path* path);
void		moon_get_current_point (moon_path *path, double *x, double *y);
void		moon_move_to (moon_path *path, double x, double y);
void		moon_line_to (moon_path *path, double x, double y);
void		moon_curve_to (moon_path *path, double x1, double y1, double x2, double y2, double x3, double y3);
void		moon_quad_curve_to (moon_path* path, double x1, double y1, double x2, double y2);
void		moon_arc_to (moon_path *path, double width, double height, double angle, gboolean large, gboolean sweep, double ex, double ey);
void		moon_ellipse (moon_path *path, double x, double y, double w, double h);
void		moon_rectangle (moon_path *path, double x, double y, double w, double h);
void		moon_rounded_rectangle (moon_path *path, double x, double y, double w, double h, double radius_x, double radius_y);
void		moon_close_path (moon_path *path);
void		moon_get_origin (moon_path *path, double *ox, double *oy);
void		moon_merge (moon_path *path, moon_path *subpath);

// for debugging purpose
void		cairo_path_display (cairo_path_t *path);
void		moon_path_display (moon_path *path);

G_END_DECLS

#endif
