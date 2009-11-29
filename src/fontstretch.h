/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * fontstretch.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __FONT_STRETCH_H__
#define __FONT_STRETCH_H__

/* @IncludeInKinds */
struct FontStretch {
	FontStretches stretch;

	FontStretch (FontStretches stretch)
	{
		this->stretch = stretch;
	}

	bool operator== (const FontStretch &v) const
	{
		return v.stretch == stretch;
	}
	
	bool operator!= (const FontStretch &v) const
	{
		return !(*this == v);
	}
};

#endif /* __FONT_STRETCH_H__ */



