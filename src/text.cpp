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

#include <string.h>

#include "text.h"


// Inline

DependencyProperty *Inline::FontFamilyProperty;
DependencyProperty *Inline::FontSizeProperty;
DependencyProperty *Inline::FontStrechProperty;
DependencyProperty *Inline::FontStyleProperty;
DependencyProperty *Inline::FontWeightProperty;
DependencyProperty *Inline::ForegroundProperty;
DependencyProperty *Inline::TextDecorationsProperty;



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
	return (char *) textblock->GetValue (TextBlock::FontFamilyProperty)->AsString ();
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
	return (char *) textblock->GetValue (TextBlock::TextProperty)->AsString ();
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
	return (char *) glyphs->GetValue (Glyphs::FontUriProperty)->AsString ();
}

void
glyphs_set_font_uri (Glyphs *glyphs, char *value)
{
	glyphs->SetValue (Glyphs::FontUriProperty, Value (value));
}

char *
glyphs_get_indices (Glyphs *glyphs)
{
	return (char *) glyphs->GetValue (Glyphs::IndicesProperty)->AsString ();
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
	return (char *) glyphs->GetValue (Glyphs::StyleSimulationsProperty)->AsString ();
}

void
glyphs_set_style_simulations (Glyphs *glyphs, char *value)
{
	glyphs->SetValue (Glyphs::StyleSimulationsProperty, Value (value));
}

char *
glyphs_get_unicode_string (Glyphs *glyphs)
{
	return (char *) glyphs->GetValue (Glyphs::UnicodeStringProperty)->AsString ();
}

void
glyphs_set_unicode_string (Glyphs *glyphs, char *value)
{
	glyphs->SetValue (Glyphs::UnicodeStringProperty, Value (value));
}
