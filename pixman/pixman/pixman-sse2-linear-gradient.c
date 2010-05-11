/*
 * Copyright © 2000 SuSE, Inc.
 * Copyright © 2007 Red Hat, Inc.
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *             2005 Lars Knoll & Zack Rusin, Trolltech
 * Copyright © 2010 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include "pixman-private.h"

#include <mmintrin.h>
#include <xmmintrin.h> /* for _mm_shuffle_pi16 and _MM_SHUFFLE */
#include <emmintrin.h>

#include <assert.h>

#include "pixman-sse2-gradient.h"

#include "pixman-sse2-gradient-walker.c"

#define LIKELY(x)       __builtin_expect((x),1)
#define UNLIKELY(x)     __builtin_expect((x),0)

#define SSE2_LINEAR_GRADIENT(lut_macro) do {		\
    /* we can process 4 pixels at a time */		\
    							\
    int count;							\
    								\
    if ((int)buffer & 0x0f)						\
    {									\
	/* we aren't aligned to a 16 byte boundary. */			\
	/* process the at most 3 pixels until we get */			\
	/* there */							\
	int initial = 16 - ((int)buffer & 0x0f);			\
									\
	lut_macro(ts, tsf, lut, shift, storeu);				\
									\
	initial >>= 2;							\
									\
	if (initial >= 3) {						\
	    *buffer++ = tsf[0];						\
	    ts = _mm_add_epi32 (ts, _mm_set1_epi32 (inc));		\
	}								\
	if (initial >= 2) {						\
	    *buffer++ = tsf[1];						\
	    ts = _mm_add_epi32 (ts, _mm_set1_epi32 (inc));		\
	}								\
	if (initial >= 1) {						\
	    *buffer++ = tsf[2];						\
	    ts = _mm_add_epi32 (ts, _mm_set1_epi32 (inc));		\
	}								\
    }									\
									\
    count = end - buffer;						\
    while (count >= 4)							\
    {									\
	lut_macro(ts, buffer, lut, shift, store);			\
									\
	buffer += 4;							\
	/* t += t + 4 * inc */						\
	ts = _mm_add_epi32 (ts, incs);					\
									\
	count -= 4;							\
    }									\
    									\
    if (buffer < end)							\
    {									\
	/* we're at most 3 pixels off, so unroll/inline it here */	\
	int rem = end - buffer;						\
									\
	lut_macro(ts, tsf, lut, shift, storeu);				\
									\
	if (rem-- > 0) buffer[0] = tsf[0];				\
	if (rem-- > 0) buffer[1] = tsf[1];				\
	if (rem-- > 0) buffer[2] = tsf[2];				\
    }									\
  } while (0)


