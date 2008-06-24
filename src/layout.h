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

#include "brush.h"
#include "font.h"
#include "list.h"


class TextRun : public List::Node {
 public:
	TextDecorations deco;
	gunichar *text;
	TextFont *font;
	Brush **fg;
	
	TextRun (const char *utf8, int len, TextDecorations deco, TextFontDescription *font, Brush **fg);
	TextRun (TextFontDescription *font);
	~TextRun ();
	
	bool IsUnderlined () { return (deco & TextDecorationsUnderline); }
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
	double bbox_height;
	double bbox_width;
	
	void LayoutWrapWithOverflow ();
	void LayoutNoWrap ();
	void LayoutWrap ();
	
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
	
	void Layout ();
	void GetActualExtents (double *width, double *height);
	void GetLayoutExtents (double *width, double *height);
	void Render (cairo_t *cr, UIElement *element, Brush *default_fg, double x, double y);
};

#endif /* __LAYOUT_H__ */
