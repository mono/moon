/*
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 * Copyright © 2000 SuSE, Inc.
 *             2005 Lars Knoll & Zack Rusin, Trolltech
 * Copyright © 2007 Red Hat, Inc.
 * Copyright © 2010 Novell, Inc.
 *
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
#include <math.h>
#include "pixman-private.h"

#include <mmintrin.h>
#include <xmmintrin.h> /* for _mm_shuffle_pi16 and _MM_SHUFFLE */
#include <emmintrin.h>

#include "pixman-sse2-gradient.h"

#include "pixman-sse2-gradient-walker.c"

#define LIKELY(x)       __builtin_expect((x),1)
#define UNLIKELY(x)     __builtin_expect((x),0)

#define SSE2_RADIAL_GRADIENT(lut_macro) do {		\
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
	float scale;							\
									\
	__m128i ts = calc_radial_det4 (B_, A4_, invA_,			\
				       pdx_, pdy_, r1sq_, invert_);	\
									\
	lut_macro(ts, tsf, lut, shift, storeu);				\
									\
	initial >>= 2;							\
									\
	if (initial >= 3)						\
	  *buffer++ = tsf[2];						\
	if (initial >= 2)						\
	  *buffer++ = tsf[1];						\
	if (initial >= 1)						\
	  *buffer++ = tsf[1];						\
									\
	scale = initial;						\
									\
	pdx_ = _mm_add_ps (pdx_, _mm_mul_ps (cx_, _mm_load1_ps (&scale))); \
	pdy_ = _mm_add_ps (pdy_, _mm_mul_ps (cy_, _mm_load1_ps (&scale))); \
	B_ = _mm_add_ps (B_, _mm_mul_ps (cB_, _mm_load1_ps (&scale)));	\
    }									\
    									\
    count = end - buffer;						\
    while (count >= 4)							\
    {									\
	__m128i ts = calc_radial_det4 (B_, A4_, invA_,			\
				       pdx_, pdy_, r1sq_, invert_);	\
									\
	lut_macro(ts, buffer, lut, shift, store);			\
									\
	buffer += 4;							\
									\
	pdx_ = _mm_add_ps (pdx_, cx_4);					\
	pdy_ = _mm_add_ps (pdy_, cy_4);					\
	B_ = _mm_add_ps (B_, cB_4);					\
									\
	count -= 4;							\
    }									\
    									\
    if (buffer < end)							\
    {									\
	/* we're at most 3 pixels off, so unroll/inline it here */	\
	int rem = end - buffer;						\
									\
	__m128i ts = calc_radial_det4 (B_, A4_, invA_,			\
				       pdx_, pdy_, r1sq_, invert_);	\
									\
	lut_macro(ts, tsf, lut, shift, storeu);				\
									\
	if (rem-- > 0) buffer[0] = tsf[0];				\
	if (rem-- > 0) buffer[1] = tsf[1];				\
	if (rem-- > 0) buffer[2] = tsf[2];				\
    }									\
  } while (0)


static force_inline __m128i
calc_radial_det4 (__m128 B, __m128 A4, __m128 invA, __m128 pdx, __m128 pdy, __m128 r1sq, __m128 invert)
{
    __m128 Bsq = _mm_mul_ps (B, B);

    /* discriminant = A4 * (pdx*pdx + pdy * pdy - r1sq) + Bsq */
    __m128 discriminant = _mm_add_ps (_mm_mul_ps (A4,
						  _mm_sub_ps (_mm_add_ps (_mm_mul_ps (pdx, pdx),
									  _mm_mul_ps (pdy, pdy)),
							      r1sq)),
				      Bsq);

    /* figure out the negative discriminants, and create a mask of 1.0 */
    /* for those discriminants that are *not* less than 0. */
    __m128 lt0_mask = _mm_cmplt_ps (discriminant, _mm_setzero_ps());

    /* zero out the negative ones */
    discriminant = _mm_andnot_ps (lt0_mask, discriminant);

    /* sqrt them all */
    discriminant = _mm_sqrt_ps (discriminant);

    discriminant = _mm_mul_ps (invA,
			       _mm_add_ps (B,
					   _mm_mul_ps (invert,
						       discriminant)));

    return  _mm_cvtps_epi32 (discriminant);
}