void
_pixman_sse2_linear_gradient_get_scanline_32 (pixman_image_t *image,
					      int             x,
					      int             y,
					      int             width,
					      uint32_t *      buffer,
					      const uint32_t *mask,
					      uint32_t        mask_bits)
{
    pixman_vector_t v, unit;
    pixman_fixed_32_32_t l;
    pixman_fixed_48_16_t dx, dy, a, b, off;
    gradient_t *gradient = (gradient_t *)image;
    source_image_t *source = (source_image_t *)image;
    linear_gradient_t *linear = (linear_gradient_t *)image;
    uint32_t *end = buffer + width;

    /* reference point is the center of the pixel */
    v.vector[0] = pixman_int_to_fixed (x) + pixman_fixed_1 / 2;
    v.vector[1] = pixman_int_to_fixed (y) + pixman_fixed_1 / 2;
    v.vector[2] = pixman_fixed_1;

    if (source->common.transform)
    {
	if (!pixman_transform_point_3d (source->common.transform, &v))
	    return;
	
	unit.vector[0] = source->common.transform->matrix[0][0];
	unit.vector[1] = source->common.transform->matrix[1][0];
	unit.vector[2] = source->common.transform->matrix[2][0];
    }
    else
    {
	unit.vector[0] = pixman_fixed_1;
	unit.vector[1] = 0;
	unit.vector[2] = 0;
    }

    dx = linear->p2.x - linear->p1.x;
    dy = linear->p2.y - linear->p1.y;

    l = dx * dx + dy * dy;

    if (l != 0)
    {
	a = (dx << 32) / l;
	b = (dy << 32) / l;
	off = (-a * linear->p1.x
	       -b * linear->p1.y) >> 16;
    }

    if (l == 0 || (unit.vector[2] == 0 && v.vector[2] == pixman_fixed_1))
    {
	pixman_fixed_48_16_t inc, t;

	/* affine transformation only */
	if (l == 0)
	{
	    t = 0;
	    inc = 0;
	}
	else
	{
	    t = ((a * v.vector[0] + b * v.vector[1]) >> 16) + off;
	    inc = (a * unit.vector[0] + b * unit.vector[1]) >> 16;
	}

	if (source->class == SOURCE_IMAGE_CLASS_VERTICAL)
	{
	    uint32_t *lut = gradient->color_lut;
	    uint32_t color;
	    __m128i colors;

	    if (lut != NULL)
	    {
		unsigned int repeat = source->common.repeat;
		int shift = 16 - gradient->color_lut_bits;

		if (repeat == PIXMAN_REPEAT_NORMAL)
		{
		    /* color = _pixman_gradient_walker_pixel_lut_repeat_normal (&walker, t); */
		    if (t < 0)
		        t = -t;
		    t = t & 0xFFFF;

		    color = lut[t >> shift];
		}
		else if (repeat == PIXMAN_REPEAT_PAD)
		{
		    /* color = _pixman_gradient_walker_pixel_lut_repeat_pad (&walker, t); */

		    t = CLAMP_MAX(t, 0xFFFF);
		    color = lut[t >> shift];
		}
		else if (repeat == PIXMAN_REPEAT_REFLECT)
		{
		    /* color = _pixman_gradient_walker_pixel_lut_repeat_reflect (&walker, t); */
		    int s = t << 15 >> 31;
		    t = (t ^ s) & 0xFFFF;

		    color = lut[x >> shift];
		}
		else /* PIXMAN_REPEAT_NONE */
		{
		    /* color = _pixman_gradient_walker_pixel_lut_repeat_none (&walker, t); */
		    if (t < 0 || t > 0xFFFF)
		      color = 0 /* transparent black */;
		    else
		      color = lut[t >> shift];
		}

		colors = _mm_set1_epi32 (color);
	    }
	    else
	    {
	        pixman_sse2_gradient_walker_t walker;

		_pixman_sse2_gradient_walker_init (&walker, gradient, source->common.repeat);

	        __m128i ts = _mm_set1_epi32 (t);
	        colors = _pixman_sse2_gradient_walker_pixel (&walker, ts);
		color = _mm_cvtsi128_si32 (colors);
	    }

	    int initial = 16 - ((int)buffer & 0x0f);
	    while (initial > 0) {
	        *buffer++ = color;
		initial -= 4;
	    }

	    assert (((int)buffer & 0x0f) == 0);

	    uint32_t *end_aligned = (uint32_t*)((char*)end - ((int)end & 0x0f));

	    while (buffer < end_aligned) {
	        _mm_store_si128 ((__m128i*)buffer, colors);
		buffer += 4;
	    }
	    if (end_aligned != end) {
	        int i;
		for (i = 0; i < (end - end_aligned);  i ++)
		    *buffer++ = color;
	    }
	}
	else
	{
	    uint32_t *lut = gradient->color_lut;
	    int shift = 16 - gradient->color_lut_bits;

	    __m128i incs = _mm_set1_epi32 (inc);
	    __m128i ts = _mm_set_epi32 (t + 3 * inc, t + 2 * inc, t + inc, t);
	    __m128i mask_0xffff =  _mm_set1_epi32(0xffff);

	    pixman_fixed_16_16_t tsf[4];

	    incs = _mm_slli_epi32 (incs, 2);

	    if (gradient->color_lut != NULL && (((int)buffer & 0x03) == 0))
	    {
		unsigned int repeat = source->common.repeat;

		if (repeat == PIXMAN_REPEAT_NORMAL)
		{
		    SSE2_LINEAR_GRADIENT(SSE2_LUT_REPEAT_NORMAL);
		}
		else if (repeat == PIXMAN_REPEAT_PAD)
		{
		    SSE2_LINEAR_GRADIENT(SSE2_LUT_REPEAT_PAD);
		}
		else if (repeat == PIXMAN_REPEAT_REFLECT)
		{
		    SSE2_LINEAR_GRADIENT(SSE2_LUT_REPEAT_REFLECT);
		}
		else /* PIXMAN_REPEAT_REFLECT */
		{
		    SSE2_LINEAR_GRADIENT(SSE2_LUT_REPEAT_NONE);
		}
	    }
	    else
	    {
	        pixman_sse2_gradient_walker_t walker;
		_pixman_sse2_gradient_walker_init (&walker, gradient, source->common.repeat);

		SSE2_LINEAR_GRADIENT(SSE2_USE_GRADIENT_WALKER);
	    }
	}
    }
#if 0
    else
    {
	/* projective transformation */
	pixman_fixed_48_16_t t;

	if (source->class == SOURCE_IMAGE_CLASS_VERTICAL)
	{
	    register uint32_t color;

	    if (v.vector[2] == 0)
	    {
		t = 0;
	    }
	    else
	    {
		pixman_fixed_48_16_t x, y;

		x = ((pixman_fixed_48_16_t) v.vector[0] << 16) / v.vector[2];
		y = ((pixman_fixed_48_16_t) v.vector[1] << 16) / v.vector[2];
		t = ((a * x + b * y) >> 16) + off;
	    }

	    color = _pixman_gradient_walker_pixel (&walker, t);

	    while (buffer < end)
		*buffer++ = color;
	}
	else
	{
	    while (buffer < end)
	    {
		if (!mask || *mask++ & mask_bits)
		{
		    if (v.vector[2] == 0)
		    {
			t = 0;
		    }
		    else
		    {
			pixman_fixed_48_16_t x, y;
			x = ((pixman_fixed_48_16_t)v.vector[0] << 16) / v.vector[2];
			y = ((pixman_fixed_48_16_t)v.vector[1] << 16) / v.vector[2];
			t = ((a * x + b * y) >> 16) + off;
		    }

		    *buffer = _pixman_gradient_walker_pixel (&walker, t);
		}
		
		++buffer;
		
		v.vector[0] += unit.vector[0];
		v.vector[1] += unit.vector[1];
		v.vector[2] += unit.vector[2];
	    }
	}
    }
#endif
}
