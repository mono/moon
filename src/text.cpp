/*
 * text.cpp: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <pango/pango.h>
#include <cairo.h>

#include <string.h>

#include "cutil.h"
#include "text.h"


static PangoStretch
get_pango_stretch (FontStretches stretch)
{
	switch (stretch) {
	case FontStretchesUltraCondensed:
		return PANGO_STRETCH_ULTRA_CONDENSED;
	case FontStretchesExtraCondensed:
		return PANGO_STRETCH_EXTRA_CONDENSED;
	case FontStretchesCondensed:
		return PANGO_STRETCH_CONDENSED;
	case FontStretchesSemiCondensed:
		return PANGO_STRETCH_SEMI_CONDENSED;
	case FontStretchesNormal: // FontStretchesMedium (alias)
	default:
		return PANGO_STRETCH_NORMAL;
	case FontStretchesSemiExpanded:
		return PANGO_STRETCH_SEMI_EXPANDED;
	case FontStretchesExpanded:
		return PANGO_STRETCH_EXPANDED;
	case FontStretchesExtraExpanded:
		return PANGO_STRETCH_EXTRA_EXPANDED;
	case FontStretchesUltraExpanded:
		return PANGO_STRETCH_ULTRA_EXPANDED;
	}
}

static PangoStyle
get_pango_style (FontStyles style)
{
	switch (style) {
	case FontStylesNormal:
	default:
		return PANGO_STYLE_NORMAL;
	case FontStylesOblique:
		return PANGO_STYLE_OBLIQUE;
	case FontStylesItalic:
		return PANGO_STYLE_ITALIC;
	}
}

static PangoWeight
get_pango_weight (FontWeights weight)
{
	// FontWeights and PangoWeight values map exactly
	
	if (weight > 900) {
		// FontWeighs have values between 100-999, Pango only allows 100-900
		return (PangoWeight) 900;
	}
	
	return (PangoWeight) weight;
}



// Inline

DependencyProperty *Inline::FontFamilyProperty;
DependencyProperty *Inline::FontSizeProperty;
DependencyProperty *Inline::FontStretchProperty;
DependencyProperty *Inline::FontStyleProperty;
DependencyProperty *Inline::FontWeightProperty;
DependencyProperty *Inline::ForegroundProperty;
DependencyProperty *Inline::TextDecorationsProperty;

Inline *
inline_new (void)
{
	return new Inline ();
}

char *
inline_get_font_family (Inline *inline_)
{
	Value *value = inline_->GetValue (Inline::FontFamilyProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
inline_set_font_family (Inline *inline_, char *value)
{
	inline_->SetValue (Inline::FontFamilyProperty, Value (value));
}

double
inline_get_font_size (Inline *inline_)
{
	return (double) inline_->GetValue (Inline::FontSizeProperty)->AsDouble ();
}

void
inline_set_font_size (Inline *inline_, double value)
{
	inline_->SetValue (Inline::FontSizeProperty, Value (value));
}

FontStretches
inline_get_font_stretch (Inline *inline_)
{
	return (FontStretches) inline_->GetValue (Inline::FontStretchProperty)->AsInt32 ();
}

void
inline_set_font_stretch (Inline *inline_, FontStretches value)
{
	inline_->SetValue (Inline::FontStretchProperty, Value (value));
}

FontStyles
inline_get_font_style (Inline *inline_)
{
	return (FontStyles) inline_->GetValue (Inline::FontStyleProperty)->AsInt32 ();
}

void
inline_set_font_style (Inline *inline_, FontStyles value)
{
	inline_->SetValue (Inline::FontStyleProperty, Value (value));
}

FontWeights
inline_get_font_weight (Inline *inline_)
{
	return (FontWeights) inline_->GetValue (Inline::FontWeightProperty)->AsInt32 ();
}

void
inline_set_font_weight (Inline *inline_, FontWeights value)
{
	inline_->SetValue (Inline::FontWeightProperty, Value (value));
}

Brush *
inline_get_foreground (Inline *inline_)
{
	return (Brush *) inline_->GetValue (Inline::ForegroundProperty)->AsBrush ();
}

void
inline_set_foreground (Inline *inline_, Brush *value)
{
	inline_->SetValue (Inline::ForegroundProperty, Value (value));
}

TextDecorations
inline_get_text_decorations (Inline *inline_)
{
	return (TextDecorations) inline_->GetValue (Inline::TextDecorationsProperty)->AsInt32 ();
}

void
inline_set_text_decorations (Inline *inline_, TextDecorations value)
{
	inline_->SetValue (Inline::TextDecorationsProperty, Value (value));
}


// TextBlock

DependencyProperty *TextBlock::ActualHeightProperty;
DependencyProperty *TextBlock::ActualWidthProperty;
DependencyProperty *TextBlock::FontFamilyProperty;
DependencyProperty *TextBlock::FontSizeProperty;
DependencyProperty *TextBlock::FontStretchProperty;
DependencyProperty *TextBlock::FontStyleProperty;
DependencyProperty *TextBlock::FontWeightProperty;
DependencyProperty *TextBlock::ForegroundProperty;
DependencyProperty *TextBlock::InlinesProperty;
DependencyProperty *TextBlock::TextProperty;
DependencyProperty *TextBlock::TextDecorationsProperty;
DependencyProperty *TextBlock::TextWrappingProperty;

void
TextBlock::SetFontSource (DependencyObject *downloader)
{
	;
}

void
TextBlock::render (Surface *s, int x, int y, int width, int height)
{
	cairo_save (s->cairo);
	Draw (s, true);
	cairo_restore (s->cairo);
}

void 
TextBlock::getbounds ()
{
	Surface *s = item_get_surface (this);
	
	// not yet attached
	if (s == NULL)
		return;
	
	cairo_save (s->cairo);
	
	Draw (s, false);
	
	cairo_stroke_extents (s->cairo, &x1, &y1, &x2, &y2);
	cairo_new_path (s->cairo);
	cairo_restore (s->cairo);
	
	// The extents are in the coordinates of the transform, translate to device coordinates
	x_cairo_matrix_transform_bounding_box (&absolute_xform, &x1, &y1, &x2, &y2);
}

bool
TextBlock::inside_object (Surface *s, double x, double y)
{
	bool ret = false;
	
	cairo_save (s->cairo);
	
	Draw (s, false);
	
	double nx = x;
	double ny = y;
	
	cairo_matrix_t inverse = absolute_xform;
	cairo_matrix_invert (&inverse);
	
	cairo_matrix_transform_point (&inverse, &nx, &ny);
	
	if (cairo_in_stroke (s->cairo, nx, ny) || cairo_in_fill (s->cairo, nx, ny))
		ret = true;
	
	cairo_new_path (s->cairo);
	
	cairo_restore (s->cairo);
	
	return ret;
}

void
TextBlock::Draw (Surface *s, bool render)
{
	PangoFontDescription *font = NULL;
	PangoLayout *layout = NULL;
	char *family, *text;
	FontStretches stretch;
	FontWeights weight;
	FontStyles style;
	double size;
	
	cairo_set_matrix (s->cairo, &absolute_xform);
	
	if ((text = textblock_get_text (this)) != NULL) {
		if (!layout)
			layout = pango_cairo_create_layout (s->cairo);
		
		// FIXME: cache the PangoFontDescription
		font = pango_font_description_new ();
		
		if ((family = textblock_get_font_family (this)))
			pango_font_description_set_family (font, family);
		
		stretch = textblock_get_font_stretch (this);
		pango_font_description_set_stretch (font, get_pango_stretch (stretch));
		
		weight = textblock_get_font_weight (this);
		pango_font_description_set_weight (font, get_pango_weight (weight));
		
		style = textblock_get_font_style (this);
		pango_font_description_set_style (font, get_pango_style (style));
		
		size = textblock_get_font_size (this);
		pango_font_description_set_absolute_size (font, size);
		
		pango_layout_set_font_description (layout, font);
		pango_layout_set_text (layout, text, -1);
		
		//pango_cairo_update_layout (cr, layout);
		pango_cairo_show_layout (s->cairo, layout);
	}
}

TextBlock *
textblock_new (void)
{
	return new TextBlock ();
}

double
textblock_get_actual_height (TextBlock *textblock)
{
	return (double) textblock->GetValue (TextBlock::ActualHeightProperty)->AsDouble ();
}

void
textblock_set_actual_height (TextBlock *textblock, double value)
{
	textblock->SetValue (TextBlock::ActualHeightProperty, Value (value));
}

double
textblock_get_actual_width (TextBlock *textblock)
{
	return (double) textblock->GetValue (TextBlock::ActualWidthProperty)->AsDouble ();
}

void
textblock_set_actual_width (TextBlock *textblock, double value)
{
	textblock->SetValue (TextBlock::ActualWidthProperty, Value (value));
}

char *
textblock_get_font_family (TextBlock *textblock)
{
	Value *value = textblock->GetValue (TextBlock::FontFamilyProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
textblock_set_font_family (TextBlock *textblock, char *value)
{
	textblock->SetValue (TextBlock::FontFamilyProperty, Value (value));
}

double
textblock_get_font_size (TextBlock *textblock)
{
	return (double) textblock->GetValue (TextBlock::FontSizeProperty)->AsDouble ();
}

void
textblock_set_font_size (TextBlock *textblock, double value)
{
	textblock->SetValue (TextBlock::FontSizeProperty, Value (value));
}

FontStretches
textblock_get_font_stretch (TextBlock *textblock)
{
	return (FontStretches) textblock->GetValue (TextBlock::FontStretchProperty)->AsInt32 ();
}

void
textblock_set_font_stretch (TextBlock *textblock, FontStretches value)
{
	textblock->SetValue (TextBlock::FontStretchProperty, Value (value));
}

FontStyles
textblock_get_font_style (TextBlock *textblock)
{
	return (FontStyles) textblock->GetValue (TextBlock::FontStyleProperty)->AsInt32 ();
}

void
textblock_set_font_style (TextBlock *textblock, FontStyles value)
{
	textblock->SetValue (TextBlock::FontStyleProperty, Value (value));
}

FontWeights
textblock_get_font_weight (TextBlock *textblock)
{
	return (FontWeights) textblock->GetValue (TextBlock::FontWeightProperty)->AsInt32 ();
}

void
textblock_set_font_weight (TextBlock *textblock, FontWeights value)
{
	textblock->SetValue (TextBlock::FontWeightProperty, Value (value));
}

Brush *
textblock_get_foreground (TextBlock *textblock)
{
	return (Brush *) textblock->GetValue (TextBlock::ForegroundProperty)->AsBrush ();
}

void
textblock_set_foreground (TextBlock *textblock, Brush *value)
{
	textblock->SetValue (TextBlock::ForegroundProperty, Value (value));
}

Inlines *
textblock_get_inlines (TextBlock *textblock)
{
	return (Inlines *) textblock->GetValue (TextBlock::InlinesProperty)->AsInlines ();
}

void
textblock_set_inlines (TextBlock *textblock, Inlines *value)
{
	textblock->SetValue (TextBlock::InlinesProperty, Value (value));
}

char *
textblock_get_text (TextBlock *textblock)
{
	Value *value = textblock->GetValue (TextBlock::TextProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
textblock_set_text (TextBlock *textblock, char *value)
{
	textblock->SetValue (TextBlock::TextProperty, Value (value));
}

TextDecorations
textblock_get_text_decorations (TextBlock *textblock)
{
	return (TextDecorations) textblock->GetValue (TextBlock::TextDecorationsProperty)->AsInt32 ();
}

void
textblock_set_text_decorations (TextBlock *textblock, TextDecorations value)
{
	textblock->SetValue (TextBlock::TextDecorationsProperty, Value (value));
}

TextWrapping
textblock_get_text_wrapping (TextBlock *textblock)
{
	return (TextWrapping) textblock->GetValue (TextBlock::TextWrappingProperty)->AsInt32 ();
}

void
textblock_set_text_wrapping (TextBlock *textblock, TextWrapping value)
{
	textblock->SetValue (TextBlock::TextWrappingProperty, Value (value));
}


// Glyphs

DependencyProperty *Glyphs::FillProperty;
DependencyProperty *Glyphs::FontRenderingEmSizeProperty;
DependencyProperty *Glyphs::FontUriProperty;
DependencyProperty *Glyphs::IndicesProperty;
DependencyProperty *Glyphs::OriginXProperty;
DependencyProperty *Glyphs::OriginYProperty;
DependencyProperty *Glyphs::StyleSimulationsProperty;
DependencyProperty *Glyphs::UnicodeStringProperty;

Brush *
glyphs_get_fill (Glyphs *glyphs)
{
	return (Brush *) glyphs->GetValue (Glyphs::FillProperty)->AsBrush ();
}

void
glyphs_set_fill (Glyphs *glyphs, Brush *value)
{
	glyphs->SetValue (Glyphs::FillProperty, Value (value));
}

double
glyphs_get_font_rendering_em_size (Glyphs *glyphs)
{
	return (double) glyphs->GetValue (Glyphs::FontRenderingEmSizeProperty)->AsDouble ();
}

void
glyphs_set_font_rendering_em_size (Glyphs *glyphs, double value)
{
	glyphs->SetValue (Glyphs::FontRenderingEmSizeProperty, Value (value));
}

char *
glyphs_get_font_uri (Glyphs *glyphs)
{
	Value *value = glyphs->GetValue (Glyphs::FontUriProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
glyphs_set_font_uri (Glyphs *glyphs, char *value)
{
	glyphs->SetValue (Glyphs::FontUriProperty, Value (value));
}

char *
glyphs_get_indices (Glyphs *glyphs)
{
	Value *value = glyphs->GetValue (Glyphs::IndicesProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
glyphs_set_indices (Glyphs *glyphs, char *value)
{
	glyphs->SetValue (Glyphs::IndicesProperty, Value (value));
}

double
glyphs_get_origin_x (Glyphs *glyphs)
{
	return (double) glyphs->GetValue (Glyphs::OriginXProperty)->AsDouble ();
}

void
glyphs_set_origin_x (Glyphs *glyphs, double value)
{
	glyphs->SetValue (Glyphs::OriginXProperty, Value (value));
}

double
glyphs_get_origin_y (Glyphs *glyphs)
{
	return (double) glyphs->GetValue (Glyphs::OriginYProperty)->AsDouble ();
}

void
glyphs_set_origin_y (Glyphs *glyphs, double value)
{
	glyphs->SetValue (Glyphs::OriginYProperty, Value (value));
}

char *
glyphs_get_style_simulations (Glyphs *glyphs)
{
	Value *value = glyphs->GetValue (Glyphs::StyleSimulationsProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
glyphs_set_style_simulations (Glyphs *glyphs, char *value)
{
	glyphs->SetValue (Glyphs::StyleSimulationsProperty, Value (value));
}

char *
glyphs_get_unicode_string (Glyphs *glyphs)
{
	Value *value = glyphs->GetValue (Glyphs::UnicodeStringProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
glyphs_set_unicode_string (Glyphs *glyphs, char *value)
{
	glyphs->SetValue (Glyphs::UnicodeStringProperty, Value (value));
}



void
text_init (void)
{
	// Inline
	Inline::FontFamilyProperty = DependencyObject::Register (Value::INLINE, "FontFamily", Value::STRING);
	Inline::FontSizeProperty = DependencyObject::Register (Value::INLINE, "FontSize", new Value (12.0));
	Inline::FontStretchProperty = DependencyObject::Register (Value::INLINE, "FontStretch", new Value (FontStretchesNormal));
	Inline::FontStyleProperty = DependencyObject::Register (Value::INLINE, "FontStyle", new Value (FontStylesNormal));
	Inline::FontWeightProperty = DependencyObject::Register (Value::INLINE, "FontWeight", new Value (FontWeightsNormal));
	Inline::ForegroundProperty = DependencyObject::Register (Value::INLINE, "Foreground", Value::BRUSH);
	Inline::TextDecorationsProperty = DependencyObject::Register (Value::INLINE, "TextDecorations", new Value (TextDecorationsNone));
	
	// TextBlock
	TextBlock::ActualHeightProperty = DependencyObject::Register (Value::TEXTBLOCK, "ActualHeight", Value::DOUBLE);
	TextBlock::ActualWidthProperty = DependencyObject::Register (Value::TEXTBLOCK, "ActualWidth", Value::DOUBLE);
	TextBlock::FontFamilyProperty = DependencyObject::Register (Value::TEXTBLOCK, "FontFamily", Value::STRING);
	TextBlock::FontSizeProperty = DependencyObject::Register (Value::TEXTBLOCK, "FontSize", new Value (12.0));
	TextBlock::FontStretchProperty = DependencyObject::Register (Value::TEXTBLOCK, "FontStretch", new Value (FontStretchesNormal));
	TextBlock::FontStyleProperty = DependencyObject::Register (Value::TEXTBLOCK, "FontStyle", new Value (FontStylesNormal));
	TextBlock::FontWeightProperty = DependencyObject::Register (Value::TEXTBLOCK, "FontWeight", new Value (FontWeightsNormal));
	TextBlock::ForegroundProperty = DependencyObject::Register (Value::TEXTBLOCK, "Foreground", Value::BRUSH);
	TextBlock::InlinesProperty = DependencyObject::Register (Value::TEXTBLOCK, "Inlines", Value::INLINES);
	TextBlock::TextProperty = DependencyObject::Register (Value::TEXTBLOCK, "Text", Value::STRING);
	TextBlock::TextDecorationsProperty = DependencyObject::Register (Value::TEXTBLOCK, "TextDecorations", new Value (TextDecorationsNone));
	TextBlock::TextWrappingProperty = DependencyObject::Register (Value::TEXTBLOCK, "TextWrapping", new Value (TextWrappingNoWrap));
	
	// Glyphs
	Glyphs::FillProperty = DependencyObject::Register (Value::GLYPHS, "Fill", Value::BRUSH);
	Glyphs::FontRenderingEmSizeProperty = DependencyObject::Register (Value::GLYPHS, "FontRenderingEmSize", Value::DOUBLE);
	Glyphs::FontUriProperty = DependencyObject::Register (Value::GLYPHS, "FontUri", Value::STRING);
	Glyphs::IndicesProperty = DependencyObject::Register (Value::GLYPHS, "Indices", Value::STRING);
	Glyphs::OriginXProperty = DependencyObject::Register (Value::GLYPHS, "OriginX", Value::DOUBLE);
	Glyphs::OriginYProperty = DependencyObject::Register (Value::GLYPHS, "OriginY", Value::DOUBLE);
	Glyphs::StyleSimulationsProperty = DependencyObject::Register (Value::GLYPHS, "StyleSimulations", Value::STRING);
	Glyphs::UnicodeStringProperty = DependencyObject::Register (Value::GLYPHS, "UnicodeString", Value::STRING);
}
