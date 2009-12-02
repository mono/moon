/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * fontweight.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __FONT_WEIGHT_H__
#define __FONT_WEIGHT_H__

/* @IncludeInKinds */
struct FontWeight {
	FontWeights weight;

	FontWeight (FontWeights weight)
	{
		this->weight = weight;
	}

	bool operator== (const FontWeight &v) const
	{
		return v.weight == weight;
	}
	
	bool operator!= (const FontWeight &v) const
	{
		return !(*this == v);
	}
};

#endif /* __FONT_WEIGHT_H__ */



