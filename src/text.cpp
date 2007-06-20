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

#include "cutil.h"
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


void
Inline::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type == Type::INLINE)
		NotifyAttacheesOfPropertyChange (prop);
	
	DependencyObject::OnPropertyChanged (prop);
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

Run::Run ()
{
	layout = NULL;
	
	foreground = NULL;
	
	/* initialize the font description */
	font = pango_font_description_new ();
	char *family = inline_get_font_family (this);
	pango_font_description_set_family (font, family);
	double size = inline_get_font_size (this);
	pango_font_description_set_size (font, (int) (size * PANGO_SCALE));
	FontStretches stretch = inline_get_font_stretch (this);
	pango_font_description_set_stretch (font, font_stretch (stretch));
	FontStyles style = inline_get_font_style (this);
	pango_font_description_set_style (font, font_style (style));
	FontWeights weight = inline_get_font_weight (this);
	pango_font_description_set_weight (font, font_weight (weight));
}

Run::~Run ()
{
	pango_font_description_free (font);
	
	if (layout != NULL)
		g_object_unref (layout);
	
	if (foreground != NULL) {
		foreground->Detach (NULL, this);
		foreground->unref ();
	}
}

void
Run::OnPropertyChanged (DependencyProperty *prop)
{
	bool font_changed = false;
	
	if (prop == Inline::FontFamilyProperty) {
		char *family = inline_get_font_family (this);
		pango_font_description_set_family (font, family);
		font_changed = true;
	} else if (prop == Inline::FontSizeProperty) {
		double size = inline_get_font_size (this);
		pango_font_description_set_size (font, (int) (size * PANGO_SCALE));
		font_changed = true;
	} else if (prop == Inline::FontStretchProperty) {
		FontStretches stretch = inline_get_font_stretch (this);
		pango_font_description_set_stretch (font, font_stretch (stretch));
		font_changed = true;
	} else if (prop == Inline::FontStyleProperty) {
		FontStyles style = inline_get_font_style (this);
		pango_font_description_set_style (font, font_style (style));
		font_changed = true;
	} else if (prop == Inline::FontWeightProperty) {
		FontWeights weight = inline_get_font_weight (this);
		pango_font_description_set_weight (font, font_weight (weight));
		font_changed = true;
	} else if (prop == Inline::ForegroundProperty) {
		if (foreground != NULL) {
			foreground->Detach (NULL, this);
			foreground->unref ();
		}
		
		if ((foreground = inline_get_foreground (this)) != NULL) {
			foreground->Attach (NULL, this);
			foreground->ref ();
		}
	} else if (prop == Run::TextProperty && layout != NULL) {
		char *text = run_get_text (this);
		pango_layout_set_text (layout, text ? text : "", -1);
	}
	
	if (font_changed && layout != NULL)
		pango_layout_set_font_description (layout, font);
	
	if (prop->type == Type::RUN)
		NotifyAttacheesOfPropertyChange (prop);
	
	Inline::OnPropertyChanged (prop);
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
	Brush *brush = default_foreground ();
	
	foreground = NULL;
	SetValue (TextBlock::ForegroundProperty, Value (brush));
	
	inlines = NULL;
	
	layout = NULL;
	
	height = -1;
	width = -1;
	
	/* initialize the font description */
	font = pango_font_description_new ();
	char *family = text_block_get_font_family (this);
	pango_font_description_set_family (font, family);
	double size = text_block_get_font_size (this);
	pango_font_description_set_size (font, (int) (size * PANGO_SCALE));
	FontStretches stretch = text_block_get_font_stretch (this);
	pango_font_description_set_stretch (font, font_stretch (stretch));
	FontStyles style = text_block_get_font_style (this);
	pango_font_description_set_style (font, font_style (style));
	FontWeights weight = text_block_get_font_weight (this);
	pango_font_description_set_weight (font, font_weight (weight));
}

