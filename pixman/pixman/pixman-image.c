/*
 * Copyright © 2000 SuSE, Inc.
 * Copyright © 2007 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "pixman-private.h"
#include "pixman-combine32.h"

#define USE_COLOR_LUT_CACHE 1

#if USE_COLOR_LUT_CACHE
typedef struct lut_cache_entry {
  char *key;
  int key_len;
  uint32_t *lut;
} lut_cache_entry_t;

#define LUT_CACHE_EXPIRE_SIZE 16
#define LUT_CACHE_SIZE 2000

static lut_cache_entry_t lut_cache [LUT_CACHE_SIZE];
static int num_lut_cache_entries = 0;

static inline int
locate_color_lut (char *key, int key_len)
{
  int i;
  for (i = 0; i < num_lut_cache_entries; i ++)
    if (key_len == lut_cache[i].key_len &&
	!memcmp (key, lut_cache[i].key, key_len))
      return i;
  return -1;
}

static inline int
promote_color_lut (int i)
{
  int delta;
  lut_cache_entry_t t;

  if (i == 0)
    return 0;

  delta = i == 1 ? 1 : 2;

  t = lut_cache[i];
  lut_cache[i] = lut_cache[i - delta];
  lut_cache[i - delta] = t;
  return i - delta;
}

static inline uint32_t*
lookup_color_lut (char *key, int key_len)
{
  int i = locate_color_lut (key, key_len);
  if (i == -1)
    return NULL;
  /* printf ("color cache %s cache hit\n", key); */
  i = promote_color_lut (i);
  return lut_cache [i].lut;
}

static inline void
add_lut_cache (char *key, int key_len, uint32_t *lut)
{
  if (num_lut_cache_entries == LUT_CACHE_SIZE-1) {
    int i;
    /* printf ("color cache table size exceeded, dropping %d\n", LUT_CACHE_EXPIRE_SIZE); */
    /* time to expire something */
    for (i = 0; i < LUT_CACHE_EXPIRE_SIZE; i ++) {
      free (lut_cache[num_lut_cache_entries-1-i].key);
      free (lut_cache[num_lut_cache_entries-1-i].lut);
    }
    num_lut_cache_entries -= LUT_CACHE_EXPIRE_SIZE;
  }

  lut_cache[num_lut_cache_entries].key = key;
  lut_cache[num_lut_cache_entries].key_len = key_len;
  lut_cache[num_lut_cache_entries].lut = lut;
  num_lut_cache_entries++;
  /* printf ("color cache %s added\n", key); */
}
#endif

static force_inline int clamp_fixed_to_ffff(pixman_fixed_t x)
{
    return x - (x >> 16);
}

static void
fill_in_32bit_lut(uint32_t *cache, int index, pixman_color_t c0, pixman_color_t c1,
		  int count)
{
    uint16_t r = c0.red;
    uint16_t g = c0.green;
    uint16_t b = c0.blue;
    uint16_t a = c0.alpha;
    int16_t dr = (c1.red - r) / (count - 1);
    int16_t dg = (c1.green - g) / (count - 1);
    int16_t db = (c1.blue - b) / (count - 1);
    int16_t da = (c1.alpha - a) / (count - 1);

    char *cb = (char*)(cache + index);

    do {
        *cb++ = (b >> 8) * (a >> 8) / 255;
	*cb++ = (g >> 8) * (a >> 8) / 255;
	*cb++ = (r >> 8) * (a >> 8) / 255;
	*cb++ = a >> 8;

	a += da;
	r += dr;
	g += dg;
	b += db;
    } while (--count != 0);
}

