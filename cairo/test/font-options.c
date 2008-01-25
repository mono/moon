/*
 * Copyright © 2008 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Chris Wilson not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Chris Wilson makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * CHRIS WILSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL CHRIS WILSON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include <cairo.h>
#include <assert.h>
#include <stdlib.h>

int
main (void)
{
    cairo_font_options_t *options1;
    cairo_font_options_t *options2;
    cairo_surface_t *surface;
    cairo_matrix_t identity;
    cairo_t *cr;
    cairo_scaled_font_t *scaled_font;

    /* first check NULL handling of cairo_font_options_t */
    options1 = cairo_font_options_create ();
    options2 = cairo_font_options_copy (NULL);

    assert (cairo_font_options_equal (options1, options2));
    assert (cairo_font_options_equal (NULL, options2));
    assert (cairo_font_options_equal (options1, NULL));

    assert (cairo_font_options_hash (options1) == cairo_font_options_hash (options2));
    assert (cairo_font_options_hash (NULL) == cairo_font_options_hash (options2));
    assert (cairo_font_options_hash (options1) == cairo_font_options_hash (NULL));

    cairo_font_options_merge (NULL, NULL);
    cairo_font_options_merge (options1, NULL);
    cairo_font_options_merge (options1, options2);

    cairo_font_options_destroy (NULL);
    cairo_font_options_destroy (options1);
    cairo_font_options_destroy (options2);

    cairo_font_options_set_antialias (NULL, CAIRO_ANTIALIAS_DEFAULT);
    cairo_font_options_get_antialias (NULL);

    cairo_font_options_set_subpixel_order (NULL, CAIRO_SUBPIXEL_ORDER_DEFAULT);
    cairo_font_options_get_subpixel_order (NULL);

    cairo_font_options_set_hint_style (NULL, CAIRO_HINT_STYLE_DEFAULT);
    cairo_font_options_get_hint_style (NULL);

    cairo_font_options_set_hint_metrics (NULL, CAIRO_HINT_METRICS_DEFAULT);
    cairo_font_options_get_hint_metrics (NULL);

    /* Now try creating fonts with NULLs */
    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 0, 0);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_matrix_init_identity (&identity);
    scaled_font = cairo_scaled_font_create (cairo_get_font_face (cr),
	                                    &identity, &identity,
					    NULL);
    assert (cairo_scaled_font_status (scaled_font) == CAIRO_STATUS_SUCCESS);
    cairo_scaled_font_get_font_options (scaled_font, NULL);
    cairo_scaled_font_destroy (scaled_font);

    cairo_set_font_options (cr, NULL);
    cairo_get_font_options (cr, NULL);

    cairo_destroy (cr);

    return 0;
}
