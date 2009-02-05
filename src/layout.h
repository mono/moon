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
	virtual Brush *Background (bool selected) = 0;
	virtual Brush *Foreground (bool selected) = 0;
};

class TextRun : public List::Node {
	const gunichar *text;
	ITextSource *source;
	TextFont *font;
	gunichar *buf;
	bool selected;
	int length;
	
 public:
	
	TextRun (const gunichar *ucs4, int len, ITextSource *source, bool selected = false);
	TextRun (const char *utf8, int len, ITextSource *source, bool selected = false);
	TextRun (ITextSource *source);
	
	virtual ~TextRun ();
	
	//
	// Property Accessors
	//
	TextDecorations Decorations () { return source->Decorations (); }
	Brush *Background () { return source->Background (selected); }
	Brush *Foreground () { return source->Foreground (selected); }
	bool IsSelected () { return selected; }
	const gunichar *Text () { return text; }
	int Length () { return length; }
	TextFont *Font () { return font; }
	
	bool IsUnderlined ()
	{
		return (Decorations () & TextDecorationsUnderline);
	}
	
	bool IsLineBreak ()
	{
		return !text || text[0] == '\r' || text[0] == '\n';
	}
};

class TextLine;

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
	
	double HorizontalAlignment (double line_width);
	
	TextLine *GetLineFromY (const Point &origin, double y, int *index);
	
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
	
	void Render (cairo_t *cr, const Point &origin, const Point &offset);
	int GetCursorFromXY (const Point &offset, double x, double y);
	Rect GetCursor (const Point &offset, int pos);
	void Layout ();
	
	void GetActualExtents (double *width, double *height);
	//void GetLayoutExtents (double *width, double *height);
};

#endif /* __LAYOUT_H__ */
