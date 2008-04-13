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
#include <pango/pango.h>

#include "downloader.h"
#include "moon-path.h"
#include "layout.h"
#include "brush.h"
#include "mango.h"
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
	
	union {
		PangoFontDescription *pango;
		TextFontDescription *custom;
	} font;

	Brush *foreground;
	bool autogen;
	
	Inline ();
	virtual Type::Kind GetObjectType () { return Type::INLINE; }
	virtual Value *GetDefaultValue (DependencyProperty *prop);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
};

char *inline_get_font_family (Inline *inline_);
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
};

Run *run_new (void);

char *run_get_text (Run *run);
void run_set_text (Run *run, const char *value);


/* @ContentProperty="Inlines" */
class TextBlock : public FrameworkElement {
	union {
		PangoFontDescription *pango;
		TextFontDescription *custom;
	} font;
	union {
		PangoLayout *pango;
		TextLayout *custom;
	} layout;
	MangoRenderer *renderer;
	Downloader *downloader;
	
	double actual_height;
	double actual_width;
	double bbox_height;
	double bbox_width;
	bool setvalue;
	bool dirty;
	
	void CalcActualWidthHeight (cairo_t *cr);
	void Layout (cairo_t *cr);
	void Paint (cairo_t *cr);
	
	char *GetText ();
	bool SetText (const char *text);
	
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
	
	void LayoutPango (cairo_t *cr);
	void LayoutSilverlight (cairo_t *cr);
	
	void DownloaderComplete ();
	
	static void data_write (void *data, int32_t offset, int32_t n, void *closure);
	static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void size_notify (int64_t size, gpointer data);
	
protected:
	virtual ~TextBlock();

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
};

TextBlock *text_block_new (void);

double text_block_get_actual_height (TextBlock *textblock);
void text_block_set_actual_height (TextBlock *textblock, double value);

double text_block_get_actual_width (TextBlock *textblock);
void text_block_set_actual_width (TextBlock *textblock, double value);

char *text_block_get_font_family (TextBlock *textblock);
void text_block_set_font_family (TextBlock *textblock, char *value);

double text_block_get_font_size (TextBlock *textblock);
void text_block_set_font_size (TextBlock *textblock, double value);

FontStretches text_block_get_font_stretch (TextBlock *textblock);
void text_block_set_font_stretch (TextBlock *textblock, FontStretches value);

FontStyles text_block_get_font_style (TextBlock *textblock);
void text_block_set_font_style (TextBlock *textblock, FontStyles value);

FontWeights text_block_get_font_weight (TextBlock *textblock);
void text_block_set_font_weight (TextBlock *textblock, FontWeights value);

Brush *text_block_get_foreground (TextBlock *textblock);
void text_block_set_foreground (TextBlock *textblock, Brush *value);

Inlines *text_block_get_inlines (TextBlock *textblock);
void text_block_set_inlines (TextBlock *textblock, Inlines *value);

char *text_block_get_text (TextBlock *textblock);
void text_block_set_text (TextBlock *textblock, const char *value);

TextDecorations text_block_get_text_decorations (TextBlock *textblock);
void text_block_set_text_decorations (TextBlock *textblock, TextDecorations value);

TextWrapping text_block_get_text_wrapping (TextBlock *textblock);
void text_block_set_text_wrapping (TextBlock *textblock, TextWrapping value);

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
	void SetIndices (const char *in);
	
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
};

Glyphs *glyphs_new (void);

Brush *glyphs_get_fill (Glyphs *glyphs);
void glyphs_set_fill (Glyphs *glyphs, Brush *value);

double glyphs_get_font_rendering_em_size (Glyphs *glyphs);
void glyphs_set_font_rendering_em_size (Glyphs *glyphs, double value);

char *glyphs_get_font_uri (Glyphs *glyphs);
void glyphs_set_font_uri (Glyphs *glyphs, char *value);

char *glyphs_get_indices (Glyphs *glyphs);
void glyphs_set_indices (Glyphs *glyphs, char *value);

double glyphs_get_origin_x (Glyphs *glyphs);
void glyphs_set_origin_x (Glyphs *glyphs, double value);

double glyphs_get_origin_y (Glyphs *glyphs);
void glyphs_set_origin_y (Glyphs *glyphs, double value);

StyleSimulations glyphs_get_style_simulations (Glyphs *glyphs);
void glyphs_set_style_simulations (Glyphs *glyphs, StyleSimulations value);

char *glyphs_get_unicode_string (Glyphs *glyphs);
void glyphs_set_unicode_string (Glyphs *glyphs, char *value);

G_END_DECLS

#endif /* __TEXT_H__ */
