#ifndef __TEXT_H__
#define __TEXT_H__

G_BEGIN_DECLS

#include <pango/pango.h>

#include "brush.h"
#include "runtime.h"

enum FontStretches {
	FontStretchesUltraCondensed = 1,
	FontStretchesExtraCondensed = 2,
	FontStretchesCondensed      = 3,
	FontStretchesSemiCondensed  = 4,
	FontStretchesNormal         = 5,
	FontStretchesMedium         = 5,
	FontStretchesSemiExpanded   = 6,
	FontStretchesExpanded       = 7,
	FontStretchesExtraExpanded  = 8,
	FontStretchesUltraExpanded  = 9
};

enum FontStyles {
	FontStylesNormal,
	FontStylesOblique,
	FontStylesItalic
};

enum FontWeights {
	FontWeightsThin       = 100,
	FontWeightsExtraLight = 200,
	FontWeightsUltraLight = 200,
	FontWeightsLight      = 300,
	FontWeightsNormal     = 400,
	FontWeightsRegular    = 400,
	FontWeightsMedium     = 500,
	FontWeightsSemiBold   = 600,
	FontWeightsDemiBold   = 600,
	FontWeightsBold       = 700,
	FontWeightsExtraBold  = 800,
	FontWeightsUltraBold  = 800,
	FontWeightsBlack      = 900,
	FontWeightsHeavy      = 900,
	FontWeightsExtraBlack = 950,
	FontWeightsUltraBlack = 950
};

enum TextDecorations {
	TextDecorationsNone,
	TextDecorationsUnderline
};

enum TextWrapping {
	TextWrappingWrap,
	TextWrappingNoWrap,
	TextWrappingWrapWithOverflow
};


void text_init (void);


class Inline : public DependencyObject {
 public:
	static DependencyProperty *FontFamilyProperty;
	static DependencyProperty *FontSizeProperty;
	static DependencyProperty *FontStretchProperty;
	static DependencyProperty *FontStyleProperty;
	static DependencyProperty *FontWeightProperty;
	static DependencyProperty *ForegroundProperty;
	static DependencyProperty *TextDecorationsProperty;
	
	Inline () { }
	virtual Value::Kind GetObjectType () { return Value::INLINE; }
	virtual void OnPropertyChanged (DependencyProperty *prop);
};

char *inline_get_font_family (Inline *inline_);
void inline_set_font_family (Inline *inline_, char *value);

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
public:
	LineBreak () { }
	virtual Value::Kind GetObjectType () { return Value::LINEBREAK; };
};

LineBreak *line_break_new (void);


class Run : public Inline {
public:
	static DependencyProperty *TextProperty;
	
	PangoFontDescription *font;
	PangoLayout *layout;
	
	Run ();
	~Run ();
	virtual Value::Kind GetObjectType () { return Value::RUN; };
	virtual void OnPropertyChanged (DependencyProperty *prop);
};

Run *run_new (void);

char *run_get_text (Run *run);
void run_set_text (Run *run, char *value);


class TextBlock : public FrameworkElement {
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
	~TextBlock ();
	virtual Value::Kind GetObjectType () { return Value::TEXTBLOCK; };
	
	void SetFontSource (DependencyObject *downloader);
	
	//
	// Overrides
	//
	virtual void render (Surface *s, int x, int y, int width, int height);
	virtual void get_size_for_brush (cairo_t *cr, double *width, double *height);
	virtual void getbounds ();
	virtual Point getxformorigin ();
	virtual bool inside_object (Surface *s, double x, double y);
	virtual void OnPropertyChanged (DependencyProperty *prop);
	
private:
	PangoFontDescription *font;
	PangoLayout *layout;
	int width, height;
	
	void Draw (Surface *s, bool render, int *w, int *h);
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
void text_block_set_text (TextBlock *textblock, char *value);

TextDecorations text_block_get_text_decorations (TextBlock *textblock);
void text_block_set_text_decorations (TextBlock *textblock, TextDecorations value);

TextWrapping text_block_get_text_wrapping (TextBlock *textblock);
void text_block_set_text_wrapping (TextBlock *textblock, TextWrapping value);


class Glyphs : public FrameworkElement {
public:
	static DependencyProperty *FillProperty;
	static DependencyProperty *FontRenderingEmSizeProperty;
	static DependencyProperty *FontUriProperty;
	static DependencyProperty *IndicesProperty;
	static DependencyProperty *OriginXProperty;
	static DependencyProperty *OriginYProperty;
	static DependencyProperty *StyleSimulationsProperty;
	static DependencyProperty *UnicodeStringProperty;
	
	Glyphs () { }
	virtual Value::Kind GetObjectType () { return Value::GLYPHS; };
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

char *glyphs_get_style_simulations (Glyphs *glyphs);
void glyphs_set_style_simulations (Glyphs *glyphs, char *value);

char *glyphs_get_unicode_string (Glyphs *glyphs);
void glyphs_set_unicode_string (Glyphs *glyphs, char *value);

G_END_DECLS

#endif /* __TEXT_H__ */
