/*
 * Copyright © 2010 Novell, Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author:  Chris Toshok (toshok@novell.com)
 */

#ifndef PIXMAN_GRADIENT_SSE2_H
#define PIXMAN_GRADIENT_SSE2_H

#define CLAMP_MAX(v,max) ((v) < 0 ? 0 : (v) > (max) ? (max) : (v))

#define SSE2_USE_GRADIENT_WALKER(_ts,_buffer,_lut,_shift,_storeop)	\
  do {									\
    _mm_##_storeop##_si128 ((__m128i*)_buffer, _pixman_sse2_gradient_walker_pixel (&walker, _ts)); \
  } while (0)

#define SSE2_LUT_REPEAT_NORMAL(_ts,_buffer,_lut,_shift,_storeop)	\
  do {									\
    __m128i x = _ts;							\
    									\
    /* t = abs(t); */							\
    /* t = t & 0xFFFF; */						\
    									\
    /* abs() hack from http://graphics.stanford.edu/~seander/bithacks.html#IntegerAbs */ \
    __m128i mask = _mm_srai_epi32 (x, 31);				\
    x = _mm_xor_si128 (_mm_add_epi32 (x, mask), mask);			\
    									\
    x = _mm_and_si128 (x, mask_0xffff);					\
    x = _mm_srai_epi32 (x, _shift);					\
    									\
    _mm_##_storeop##_si128 ((__m128i*)_buffer, x);			\
    									\
    _buffer[0] = _lut[_buffer[0]];					\
    _buffer[1] = _lut[_buffer[1]];					\
    _buffer[2] = _lut[_buffer[2]];					\
    _buffer[3] = _lut[_buffer[3]];					\
} while (0)

#define SSE2_LUT_REPEAT_REFLECT(_ts,_buffer,_lut,_shift,_storeop)	\
  do {									\
    __m128i x = _ts;							\
    /* s = t << 15 >> 31; */						\
    /* t = (t ^ s) & 0xFFFF; */						\
    /* t >>= shift */							\
    __m128i ss =  _mm_srai_epi32 (_mm_slli_epi32 (ts, 15), 31);		\
    x = _mm_xor_si128 (ss, x);						\
    x = _mm_and_si128 (x, mask_0xffff);					\
    x = _mm_srai_epi32 (x, _shift);					\
    									\
    _mm_##_storeop##_si128 ((__m128i*)_buffer, x);			\
    									\
    _buffer[0] = _lut[_buffer[0]];					\
    _buffer[1] = _lut[_buffer[1]];					\
    _buffer[2] = _lut[_buffer[2]];					\
    _buffer[3] = _lut[_buffer[3]];					\
} while (0)

#define SSE2_LUT_REPEAT_NONE(_ts,_buffer,_lut,_shift,_storeop)		\
  do {									\
    __m128i x = ts;							\
    									\
    __m128i cmp_lt0 = _mm_cmplt_epi32 (x, _mm_set1_epi32 (0));		\
    __m128i cmp_gt0xffff = _mm_cmpgt_epi32 (x, mask_0xffff);		\
    									\
    /* cmp_mask_in_range = 0xffffffff where values are >= 0 and <= 0xffff, */ \
    /* cmp_mask_in_range = 0x00000000 where values are < 0 or > 0xffff */ \
    									\
    /* cmp_mask_out_of_range = 0xffffffff where values are < 0 or > 0xffff, */ \
    /* cmp_mask_out_of_range = 0x00000000 where values are >= 0 and <= 0xffff */ \
    __m128i cmp_mask_out_of_range = _mm_or_si128 (cmp_lt0, cmp_gt0xffff); \
    __m128i cmp_mask_in_range = _mm_andnot_si128 (cmp_mask_out_of_range, \
						  _mm_set1_epi32 (0xffffffff)); \
    									\
    /* take care of the shift now, since we don't want to shift the sentinel (0xffffffff) */ \
    x = _mm_srai_epi32 (x, _shift);					\
    									\
    /* or together the masked in range x values and the out of range sentinel values */ \
    x = _mm_or_si128 (_mm_and_si128 (cmp_mask_in_range, x),		\
		      _mm_and_si128 (cmp_mask_out_of_range, _mm_set1_epi32 (0xffffffff))); \
    									\
    _mm_##_storeop##_si128 ((__m128i*)_buffer, x);			\
    									\
    _buffer[0] = _buffer[0] == 0xffffffff ? 0 : _lut[_buffer[0]];	\
    _buffer[1] = _buffer[1] == 0xffffffff ? 0 : _lut[_buffer[1]];	\
    _buffer[2] = _buffer[2] == 0xffffffff ? 0 : _lut[_buffer[2]];	\
    _buffer[3] = _buffer[3] == 0xffffffff ? 0 : _lut[_buffer[3]];	\
} while (0)

#define SSE2_LUT_REPEAT_PAD(_ts,_buffer,_lut,_shift,_storeop)		\
  do {									\
  									\
    /* t = t < 0 ? 0 : t > 0xffff ? 0xffff : t */			\
    /* t >>= shift */							\
    									\
    __m128i x = ts;							\
    __m128i cmp;							\
    									\
    /* handle t = t < 0 ? 0 */						\
    cmp = _mm_cmpgt_epi32 (x, _mm_set1_epi32 (0));			\
    x = _mm_and_si128 (x, cmp);						\
    									\
    /* handle t > 0xffff ? 0xffff */					\
    cmp = _mm_cmpgt_epi32 (x, mask_0xffff);				\
    x = _mm_andnot_si128 (cmp, x);					\
    x = _mm_add_epi32 (x, _mm_and_si128 (cmp, mask_0xffff));		\
    									\
    x = _mm_srai_epi32 (x, _shift);					\
    									\
    _mm_##_storeop##_si128 ((__m128i*)_buffer, x);			\
    									\
    _buffer[0] = _lut[_buffer[0]];					\
    _buffer[1] = _lut[_buffer[1]];					\
    _buffer[2] = _lut[_buffer[2]];					\
    _buffer[3] = _lut[_buffer[3]];					\
} while (0)

#endif
