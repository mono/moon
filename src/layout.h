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
#include <fonts.h>
#include <list.h>

class TextLayout;

class ITextAttributes {
 public:
	virtual ~ITextAttributes () {};
	virtual TextFontDescription *FontDescription () = 0;
	virtual TextDecorations Decorations () = 0;
	virtual Brush *Background (bool selected) = 0;
	virtual Brush *Foreground (bool selected) = 0;
};

class TextLayoutAttributes : public List::Node {
 public:
	ITextAttributes *source;
	int start;
	
	TextLayoutAttributes (ITextAttributes *source, int start)
	{
		this->source = source;
		this->start = start;
	}
	
	Brush *Background (bool selected)
	{
		return source->Background (selected);
	}
	
	Brush *Foreground (bool selected)
	{
		return source->Foreground (selected);
	}
	
	TextFont *Font ()
	{
		return source->FontDescription ()->GetFont ();
	}
	
	bool IsUnderlined ()
	{
		return (source->Decorations () & TextDecorationsUnderline);
	}
};

struct TextLayoutGlyphCluster {
	int start, length;
	moon_path *path;
	double uadvance;
	double advance;
	bool selected;
	
	TextLayoutGlyphCluster (int start, int length);
	~TextLayoutGlyphCluster ();
	
	void Render (cairo_t *cr, const Point &origin, TextLayoutAttributes *attrs, const char *text, double x, double y, bool uline_full);
};

struct TextLayoutLine {
	int start, length, offset, count;
	TextLayout *layout;
	GPtrArray *runs;
	double advance;
	double descend;
	double height;
	double width;
	
	TextLayoutLine (TextLayout *layout, int start, int offset);
	~TextLayoutLine ();
	
	void Render (cairo_t *cr, const Point &origin, double left, double top);
	
	int GetCursorFromX (const Point &offset, double x);
};

struct TextLayoutRun {
	TextLayoutAttributes *attrs;
	int start, length, count;
	TextLayoutLine *line;
	GPtrArray *clusters;
	double advance;
	
	TextLayoutRun (TextLayoutLine *line, TextLayoutAttributes *attrs, int start);
	~TextLayoutRun ();
	
	void Render (cairo_t *cr, const Point &origin, double x, double y, bool is_last_run);
	void GenerateCache ();
	void ClearCache ();
};

class TextLayout {
	LineStackingStrategy strategy;
	TextAlignment alignment;
	TextWrapping wrapping;
	int selection_length;
	int selection_start;
	double base_descent;
	double base_height;
	double avail_width;
	double line_height;
	double max_height;
	double max_width;
	List *attributes;
	bool is_wrapped;
	char *text;
	int length;
	int count;
	
	// cached data
	double actual_height;
	double actual_width;
	GPtrArray *lines;
	
	bool OverrideLineHeight () { return (strategy == LineStackingStrategyBlockLineHeight && line_height != 0); }
	double LineHeightOverride ();
	double DescendOverride ();
	
	void ClearCache ();
	void ClearLines ();
	
 public:
	TextLayout ();
	~TextLayout ();
	
	//
	// Property Accessors
	//
	// Set[Property]() accessors return %true if the extents have
	// changed or %false otherwise.
	//
	
	int GetSelectionLength () { return selection_length; }
	int GetSelectionStart () { return selection_start; }
	
	LineStackingStrategy GetLineStackingStrtegy () { return strategy; }
	bool SetLineStackingStrategy (LineStackingStrategy mode);
	
	List *GetTextAttributes () { return attributes; }
	bool SetTextAttributes (List *attrs);
	
	TextAlignment GetTextAlignment () { return alignment; }
	bool SetTextAlignment (TextAlignment align);
	
	TextWrapping GetTextWrapping () { return wrapping; }
	bool SetTextWrapping (TextWrapping mode);
	
	double GetLineHeight () { return line_height; }
	bool SetLineHeight (double height);
	
	double GetMaxHeight () { return max_height; }
	bool SetMaxHeight (double height);
	
	double GetAvailableWidth () { return avail_width; }
	bool SetAvailableWidth (double width) { avail_width = width; return false; }
	
	double GetMaxWidth () { return max_width; }
	bool SetMaxWidth (double width);
	
	bool SetText (const char *str, int len = -1);
	const char *GetText () { return text; }
	
	void SetBaseFont (const TextFont *font);
	
	//
	// Methods
	//
	
	void Render (cairo_t *cr, const Point &origin, const Point &offset);
	void Select (int start, int length, bool byte_offsets = false);
	void ResetState ();
	void Layout ();
	
	double HorizontalAlignment (double line_width);
	
	TextLayoutLine *GetLineFromY (const Point &offset, double y, int *index = NULL);
	TextLayoutLine *GetLineFromIndex (int index);
	int GetLineCount () { return lines->len; }
	
	int GetCursorFromXY (const Point &offset, double x, double y);
	Rect GetCursor (const Point &offset, int pos);
	
	void GetActualExtents (double *width, double *height);
	Rect GetRenderExtents ();
};

#endif /* __LAYOUT_H__ */

