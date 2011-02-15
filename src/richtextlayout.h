/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * textlayout.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __RICH_TEXT_LAYOUT_H__
#define __RICH_TEXT_LAYOUT_H__

#include <cairo.h>
#include <glib.h>
#include <math.h>

#include "brush.h"
#include "enums.h"
#include "fonts.h"
#include "list.h"
#include "textpointer.h"
#include "textselection.h"
#include "textlayout.h"
#include "textelement.h"

namespace Moonlight {

class RichTextLayout;
class RichTextLayoutInline;

class RichTextLayoutLine {
public:
	RichTextLayoutLine (RichTextLayout *layout, const TextPointer& start, double y);
	~RichTextLayoutLine ();

	void Render (cairo_t *cr, const Point &origin, double left, double top);
	
	TextPointer GetLocationFromX (const Point &offset, double x);

	void AddInline (RichTextLayoutInline *inline_);

	TextPointer start;
	TextPointer end;

	RichTextLayout *layout;
	GPtrArray *inlines;
	double descend;
	double y;
	Size size;
};

class RichTextLayoutInline {
public:
	RichTextLayoutInline (RichTextLayoutLine *line, TextLayoutAttributes *attrs, const TextPointer& start, const TextPointer &end)
	  : attrs (attrs), line (line), start (start), end (end)
	{
		descend = attrs->Font()->Descender ();
		size.height = attrs->Font()->Height ();
	}

	virtual ~RichTextLayoutInline () {}

	
	virtual void Render (cairo_t *cr, const Point &origin, double x, double y, bool is_last_run) { }

	virtual const char *GetText() { return ""; }

	virtual int GetIndexByXOffset (double xoffset) { return 0; }
	virtual double GetXOffsetByIndex (int index) { return 0.0; }

	TextLayoutAttributes *attrs;
	RichTextLayoutLine *line;
	TextPointer start;
	TextPointer end;

	double descend;
	Point position;
	Size size;
};

class RichTextLayoutInlineGlyphs : public RichTextLayoutInline {
public:
	RichTextLayoutInlineGlyphs (RichTextLayoutLine *line, TextLayoutAttributes *attrs, const TextPointer& start, const TextPointer &end)
	  : RichTextLayoutInline (line, attrs, start, end)
	{
		path = NULL;
		xoffset_table = NULL;
	}

	virtual ~RichTextLayoutInlineGlyphs ()
	{
		if (path)
			moon_path_destroy (path);
		delete xoffset_table;
	}

	virtual const char *GetText() { return ((Run*)start.GetParent())->GetText(); }

	virtual int GetIndexByXOffset (double xoffset);
	virtual double GetXOffsetByIndex (int index);

	virtual void Render (cairo_t *cr, const Point &origin, double x, double y, bool is_last_run);

	void GenerateCache ();
	void ClearCache ();

	moon_path *path;
	double *xoffset_table;
	double uadvance;
};

class RichTextLayoutInlineUIElement : public RichTextLayoutInline {
public:
	RichTextLayoutInlineUIElement (RichTextLayoutLine *line, TextLayoutAttributes *attrs, const TextPointer& start, UIElement *uielement)
		: RichTextLayoutInline (line, attrs, start, start),
		  uielement (uielement)
	{
		// FIXME: should uielement should be a weakref?
	}

	virtual ~RichTextLayoutInlineUIElement () {}

	UIElement *uielement;
};

class RichTextLayout {
 public:
	TextPointer selection_start;
	TextPointer selection_end;

	TextAlignment alignment;
	TextWrapping wrapping;
	double avail_width;
	double max_height;
	double max_width;
	bool is_wrapped;

	BlockCollection *blocks; // FIXME do we need to hang on to this?

	// cached data
	double actual_height;
	double actual_width;
	GPtrArray *lines;
	
	void ClearCache ();
	void ClearLines ();
	
	RichTextLayout ();
	~RichTextLayout ();
	
	//
	// Property Accessors
	//
	// Set[Property]() accessors return %true if the extents have
	// changed or %false otherwise.
	//
	
	TextAlignment GetTextAlignment () { return alignment; }
	bool SetTextAlignment (TextAlignment align);
	
	TextWrapping GetTextWrapping () { return wrapping; }
	bool SetTextWrapping (TextWrapping mode);
	
	double GetMaxHeight () { return max_height; }
	bool SetMaxHeight (double height);
	
	double GetAvailableWidth () { return avail_width; }
	bool SetAvailableWidth (double width) { avail_width = width; return false; }
	
	double GetMaxWidth () { return max_width; }
	bool SetMaxWidth (double width);

	bool SetBlocks (BlockCollection *blocks);
	
	//
	// Methods
	//
	
	void Render (cairo_t *cr, const Point &origin, const Point &offset);
	void Select (TextSelection *selection);
	void ResetState ();


	const char* AddWordsToLine (RichTextLayoutLine *line, TextLayoutAttributes *attrs, const char *inptr, const char *inend, TextPointer start, TextPointer end, TextPointer *endpoint);

	void Layout (RichTextBoxView *rtbview);
	void LayoutBlock (RichTextBoxView *rtbview, Block *b);

	double GetBaselineOffset ();

	double HorizontalAlignment (double line_width);
	
	RichTextLayoutLine *GetLineFromY (const Point &offset, double y, int *index = NULL);
	RichTextLayoutLine *GetLineFromIndex (int index);
	Rect GetCharacterRect (const TextPointer *tp, LogicalDirection direction);
	int GetLineCount () { return lines->len; }
	
	TextPointer GetLocationFromXY (const Point &offset, double x, double y);

	Rect GetCursor (const Point &offset, TextPointer* tp);
	
	void GetActualExtents (double *width, double *height);
	Rect GetRenderExtents ();

	void AddLine (RichTextLayoutLine *line);
};

};


#endif /* __RICH_TEXT_LAYOUT_H__ */
