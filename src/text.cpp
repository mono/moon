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
	
	text_height = -1.0;
	text_width = -1.0;
	text_dir = 0;
	
	/* initialize the font description */
	font = pango_font_description_new ();
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
		pango_font_description_set_absolute_size (font, size * PANGO_SCALE);
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
		text_height = -1.0;
		text_width = -1.0;
		text_dir = 0;
	}
	
	if (font_changed) {
		if (layout != NULL)
			pango_layout_set_font_description (layout, font);
		
		text_height = -1.0;
		text_width = -1.0;
	}
	
	if (prop->type == Type::RUN)
		NotifyAttacheesOfPropertyChange (prop);
	
	// this will notify attachees of font property changes
	Inline::OnPropertyChanged (prop);
}

void
Run::SetValue (DependencyProperty *prop, Value *value)
{
	if (prop == Run::TextProperty && value != NULL) {
		// Note: Any leading or trailing whitespace is not
		// preserved when setting the Text property.
		g_strstrip (value->AsString ());
	}
	
	Inline::SetValue (prop, value);
}

void
Run::SetValue (DependencyProperty *prop, Value value)
{
	Inline::SetValue (prop, value);
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
	
	block_height = -1.0;
	block_width = -1.0;
	
	text_height = -1.0;
	text_width = -1.0;
	
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
	Paint (s->cairo);
	cairo_restore (s->cairo);
}

void 
TextBlock::getbounds ()
{
	Surface *s = item_get_surface (this);
	
	if (s == NULL)
		return;
	
	if (block_width < 0.0)
		CalcActualWidthHeight (s->cairo);
	
	// optimization: use the cached width/height and draw
	// a simple rectangle to get bounding box
	cairo_save (s->cairo);
	cairo_set_matrix (s->cairo, &absolute_xform);
	cairo_set_line_width (s->cairo, 1);
	cairo_rectangle (s->cairo, 0, 0, block_width, block_height);
	cairo_stroke_extents (s->cairo, &x1, &y1, &x2, &y2);
	cairo_new_path (s->cairo);
	cairo_restore (s->cairo);
	
	//text_block_set_actual_height (this, y2 - y1);
	//text_block_set_actual_width (this, x2 - x1);
	
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
	
	if (block_width < 0.0)
		CalcActualWidthHeight (s->cairo);
	
	return Point (user_xform_origin.x * block_width, user_xform_origin.y * block_height);
}

bool
TextBlock::inside_object (Surface *s, double x, double y)
{
	// FIXME: this code probably doesn't work
	cairo_matrix_t inverse = absolute_xform;
	bool ret = false;
	double nx = x;
	double ny = y;
	
	cairo_save (s->cairo);
	cairo_set_matrix (s->cairo, &absolute_xform);
	
	Layout (s->cairo);
	
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
	if (block_width < 0.0)
		CalcActualWidthHeight (cr);
	
	*height = block_height;
	*width = block_width;
}

void
TextBlock::CalcActualWidthHeight (cairo_t *cr)
{
	cairo_surface_t *surface = NULL;
	Collection::Node *node;
	bool destroy = false;
	
	if (cr == NULL) {
		// FIXME: we need better width/height values here
		printf ("CalcWidthHeight called before surface available for TextBlock with Text=\"%s\"\n", text_block_get_text (this));
		surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1280, 1024);
		cr = cairo_create (surface);
		destroy = true;
	} else {
		cairo_save (cr);
	}
	
	cairo_identity_matrix (cr);
	
	Layout (cr);
	
	if (destroy) {
		if (inlines != NULL && (node = (Collection::Node *) inlines->list->First ())) {
			do {
				if (node->obj->GetObjectType () == Type::RUN) {
					g_object_unref (((Run *) node->obj)->layout);
					((Run *) node->obj)->layout = NULL;
				}
				node = (Collection::Node *) node->Next ();
			} while (node != NULL);
		}
		
		g_object_unref (layout);
		layout = NULL;
		
		//cairo_surface_destroy (surface);
		cairo_destroy (cr);
	} else {
		cairo_new_path (cr);
		cairo_restore (cr);
	}
	
	text_block_set_actual_height (this, block_height);
	text_block_set_actual_width (this, block_width);
}