TextBlock::~TextBlock ()
{
	pango_font_description_free (font);
	
	if (layout)
		g_object_unref (layout);
	
	if (inlines != NULL) {
		inlines->Detach (NULL, this);
		inlines->unref ();
	}
	
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
TextBlock::render (Surface *s, int x, int y, int width, int height)
{
	cairo_save (s->cairo);
	cairo_set_matrix (s->cairo, &absolute_xform);
	Draw (s, true, NULL, NULL);
	cairo_restore (s->cairo);
}

void 
TextBlock::getbounds ()
{
	Surface *s = item_get_surface (this);
	
	if (s == NULL)
		return;
	
	if (width == -1 || height == -1) {
		// need to recompute cached width/height for optimization
		cairo_save (s->cairo);
		cairo_identity_matrix (s->cairo);
		Draw (s, false, &width, &height);
		cairo_new_path (s->cairo);
		cairo_restore (s->cairo);
	}
	
	// optimization: use the cached width/height and draw
	// a simple rectangle to get bounding box
	cairo_save (s->cairo);
	cairo_set_matrix (s->cairo, &absolute_xform);
	cairo_set_line_width (s->cairo, 1);
	cairo_rectangle (s->cairo, 0, 0, width, height);
	cairo_stroke_extents (s->cairo, &x1, &y1, &x2, &y2);
	cairo_new_path (s->cairo);
	cairo_restore (s->cairo);
	
	text_block_set_actual_height (this, y2 - y1);
	text_block_set_actual_width (this, x2 - x1);
	
	// The extents are in the coordinates of the transform, translate to device coordinates
	x_cairo_matrix_transform_bounding_box (&absolute_xform, &x1, &y1, &x2, &y2);
}

Point
TextBlock::getxformorigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	Surface *s = item_get_surface (this);
	
	if (s == NULL)
		return Point (0.0, 0.0);
	
	if (width == -1 || height == -1) {
		cairo_save (s->cairo);
		cairo_identity_matrix (s->cairo);
		Draw (s, false, &width, &height);
		cairo_new_path (s->cairo);
		cairo_restore (s->cairo);
	}
	
	return Point (user_xform_origin.x * width, user_xform_origin.y * height);
}

