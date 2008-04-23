/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * text.h: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __TEXT_H__
#define __TEXT_H__

G_BEGIN_DECLS

#include <cairo.h>

#include "downloader.h"
#include "moon-path.h"
#include "layout.h"
#include "brush.h"
#include "font.h"


void text_init (void);
void text_destroy (void);

class Inline : public DependencyObject {
 protected:
	virtual ~Inline ();

 public:
	static DependencyProperty *FontFamilyProperty;
	static DependencyProperty *FontSizeProperty;
	static DependencyProperty *FontStretchProperty;
	static DependencyProperty *FontStyleProperty;
	static DependencyProperty *FontWeightProperty;
	static DependencyProperty *ForegroundProperty;
	static DependencyProperty *TextDecorationsProperty;
	
	TextFontDescription *font;
	
	Brush *foreground;
	bool autogen;
	
	Inline ();
	virtual Type::Kind GetObjectType () { return Type::INLINE; }
	virtual Value *GetDefaultValue (DependencyProperty *prop);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
};

const char *inline_get_font_family (Inline *inline_);
void inline_set_font_family (Inline *inline_, const char *value);

double inline_get_font_size (Inline *inline_);
void inline_set_font_size (Inline *inline_, double value);

FontStretches inline_get_font_stretch (Inline *inline_);
void inline_set_font_stretch (Inline *inline_, FontStretches value);

FontStyles inline_get_font_style (Inline *inline_);
void inline_set_font_style (Inline *inline_, FontStyles value);

FontWeights inline_get_font_weight (Inline *inline_);
void inline_set_font_weight (Inline *inline_, FontWeights value);

Brush *inline_get_foreground (Inline *inline_);
void inline_set_foreground (Inline *inline_, Brush *value);

TextDecorations inline_get_text_decorations (Inline *inline_);
void inline_set_text_decorations (Inline *inline_, TextDecorations value);


class LineBreak : public Inline {
 protected:
	virtual ~LineBreak () {}

 public:
	LineBreak () { }
	virtual Type::Kind GetObjectType () { return Type::LINEBREAK; };
};

LineBreak *line_break_new (void);


/* @ContentProperty="Text" */
class Run : public Inline {
 protected:
	virtual ~Run () {}

 public:
	static DependencyProperty *TextProperty;
	
	Run () { }
	virtual Type::Kind GetObjectType () { return Type::RUN; };
	
	// property accessors
	void SetText (const char *text);
	const char *GetText ();
};

Run *run_new (void);

const char *run_get_text (Run *run);
void run_set_text (Run *run, const char *value);


/* @ContentProperty="Inlines" */
class TextBlock : public FrameworkElement {
	TextFontDescription *font;
	TextLayout *layout;
	Downloader *downloader;
	
	double actual_height;
	double actual_width;
	double bbox_height;
	double bbox_width;
	bool setvalue;
	bool dirty;
	
	void SetActualHeight (double height);
	void SetActualWidth (double width);
	
	void CalcActualWidthHeight (cairo_t *cr);
	void Layout (cairo_t *cr);
	void Paint (cairo_t *cr);
	
	char *GetTextInternal ();
	bool SetTextInternal (const char *text);
	
	double GetBoundingWidth ()
	{
		if (dirty)
			CalcActualWidthHeight (NULL);
		return bbox_width;
	}
	
	double GetBoundingHeight ()
	{
		if (dirty)
			CalcActualWidthHeight (NULL);
		return bbox_height;
	}
	
	void LayoutSilverlight (cairo_t *cr);
	
	void DownloaderComplete ();
	
	static void data_write (void *data, int32_t offset, int32_t n, void *closure);
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void size_notify (int64_t size, gpointer data);
	
 protected:
	virtual ~TextBlock ();

 public:
	static DependencyProperty *ActualHeightProperty;
	static DependencyProperty *ActualWidthProperty;
	static DependencyProperty *FontFamilyProperty;
	static DependencyProperty *FontSizeProperty;
	static DependencyProperty *FontStretchProperty;
	static DependencyProperty *FontStyleProperty;
	static DependencyProperty *FontWeightProperty;
	static DependencyProperty *ForegroundProperty;
	static DependencyProperty *InlinesProperty;
	static DependencyProperty *TextProperty;
	static DependencyProperty *TextDecorationsProperty;
	static DependencyProperty *TextWrappingProperty;
	
	TextBlock ();
	virtual Type::Kind GetObjectType () { return Type::TEXTBLOCK; };
	
	void SetFontSource (Downloader *downloader);
	
	//
	// Overrides
	//
	virtual void Render (cairo_t *cr, int x, int y, int width, int height);
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void ComputeBounds ();
	virtual bool InsideObject (cairo_t *cr, double x, double y);
	virtual Point GetTransformOrigin ();
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args);
	
	virtual Value *GetValue (DependencyProperty *property);
	
	//
	// Property Accessors
	//
	double GetActualWidth ()
	{
		if (dirty)
			CalcActualWidthHeight (NULL);
		return actual_width;
	}
	
	double GetActualHeight ()
	{
		if (dirty)
			CalcActualWidthHeight (NULL);
		return actual_height;
	}
	
	void SetFontFamily (const char *family);
	const char *GetFontFamily ();
	
	void SetFontSize (double size);
	double GetFontSize ();
	
	void SetFontStretch (FontStretches stretch);
	FontStretches GetFontStretch ();
	
	void SetFontStyle (FontStyles style);
	FontStyles GetFontStyle ();
	
	void SetFontWeight (FontWeights weight);
	FontWeights GetFontWeight ();
	
	void SetForeground (Brush *fg);
	Brush *GetForeground ();
	
	void SetInlines (Inlines *inlines);
	Inlines *GetInlines ();
	
	void SetText (const char *text);
	const char *GetText ();
	
	void SetTextDecorations (TextDecorations decorations);
	TextDecorations GetTextDecorations ();
	
	void SetTextWrapping (TextWrapping wrapping);
	TextWrapping GetTextWrapping ();
};