void
_pixman_gradient_build_32bit_lut (gradient_t *gradient)
{
    int cache_size;
    int i;
#if USE_COLOR_LUT_CACHE
    int key_length = (sizeof(int) /* cache_size*/ +
		      sizeof(int) /* n_stops */ +
		      gradient->n_stops * (sizeof (pixman_fixed_t) /* stop.x */ +
					   sizeof (pixman_color_t) /* stop.color */));
    char *key;
    char *p;
#endif

    if (gradient->color_lut_bits == 0)
    {
        gradient->color_lut = NULL;
	return;
    }

    cache_size = 1 << gradient->color_lut_bits;

#if USE_COLOR_LUT_CACHE
    key = (char*)malloc (key_length);
    p = key;

    memcpy (p, &cache_size, sizeof (int)); p += sizeof (int);
    memcpy (p, &gradient->n_stops, sizeof (int)); p += sizeof (int);

    for (i = 0; i < gradient->n_stops; i ++) {
      memcpy (p, &gradient->stops[i].x, sizeof (pixman_fixed_t)); p += sizeof (pixman_fixed_t);
      memcpy (p, &gradient->stops[i].color, sizeof (pixman_color_t)); p += sizeof (pixman_color_t);
    }

    gradient->color_lut = lookup_color_lut (key, key_length);
    if (gradient->color_lut) {
      free (key);
      return;
    }
#endif

    gradient->color_lut = (uint32_t*)pixman_malloc_ab (cache_size, sizeof (uint32_t));
    if (gradient->color_lut == NULL)
        return;

    if (gradient->n_stops == 2)
    {
        fill_in_32bit_lut (gradient->color_lut, 0, gradient->stops[0].color, gradient->stops[1].color,
			   cache_size);
    }
    else
    {
        int prevIndex = 0;
	if (gradient->stops[0].x != 0) {
	    int nextIndex = clamp_fixed_to_ffff(gradient->stops[0].x) >> (16 - gradient->color_lut_bits);

	    if (nextIndex > prevIndex)
	        fill_in_32bit_lut (gradient->color_lut, prevIndex,
				   gradient->stops[0].color,
				   gradient->stops[0].color,
				   nextIndex - prevIndex + 1);
	    prevIndex = nextIndex;
	}
	for (i = 1; i < gradient->n_stops; i++)
        {
	    int nextIndex = clamp_fixed_to_ffff(gradient->stops[i].x) >> (16 - gradient->color_lut_bits);

	    if (nextIndex > prevIndex)
	        fill_in_32bit_lut (gradient->color_lut, prevIndex,
				   gradient->stops[i-1].color,
				   gradient->stops[i].color,
				   nextIndex - prevIndex + 1);
	    prevIndex = nextIndex;
	}
	if (pixman_fixed_1 != gradient->stops[gradient->n_stops - 1].x) {
	    int nextIndex = clamp_fixed_to_ffff(pixman_fixed_1) >> (16 - gradient->color_lut_bits);

	    if (nextIndex > prevIndex)
	        fill_in_32bit_lut(gradient->color_lut, prevIndex,
				  gradient->stops[gradient->n_stops - 1].color,
				  gradient->stops[gradient->n_stops - 1].color,
				  nextIndex - prevIndex + 1);
	    prevIndex = nextIndex;
	}

    }

#if USE_COLOR_LUT_CACHE
    add_lut_cache (key, key_length, gradient->color_lut);
#endif
}

pixman_bool_t
_pixman_init_gradient (gradient_t *                  gradient,
                       const pixman_gradient_stop_t *stops,
                       int                           n_stops)
{
    return_val_if_fail (n_stops > 0, FALSE);

    gradient->stops = pixman_malloc_ab (n_stops, sizeof (pixman_gradient_stop_t));
    if (!gradient->stops)
	return FALSE;

    memcpy (gradient->stops, stops, n_stops * sizeof (pixman_gradient_stop_t));

    gradient->n_stops = n_stops;

    gradient->stop_range = 0xffff;

    gradient->color_tolerance = 0.0;
    gradient->color_lut_bits = 0;
    gradient->color_lut = NULL;

    gradient->common.class = SOURCE_IMAGE_CLASS_UNKNOWN;

    return TRUE;
}

/*
 * By default, just evaluate the image at 32bpp and expand.  Individual image
 * types can plug in a better scanline getter if they want to. For example
 * we  could produce smoother gradients by evaluating them at higher color
 * depth, but that's a project for the future.
 */
void
_pixman_image_get_scanline_generic_64 (pixman_image_t * image,
                                       int              x,
                                       int              y,
                                       int              width,
                                       uint32_t *       buffer,
                                       const uint32_t * mask,
                                       uint32_t         mask_bits)
{
    uint32_t *mask8 = NULL;

    /* Contract the mask image, if one exists, so that the 32-bit fetch
     * function can use it.
     */
    if (mask)
    {
	mask8 = pixman_malloc_ab (width, sizeof(uint32_t));
	if (!mask8)
	    return;

	pixman_contract (mask8, (uint64_t *)mask, width);
    }

    /* Fetch the source image into the first half of buffer. */
    _pixman_image_get_scanline_32 (image, x, y, width, (uint32_t*)buffer, mask8,
                                   mask_bits);

    /* Expand from 32bpp to 64bpp in place. */
    pixman_expand ((uint64_t *)buffer, buffer, PIXMAN_a8r8g8b8, width);

    free (mask8);
}

pixman_image_t *
_pixman_image_allocate (void)
{
    pixman_image_t *image = malloc (sizeof (pixman_image_t));

    if (image)
    {
	image_common_t *common = &image->common;

	pixman_region32_init (&common->clip_region);

	common->have_clip_region = FALSE;
	common->clip_sources = FALSE;
	common->transform = NULL;
	common->repeat = PIXMAN_REPEAT_NONE;
	common->filter = PIXMAN_FILTER_NEAREST;
	common->filter_params = NULL;
	common->n_filter_params = 0;
	common->alpha_map = NULL;
	common->component_alpha = FALSE;
	common->ref_count = 1;
	common->classify = NULL;
	common->client_clip = FALSE;
	common->destroy_func = NULL;
	common->destroy_data = NULL;
	common->dirty = TRUE;
    }

    return image;
}

