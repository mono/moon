/*
 * Copyright © 2000 SuSE, Inc.
 * Copyright © 2007, 2009 Red Hat, Inc.
 * Copyright © 2009 Soren Sandmann
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
#include "pixman-private.h"

void
_pixman_general_solid_fill_get_scanline_32 (pixman_image_t *image,
					    int             x,
					    int             y,
					    int             width,
					    uint32_t *      buffer,
					    const uint32_t *mask,
					    uint32_t        mask_bits)
{
    uint32_t *end = buffer + width;
    uint32_t color = image->solid.color_32;

    while (buffer < end)
	*(buffer++) = color;

    return;
}

void
_pixman_general_solid_fill_get_scanline_64 (pixman_image_t *image,
					    int             x,
					    int             y,
					    int             width,
					    uint32_t *      buffer,
					    const uint32_t *mask,
					    uint32_t        mask_bits)
{
    uint64_t *b = (uint64_t *)buffer;
    uint64_t *e = b + width;
    uint64_t color = image->solid.color_64;

    while (b < e)
	*(b++) = color;
}