TextBlock *text_block_new (void);

double text_block_get_actual_height (TextBlock *textblock);
double text_block_get_actual_width (TextBlock *textblock);

const char *text_block_get_font_family (TextBlock *textblock);
void text_block_set_font_family (TextBlock *textblock, const char *family);

double text_block_get_font_size (TextBlock *textblock);
void text_block_set_font_size (TextBlock *textblock, double size);

FontStretches text_block_get_font_stretch (TextBlock *textblock);
void text_block_set_font_stretch (TextBlock *textblock, FontStretches stretch);

FontStyles text_block_get_font_style (TextBlock *textblock);
void text_block_set_font_style (TextBlock *textblock, FontStyles style);

FontWeights text_block_get_font_weight (TextBlock *textblock);
void text_block_set_font_weight (TextBlock *textblock, FontWeights weight);

Brush *text_block_get_foreground (TextBlock *textblock);
void text_block_set_foreground (TextBlock *textblock, Brush *foreground);

Inlines *text_block_get_inlines (TextBlock *textblock);
void text_block_set_inlines (TextBlock *textblock, Inlines *inlines);

const char *text_block_get_text (TextBlock *textblock);
void text_block_set_text (TextBlock *textblock, const char *text);

TextDecorations text_block_get_text_decorations (TextBlock *textblock);
void text_block_set_text_decorations (TextBlock *textblock, TextDecorations decorations);

TextWrapping text_block_get_text_wrapping (TextBlock *textblock);
void text_block_set_text_wrapping (TextBlock *textblock, TextWrapping wrapping);

void text_block_set_font_source (TextBlock *textblock, Downloader *downloader);


class Glyphs : public FrameworkElement {
	TextFontDescription *desc;
	Downloader *downloader;
	
	cairo_path_t *path;
	gunichar *text;
	List *attrs;
	Brush *fill;
	int index;
	
	bool origin_y_specified;
	double origin_x;
	double origin_y;
	double height;
	double width;
	
	bool invalid;
	bool dirty;
	
	void Layout ();
	void SetIndicesInternal (const char *in);
	
	void DownloaderComplete ();
	
	static void data_write (void *data, int32_t offset, int32_t n, void *closure);
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void size_notify (int64_t size, gpointer data);
	
 protected:
	virtual ~Glyphs ();

 public:
	static DependencyProperty *FillProperty;
	static DependencyProperty *FontRenderingEmSizeProperty;
	static DependencyProperty *FontUriProperty;
	static DependencyProperty *IndicesProperty;
	static DependencyProperty *OriginXProperty;
	static DependencyProperty *OriginYProperty;
	static DependencyProperty *StyleSimulationsProperty;
	static DependencyProperty *UnicodeStringProperty;
	
	Glyphs ();
	
	virtual Type::Kind GetObjectType () { return Type::GLYPHS; };
	
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual void Render (cairo_t *cr, int x, int y, int width, int height);
	virtual void ComputeBounds ();
	virtual Point GetTransformOrigin ();
	virtual Point GetOriginPoint ();
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	
	//
	// Property Accessors
	//
	void SetFill (Brush *fill);
	Brush *GetFill ();
	
	void SetFontRenderingEmSize (double size);
	double GetFontRenderingEmSize ();
	
	void SetFontUri (const char *uri);
	const char *GetFontUri ();
	
	void SetIndices (const char *indices);
	const char *GetIndices ();
	
	void SetOriginX (double origin);
	double GetOriginX ();
	
	void SetOriginY (double origin);
	double GetOriginY ();
	
	void SetStyleSimulations (StyleSimulations style);
	StyleSimulations GetStyleSimulations ();
	
	void SetUnicodeString (const char *unicode);
	const char *GetUnicodeString ();
};

Glyphs *glyphs_new (void);

Brush *glyphs_get_fill (Glyphs *glyphs);
void glyphs_set_fill (Glyphs *glyphs, Brush *fill);

double glyphs_get_font_rendering_em_size (Glyphs *glyphs);
void glyphs_set_font_rendering_em_size (Glyphs *glyphs, double size);

const char *glyphs_get_font_uri (Glyphs *glyphs);
void glyphs_set_font_uri (Glyphs *glyphs, const char *uri);

const char *glyphs_get_indices (Glyphs *glyphs);
void glyphs_set_indices (Glyphs *glyphs, const char *indices);

double glyphs_get_origin_x (Glyphs *glyphs);
void glyphs_set_origin_x (Glyphs *glyphs, double origin);

double glyphs_get_origin_y (Glyphs *glyphs);
void glyphs_set_origin_y (Glyphs *glyphs, double origin);

StyleSimulations glyphs_get_style_simulations (Glyphs *glyphs);
void glyphs_set_style_simulations (Glyphs *glyphs, StyleSimulations style);

const char *glyphs_get_unicode_string (Glyphs *glyphs);
void glyphs_set_unicode_string (Glyphs *glyphs, const char *unicode);

G_END_DECLS

#endif /* __TEXT_H__ */
