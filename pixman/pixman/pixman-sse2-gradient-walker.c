/*
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *             2005 Lars Knoll & Zack Rusin, Trolltech
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

typedef struct pixman_sse2_gradient_walker pixman_sse2_gradient_walker_t;

struct pixman_sse2_gradient_walker
{
    __m128i                 left_ag;
    __m128i                 left_rb;
    __m128i                 right_ag;
    __m128i                 right_rb;
    __m128i                 left_x;
    __m128i                 right_x;
    __m128i                 stepper;

    pixman_gradient_stop_t *stops;
    int                     num_stops;
    unsigned int            spread;
    int                     need_reset;
};

static inline void
_pixman_sse2_gradient_walker_init (pixman_sse2_gradient_walker_t *walker,
				   gradient_t *              gradient,
				   unsigned int              spread)
{
    walker->num_stops        = gradient->n_stops;
    walker->stops            = gradient->stops;
    walker->left_x           = _mm_set1_epi32 (0);
    walker->right_x          = _mm_set1_epi32 (0x10000);
    walker->stepper          = _mm_set1_epi32 (0);
    walker->left_ag          = _mm_set1_epi32 (0);
    walker->left_rb          = _mm_set1_epi32 (0);
    walker->right_ag         = _mm_set1_epi32 (0);
    walker->right_rb         = _mm_set1_epi32 (0);
    walker->spread           = spread;

    walker->need_reset       = TRUE;
}

static inline void
_pixman_sse2_gradient_walker_reset (pixman_sse2_gradient_walker_t *walker,
				    __m128i                   pos_)
{
    walker->need_reset = FALSE;

    int32_t x, pos[4], left_x[4], right_x[4];
    pixman_color_t *left_c[4], *right_c[4];
    uint32_t left_ag[4], left_rb[4], right_ag[4], right_rb[4];
    uint32_t stepper[4];
    int i, n, count = walker->num_stops;
    pixman_gradient_stop_t *      stops = walker->stops;

    static const pixman_color_t transparent_black = { 0, 0, 0, 0 };

    _mm_storeu_si128 ((__m128i*)pos, pos_);

    memset (left_x, 0, sizeof (left_x));
    memset (right_x, 0, sizeof (right_x));
    memset (left_ag, 0, sizeof (left_ag));
    memset (left_rb, 0, sizeof (left_rb));
    memset (right_ag, 0, sizeof (right_ag));
    memset (right_rb, 0, sizeof (right_rb));
    memset (stepper, 0, sizeof (stepper));

    for (i = 0; i < 4; i ++)
    {
	switch (walker->spread)
	{
	case PIXMAN_REPEAT_NORMAL:
	    x = pos[i] & 0xFFFF;
            for (n = 0; n < count; n++)
                if (x < stops[n].x)
                    break;
            if (n == 0)
            {
                left_x[i] =  stops[count - 1].x - 0x10000;
                left_c[i] = &stops[count - 1].color;
            }
            else
            {
                left_x[i] =  stops[n - 1].x;
                left_c[i] = &stops[n - 1].color;
            }

            if (n == count)
            {
                right_x[i] =  stops[0].x + 0x10000;
                right_c[i] = &stops[0].color;
            }
            else
            {
                right_x[i] =  stops[n].x;
                right_c[i] = &stops[n].color;
            }
            left_x[i]  += (pos[i] - x);
            right_x[i] += (pos[i] - x);
            break;

    case PIXMAN_REPEAT_PAD:
            for (n = 0; n < count; n++)
                if (pos[i] < stops[n].x)
                    break;

            if (n == 0)
            {
                left_x[i] =  INT32_MIN;
                left_c[i] = &stops[0].color;
            }
            else
            {
                left_x[i] =  stops[n - 1].x;
                left_c[i] = &stops[n - 1].color;
            }

            if (n == count)
            {
                right_x[i] =  INT32_MAX;
                right_c[i] = &stops[n - 1].color;
            }
            else
            {
                right_x[i] =  stops[n].x;
                right_c[i] = &stops[n].color;
            }
            break;

    case PIXMAN_REPEAT_REFLECT:
            x = (int32_t)pos[i] & 0xFFFF;
            if ((int32_t)pos[i] & 0x10000)
                x = 0x10000 - x;
            for (n = 0; n < count; n++)
                if (x < stops[n].x)
                    break;

            if (n == 0)
            {
                left_x[i] =  -stops[0].x;
                left_c[i] = &stops[0].color;
            }
            else
            {
                left_x[i] =  stops[n - 1].x;
                left_c[i] = &stops[n - 1].color;
            }

            if (n == count)
            {
                right_x[i] = 0x20000 - stops[n - 1].x;
                right_c[i] = &stops[n - 1].color;
            }
            else
            {
                right_x[i] =  stops[n].x;
                right_c[i] = &stops[n].color;
            }

            if ((int32_t)pos[i] & 0x10000)
            {
                pixman_color_t  *tmp_c;
                int32_t tmp_x;

                tmp_x      = 0x10000 - right_x[i];
                right_x[i] = 0x10000 - left_x[i];
                left_x[i]  = tmp_x;

                tmp_c      = right_c[i];
                right_c[i] = left_c[i];
                left_c[i]  = tmp_c;

                x = 0x10000 - x;
            }
            left_x[i]  += (pos[i] - x);
            right_x[i] += (pos[i] - x);
            break;

    default:  /* REPEAT_NONE */
            for (n = 0; n < count; n++)
                if (pos[i] < stops[n].x)
                    break;

            if (n == 0)
            {
                left_x[i]  =  INT32_MIN;
                right_x[i] =  stops[0].x;
                left_c[i]  = right_c[i] = (pixman_color_t*) &transparent_black;
            }
            else if (n == count)
            {
                left_x[i]  = stops[n - 1].x;
                right_x[i] = INT32_MAX;
                left_c[i]  = right_c[i] = (pixman_color_t*) &transparent_black;
            }
            else
            {
                left_x[i]  =  stops[n - 1].x;
                right_x[i] =  stops[n].x;
                left_c[i]  = &stops[n - 1].color;
                right_c[i] = &stops[n].color;
            }
	}

	left_ag[i]  = ((left_c[i]->alpha >> 8) << 16)   | (left_c[i]->green >> 8);
	left_rb[i]  = ((left_c[i]->red & 0xff00) << 8)  | (left_c[i]->blue >> 8);
	right_ag[i] = ((right_c[i]->alpha >> 8) << 16)  | (right_c[i]->green >> 8);
	right_rb[i] = ((right_c[i]->red & 0xff00) << 8) | (right_c[i]->blue >> 8);

	if (left_x[i] == right_x[i]                ||
	    ( left_ag[i] == right_ag[i] &&
	      left_rb[i] == right_rb[i] )   )
	{
	    stepper[i] = 0;
	}
	else
	{
	    int32_t width = right_x[i] - left_x[i];
	    stepper[i] = ((1 << 24) + width / 2) / width;
	}
    }

    walker->left_x   = _mm_loadu_si128 ((__m128i*)left_x);
    walker->right_x  = _mm_loadu_si128 ((__m128i*)right_x);

    walker->left_ag  = _mm_loadu_si128 ((__m128i*)left_ag);
    walker->left_rb  = _mm_loadu_si128 ((__m128i*)left_rb);

    walker->right_ag = _mm_loadu_si128 ((__m128i*)right_ag);
    walker->right_rb = _mm_loadu_si128 ((__m128i*)right_rb);

    walker->stepper  = _mm_loadu_si128 ((__m128i*)stepper);

}