bool
TextBlock::inside_object (Surface *s, double x, double y)
{
	bool ret = false;
	
	cairo_save (s->cairo);
	cairo_set_matrix (s->cairo, &absolute_xform);
	
	Draw (s, false, NULL, NULL);
	
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
TextBlock::get_size_for_brush (cairo_t *cr, double *width, double *height)
{
	Surface *s = item_get_surface (this);
	
	if (s != NULL && this->width == -1 || this->height == -1) {
		cairo_save (s->cairo);
		cairo_identity_matrix (s->cairo);
		Draw (s, false, &this->width, &this->height);
		cairo_new_path (s->cairo);
		cairo_restore (s->cairo);
	}
	
	*height = (double) this->height;
	*width = (double) this->width;
}

void
TextBlock::Draw (Surface *s, bool render, int *w, int *h)
{
	int full_width = 0, full_height = 0;
	char *text;
	
	text = text_block_get_text (this);
	
	if (layout == NULL) {
		layout = pango_cairo_create_layout (s->cairo);
		pango_layout_set_text (layout, text ? text : "", -1);
		pango_layout_set_font_description (layout, font);	
	} else {
		pango_cairo_update_layout (s->cairo, layout);
	}
	
	// FIXME: I wonder if inline Runs use their parent TextBlock
	// brush if they don't specify their own?
	//
	// If they do, we need to:
	//   1. default the inline foreground brush to NULL in text_init()
	//   2. keep a pointer to the TextBlock brush for the inlines loop
	//      to reuse.
	//
	// If they don't, we need to:
	//   1. probably change SetupBrush to take width/height args instead of
	//      calling uielement->get_size_for_brush() because for TextBlocks,
	//      that gets the entire size, including inlines - and we need more
	//      fine-grained control.
	//
	
	if (text && *text) {
		if (render) {
			foreground->SetupBrush (s->cairo, this);
			pango_cairo_show_layout (s->cairo, layout);
		} else
			pango_cairo_layout_path (s->cairo, layout);
		
		pango_layout_get_pixel_size (layout, &full_width, &full_height);
	}
	
	if (inlines != NULL) {
		Collection::Node *node = (Collection::Node *) inlines->list->First ();
		int width, height, x = 0, y = 0;
		bool newline = false;
		int line_height;
		Inline *item;
		Run *run;
		
		line_height = full_height;
		height = full_height;
		width = full_width;
		
		while (node != NULL) {
			item = (Inline *) node->obj;
			
			//
			// FIXME: This code won't work properly for RTL text
			//
			// There might also be a better way of doing this?
			//
			
			switch (item->GetObjectType ()) {
			case Type::RUN:
				run = (Run *) item;
				
				text = run_get_text (run);
				//printf ("<Run>%s</Run>\n", text ? text : "(null)");
				
				if (text == NULL || *text == '\0') {
					// optimization
					break;
				}
				
				x += width;
				//printf ("moving to (%d, %d)\n", x, y);
				cairo_move_to (s->cairo, x, y);
				
				if (run->layout == NULL) {
					run->layout = pango_cairo_create_layout (s->cairo);
					pango_layout_set_text (run->layout, text ? text : "", -1);
					pango_layout_set_font_description (run->layout, run->font);
				} else {
					pango_cairo_update_layout (s->cairo, run->layout);
				}
				
				if (render) {
					if (run->foreground != NULL)
						run->foreground->SetupBrush (s->cairo, this);
					else
						foreground->SetupBrush (s->cairo, this);
					
					pango_cairo_show_layout (s->cairo, run->layout);
				} else
					pango_cairo_layout_path (s->cairo, run->layout);
				
				pango_layout_get_pixel_size (run->layout, &width, &height);
				
				newline == newline || strchr (text, '\n');
				if (height > line_height || newline) {
					if (height > line_height)
						full_height += (height - line_height);
					line_height = height;
				}
				
				if ((x + width) > full_width)
					full_width = x + width;
				
				newline = false;
				
				break;
			case Type::LINEBREAK:
				//printf ("<LineBreak/>\n");
				y += line_height;
				//printf ("moving to (%d, %d)\n", x, y);
				cairo_move_to (s->cairo, 0, y);
				full_height += height;
				line_height = height;
				newline = true;
				width = 0;
				break;
			default:
				printf ("unknown Inline item\n");
				break;
			}
			
			node = (Collection::Node *) node->Next ();
		}
	}
	
	if (w)
		*w = full_width;
	
	if (h)
		*h = full_height;
}

void
TextBlock::OnPropertyChanged (DependencyProperty *prop)
{
	bool font_changed = false;
	
	if (prop->type != Type::TEXTBLOCK) {
		FrameworkElement::OnPropertyChanged (prop);
		return;
	}
	
	if (prop == TextBlock::ActualHeightProperty || prop == TextBlock::ActualWidthProperty)
		return;
	
	if (prop == TextBlock::FontFamilyProperty) {
		char *family = text_block_get_font_family (this);
		pango_font_description_set_family (font, family);
		font_changed = true;
	} else if (prop == TextBlock::FontSizeProperty) {
		double size = text_block_get_font_size (this);
		pango_font_description_set_size (font, (int) (size * PANGO_SCALE));
		font_changed = true;
	} else if (prop == TextBlock::FontStretchProperty) {
		FontStretches stretch = text_block_get_font_stretch (this);
		pango_font_description_set_stretch (font, font_stretch (stretch));
		font_changed = true;
	} else if (prop == TextBlock::FontStyleProperty) {
		FontStyles style = text_block_get_font_style (this);
		pango_font_description_set_style (font, font_style (style));
		font_changed = true;
	} else if (prop == TextBlock::FontWeightProperty) {
		FontWeights weight = text_block_get_font_weight (this);
		pango_font_description_set_weight (font, font_weight (weight));
		font_changed = true;
	} else if (prop == TextBlock::TextProperty && layout != NULL) {
		char *text = text_block_get_text (this);
		pango_layout_set_text (layout, text ? text : "", -1);
	} else if (prop == TextBlock::InlinesProperty) {
		if (inlines != NULL) {
			inlines->Detach (NULL, this);
			inlines->unref ();
		}
		
		if ((inlines = text_block_get_inlines (this)) != NULL) {
			inlines->Attach (NULL, this);
			inlines->ref ();
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
	
	if (font_changed && layout != NULL)
		pango_layout_set_font_description (layout, font);
	
	height = -1;
	width = -1;
	
	FullInvalidate (false);
}

void
TextBlock::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	FullInvalidate (false);
	
	FrameworkElement::OnSubPropertyChanged (prop, subprop);
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
	Inline::FontFamilyProperty = DependencyObject::Register (Type::INLINE, "FontFamily", new Value ("Lucida Sans"));
	Inline::FontSizeProperty = DependencyObject::Register (Type::INLINE, "FontSize", new Value (14.666));
	Inline::FontStretchProperty = DependencyObject::Register (Type::INLINE, "FontStretch", new Value (FontStretchesNormal));
	Inline::FontStyleProperty = DependencyObject::Register (Type::INLINE, "FontStyle", new Value (FontStylesNormal));
	Inline::FontWeightProperty = DependencyObject::Register (Type::INLINE, "FontWeight", new Value (FontWeightsNormal));
	Inline::ForegroundProperty = DependencyObject::Register (Type::INLINE, "Foreground", Type::BRUSH);
	Inline::TextDecorationsProperty = DependencyObject::Register (Type::INLINE, "TextDecorations", new Value (TextDecorationsNone));
	
	
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
