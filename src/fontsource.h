/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * fontsource.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __FONT_SOURCE_H__
#define __FONT_SOURCE_H__

#include <glib.h>
#include <string.h>

#include "utils.h"

namespace Moonlight {

enum FontSourceType {
	FontSourceTypeManagedStream,
	FontSourceTypeGlyphTypeface,
};

/* @IncludeInKinds */
struct FontSource {
	FontSourceType type;

	union {
		ManagedStreamCallbacks *stream;
		GlyphTypeface *typeface;
	} source;

	bool operator== (const FontSource& f) const
	{
		if (type != f.type)
			return false;

		switch (type) {
		case FontSourceTypeManagedStream:
			return source.stream->handle == f.source.stream->handle;
		case FontSourceTypeGlyphTypeface:
			return source.typeface == f.source.typeface;
		}

		// Should never be reached.
		return false;
	}

	bool operator!= (const FontSource& f) const
	{
		return !(*this == f);
	}
};

};
#endif /* __FONT_SOURCE_H__ */
