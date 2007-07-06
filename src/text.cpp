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
#include <cairo.h>

#include <string.h>

#include "runtime.h"
#include "color.h"
#include "text.h"


static PangoStretch
font_stretch (FontStretches stretch)
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
font_style (FontStyles style)
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
font_weight (FontWeights weight)
{
	// FontWeights and PangoWeight values map exactly
	
	if (weight > 900) {
		// FontWeighs have values between 100-999, Pango only allows 100-900
		return (PangoWeight) 900;
	}
	
	return (PangoWeight) weight;
}


static Brush *
default_foreground (void)
{
	SolidColorBrush *brush = new SolidColorBrush ();
	Color *color = color_from_str ("black");
	solid_color_brush_set_color (brush, color);
	delete color;
	
	return (Brush *) brush;
}


// Inline

DependencyProperty *Inline::FontFamilyProperty;
DependencyProperty *Inline::FontSizeProperty;
DependencyProperty *Inline::FontStretchProperty;
DependencyProperty *Inline::FontStyleProperty;
DependencyProperty *Inline::FontWeightProperty;
DependencyProperty *Inline::ForegroundProperty;
DependencyProperty *Inline::TextDecorationsProperty;

Inline::Inline ()
{
	foreground = NULL;
	
	/* initialize the font description */
	font = pango_font_description_new ();
}

Inline::~Inline ()
{
	pango_font_description_free (font);
	
	if (foreground != NULL) {
		foreground->Detach (NULL, this);
		foreground->unref ();
	}
}

void
Inline::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::INLINE) {
		DependencyObject::OnPropertyChanged (prop);
		return;
	}
    
	if (prop == Inline::FontFamilyProperty) {
		char *family = inline_get_font_family (this);
		pango_font_description_set_family (font, family);
	} else if (prop == Inline::FontSizeProperty) {
		double size = inline_get_font_size (this);
		pango_font_description_set_absolute_size (font, size * PANGO_SCALE);
	} else if (prop == Inline::FontStretchProperty) {
		FontStretches stretch = inline_get_font_stretch (this);
		pango_font_description_set_stretch (font, font_stretch (stretch));
	} else if (prop == Inline::FontStyleProperty) {
		FontStyles style = inline_get_font_style (this);
		pango_font_description_set_style (font, font_style (style));
	} else if (prop == Inline::FontWeightProperty) {
		FontWeights weight = inline_get_font_weight (this);
		pango_font_description_set_weight (font, font_weight (weight));
	} else if (prop == Inline::ForegroundProperty) {
		if (foreground != NULL) {
			foreground->Detach (NULL, this);
			foreground->unref ();
		}
		
		if ((foreground = inline_get_foreground (this)) != NULL) {
			foreground->Attach (NULL, this);
			foreground->ref ();
		}
	}
	
	NotifyAttacheesOfPropertyChange (prop);
}

void
Inline::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (prop == Inline::ForegroundProperty) {
		// this isn't exactly what we want, I don't
		// think... but it'll have to do.
		NotifyAttacheesOfPropertyChange (prop);
	}
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
	Value *value = inline_->GetValue (Inline::ForegroundProperty);
	
	return value ? (Brush *) value->AsBrush () : NULL;
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


// LineBreak

LineBreak *
line_break_new (void)
{
	return new LineBreak ();
}


// Run

DependencyProperty *Run::TextProperty;

void
Run::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::RUN) {
		Inline::OnPropertyChanged (prop);
		return;
	}
    
	NotifyAttacheesOfPropertyChange (prop);
}

Run *
run_new (void)
{
	return new Run ();
}

