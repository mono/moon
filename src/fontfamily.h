/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * fontfamily.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __FONT_FAMILY_H__
#define __FONT_FAMILY_H__

#include <glib.h>
#include <string.h>

namespace Moonlight {

/* @IncludeInKinds */
class FontFamily {
 public:
	char *source;
	
	FontFamily (const char *source)
	{
		this->source = g_strdup (source);
	}

	FontFamily (const FontFamily& family)
	{
		this->source = g_strdup (family.source);
	}

	FontFamily& operator= (const FontFamily& other)
	{
		if (this != &other) {
			g_free (this->source);
			this->source = g_strdup (other.source);
		}
		return *this;
	}

	~FontFamily ()
	{
		g_free (source);
	}

	bool operator== (const FontFamily &v) const
	{
		return strcmp (v.source, source) == 0;
	}
	
	bool operator!= (const FontFamily &v) const
	{
		return !(*this == v);
	}
};

};
#endif /* __FONT_FAMILY_H__ */