source_image_class_t
_pixman_image_classify (pixman_image_t *image,
                        int             x,
                        int             y,
                        int             width,
                        int             height)
{
    if (image->common.classify)
	return image->common.classify (image, x, y, width, height);
    else
	return SOURCE_IMAGE_CLASS_UNKNOWN;
}

void
_pixman_image_get_scanline_32 (pixman_image_t *image,
                               int             x,
                               int             y,
                               int             width,
                               uint32_t *      buffer,
                               const uint32_t *mask,
                               uint32_t        mask_bits)
{
    image->common.get_scanline_32 (image, x, y, width, buffer, mask, mask_bits);
}

/* Even thought the type of buffer is uint32_t *, the function actually expects
 * a uint64_t *buffer.
 */
void
_pixman_image_get_scanline_64 (pixman_image_t *image,
                               int             x,
                               int             y,
                               int             width,
                               uint32_t *      buffer,
                               const uint32_t *unused,
                               uint32_t        unused2)
{
    image->common.get_scanline_64 (image, x, y, width, buffer, unused, unused2);
}

static void
image_property_changed (pixman_image_t *image)
{
    image->common.dirty = TRUE;
}

/* Ref Counting */
PIXMAN_EXPORT pixman_image_t *
pixman_image_ref (pixman_image_t *image)
{
    image->common.ref_count++;

    return image;
}

/* returns TRUE when the image is freed */
PIXMAN_EXPORT pixman_bool_t
pixman_image_unref (pixman_image_t *image)
{
    image_common_t *common = (image_common_t *)image;

    common->ref_count--;

    if (common->ref_count == 0)
    {
	if (image->common.destroy_func)
	    image->common.destroy_func (image, image->common.destroy_data);

	pixman_region32_fini (&common->clip_region);

	if (common->transform)
	    free (common->transform);

	if (common->filter_params)
	    free (common->filter_params);

	if (common->alpha_map)
	    pixman_image_unref ((pixman_image_t *)common->alpha_map);

	if (image->type == LINEAR ||
	    image->type == RADIAL ||
	    image->type == CONICAL)
	{
	    if (image->gradient.stops)
		free (image->gradient.stops);
	}

	if (image->type == BITS && image->bits.free_me)
	    free (image->bits.free_me);

	free (image);

	return TRUE;
    }

    return FALSE;
}

PIXMAN_EXPORT void
pixman_image_set_destroy_function (pixman_image_t *            image,
                                   pixman_image_destroy_func_t func,
                                   void *                      data)
{
    image->common.destroy_func = func;
    image->common.destroy_data = data;
}

PIXMAN_EXPORT void *
pixman_image_get_destroy_data (pixman_image_t *image)
{
  return image->common.destroy_data;
}

void
_pixman_image_reset_clip_region (pixman_image_t *image)
{
    image->common.have_clip_region = FALSE;
}

static pixman_bool_t out_of_bounds_workaround = TRUE;

/* Old X servers rely on out-of-bounds accesses when they are asked
 * to composite with a window as the source. They create a pixman image
 * pointing to some bogus position in memory, but then they set a clip
 * region to the position where the actual bits are.
 *
 * Due to a bug in old versions of pixman, where it would not clip
 * against the image bounds when a clip region was set, this would
 * actually work. So by default we allow certain out-of-bound access
 * to happen unless explicitly disabled.
 *
 * Fixed X servers should call this function to disable the workaround.
 */
PIXMAN_EXPORT void
pixman_disable_out_of_bounds_workaround (void)
{
    out_of_bounds_workaround = FALSE;
}

static pixman_bool_t
source_image_needs_out_of_bounds_workaround (bits_image_t *image)
{
    if (image->common.clip_sources                      &&
        image->common.repeat == PIXMAN_REPEAT_NONE      &&
	image->common.have_clip_region			&&
        out_of_bounds_workaround)
    {
	if (!image->common.client_clip)
	{
	    /* There is no client clip, so if the clip region extends beyond the
	     * drawable geometry, it must be because the X server generated the
	     * bogus clip region.
	     */
	    const pixman_box32_t *extents =
		pixman_region32_extents (&image->common.clip_region);

	    if (extents->x1 >= 0 && extents->x2 <= image->width &&
		extents->y1 >= 0 && extents->y2 <= image->height)
	    {
		return FALSE;
	    }
	}

	return TRUE;
    }

    return FALSE;
}

