/*
 * Copyright © 2007 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: Behdad Esfahbod <behdad@behdad.org>
 */

#include "cairo-test.h"

#define SIZE 90

static cairo_test_draw_function_t draw;

cairo_test_t test = {
    "extend-pad-similar",
    "Test CAIRO_EXTEND_PAD for surface patterns",
    SIZE, SIZE,
    draw
};

static cairo_surface_t *
create_source_surface (cairo_surface_t *target)
{
    const int surface_size = (SIZE - 30) / 10;
    cairo_surface_t *surface;
    cairo_t *cr;

    /* Create an image surface with my favorite four colors in each
     * quadrant. */
    surface = cairo_surface_create_similar (target, CAIRO_CONTENT_COLOR,
					    surface_size, surface_size);
    cr = cairo_create (surface);
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_rectangle (cr,
		     0, 0,
		     surface_size / 2, surface_size / 2);
    cairo_fill (cr);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_rectangle (cr,
		     surface_size / 2, 0,
		     surface_size / 2, surface_size / 2);
    cairo_fill (cr);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_rectangle (cr,
		     0, surface_size / 2,
		     surface_size / 2, surface_size / 2);
    cairo_fill (cr);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_rectangle (cr,
		     surface_size / 2, surface_size / 2,
		     surface_size / 2, surface_size / 2);
    cairo_fill (cr);
    cairo_destroy (cr);

    return surface;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *surface;

    surface = create_source_surface (cairo_get_group_target (cr));

    cairo_set_source_rgba (cr, 0, 0, 0, 1);
    cairo_rectangle (cr, 0, 0, SIZE, SIZE);
    cairo_fill (cr);

    cairo_scale (cr, 10, 10);
    cairo_set_source_surface (cr, surface, 1.5, 1.5);
    cairo_surface_destroy (surface);

    cairo_pattern_set_extend (cairo_get_source (cr), CAIRO_EXTEND_PAD);
    cairo_rectangle (cr, 1.5, 1.5, 6, 6);
    cairo_clip (cr);

    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

int
main (void)
{
    return cairo_test (&test);
}