Value *
TextBlock::GetValue (DependencyProperty *prop)
{
	if ((prop == TextBlock::ActualWidthProperty ||
	     prop == TextBlock::ActualHeightProperty) && block_width < 0.0) {
		Surface *s = item_get_surface (this);
		printf ("GetValue for actual width/height requested before calculated\n");
		CalcActualWidthHeight (s ? s->cairo : NULL);
	}
	
	return FrameworkElement::GetValue (prop);
}

void
TextBlock::SetValue (DependencyProperty *prop, Value *value)
{
	if (prop == TextBlock::TextProperty && value != NULL) {
		// Note: Any leading or trailing whitespace is not
		// preserved when setting the Text property.
		g_strstrip (value->AsString ());
	}
	
	FrameworkElement::SetValue (prop, value);
}

void
TextBlock::SetValue (DependencyProperty *prop, Value value)
{
	FrameworkElement::SetValue (prop, value);
}

void
TextBlock::Layout (cairo_t *cr)
{
	double width, height, x = 0, y = 0;
	PangoFontMask font_mask;
	char *text;
	int w, h;
	
	text = text_block_get_text (this);
	
	if (layout == NULL) {
		layout = pango_cairo_create_layout (cr);
		pango_layout_set_text (layout, text ? text : "", -1);
		pango_layout_set_font_description (layout, font);	
	} else {
		pango_cairo_update_layout (cr, layout);
	}
	
	font_mask = pango_font_description_get_set_fields (font);
	
	if (text && *text) {
		pango_cairo_layout_path (cr, layout);
		pango_layout_get_pixel_size (layout, &w, &h);
		block_height = (double) h;
		block_width = (double) w;
		text_height = (double) h;
		text_width = (double) w;
		text_dir = 1; // FIXME
	} else {
		block_height = 0.0;
		block_width = 0.0;
		text_height = 0.0;
		text_width = 0.0;
		text_dir = 0;
	}
	
	// Note: we don't worry about alignment here because all we
	// really care about is width/height values for each Run
	// element and the overall width/height of each line (for
	// calculating block width/height).
	
	// FIXME: we need to take the TextWrapping property into
	// consideration... and possibly embedded \n's? Ugh, this will
	// be a bitch.
	
	if (inlines != NULL) {
		Collection::Node *node = (Collection::Node *) inlines->list->First ();
		PangoFontMask run_mask, inherited_mask;
		PangoFontMask inherited;
		bool newline = false;
		double line_height;
		Inline *item;
		Run *run;
		
		line_height = text_height;
		height = text_height;
		width = text_width;
		
		while (node != NULL) {
			item = (Inline *) node->obj;
			
			// FIXME: Figure out RTL vs LTR direction for each run and
			// cache it to make calculating offsets in ::Paint easier.
			
			switch (item->GetObjectType ()) {
			case Type::RUN:
				run = (Run *) item;
				
				text = run_get_text (run);
				//printf ("<Run>%s</Run>\n", text ? text : "(null)");
				
				if (text == NULL || *text == '\0') {
					// optimization
					run->text_height = 0;
					run->text_width = 0;
					run->text_dir = 1;
					break;
				}
				
				x += width;
				//printf ("moving to (%d, %d)\n", x, y);
				cairo_move_to (cr, x, y);
				
				if (run->layout == NULL) {
					run->layout = pango_cairo_create_layout (cr);
					pango_layout_set_text (run->layout, text ? text : "", -1);
				} else {
					pango_cairo_update_layout (cr, run->layout);
				}
				
				// Runs share their parent TextBlock's font properties if
				// they don't specify their own.
				run_mask = pango_font_description_get_set_fields (run->font);
				pango_font_description_merge (run->font, font, false);
				inherited_mask = (PangoFontMask) (font_mask & ~run_mask);
				
				if (inherited_mask != 0) {
					pango_layout_set_font_description (run->layout, run->font);
					pango_font_description_unset_fields (run->font, inherited_mask);
				}
				
				pango_cairo_layout_path (cr, run->layout);
				
				pango_layout_get_pixel_size (run->layout, &w, &h);
				run->text_height = (double) h;
				run->text_width = (double) w;
				run->text_dir = 1; // FIXME
				
				if (run->text_height > line_height || newline) {
					if (run->text_height > line_height)
						block_height += (run->text_height - line_height);
					line_height = run->text_height;
				}
				
				if ((x + run->text_width) > block_width)
					block_width = x + run->text_width;
				
				height = run->text_height;
				width = run->text_width;
				newline = false;
				
				break;
			case Type::LINEBREAK:
				//printf ("<LineBreak/>\n");
				y += line_height;
				block_height += height;
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
}

static void
line_width_height (Collection::Node *iNode, double *width, double *height)
{
	double w = 0, h = 0;
	Inline *item;
	
	while (iNode != NULL) {
		item = (Inline *) iNode->obj;
		if (item->GetObjectType () == Type::LINEBREAK)
			break;
		
		if (((Run *) item)->text_height > h)
			h = ((Run *) item)->text_height;
		
		w += ((Run *) item)->text_width;
		
		iNode = (Collection::Node *) iNode->Next ();
	}
	
	*height = h;
	*width = w;
}

void
TextBlock::Paint (cairo_t *cr)
{
	Collection::Node *node = inlines ? (Collection::Node *) inlines->list->First () : NULL;
	double run_height, dx, dy, x, y = 0;
        double line_width, line_height;
	PangoFontMask font_mask;
	int8_t dir;
	char *text;
	Brush *fg;
	
	if (block_width < 0.0)
		CalcActualWidthHeight (cr);
	
	font_mask = pango_font_description_get_set_fields (font);
	
	if (foreground == NULL) {
		fg = default_foreground ();
	} else {
		fg = foreground;
		fg->ref ();
	}
	
	line_width_height (node, &line_width, &line_height);
	
	text = text_block_get_text (this);
	
	if (layout == NULL) {
		layout = pango_cairo_create_layout (cr);
		pango_layout_set_text (layout, text ? text : "", -1);
		pango_layout_set_font_description (layout, font);	
	}
	
	if (text && *text) {
		if (text_height > line_height)
			line_height = text_height;
		
		line_width += text_width;
		
		// dir keeps track of overall line direction
		dir = text_dir;
		
		// run_height keeps track of the previous run's height (for newline offset calculations)
		run_height = text_height;
		
		// x,y keep track of cursor position
		if (text_dir < 0)
			x = block_width;
		else
			x = 0;
		
		// dx,dy keep track of x,y deltas for placement (dy for font height)
		if (text_height < line_height)
			dy = line_height - text_height;
		else
			dy = 0;
		
		dx = 0;
		
		//printf ("<TextBlock> moving to (%g, %g)\n", x + dx, y + dy);
		cairo_move_to (cr, x + dx, y + dy);
		pango_cairo_update_layout (cr, layout);
		fg->SetupBrush (cr, this);
		pango_cairo_show_layout (cr, layout);
		x += (dir * text_width);
	} else {
		if (node && ((Inline *) node->obj)->GetObjectType () == Type::RUN) {
			run_height = ((Run *) node->obj)->text_height;
			dir = ((Run *) node->obj)->text_dir;
		} else {
			run_height = 0;
			dir = 0;
		}
		
		x = dir < 0 ? block_width : 0;
	}
	
	// Note: all text per line is aligned along the bottom edge
	
	if (inlines != NULL) {
		PangoFontMask run_mask, inherited_mask;
		PangoFontMask inherited;
		Collection::Node *next;
		bool newline = false;
		Inline *item;
		Run *run;
		
		while (node != NULL) {
			item = (Inline *) node->obj;
			
			// FIXME: consider run->text_dir for horizontal placement of text runs
			
			switch (item->GetObjectType ()) {
			case Type::RUN:
				run = (Run *) item;
				
				text = run_get_text (run);
				//printf ("<Run>%s</Run>\n", text ? text : "(null)");
				
				if (text == NULL || *text == '\0') {
					// optimization
					break;
				}
				
				if (run->layout == NULL) {
					run->layout = pango_cairo_create_layout (cr);
					pango_layout_set_text (run->layout, text ? text : "", -1);
				}
				
				// calculate dx
				if (dir >= 0 && run->text_dir < 0)
					dx = run->text_width;
				else if (dir < 0 && run->text_dir > 0)
					dx = -run->text_width;
				else
					dx = 0;
				
				// calculate dy
				if (run->text_height < line_height)
					dy = line_height - run->text_height;
				else
					dy = 0;
				
				//printf ("\tmoving to (%g, %g)\n", x + dx, y + dy);
				cairo_move_to (cr, x + dx, y + dy);
				pango_cairo_update_layout (cr, run->layout);
				x += (run->text_dir * run->text_width) + dx;
				
				// Runs share their parent TextBlock's font properties if
				// they don't specify their own.
				run_mask = pango_font_description_get_set_fields (run->font);
				pango_font_description_merge (run->font, font, false);
				inherited_mask = (PangoFontMask) (font_mask & ~run_mask);
				
				if (inherited_mask != 0) {
					pango_layout_set_font_description (run->layout, run->font);
					pango_font_description_unset_fields (run->font, inherited_mask);
				}
				
				// Runs share their parent TextBlock's Foreground
				// property if they don't specify one of their own.
				if (run->foreground != NULL)
					run->foreground->SetupBrush (cr, this);
				else
					fg->SetupBrush (cr, this);
				
				pango_cairo_show_layout (cr, run->layout);
				
				run_height = run->text_height;
				newline = false;
				
				break;
			case Type::LINEBREAK:
				//printf ("<LineBreak/>\n");
				y += line_height;
				newline = true;
				
				next = (Collection::Node *) node->Next ();
				if (next != NULL) {
					if (((Inline *) next->obj)->GetObjectType () == Type::RUN) {
						line_width_height (next, &line_width, &line_height);
						dir = ((Run *) next->obj)->text_dir;
						x = dir < 0 ? block_width : 0;
					} else {
						// use the last run height as the line height
						line_height = run_height;
					}
					
					//printf ("\tnext line's width = %g, height = %g\n", line_width, line_height);
				}
				
				break;
			default:
				printf ("unknown Inline item\n");
				break;
			}
			
			node = (Collection::Node *) node->Next ();
		}
	}
	
	fg->unref ();
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
		pango_font_description_set_absolute_size (font, size * PANGO_SCALE);
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
		block_height = -1.0;
		block_width = -1.0;
		text_height = -1.0;
		text_width = -1.0;
		text_dir = 0;
	} else if (prop == TextBlock::InlinesProperty) {
		if (inlines != NULL) {
			inlines->Detach (NULL, this);
			inlines->unref ();
		}
		
		if ((inlines = text_block_get_inlines (this)) != NULL) {
			inlines->Attach (NULL, this);
			inlines->ref ();
		}
		
		block_height = -1.0;
		block_width = -1.0;
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
	
	if (font_changed) {
		if (layout != NULL)
			pango_layout_set_font_description (layout, font);
		
		block_height = -1.0;
		block_width = -1.0;
		text_height = -1.0;
		text_width = -1.0;
	}
	
	FrameworkElement::OnPropertyChanged (prop);
	
	FullInvalidate (false);
}

void
TextBlock::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	FrameworkElement::OnSubPropertyChanged (prop, subprop);
	
	if (subprop->type == Type::INLINES) {
		// will need to recalculate layout
		block_height = -1.0;
		block_width = -1.0;
	}
	
	FullInvalidate (false);
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
Glyphs::render (Surface *s, int x, int y, int width, int height)
{
	// FIXME: implement me
}

void 
Glyphs::getbounds ()
{
	Surface *s = item_get_surface (this);
	
	if (s == NULL)
		return;
	
	// FIXME: implement me
	x1 = y1 = x2 = y2 = 0;
}

Point
Glyphs::getxformorigin ()
{
	// FIXME: implement me
	return Point (0.0, 0.0);
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