void
_pixman_image_compute_info (pixman_image_t *image)
{
    pixman_format_code_t code;
    uint32_t flags = 0;

    /* Transform */
    if (!image->common.transform)
    {
	flags |= (FAST_PATH_ID_TRANSFORM | FAST_PATH_X_UNIT_POSITIVE);
    }
    else
    {
	if (image->common.transform->matrix[0][1] == 0 &&
	    image->common.transform->matrix[1][0] == 0 &&
	    image->common.transform->matrix[2][0] == 0 &&
	    image->common.transform->matrix[2][1] == 0 &&
	    image->common.transform->matrix[2][2] == pixman_fixed_1)
	{
	    flags |= FAST_PATH_SCALE_TRANSFORM;
	}

	if (image->common.transform->matrix[0][0] > 0)
	    flags |= FAST_PATH_X_UNIT_POSITIVE;
    }

    /* Alpha map */
    if (!image->common.alpha_map)
	flags |= FAST_PATH_NO_ALPHA_MAP;

    /* Filter */
    switch (image->common.filter)
    {
    case PIXMAN_FILTER_NEAREST:
    case PIXMAN_FILTER_FAST:
	flags |= (FAST_PATH_NEAREST_FILTER | FAST_PATH_NO_CONVOLUTION_FILTER);
	break;

    case PIXMAN_FILTER_CONVOLUTION:
	break;

    default:
	flags |= FAST_PATH_NO_CONVOLUTION_FILTER;
	break;
    }

    /* Repeat mode */
    switch (image->common.repeat)
    {
    case PIXMAN_REPEAT_NONE:
	flags |= FAST_PATH_NO_REFLECT_REPEAT | FAST_PATH_NO_PAD_REPEAT;
	break;

    case PIXMAN_REPEAT_REFLECT:
	flags |= FAST_PATH_NO_PAD_REPEAT | FAST_PATH_NO_NONE_REPEAT;
	break;

    case PIXMAN_REPEAT_PAD:
	flags |= FAST_PATH_NO_REFLECT_REPEAT | FAST_PATH_NO_NONE_REPEAT;
	break;

    default:
	flags |= FAST_PATH_NO_REFLECT_REPEAT | FAST_PATH_NO_PAD_REPEAT | FAST_PATH_NO_NONE_REPEAT;
	break;
    }

    /* Component alpha */
    if (image->common.component_alpha)
	flags |= FAST_PATH_COMPONENT_ALPHA;
    else
	flags |= FAST_PATH_UNIFIED_ALPHA;

    flags |= (FAST_PATH_NO_ACCESSORS | FAST_PATH_NO_WIDE_FORMAT);

    /* Type specific checks */
    switch (image->type)
    {
    case SOLID:
	code = PIXMAN_solid;

	if (image->solid.color.alpha == 0xffff)
	    flags |= FAST_PATH_IS_OPAQUE;
	break;

    case BITS:
	if (image->bits.width == 1	&&
	    image->bits.height == 1	&&
	    image->common.repeat != PIXMAN_REPEAT_NONE)
	{
	    code = PIXMAN_solid;
	}
	else
	{
	    code = image->bits.format;

	    if (!image->common.transform &&
		image->common.repeat == PIXMAN_REPEAT_NORMAL)
	    {
		flags |= FAST_PATH_SIMPLE_REPEAT;
	    }
	}

	if (image->common.repeat != PIXMAN_REPEAT_NONE				&&
	    !PIXMAN_FORMAT_A (image->bits.format)				&&
	    PIXMAN_FORMAT_TYPE (image->bits.format) != PIXMAN_TYPE_GRAY		&&
	    PIXMAN_FORMAT_TYPE (image->bits.format) != PIXMAN_TYPE_COLOR)
	{
	    flags |= FAST_PATH_IS_OPAQUE;
	}

	if (source_image_needs_out_of_bounds_workaround (&image->bits))
	    flags |= FAST_PATH_NEEDS_WORKAROUND;

	if (image->bits.read_func || image->bits.write_func)
	    flags &= ~FAST_PATH_NO_ACCESSORS;

	if (PIXMAN_FORMAT_IS_WIDE (image->bits.format))
	    flags &= ~FAST_PATH_NO_WIDE_FORMAT;
	break;

    case LINEAR:
    case RADIAL:
	code = PIXMAN_unknown;

	if (image->common.repeat != PIXMAN_REPEAT_NONE)
	{
	    int i;

	    flags |= FAST_PATH_IS_OPAQUE;
	    for (i = 0; i < image->gradient.n_stops; ++i)
	    {
		if (image->gradient.stops[i].color.alpha != 0xffff)
		{
		    flags &= ~FAST_PATH_IS_OPAQUE;
		    break;
		}
	    }
	}

	/* Convert from the color tolerance value to a number of bits
	 * in the lookup table.  This isn't really the way tolerance
	 * should work (since it is meant to express a maximum error
	 * for the rasterized gradient), but we can use it as a hint.
	 */
	if (image->gradient.color_tolerance == 0.0)
	    image->gradient.color_lut_bits = 0;
	else if (image->gradient.color_tolerance <= 0.5)
	    image->gradient.color_lut_bits = 9;
	else if (image->gradient.color_tolerance <= 1.0)
	    image->gradient.color_lut_bits = 8;

	break;

    default:
	code = PIXMAN_unknown;
	break;
    }

    /* Both alpha maps and convolution filters can introduce
     * non-opaqueness in otherwise opaque images. Also
     * an image with component alpha turned on is only opaque
     * if all channels are opaque, so we simply turn it off
     * unconditionally for those images.
     */
    if (image->common.alpha_map					||
	image->common.filter == PIXMAN_FILTER_CONVOLUTION	||
	image->common.component_alpha)
    {
	flags &= ~FAST_PATH_IS_OPAQUE;
    }

    image->common.flags = flags;
    image->common.extended_format_code = code;
}