/* this is a rather unsafe operation since it assumes that any
 * multiplication result will fit in a 32 bit integer..
 */
static force_inline __m128i
epi32__mul_epi32_epi32 (__m128i x, __m128i y)
{
    __m128i r1, r2;

    // r1[0] * r2[0], r1[2] * r2[2]
    r1 = _mm_mul_epu32 (x, y);

    // r1[1] * r2[1], r1[3] * r2[3]
    r2 = _mm_mul_epu32 (_mm_shuffle_epi32 (x, _MM_SHUFFLE(2, 3, 0, 1)),
			_mm_shuffle_epi32 (y, _MM_SHUFFLE(2, 3, 0, 1)));

    r1 = _mm_and_si128 (r1, _mm_set_epi32 (0x00000000, 0xffffffff, 0x00000000, 0xffffffff));
    r2 = _mm_and_si128 (_mm_shuffle_epi32 (r2, _MM_SHUFFLE(2, 3, 0, 1)),
			_mm_set_epi32 (0xffffffff, 0x00000000, 0xffffffff, 0x00000000));

    return _mm_or_si128 (r1, r2);
}

static force_inline __m128i
_pixman_sse2_gradient_walker_pixel (pixman_sse2_gradient_walker_t *walker,
				    __m128i xs)
{
    __m128i dist, idist;

    __m128i t1, t2, a, color;


    // this block basically inlines PIXMAN_GRADIENT_WALKER_NEED_RESET (walker, xs)
    uint32_t cmp_result = 0;
    if (walker->need_reset)
        cmp_result = 1;
    if (!cmp_result)
    {
        __m128i cmp = _mm_or_si128 (_mm_or_si128 (_mm_cmplt_epi32 (xs, walker->left_x),
						  _mm_cmpgt_epi32 (xs, walker->right_x)),
				    _mm_cmpeq_epi32 (xs, walker->right_x));
					      
	uint32_t cmp_results[4];

	_mm_storeu_si128 ((__m128i*)cmp_results, cmp);
	cmp_result = cmp_results[0] | cmp_results[1] | cmp_results[2] | cmp_results[3];
    }

    if (cmp_result /* if any of the positions require a reset, we reset them all */)
        _pixman_sse2_gradient_walker_reset (walker, xs);


    dist = _mm_sub_epi32 (xs, walker->left_x);

    dist = epi32__mul_epi32_epi32 (dist, walker->stepper);

    dist = _mm_srli_epi32 (dist, 16);

    idist = _mm_sub_epi32 (_mm_set1_epi32 (256), dist);

    t1 = _mm_add_epi32 (epi32__mul_epi32_epi32 (walker->left_rb, idist),
			epi32__mul_epi32_epi32 (walker->right_rb, dist));
    t1 = _mm_and_si128 (_mm_srli_epi32 (t1, 8), _mm_set1_epi32 (0xff00ff));

    t2 = _mm_add_epi32 (epi32__mul_epi32_epi32 (walker->left_ag, idist),
			epi32__mul_epi32_epi32 (walker->right_ag, dist));
    t2 = _mm_and_si128 (t2, _mm_set1_epi32 (0xff00ff00));

    color = _mm_and_si128 (t2, _mm_set1_epi32 (0xff000000));
    a = _mm_srli_epi32 (t2, 24);

    t1 = _mm_add_epi32 (epi32__mul_epi32_epi32 (t1, a),
			_mm_set1_epi32 (0x800080));

    t1 = _mm_srli_epi32 (_mm_add_epi32 (t1, _mm_and_si128 (_mm_srli_epi32 (t1, 8), _mm_set1_epi32 (0xff00ff))), 8);


    t2 = _mm_add_epi32 (epi32__mul_epi32_epi32 (_mm_srli_epi32 (t2, 8), a),
			_mm_set1_epi32 (0x800080));

    t2 = _mm_add_epi32 (t2, _mm_and_si128 (_mm_srli_epi32 (t2, 8), _mm_set1_epi32 (0xff00ff)));

    return _mm_or_si128 (_mm_or_si128 (color,
				       _mm_and_si128 (t1, _mm_set1_epi32 (0xff00ff))),
			 _mm_and_si128 (t2, _mm_set1_epi32 (0xff00)));
}

