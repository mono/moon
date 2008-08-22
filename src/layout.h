/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * layout.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include <cairo.h>
#include <glib.h>
#include <math.h>

#include <brush.h>
#include <enums.h>
#include <font.h>
#include <list.h>


class TextRun : public List::Node {
 public:
	TextDecorations deco;
	gunichar *text;
	TextFont *font;
	Brush **fg;
	
	TextRun (const gunichar *ucs4, int len, TextDecorations deco, TextFontDescription *font, Brush **fg);
	TextRun (const char *utf8, int len, TextDecorations deco, TextFontDescription *font, Brush **fg);
	TextRun (TextFontDescription *font);
	~TextRun ();
	
	bool IsUnderlined () { return (deco & TextDecorationsUnderline); }
};

class TextLayoutHints {
	LineStackingStrategy strategy;
	TextAlignment alignment;
	double lineHeight;
	
 public:
	TextLayoutHints (TextAlignment align, LineStackingStrategy strat, double height)
	{
		lineHeight = height;
		strategy = strat;
		alignment = align;
	}
	
	void SetLineStackingStrategy (LineStackingStrategy strat) { strategy = strat; }
	//LineStackingStrategy GetLineStackingStrtegy () { return strategy; }
	
	void SetLineHeight (double height) { lineHeight = height; }
	double GetLineHeight () { return lineHeight; }
	
	void SetTextAlignment (TextAlignment align) { alignment = align; }
	TextAlignment GetTextAlignment () { return alignment; }
	
	bool OverrideLineHeight () { return (strategy == LineStackingStrategyBlockLineHeight && !isnan (lineHeight)); }
};

class TextLayout {
	// User-set data
	TextWrapping wrapping;
	double max_height;
	double max_width;
	List *runs;
	
	// Internal representation
	List *lines;
	
	// cached info
	double actual_height;
	double actual_width;
	
	void LayoutWrapWithOverflow (TextLayoutHints *hints);
	void LayoutNoWrap (TextLayoutHints *hints);
	void LayoutWrap (TextLayoutHints *hints);
	
 public:
	
	TextLayout ();
	~TextLayout ();
	
	double GetMaxWidth ();
	void SetMaxWidth (double width);
	
	double GetMaxHeight ();
	void SetMaxHeight (double height);
	
	TextWrapping GetWrapping ();
	void SetWrapping (TextWrapping wrapping);
	
	List *GetTextRuns ();
	void SetTextRuns (List *runs);
	
	void Layout (TextLayoutHints *hints);
	
	void GetActualExtents (double *width, double *height);
	//void GetLayoutExtents (double *width, double *height);
	
	void Render (cairo_t *cr, TextLayoutHints *hints, UIElement *element, Brush *default_fg, double x, double y);
};

#endif /* __LAYOUT_H__ */