PIXMAN_EXPORT pixman_bool_t
pixman_image_set_clip_region32 (pixman_image_t *   image,
                                pixman_region32_t *region)
{
    image_common_t *common = (image_common_t *)image;
    pixman_bool_t result;

    if (region)
    {
	if ((result = pixman_region32_copy (&common->clip_region, region)))
	    image->common.have_clip_region = TRUE;
    }
    else
    {
	_pixman_image_reset_clip_region (image);

	result = TRUE;
    }

    image_property_changed (image);

    return result;
}

PIXMAN_EXPORT pixman_bool_t
pixman_image_set_clip_region (pixman_image_t *   image,
                              pixman_region16_t *region)
{
    image_common_t *common = (image_common_t *)image;
    pixman_bool_t result;

    if (region)
    {
	if ((result = pixman_region32_copy_from_region16 (&common->clip_region, region)))
	    image->common.have_clip_region = TRUE;
    }
    else
    {
	_pixman_image_reset_clip_region (image);

	result = TRUE;
    }

    image_property_changed (image);

    return result;
}

PIXMAN_EXPORT void
pixman_image_set_has_client_clip (pixman_image_t *image,
                                  pixman_bool_t   client_clip)
{
    image->common.client_clip = client_clip;
}

PIXMAN_EXPORT pixman_bool_t
pixman_image_set_transform (pixman_image_t *          image,
                            const pixman_transform_t *transform)
{
    static const pixman_transform_t id =
    {
	{ { pixman_fixed_1, 0, 0 },
	  { 0, pixman_fixed_1, 0 },
	  { 0, 0, pixman_fixed_1 } }
    };

    image_common_t *common = (image_common_t *)image;
    pixman_bool_t result;

    if (common->transform == transform)
	return TRUE;

    if (memcmp (&id, transform, sizeof (pixman_transform_t)) == 0)
    {
	free (common->transform);
	common->transform = NULL;
	result = TRUE;

	goto out;
    }

    if (common->transform == NULL)
	common->transform = malloc (sizeof (pixman_transform_t));

    if (common->transform == NULL)
    {
	result = FALSE;

	goto out;
    }

    memcpy (common->transform, transform, sizeof(pixman_transform_t));

    result = TRUE;

out:
    image_property_changed (image);

    return result;
}

PIXMAN_EXPORT void
pixman_image_set_color_tolerance (pixman_image_t *image,
				  double tolerance)
{
    if (image->type != LINEAR &&
	image->type != RADIAL &&
	image->type != CONICAL)
      /* color tolerance is only applicable to the gradient types */
      return;

    image->gradient.color_tolerance = tolerance;

    image_property_changed (image);
}

PIXMAN_EXPORT void
pixman_image_set_repeat (pixman_image_t *image,
                         pixman_repeat_t repeat)
{
    image->common.repeat = repeat;

    image_property_changed (image);
}

PIXMAN_EXPORT pixman_bool_t
pixman_image_set_filter (pixman_image_t *      image,
                         pixman_filter_t       filter,
                         const pixman_fixed_t *params,
                         int                   n_params)
{
    image_common_t *common = (image_common_t *)image;
    pixman_fixed_t *new_params;

    if (params == common->filter_params && filter == common->filter)
	return TRUE;

    new_params = NULL;
    if (params)
    {
	new_params = pixman_malloc_ab (n_params, sizeof (pixman_fixed_t));
	if (!new_params)
	    return FALSE;

	memcpy (new_params,
	        params, n_params * sizeof (pixman_fixed_t));
    }

    common->filter = filter;

    if (common->filter_params)
	free (common->filter_params);

    common->filter_params = new_params;
    common->n_filter_params = n_params;

    image_property_changed (image);
    return TRUE;
}

PIXMAN_EXPORT void
pixman_image_set_source_clipping (pixman_image_t *image,
                                  pixman_bool_t   clip_sources)
{
    image->common.clip_sources = clip_sources;

    image_property_changed (image);
}