void
_pixman_sse2_radial_gradient_get_scanline_32 (pixman_image_t *image,
					      int             x,
					      int             y,
					      int             width,
					      uint32_t *      buffer,
					      const uint32_t *mask,
					      uint32_t        mask_bits)
{
    /*
     * In the radial gradient problem we are given two circles (c₁,r₁) and
     * (c₂,r₂) that define the gradient itself. Then, for any point p, we
     * must compute the value(s) of t within [0.0, 1.0] representing the
     * circle(s) that would color the point.
     *
     * There are potentially two values of t since the point p can be
     * colored by both sides of the circle, (which happens whenever one
     * circle is not entirely contained within the other).
     *
     * If we solve for a value of t that is outside of [0.0, 1.0] then we
     * use the extend mode (NONE, REPEAT, REFLECT, or PAD) to map to a
     * value within [0.0, 1.0].
     *
     * Here is an illustration of the problem:
     *
     *              p₂
     *           p  •
     *           •   ╲
     *        ·       ╲r₂
     *  p₁ ·           ╲
     *  •              θ╲
     *   ╲             ╌╌•
     *    ╲r₁        ·   c₂
     *    θ╲    ·
     *    ╌╌•
     *      c₁
     *
     * Given (c₁,r₁), (c₂,r₂) and p, we must find an angle θ such that two
     * points p₁ and p₂ on the two circles are collinear with p. Then, the
     * desired value of t is the ratio of the length of p₁p to the length
     * of p₁p₂.
     *
     * So, we have six unknown values: (p₁x, p₁y), (p₂x, p₂y), θ and t.
     * We can also write six equations that constrain the problem:
     *
     * Point p₁ is a distance r₁ from c₁ at an angle of θ:
     *
     *	1. p₁x = c₁x + r₁·cos θ
     *	2. p₁y = c₁y + r₁·sin θ
     *
     * Point p₂ is a distance r₂ from c₂ at an angle of θ:
     *
     *	3. p₂x = c₂x + r2·cos θ
     *	4. p₂y = c₂y + r2·sin θ
     *
     * Point p lies at a fraction t along the line segment p₁p₂:
     *
     *	5. px = t·p₂x + (1-t)·p₁x
     *	6. py = t·p₂y + (1-t)·p₁y
     *
     * To solve, first subtitute 1-4 into 5 and 6:
     *
     * px = t·(c₂x + r₂·cos θ) + (1-t)·(c₁x + r₁·cos θ)
     * py = t·(c₂y + r₂·sin θ) + (1-t)·(c₁y + r₁·sin θ)
     *
     * Then solve each for cos θ and sin θ expressed as a function of t:
     *
     * cos θ = (-(c₂x - c₁x)·t + (px - c₁x)) / ((r₂-r₁)·t + r₁)
     * sin θ = (-(c₂y - c₁y)·t + (py - c₁y)) / ((r₂-r₁)·t + r₁)
     *
     * To simplify this a bit, we define new variables for several of the
     * common terms as shown below:
     *
     *              p₂
     *           p  •
     *           •   ╲
     *        ·  ┆    ╲r₂
     *  p₁ ·     ┆     ╲
     *  •     pdy┆      ╲
     *   ╲       ┆       •c₂
     *    ╲r₁    ┆   ·   ┆
     *     ╲    ·┆       ┆cdy
     *      •╌╌╌╌┴╌╌╌╌╌╌╌┘
     *    c₁  pdx   cdx
     *
     * cdx = (c₂x - c₁x)
     * cdy = (c₂y - c₁y)
     *  dr =  r₂-r₁
     * pdx =  px - c₁x
     * pdy =  py - c₁y
     *
     * Note that cdx, cdy, and dr do not depend on point p at all, so can
     * be pre-computed for the entire gradient. The simplifed equations
     * are now:
     *
     * cos θ = (-cdx·t + pdx) / (dr·t + r₁)
     * sin θ = (-cdy·t + pdy) / (dr·t + r₁)
     *
     * Finally, to get a single function of t and eliminate the last
     * unknown θ, we use the identity sin²θ + cos²θ = 1. First, square
     * each equation, (we knew a quadratic was coming since it must be
     * possible to obtain two solutions in some cases):
     *
     * cos²θ = (cdx²t² - 2·cdx·pdx·t + pdx²) / (dr²·t² + 2·r₁·dr·t + r₁²)
     * sin²θ = (cdy²t² - 2·cdy·pdy·t + pdy²) / (dr²·t² + 2·r₁·dr·t + r₁²)
     *
     * Then add both together, set the result equal to 1, and express as a
     * standard quadratic equation in t of the form At² + Bt + C = 0
     *
     * (cdx² + cdy² - dr²)·t² - 2·(cdx·pdx + cdy·pdy + r₁·dr)·t + (pdx² + pdy² - r₁²) = 0
     *
     * In other words:
     *
     * A = cdx² + cdy² - dr²
     * B = -2·(pdx·cdx + pdy·cdy + r₁·dr)
     * C = pdx² + pdy² - r₁²
     *
     * And again, notice that A does not depend on p, so can be
     * precomputed. From here we just use the quadratic formula to solve
     * for t:
     *
     * t = (-2·B ± ⎷(B² - 4·A·C)) / 2·A
     */

    gradient_t *gradient = (gradient_t *)image;
    source_image_t *source = (source_image_t *)image;
    radial_gradient_t *radial = (radial_gradient_t *)image;
    uint32_t *end = buffer + width;
    float cx = 1.;
    float cy = 0.;
    float cz = 0.;
    float rx = x + 0.5;
    float ry = y + 0.5;
    float rz = 1.;

    if (source->common.transform)
    {
	pixman_vector_t v;
	/* reference point is the center of the pixel */
	v.vector[0] = pixman_int_to_fixed (x) + pixman_fixed_1 / 2;
	v.vector[1] = pixman_int_to_fixed (y) + pixman_fixed_1 / 2;
	v.vector[2] = pixman_fixed_1;
	
	if (!pixman_transform_point_3d (source->common.transform, &v))
	    return;

	cx = source->common.transform->matrix[0][0] / 65536.;
	cy = source->common.transform->matrix[1][0] / 65536.;
	cz = source->common.transform->matrix[2][0] / 65536.;
	
	rx = v.vector[0] / 65536.;
	ry = v.vector[1] / 65536.;
	rz = v.vector[2] / 65536.;
    }

    /* When computing t over a scanline, we notice that some expressions
     * are constant so we can compute them just once. Given:
     *
     * t = (-2·B ± ⎷(B² - 4·A·C)) / 2·A
     *
     * where
     *
     * A = cdx² + cdy² - dr² [precomputed as radial->A]
     * B = -2·(pdx·cdx + pdy·cdy + r₁·dr)
     * C = pdx² + pdy² - r₁²
     *
     * Since we have an affine transformation, we know that (pdx, pdy)
     * increase linearly with each pixel,
     *
     * pdx = pdx₀ + n·cx,
     * pdy = pdy₀ + n·cy,
     *
     * we can then express B in terms of an linear increment along
     * the scanline:
     *
     * B = B₀ + n·cB, with
     * B₀ = -2·(pdx₀·cdx + pdy₀·cdy + r₁·dr) and
     * cB = -2·(cx·cdx + cy·cdy)
     *
     * Thus we can replace the full evaluation of B per-pixel (4 multiplies,
     * 2 additions) with a single addition.
     */
    float r1   = radial->c1.radius / 65536.;
    float r1sq = r1 * r1;
    float pdx  = rx - radial->c1.x / 65536.;
    float pdy  = ry - radial->c1.y / 65536.;
    float A = radial->A;
    float invA = -65536. / (2. * A);
    float A4 = -4. * A;
    float B  = -2. * (pdx*radial->cdx + pdy*radial->cdy + r1*radial->dr);
    float cB = -2. *  (cx*radial->cdx +  cy*radial->cdy);
    pixman_bool_t invert = A * radial->dr < 0;

    uint32_t *lut = gradient->color_lut;

    int shift = 16 - gradient->color_lut_bits;

    __m128i mask_0xffff =  _mm_set1_epi32(0xffff);
    __m128 invert_ = _mm_set1_ps (invert ? 1.0 : -1.0);
    __m128 A4_ = _mm_set1_ps (A4);
    __m128 invA_ = _mm_set1_ps (invA);
    __m128 r1sq_ = _mm_set1_ps (r1sq);
    __m128 pdx_ = _mm_set_ps (pdx + 3 * cx, pdx + 2 * cx, pdx + cx, pdx);
    __m128 pdy_ = _mm_set_ps (pdy + 3 * cy, pdy + 2 * cy, pdy + cy, pdy);
    __m128 B_ = _mm_set_ps (B + 3 * cB, B + 2 * cB, B + cB, B);	
    __m128 cx_ = _mm_set1_ps (cx);
    __m128 cy_ = _mm_set1_ps (cy);
    __m128 cB_ = _mm_set1_ps (cB);
    __m128 cx_4 = _mm_mul_ps (cx_, _mm_set1_ps (4.0));
    __m128 cy_4 = _mm_mul_ps (cy_, _mm_set1_ps (4.0));
    __m128 cB_4 = _mm_mul_ps (cB_, _mm_set1_ps (4.0));

    pixman_fixed_16_16_t tsf[4];

    if (gradient->color_lut)
    {
        unsigned int repeat = source->common.repeat;
        if (repeat == PIXMAN_REPEAT_NORMAL)
	{
	    SSE2_RADIAL_GRADIENT(SSE2_LUT_REPEAT_NORMAL);
	}
	else if (repeat == PIXMAN_REPEAT_PAD)
	{
	    SSE2_RADIAL_GRADIENT(SSE2_LUT_REPEAT_PAD);
	}
	else if (repeat == PIXMAN_REPEAT_REFLECT)
	{
	    SSE2_RADIAL_GRADIENT(SSE2_LUT_REPEAT_REFLECT);
	}
	else /* PIXMAN_REPEAT_NONE */
	{
	    SSE2_RADIAL_GRADIENT(SSE2_LUT_REPEAT_NONE);
	}
    }
    else
    {
        pixman_sse2_gradient_walker_t walker;
	_pixman_sse2_gradient_walker_init (&walker, gradient, source->common.repeat);

	SSE2_RADIAL_GRADIENT(SSE2_USE_GRADIENT_WALKER);
    }
}