char *
run_get_text (Run *run)
{
	Value *value = run->GetValue (Run::TextProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
run_set_text (Run *run, char *value)
{
	run->SetValue (Run::TextProperty, Value (value));
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


TextBlock::TextBlock ()
{
	renderer = (MangoRenderer *) mango_renderer_new ();
	
	foreground = NULL;
	
	layout = NULL;
	
	actual_height = 0.0;
	actual_width = 0.0;
	
	/* initialize the font description */
	font = pango_font_description_new ();
	char *family = text_block_get_font_family (this);
	pango_font_description_set_family (font, family);
	double size = text_block_get_font_size (this);
	pango_font_description_set_absolute_size (font, size * PANGO_SCALE);
	FontStretches stretch = text_block_get_font_stretch (this);
	pango_font_description_set_stretch (font, font_stretch (stretch));
	FontStyles style = text_block_get_font_style (this);
	pango_font_description_set_style (font, font_style (style));
	FontWeights weight = text_block_get_font_weight (this);
	pango_font_description_set_weight (font, font_weight (weight));

	// this has to come last, since in our OnPropertyChanged
	// method we update our bounds.
	Brush *brush = default_foreground ();
	SetValue (TextBlock::ForegroundProperty, Value (brush));
}

TextBlock::~TextBlock ()
{
	pango_font_description_free (font);
	
	if (layout)
		g_object_unref (layout);
	
	g_object_unref (renderer);
	
	if (foreground != NULL) {
		foreground->Detach (NULL, this);
		foreground->unref ();
	}
}

void
TextBlock::SetFontSource (DependencyObject *downloader)
{
	;
}

void
TextBlock::Render (cairo_t *cr, int x, int y, int width, int height)
{
	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	Paint (cr);
	cairo_restore (cr);
}

void 
TextBlock::ComputeBounds ()
{
	cairo_t *cr = measuring_context_create ();
	double x1, y1, x2, y2;
	
	// optimization: use the cached width/height and draw
	// a simple rectangle to get bounding box
	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	cairo_set_line_width (cr, 1);
	cairo_rectangle (cr, 0, 0, actual_width, actual_height);
	// XXX this next call will hopefully become unnecessary in a
	// later version of cairo.
	cairo_identity_matrix (cr);
	cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
	cairo_new_path (cr);
	cairo_restore (cr);

	bounds = Rect (x1 - 1, y1 - 1, x2-x1 + 2, y2-y1 + 2);

	measuring_context_destroy (cr);
}

Point
TextBlock::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	
	return Point (user_xform_origin.x * actual_width, user_xform_origin.y * actual_height);
}

bool
TextBlock::InsideObject (cairo_t *cr, double x, double y)
{
	// FIXME: this code probably doesn't work
	cairo_matrix_t inverse = absolute_xform;
	bool ret = false;
	double nx = x;
	double ny = y;
	
	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	
	Layout (cr);
	
	cairo_matrix_invert (&inverse);
	cairo_matrix_transform_point (&inverse, &nx, &ny);
	
	if (cairo_in_stroke (cr, nx, ny) || cairo_in_fill (cr, nx, ny))
		ret = true;
	
	cairo_new_path (cr);
	
	cairo_restore (cr);
	
	return ret;
}

void
TextBlock::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	*height = actual_height;
	*width = actual_width;
}

void
TextBlock::CalcActualWidthHeight (cairo_t *cr)
{
	bool destroy = false;

	if (cr == NULL) {
		// FIXME: we need better width/height values here
// 		printf ("CalcActualWidthHeight called before surface available for TextBlock Text=\"%s\"\n",
// 			text_block_get_text (this));
		cr = measuring_context_create ();
		destroy = true;
	} else {
		cairo_save (cr);
	}
	
	cairo_identity_matrix (cr);
	
	Layout (cr);
	
	if (destroy) {
		measuring_context_destroy (cr);
	} else {
		cairo_new_path (cr);
		cairo_restore (cr);
	}
	
	text_block_set_actual_height (this, actual_height);
	text_block_set_actual_width (this, actual_width);
}

void
TextBlock::Layout (cairo_t *cr)
{
	PangoAttribute *uline_attr = NULL;
	PangoAttribute *font_attr = NULL;
	PangoAttribute *fg_attr = NULL;
	PangoAttribute *attr = NULL;
	TextDecorations decorations;
	double clip_height, width;
	PangoFontMask font_mask;
	PangoAttrList *attrs;
	size_t start, end;
	bool clip = false;
	GString *block;
	char *text;
	Brush *fg;
	int w, h;
	
	if (foreground == NULL) {
		fg = default_foreground ();
	} else {
		fg = foreground;
		fg->ref ();
	}
	
	if (layout == NULL)
		layout = pango_cairo_create_layout (cr);
	
	clip_height = framework_element_get_height (this);
	switch (text_block_get_text_wrapping (this)) {
	case TextWrappingWrap:
		// same as w/ Overflow except we clip height (if defined)
		if (clip_height > 0.0)
			clip = true;
	case TextWrappingWrapWithOverflow:
		pango_layout_set_wrap (layout, PANGO_WRAP_WORD_CHAR);
		
		width = framework_element_get_width (this);
		
		if (width > 0.0)
			pango_layout_set_width (layout, (int) width * PANGO_SCALE);
		else
			pango_layout_set_width (layout, -1);
		break;
	default:
		pango_layout_set_width (layout, -1);
		break;
	}
	
	block = g_string_new ("");
	attrs = pango_attr_list_new ();
	
	font_mask = pango_font_description_get_set_fields (font);
	decorations = text_block_get_text_decorations (this);
	text = text_block_get_text (this);
	if (text && *text) {
		g_string_append (block, text);
		end = block->len;
		start = 0;
		
		font_attr = pango_attr_font_desc_new (font);
		font_attr->start_index = start;
		font_attr->end_index = end;
		
		pango_attr_list_insert (attrs, font_attr);
		
		if (decorations == TextDecorationsUnderline) {
			uline_attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
			uline_attr->start_index = start;
			uline_attr->end_index = end;
			
			pango_attr_list_insert (attrs, uline_attr);
		} else {
			uline_attr = NULL;
		}
		
		fg_attr = mango_attr_foreground_new (this, fg);
		fg_attr->start_index = start;
		fg_attr->end_index = end;
		
		pango_attr_list_insert (attrs, fg_attr);
	}
	
	Inlines *inlines = text_block_get_inlines (this);
	
	if (inlines != NULL) {
		Collection::Node *node = (Collection::Node *) inlines->list->First ();
		PangoFontMask run_mask, inherited_mask;
		TextDecorations deco;
		Value *value;
		Inline *item;
		Run *run;
		
		while (node != NULL) {
			item = (Inline *) node->obj;
			
			switch (item->GetObjectType ()) {
			case Type::RUN:
				run = (Run *) item;
				
				text = run_get_text (run);
				
				if (text == NULL || *text == '\0') {
					// optimization
					goto loop;
				}
				
				start = block->len;
				g_string_append (block, text);
				end = block->len;
				break;
			case Type::LINEBREAK:
				start = block->len;
				g_string_append_c (block, '\n');
				end = block->len;
				break;
			default:
				goto loop;
				break;
			}
			
			// Inlines inherit their parent TextBlock's font properties if
			// they don't specify their own.
			run_mask = pango_font_description_get_set_fields (item->font);
			pango_font_description_merge (item->font, font, false);
			inherited_mask = (PangoFontMask) (font_mask & ~run_mask);
			
			attr = pango_attr_font_desc_new (item->font);
			attr->start_index = start;
			attr->end_index = end;
			
			if (!font_attr || !pango_attribute_equal ((const PangoAttribute *) font_attr, (const PangoAttribute *) attr)) {
				pango_attr_list_insert (attrs, attr);
				font_attr = attr;
			} else {
				pango_attribute_destroy (attr);
				font_attr->end_index = end;
			}
			
			if (inherited_mask != 0)
				pango_font_description_unset_fields (item->font, inherited_mask);
			
			// Inherit the TextDecorations from the parent TextBlock if unset
			value = item->GetValue (Inline::TextDecorationsProperty);
			deco = value ? (TextDecorations) value->AsInt32 () : decorations;
			if (deco == TextDecorationsUnderline) {
				if (uline_attr == NULL) {
					uline_attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
					uline_attr->start_index = start;
					uline_attr->end_index = end;
					
					pango_attr_list_insert (attrs, uline_attr);
				} else {
					uline_attr->end_index = end;
				}
			} else {
				uline_attr = NULL;
			}
			
			// Inlines also inherit their Foreground property from their parent
			// TextBlock if not set explicitly
			if (item->foreground)
				attr = mango_attr_foreground_new (this, item->foreground);
			else
				attr = mango_attr_foreground_new (this, fg);
			attr->start_index = start;
			attr->end_index = end;
			
			if (!fg_attr || !pango_attribute_equal ((const PangoAttribute *) fg_attr, (const PangoAttribute *) attr)) {
				pango_attr_list_insert (attrs, attr);
				fg_attr = attr;
			} else {
				pango_attribute_destroy (attr);
				fg_attr->end_index = end;
			}
			
			pango_attr_list_insert (attrs, fg_attr);
			
		loop:
			node = (Collection::Node *) node->Next ();
		}
	}
	
	// Now that we have our PangoAttrList setup, set it and the text on the PangoLayout
	pango_layout_set_text (layout, block->str, block->len);
	g_string_free (block, true);
	
	pango_layout_set_attributes (layout, attrs);
	
	pango_cairo_update_layout (cr, layout);
	mango_renderer_set_cairo_context (renderer, cr);
	mango_renderer_layout_path (renderer, layout);
	pango_layout_get_pixel_size (layout, &w, &h);
	
	actual_height = (double) h;
	actual_width = (double) w;
	
	if (clip && actual_height > clip_height)
		actual_height = clip_height;
}

void
TextBlock::Paint (cairo_t *cr)
{
	TextWrapping wrapping = text_block_get_text_wrapping (this);
	double h = framework_element_get_height (this);
	double w = framework_element_get_width (this);

	if (wrapping == TextWrappingWrapWithOverflow)
		h = actual_height;
	
	if (w > 0.0 && h > 0.0) {
		cairo_rectangle (cr, 0, 0, w, h);
		cairo_clip (cr);
	}
	
	pango_cairo_update_layout (cr, layout);
	mango_renderer_set_cairo_context (renderer, cr);
	mango_renderer_show_layout (renderer, layout);
}

void
TextBlock::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::TEXTBLOCK) {
		FrameworkElement::OnPropertyChanged (prop);
		return;
	}
	
	if (prop == TextBlock::FontFamilyProperty) {
		char *family = text_block_get_font_family (this);
		pango_font_description_set_family (font, family);
	} else if (prop == TextBlock::FontSizeProperty) {
		double size = text_block_get_font_size (this);
		pango_font_description_set_absolute_size (font, size * PANGO_SCALE);
	} else if (prop == TextBlock::FontStretchProperty) {
		FontStretches stretch = text_block_get_font_stretch (this);
		pango_font_description_set_stretch (font, font_stretch (stretch));
	} else if (prop == TextBlock::FontStyleProperty) {
		FontStyles style = text_block_get_font_style (this);
		pango_font_description_set_style (font, font_style (style));
	} else if (prop == TextBlock::FontWeightProperty) {
		FontWeights weight = text_block_get_font_weight (this);
		pango_font_description_set_weight (font, font_weight (weight));
	} else if (prop == TextBlock::TextProperty && layout != NULL) {
		char *text = text_block_get_text (this);
		pango_layout_set_text (layout, text ? text : "", -1);
	} else if (prop == TextBlock::InlinesProperty) {
		Inlines *newcol = GetValue (prop)->AsInlines();

		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	} else if (prop == TextBlock::ForegroundProperty) {
		if (foreground != NULL) {
			foreground->Detach (NULL, this);
			foreground->unref ();
		}
		
		if ((foreground = text_block_get_foreground (this)) != NULL) {
			foreground->Attach (NULL, this);
			foreground->ref ();
		}
	}

	if (prop != TextBlock::ActualHeightProperty && prop != TextBlock::ActualWidthProperty) {
		CalcActualWidthHeight (NULL);
		UpdateBounds (true);
	}

	NotifyAttacheesOfPropertyChange (prop);
}