/* Unlike all the other property setters, this function does not
 * copy the content of indexed. Doing this copying is simply
 * way, way too expensive.
 */
PIXMAN_EXPORT void
pixman_image_set_indexed (pixman_image_t *        image,
                          const pixman_indexed_t *indexed)
{
    bits_image_t *bits = (bits_image_t *)image;

    bits->indexed = indexed;

    image_property_changed (image);
}

PIXMAN_EXPORT void
pixman_image_set_alpha_map (pixman_image_t *image,
                            pixman_image_t *alpha_map,
                            int16_t         x,
                            int16_t         y)
{
    image_common_t *common = (image_common_t *)image;

    return_if_fail (!alpha_map || alpha_map->type == BITS);

    if (common->alpha_map != (bits_image_t *)alpha_map)
    {
	if (common->alpha_map)
	    pixman_image_unref ((pixman_image_t *)common->alpha_map);

	if (alpha_map)
	    common->alpha_map = (bits_image_t *)pixman_image_ref (alpha_map);
	else
	    common->alpha_map = NULL;
    }

    common->alpha_origin_x = x;
    common->alpha_origin_y = y;

    image_property_changed (image);
}

PIXMAN_EXPORT void
pixman_image_set_component_alpha   (pixman_image_t *image,
                                    pixman_bool_t   component_alpha)
{
    image->common.component_alpha = component_alpha;

    image_property_changed (image);
}

PIXMAN_EXPORT void
pixman_image_set_accessors (pixman_image_t *           image,
                            pixman_read_memory_func_t  read_func,
                            pixman_write_memory_func_t write_func)
{
    return_if_fail (image != NULL);

    if (image->type == BITS)
    {
	image->bits.read_func = read_func;
	image->bits.write_func = write_func;

	image_property_changed (image);
    }
}

PIXMAN_EXPORT uint32_t *
pixman_image_get_data (pixman_image_t *image)
{
    if (image->type == BITS)
	return image->bits.bits;

    return NULL;
}

PIXMAN_EXPORT int
pixman_image_get_width (pixman_image_t *image)
{
    if (image->type == BITS)
	return image->bits.width;

    return 0;
}

PIXMAN_EXPORT int
pixman_image_get_height (pixman_image_t *image)
{
    if (image->type == BITS)
	return image->bits.height;

    return 0;
}

PIXMAN_EXPORT int
pixman_image_get_stride (pixman_image_t *image)
{
    if (image->type == BITS)
	return image->bits.rowstride * (int) sizeof (uint32_t);

    return 0;
}

PIXMAN_EXPORT int
pixman_image_get_depth (pixman_image_t *image)
{
    if (image->type == BITS)
	return PIXMAN_FORMAT_DEPTH (image->bits.format);

    return 0;
}

PIXMAN_EXPORT pixman_format_code_t
pixman_image_get_format (pixman_image_t *image)
{
    if (image->type == BITS)
	return image->bits.format;

    return 0;
}

uint32_t
_pixman_image_get_solid (pixman_image_t *     image,
                         pixman_format_code_t format)
{
    uint32_t result;

    _pixman_image_get_scanline_32 (image, 0, 0, 1, &result, NULL, 0);

    /* If necessary, convert RGB <--> BGR. */
    if (PIXMAN_FORMAT_TYPE (format) != PIXMAN_TYPE_ARGB)
    {
	result = (((result & 0xff000000) >>  0) |
	          ((result & 0x00ff0000) >> 16) |
	          ((result & 0x0000ff00) >>  0) |
	          ((result & 0x000000ff) << 16));
    }

    return result;
}


/* linear gradients */
static source_image_class_t
linear_gradient_classify (pixman_image_t *image,
                          int             x,
                          int             y,
                          int             width,
                          int             height)
{
    linear_gradient_t *linear = (linear_gradient_t *)image;
    pixman_vector_t v;
    pixman_fixed_32_32_t l;
    pixman_fixed_48_16_t dx, dy, a, b, off;
    pixman_fixed_48_16_t factors[4];
    int i;

    image->source.class = SOURCE_IMAGE_CLASS_UNKNOWN;

    dx = linear->p2.x - linear->p1.x;
    dy = linear->p2.y - linear->p1.y;

    l = dx * dx + dy * dy;

    if (l)
    {
	a = (dx << 32) / l;
	b = (dy << 32) / l;
    }
    else
    {
	a = b = 0;
    }

    off = (-a * linear->p1.x
           -b * linear->p1.y) >> 16;

    for (i = 0; i < 3; i++)
    {
	v.vector[0] = pixman_int_to_fixed ((i % 2) * (width  - 1) + x);
	v.vector[1] = pixman_int_to_fixed ((i / 2) * (height - 1) + y);
	v.vector[2] = pixman_fixed_1;

	if (image->common.transform)
	{
	    if (!pixman_transform_point_3d (image->common.transform, &v))
	    {
		image->source.class = SOURCE_IMAGE_CLASS_UNKNOWN;

		return image->source.class;
	    }
	}

	factors[i] = ((a * v.vector[0] + b * v.vector[1]) >> 16) + off;
    }

    if (factors[2] == factors[0])
	image->source.class = SOURCE_IMAGE_CLASS_HORIZONTAL;
    else if (factors[1] == factors[0])
	image->source.class = SOURCE_IMAGE_CLASS_VERTICAL;

    return image->source.class;
}


