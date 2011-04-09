/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * font-utils.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __FONT_UTILS_H__
#define __FONT_UTILS_H__

#include "enums.h"

namespace Moonlight {

enum FontProperties {
	FontPropertyStretch = (1 << 0),
	FontPropertyWeight = (1 << 1),
	FontPropertyStyle = (1 << 2)
};

struct FontStyleInfo {
	char *family_name;
	FontStretches stretch;
	FontWeights weight;
	FontStyles style;
	int set;
};

void font_style_info_init (FontStyleInfo *info, const char *family);

void font_style_info_parse (FontStyleInfo *info, const char *style, bool family);

void font_style_info_hydrate (FontStyleInfo *info, const char *family, FontStretches stretch, FontWeights weight, FontStyles style);

int font_style_info_diff (const FontStyleInfo *actual, const FontStyleInfo *desired);

// this is for debug only
const char *font_style_info_to_string (FontStretches stretch, FontWeights weight, FontStyles style);

};

#endif /* __FONT_UTILS_H__ */