void
TextBlock::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (prop == TextBlock::ForegroundProperty) {
		CalcActualWidthHeight (NULL);
		UpdateBounds (true);
	} else
		FrameworkElement::OnSubPropertyChanged (prop, subprop);
}

void
TextBlock::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	CalcActualWidthHeight (NULL);
	UpdateBounds (true);
}

TextBlock *
text_block_new (void)
{
	return new TextBlock ();
}

double
text_block_get_actual_height (TextBlock *textblock)
{
	return (double) textblock->GetValue (TextBlock::ActualHeightProperty)->AsDouble ();
}

void
text_block_set_actual_height (TextBlock *textblock, double value)
{
	textblock->SetValue (TextBlock::ActualHeightProperty, Value (value));
}

double
text_block_get_actual_width (TextBlock *textblock)
{
	return (double) textblock->GetValue (TextBlock::ActualWidthProperty)->AsDouble ();
}

void
text_block_set_actual_width (TextBlock *textblock, double value)
{
	textblock->SetValue (TextBlock::ActualWidthProperty, Value (value));
}

char *
text_block_get_font_family (TextBlock *textblock)
{
	Value *value = textblock->GetValue (TextBlock::FontFamilyProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
text_block_set_font_family (TextBlock *textblock, char *value)
{
	textblock->SetValue (TextBlock::FontFamilyProperty, Value (value));
}

double
text_block_get_font_size (TextBlock *textblock)
{
	return (double) textblock->GetValue (TextBlock::FontSizeProperty)->AsDouble ();
}

void
text_block_set_font_size (TextBlock *textblock, double value)
{
	textblock->SetValue (TextBlock::FontSizeProperty, Value (value));
}

FontStretches
text_block_get_font_stretch (TextBlock *textblock)
{
	return (FontStretches) textblock->GetValue (TextBlock::FontStretchProperty)->AsInt32 ();
}

void
text_block_set_font_stretch (TextBlock *textblock, FontStretches value)
{
	textblock->SetValue (TextBlock::FontStretchProperty, Value (value));
}

FontStyles
text_block_get_font_style (TextBlock *textblock)
{
	return (FontStyles) textblock->GetValue (TextBlock::FontStyleProperty)->AsInt32 ();
}

void
text_block_set_font_style (TextBlock *textblock, FontStyles value)
{
	textblock->SetValue (TextBlock::FontStyleProperty, Value (value));
}

FontWeights
text_block_get_font_weight (TextBlock *textblock)
{
	return (FontWeights) textblock->GetValue (TextBlock::FontWeightProperty)->AsInt32 ();
}

void
text_block_set_font_weight (TextBlock *textblock, FontWeights value)
{
	textblock->SetValue (TextBlock::FontWeightProperty, Value (value));
}

Brush *
text_block_get_foreground (TextBlock *textblock)
{
	Value *value = textblock->GetValue (TextBlock::ForegroundProperty);
	
	return value ? (Brush *) value->AsBrush () : NULL;
}

void
text_block_set_foreground (TextBlock *textblock, Brush *value)
{
	textblock->SetValue (TextBlock::ForegroundProperty, Value (value));
}

Inlines *
text_block_get_inlines (TextBlock *textblock)
{
	Value *value = textblock->GetValue (TextBlock::InlinesProperty);
	
	return value ? (Inlines *) value->AsInlines () : NULL;
}

void
text_block_set_inlines (TextBlock *textblock, Inlines *value)
{
	textblock->SetValue (TextBlock::InlinesProperty, Value (value));
}

char *
text_block_get_text (TextBlock *textblock)
{
	Value *value = textblock->GetValue (TextBlock::TextProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
text_block_set_text (TextBlock *textblock, char *value)
{
	textblock->SetValue (TextBlock::TextProperty, Value (value));
}

TextDecorations
text_block_get_text_decorations (TextBlock *textblock)
{
	return (TextDecorations) textblock->GetValue (TextBlock::TextDecorationsProperty)->AsInt32 ();
}

void
text_block_set_text_decorations (TextBlock *textblock, TextDecorations value)
{
	textblock->SetValue (TextBlock::TextDecorationsProperty, Value (value));
}

TextWrapping
text_block_get_text_wrapping (TextBlock *textblock)
{
	return (TextWrapping) textblock->GetValue (TextBlock::TextWrappingProperty)->AsInt32 ();
}

void
text_block_set_text_wrapping (TextBlock *textblock, TextWrapping value)
{
	textblock->SetValue (TextBlock::TextWrappingProperty, Value (value));
}

void
text_block_set_font_source (TextBlock *textblock, DependencyObject *Downloader)
{
	textblock->SetFontSource (Downloader);
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

void
Glyphs::Render (cairo_t *cr, int x, int y, int width, int height)
{
	// FIXME: implement me
}

void 
Glyphs::ComputeBounds ()
{
	// FIXME: implement me
	bounds = Rect (0, 0, 0, 0);
}

Point
Glyphs::GetTransformOrigin ()
{
	// FIXME: implement me
	return Point (0.0, 0.0);
}

void
Glyphs::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (prop == Glyphs::FillProperty) {
		printf ("Glyphs::FillProperty subproperty changed\n");
	}
	else
		FrameworkElement::OnSubPropertyChanged (prop, subprop);
}

void
Glyphs::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::GLYPHS) {
		FrameworkElement::OnPropertyChanged (prop);
		return;
	}
	
	if (prop == Glyphs::FillProperty) {
		printf ("Glyphs::Fill property changed\n");
	} else if (prop == Glyphs::FontRenderingEmSizeProperty) {
		double size = glyphs_get_font_rendering_em_size (this);
		printf ("Glyphs::FontRenderingEmSize property changed to %g\n", size);
	} else if (prop == Glyphs::FontUriProperty) {
		char *uri = glyphs_get_font_uri (this);
		printf ("Glyphs::FontUri property changed to %s\n", uri);
	} else if (prop == Glyphs::IndicesProperty) {
		char *indices = glyphs_get_indices (this);
		printf ("Glyphs::Indicies property changed to %s\n", indices);
	} else if (prop == Glyphs::OriginXProperty) {
		double x = glyphs_get_origin_x (this);
		printf ("Glyphs::OriginX property changed to %g\n", x);
	} else if (prop == Glyphs::OriginXProperty) {
		double y = glyphs_get_origin_y (this);
		printf ("Glyphs::OriginY property changed to %g\n", y);
	} else if (prop == Glyphs::StyleSimulationsProperty) {
		char *sims = glyphs_get_style_simulations (this);
		printf ("Glyphs::StyleSimulations property changed to %s\n", sims);
	} else if (prop == Glyphs::UnicodeStringProperty) {
		char *str = glyphs_get_unicode_string (this);
		printf ("Glyphs::UnicodeString property changed to %s\n", str);
	}
	
	FullInvalidate (false);

	NotifyAttacheesOfPropertyChange (prop);
}

Glyphs *
glyphs_new (void)
{
	return new Glyphs ();
}

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
	Inline::FontFamilyProperty = DependencyObject::Register (Type::INLINE, "FontFamily", Type::STRING);
	Inline::FontSizeProperty = DependencyObject::Register (Type::INLINE, "FontSize", Type::DOUBLE);
	Inline::FontStretchProperty = DependencyObject::Register (Type::INLINE, "FontStretch", Type::INT32);
	Inline::FontStyleProperty = DependencyObject::Register (Type::INLINE, "FontStyle", Type::INT32);
	Inline::FontWeightProperty = DependencyObject::Register (Type::INLINE, "FontWeight", Type::INT32);
	Inline::ForegroundProperty = DependencyObject::Register (Type::INLINE, "Foreground", Type::BRUSH);
	Inline::TextDecorationsProperty = DependencyObject::Register (Type::INLINE, "TextDecorations", Type::INT32);
	
	
	// Run
	Run::TextProperty = DependencyObject::Register (Type::RUN, "Text", Type::STRING);
	
	
	// TextBlock
	TextBlock::ActualHeightProperty = DependencyObject::Register (Type::TEXTBLOCK, "ActualHeight", Type::DOUBLE);
	TextBlock::ActualWidthProperty = DependencyObject::Register (Type::TEXTBLOCK, "ActualWidth", Type::DOUBLE);
	TextBlock::FontFamilyProperty = DependencyObject::Register (Type::TEXTBLOCK, "FontFamily", new Value ("Lucida Sans"));
	TextBlock::FontSizeProperty = DependencyObject::Register (Type::TEXTBLOCK, "FontSize", new Value (14.666));
	TextBlock::FontStretchProperty = DependencyObject::Register (Type::TEXTBLOCK, "FontStretch", new Value (FontStretchesNormal));
	TextBlock::FontStyleProperty = DependencyObject::Register (Type::TEXTBLOCK, "FontStyle", new Value (FontStylesNormal));
	TextBlock::FontWeightProperty = DependencyObject::Register (Type::TEXTBLOCK, "FontWeight", new Value (FontWeightsNormal));
	TextBlock::ForegroundProperty = DependencyObject::Register (Type::TEXTBLOCK, "Foreground", Type::BRUSH);
	TextBlock::InlinesProperty = DependencyObject::Register (Type::TEXTBLOCK, "Inlines", Type::INLINES);
	TextBlock::TextProperty = DependencyObject::Register (Type::TEXTBLOCK, "Text", Type::STRING);
	TextBlock::TextDecorationsProperty = DependencyObject::Register (Type::TEXTBLOCK, "TextDecorations", new Value (TextDecorationsNone));
	TextBlock::TextWrappingProperty = DependencyObject::Register (Type::TEXTBLOCK, "TextWrapping", new Value (TextWrappingNoWrap));
	
	
	// Glyphs
	Glyphs::FillProperty = DependencyObject::Register (Type::GLYPHS, "Fill", Type::BRUSH);
	Glyphs::FontRenderingEmSizeProperty = DependencyObject::Register (Type::GLYPHS, "FontRenderingEmSize", new Value (0.0));
	Glyphs::FontUriProperty = DependencyObject::Register (Type::GLYPHS, "FontUri", Type::STRING);
	Glyphs::IndicesProperty = DependencyObject::Register (Type::GLYPHS, "Indices", Type::STRING);
	Glyphs::OriginXProperty = DependencyObject::Register (Type::GLYPHS, "OriginX", new Value (0.0));
	Glyphs::OriginYProperty = DependencyObject::Register (Type::GLYPHS, "OriginY", new Value (0.0));
	Glyphs::StyleSimulationsProperty = DependencyObject::Register (Type::GLYPHS, "StyleSimulations", Type::STRING);
	Glyphs::UnicodeStringProperty = DependencyObject::Register (Type::GLYPHS, "UnicodeString", Type::STRING);
}