static void
linear_gradient_property_changed (pixman_implementation_t *imp, pixman_image_t *image)
{
    image->common.get_scanline_32 = imp->get_scanline_fetcher_32 (imp, image);
    image->common.get_scanline_64 = imp->get_scanline_fetcher_64 (imp, image);

    _pixman_gradient_build_32bit_lut ((gradient_t*)image);
}

PIXMAN_EXPORT pixman_image_t *
pixman_image_create_linear_gradient (pixman_point_fixed_t *        p1,
                                     pixman_point_fixed_t *        p2,
                                     const pixman_gradient_stop_t *stops,
                                     int                           n_stops)
{
    pixman_image_t *image;
    linear_gradient_t *linear;

    return_val_if_fail (n_stops >= 2, NULL);

    image = _pixman_image_allocate ();

    if (!image)
	return NULL;

    linear = &image->linear;

    if (!_pixman_init_gradient (&linear->common, stops, n_stops))
    {
	free (image);
	return NULL;
    }

    linear->p1 = *p1;
    linear->p2 = *p2;

    image->type = LINEAR;
    image->source.class = SOURCE_IMAGE_CLASS_UNKNOWN;
    image->common.classify = linear_gradient_classify;
    image->common.property_changed = linear_gradient_property_changed;

    return image;
}

/* radial gradients */

static void
radial_gradient_property_changed (pixman_implementation_t *imp, pixman_image_t *image)
{
    image->common.get_scanline_32 = imp->get_scanline_fetcher_32 (imp, image);
    image->common.get_scanline_64 = imp->get_scanline_fetcher_64 (imp, image);

    _pixman_gradient_build_32bit_lut ((gradient_t*)image);
}

PIXMAN_EXPORT pixman_image_t *
pixman_image_create_radial_gradient (pixman_point_fixed_t *        inner,
                                     pixman_point_fixed_t *        outer,
                                     pixman_fixed_t                inner_radius,
                                     pixman_fixed_t                outer_radius,
                                     const pixman_gradient_stop_t *stops,
                                     int                           n_stops)
{
    pixman_image_t *image;
    radial_gradient_t *radial;

    return_val_if_fail (n_stops >= 2, NULL);

    image = _pixman_image_allocate ();

    if (!image)
	return NULL;

    radial = &image->radial;

    if (!_pixman_init_gradient (&radial->common, stops, n_stops))
    {
	free (image);
	return NULL;
    }

    image->type = RADIAL;

    radial->c1.x = inner->x;
    radial->c1.y = inner->y;
    radial->c1.radius = inner_radius;
    radial->c2.x = outer->x;
    radial->c2.y = outer->y;
    radial->c2.radius = outer_radius;
    radial->cdx = (float)pixman_fixed_to_double (radial->c2.x - radial->c1.x);
    radial->cdy = (float)pixman_fixed_to_double (radial->c2.y - radial->c1.y);
    radial->dr = (float)pixman_fixed_to_double (radial->c2.radius - radial->c1.radius);
    radial->A = (radial->cdx * radial->cdx +
		 radial->cdy * radial->cdy -
		 radial->dr  * radial->dr);

    image->common.property_changed = radial_gradient_property_changed;

    return image;
}

/* conical gradients */

static void
conical_gradient_property_changed (pixman_implementation_t *imp, pixman_image_t *image)
{
    image->common.get_scanline_32 = imp->get_scanline_fetcher_32 (imp, image);
    image->common.get_scanline_64 = imp->get_scanline_fetcher_64 (imp, image);

    _pixman_gradient_build_32bit_lut ((gradient_t*)image);
}

PIXMAN_EXPORT pixman_image_t *
pixman_image_create_conical_gradient (pixman_point_fixed_t *        center,
                                      pixman_fixed_t                angle,
                                      const pixman_gradient_stop_t *stops,
                                      int                           n_stops)
{
    pixman_image_t *image = _pixman_image_allocate ();
    conical_gradient_t *conical;

    if (!image)
	return NULL;

    conical = &image->conical;

    if (!_pixman_init_gradient (&conical->common, stops, n_stops))
    {
	free (image);
	return NULL;
    }

    image->type = CONICAL;
    conical->center = *center;
    conical->angle = angle;

    image->common.property_changed = conical_gradient_property_changed;

    return image;
}


