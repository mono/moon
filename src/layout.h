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


class ITextSource {
 public:
	virtual TextFontDescription *FontDescription () = 0;
	virtual TextDecorations Decorations () = 0;
	virtual Brush *Foreground () = 0;
};

class TextRun : public List::Node {
 public:
	ITextSource *source;
	TextDecorations deco;
	TextFont *font;
	gunichar *text;
	
	TextRun (const gunichar *ucs4, int len, ITextSource *source);
	TextRun (const char *utf8, int len, ITextSource *source);
	TextRun (ITextSource *source);
	
	virtual ~TextRun ();
	
	bool IsUnderlined () { return (source->Decorations () & TextDecorationsUnderline); }
};

struct TextSelection {
	Brush *background;
	Brush *foreground;
	int start, length;
};

class TextLayout {
	// User-set data
	LineStackingStrategy strategy;
	TextAlignment alignment;
	TextWrapping wrapping;
	double line_height;
	double max_height;
	double max_width;
	List *runs;
	
	// Internal representation
	List *lines;
	
	// cached info
	double actual_height;
	double actual_width;
	
	bool OverrideLineHeight () { return (strategy == LineStackingStrategyBlockLineHeight && !isnan (line_height)); }
	
	void LayoutWrapWithOverflow ();
	void LayoutNoWrap ();
	void LayoutWrap ();
	
 public:
	
	TextLayout ();
	~TextLayout ();
	
	//
	// Property Accessors
	//
	// Set[Property]() accessors return %true if the extents have
	// changed or %false otherwise.
	//
	
	LineStackingStrategy GetLineStackingStrtegy () { return strategy; }
	bool SetLineStackingStrategy (LineStackingStrategy strategy);
	
	TextAlignment GetTextAlignment () { return alignment; }
	bool SetTextAlignment (TextAlignment alignment);
	
	TextWrapping GetTextWrapping () { return wrapping; }
	bool SetTextWrapping (TextWrapping wrapping);
	
	double GetLineHeight () { return line_height; }
	bool SetLineHeight (double height);
	
	double GetMaxHeight () { return max_height; }
	bool SetMaxHeight (double height);
	
	double GetMaxWidth () { return max_width; }
	bool SetMaxWidth (double width);
	
	List *GetTextRuns () { return runs; }
	bool SetTextRuns (List *runs);
	
	//
	// Methods
	//
	
	void Render (cairo_t *cr, const Point &origin, const Point &offset, TextSelection *selection = NULL, int cursor = -1);
	void Layout ();
	
	void GetActualExtents (double *width, double *height);
	//void GetLayoutExtents (double *width, double *height);
};

#endif /* __LAYOUT_H__ */