/* solid fill images */
static source_image_class_t
solid_fill_classify (pixman_image_t *image,
                     int             x,
                     int             y,
                     int             width,
                     int             height)
{
    return (image->source.class = SOURCE_IMAGE_CLASS_HORIZONTAL);
}

static void
solid_fill_property_changed (pixman_implementation_t *imp, pixman_image_t *image)
{
    image->common.get_scanline_32 = imp->get_scanline_fetcher_32 (imp, image);
    image->common.get_scanline_64 = imp->get_scanline_fetcher_64 (imp, image);
}

static uint32_t
color_to_uint32 (const pixman_color_t *color)
{
    return
        (color->alpha >> 8 << 24) |
        (color->red >> 8 << 16) |
        (color->green & 0xff00) |
        (color->blue >> 8);
}

static uint64_t
color_to_uint64 (const pixman_color_t *color)
{
    return
        ((uint64_t)color->alpha << 48) |
        ((uint64_t)color->red << 32) |
        ((uint64_t)color->green << 16) |
        ((uint64_t)color->blue);
}

PIXMAN_EXPORT pixman_image_t *
pixman_image_create_solid_fill (pixman_color_t *color)
{
    pixman_image_t *img = _pixman_image_allocate ();

    if (!img)
	return NULL;

    img->type = SOLID;
    img->solid.color = *color;
    img->solid.color_32 = color_to_uint32 (color);
    img->solid.color_64 = color_to_uint64 (color);

    img->source.class = SOURCE_IMAGE_CLASS_UNKNOWN;
    img->common.classify = solid_fill_classify;
    img->common.property_changed = solid_fill_property_changed;

    return img;
}


/* bits images */

static void
bits_image_property_changed (pixman_implementation_t *imp, pixman_image_t *image)
{
    bits_image_t *bits = (bits_image_t *)image;

    _pixman_bits_image_setup_raw_accessors (bits);

    image->common.get_scanline_32 = (*imp->get_scanline_fetcher_32) (imp, image);
    image->common.get_scanline_64 = (*imp->get_scanline_fetcher_64) (imp, image);

    bits->fetch_pixel_32 = (*imp->get_pixel_fetcher_32) (imp, bits);
    bits->fetch_pixel_64 = (*imp->get_pixel_fetcher_64) (imp, bits);

    bits->store_scanline_32 = (*imp->get_scanline_storer_32) (imp, bits);
    bits->store_scanline_64 = (*imp->get_scanline_storer_64) (imp, bits);
}

static uint32_t *
create_bits (pixman_format_code_t format,
             int                  width,
             int                  height,
             int *                rowstride_bytes)
{
    int stride;
    int buf_size;
    int bpp;

    /* what follows is a long-winded way, avoiding any possibility of integer
     * overflows, of saying:
     * stride = ((width * bpp + 0x1f) >> 5) * sizeof (uint32_t);
     */

    bpp = PIXMAN_FORMAT_BPP (format);
    if (pixman_multiply_overflows_int (width, bpp))
	return NULL;

    stride = width * bpp;
    if (pixman_addition_overflows_int (stride, 0x1f))
	return NULL;

    stride += 0x1f;
    stride >>= 5;

    stride *= sizeof (uint32_t);

    if (pixman_multiply_overflows_int (height, stride))
	return NULL;

    buf_size = height * stride;

    if (rowstride_bytes)
	*rowstride_bytes = stride;

    return calloc (buf_size, 1);
}

PIXMAN_EXPORT pixman_image_t *
pixman_image_create_bits (pixman_format_code_t format,
                          int                  width,
                          int                  height,
                          uint32_t *           bits,
                          int                  rowstride_bytes)
{
    pixman_image_t *image;
    uint32_t *free_me = NULL;

    /* must be a whole number of uint32_t's
     */
    return_val_if_fail (
	bits == NULL || (rowstride_bytes % sizeof (uint32_t)) == 0, NULL);

    return_val_if_fail (PIXMAN_FORMAT_BPP (format) >= PIXMAN_FORMAT_DEPTH (format), NULL);

    if (!bits && width && height)
    {
	free_me = bits = create_bits (format, width, height, &rowstride_bytes);
	if (!bits)
	    return NULL;
    }

    image = _pixman_image_allocate ();

    if (!image)
    {
	if (free_me)
	    free (free_me);

	return NULL;
    }

    image->type = BITS;
    image->bits.format = format;
    image->bits.width = width;
    image->bits.height = height;
    image->bits.bits = bits;
    image->bits.free_me = free_me;
    image->bits.read_func = NULL;
    image->bits.write_func = NULL;

    /* The rowstride is stored in number of uint32_t */
    image->bits.rowstride = rowstride_bytes / (int) sizeof (uint32_t);

    image->bits.indexed = NULL;

    image->common.property_changed = bits_image_property_changed;

    _pixman_image_reset_clip_region (image);

    return image;
}
