/*
 * brush.cpp: Brushes
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *   Sebastien Pouliot (spouliot@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <math.h>
#include "brush.h"
#include "color.h"
#include "frameworkelement.h"
#include "transform.h"

//
// SL-Cairo convertion and helper routines
//

static cairo_extend_t
convert_gradient_spread_method (GradientSpreadMethod method)
{
	switch (method) {
	case GradientSpreadMethodPad:
		return CAIRO_EXTEND_PAD;
	case GradientSpreadMethodReflect:
		return CAIRO_EXTEND_REFLECT;
	case GradientSpreadMethodRepeat:
		return CAIRO_EXTEND_REPEAT;
	}
	g_warning ("Invalid GradientSpreadMethod value");
	return CAIRO_EXTEND_NONE;
}

//
// Brush
//

DependencyProperty *Brush::OpacityProperty;
DependencyProperty *Brush::RelativeTransformProperty;
DependencyProperty *Brush::TransformProperty;

// used only for notifying attachees
DependencyProperty *Brush::ChangedProperty;


Brush *
brush_new (void)
{
	return new Brush ();
}

double
brush_get_opacity (Brush *brush)
{
	return brush->GetValue (Brush::OpacityProperty)->AsDouble();
}

void
brush_set_opacity (Brush *brush, double opacity)
{
	brush->SetValue (Brush::OpacityProperty, Value (opacity));
}

TransformGroup *
brush_get_relative_transform (Brush *brush)
{
	Value *value = brush->GetValue (Brush::RelativeTransformProperty);
	return value ? value->AsTransformGroup() : NULL;
}

void
brush_set_relative_transform (Brush *brush, TransformGroup* transform_group)
{
	brush->SetValue (Brush::RelativeTransformProperty, Value (transform_group));
}

TransformGroup *
brush_get_transform (Brush *brush)
{
	Value *value = brush->GetValue (Brush::TransformProperty);
	return value ? value->AsTransformGroup() : NULL;
}

void
brush_set_transform (Brush *brush, TransformGroup* transform_group)
{
	brush->SetValue (Brush::TransformProperty, Value (transform_group));
}

/*
 * Combine UIElement Opacity (including all parents) with Brush Opacity
 * NOTE: sadly we cannot handle Opacity at this level, each brush sub-type needs to handle it
 */
double
Brush::GetTotalOpacity (UIElement *uielement)
{
	double opacity = uielement ? uielement->GetTotalOpacity () : 1.0;

	double brush_opacity = brush_get_opacity (this);
	if (brush_opacity < 1.0)
		opacity *= brush_opacity;

	return opacity;
}

bool
Brush::SetupBrush (cairo_t *cairo, UIElement *uielement)
{
	g_warning ("Brush:SetupBrush has been called. The derived class should have overridden it.");
	return FALSE;
}

void
Brush::OnPropertyChanged (DependencyProperty *prop)
{
	//
	// If any of our properties change, we have to notify our
	// owners that they must repaint (all of our properties have
	// a visible effect
	//
	if (prop->type == Type::BRUSH)
		NotifyAttacheesOfPropertyChange (prop);
	
	DependencyObject::OnPropertyChanged (prop);
}

//
// SolidColorBrush
//

DependencyProperty* SolidColorBrush::ColorProperty;

bool
SolidColorBrush::SetupBrush (cairo_t *target, UIElement *uielement)
{
	Color *color = solid_color_brush_get_color (this);

	// apply UIElement opacity and Brush opacity on color's alpha
	double alpha = color->a * GetTotalOpacity (uielement);

	cairo_set_source_rgba (target, color->r, color->g, color->b, alpha);

	// [Relative]Transform do not apply to solid color brush
	return (alpha > 0.0);
}

void 
SolidColorBrush::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == SolidColorBrush::ColorProperty)
		NotifyAttacheesOfPropertyChange (prop);
	
	Brush::OnPropertyChanged (prop);
}

Color *
solid_color_brush_get_color (SolidColorBrush *solid_color_brush)
{
	return solid_color_brush->GetValue (SolidColorBrush::ColorProperty)->AsColor();
}

void
solid_color_brush_set_color (SolidColorBrush *solid_color_brush, Color *color)
{
	solid_color_brush->SetValue (SolidColorBrush::ColorProperty, Value (*color));
}

SolidColorBrush*
solid_color_brush_new (void)
{
	return new SolidColorBrush ();
}


//
// GradientBrush
//

DependencyProperty* GradientBrush::ColorInterpolationModeProperty;
DependencyProperty* GradientBrush::GradientStopsProperty;
DependencyProperty* GradientBrush::MappingModeProperty;
DependencyProperty* GradientBrush::SpreadMethodProperty;

GradientBrush*
gradient_brush_new (void)
{
	return new GradientBrush ();
}

ColorInterpolationMode
gradient_brush_get_color_interpolation_mode (GradientBrush *brush)
{
	return (ColorInterpolationMode) brush->GetValue (GradientBrush::ColorInterpolationModeProperty)->AsInt32();
}

void
gradient_brush_set_color_interpolation_mode (GradientBrush *brush, ColorInterpolationMode mode)
{
	brush->SetValue (GradientBrush::ColorInterpolationModeProperty, Value (mode));
}

GradientStopCollection*
gradient_brush_get_gradient_stops (GradientBrush *brush)
{
	Value *value = brush->GetValue (GradientBrush::GradientStopsProperty);
	return (GradientStopCollection*) (value ? value->AsGradientStopCollection() : NULL);
}

void
gradient_brush_set_gradient_stops (GradientBrush *brush, GradientStopCollection* collection)
{
	brush->SetValue (GradientBrush::GradientStopsProperty, Value (collection));
}

BrushMappingMode
gradient_brush_get_mapping_mode (GradientBrush *brush)
{
	return (BrushMappingMode) brush->GetValue (GradientBrush::MappingModeProperty)->AsInt32();
}

void
gradient_brush_set_mapping_mode (GradientBrush *brush, BrushMappingMode mode)
{
	brush->SetValue (GradientBrush::MappingModeProperty, Value (mode));
}

GradientSpreadMethod 
gradient_brush_get_spread (GradientBrush *brush)
{
	return (GradientSpreadMethod) brush->GetValue (GradientBrush::SpreadMethodProperty)->AsInt32();
}

void
gradient_brush_set_spread (GradientBrush *brush, GradientSpreadMethod method)
{
	brush->SetValue (GradientBrush::SpreadMethodProperty, Value (method));
}

GradientBrush::GradientBrush ()
{
	this->SetValue (GradientBrush::GradientStopsProperty, Value (new GradientStopCollection ()));
}

GradientBrush::~GradientBrush ()
{
}

void
GradientBrush::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == GradientBrush::GradientStopsProperty) {
		GradientStopCollection *newcol = GetValue (prop)->AsGradientStopCollection();
		
		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
		
		NotifyAttacheesOfPropertyChange (prop);
	}
	
	Brush::OnPropertyChanged (prop);
}

bool
GradientBrush::SetupGradient (cairo_pattern_t *pattern, UIElement *uielement)
{
	GradientStopCollection *children = GetValue (GradientBrush::GradientStopsProperty)->AsGradientStopCollection ();
	GradientSpreadMethod gsm = gradient_brush_get_spread (this);
	cairo_pattern_set_extend (pattern, convert_gradient_spread_method (gsm));
	Collection::Node *node;
	
	// TODO - BrushMappingMode is ignore (use a matrix)

	// TODO - ColorInterpolationModeProperty is ignored (map to ?)

	double opacity = GetTotalOpacity (uielement);
	node = (Collection::Node *) children->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->Next ()) {
		GradientStop *stop = (GradientStop *) node->obj;
		Color *color = gradient_stop_get_color (stop);
		double offset = gradient_stop_get_offset (stop);
		double alpha = (opacity < 1.0) ? color->a * opacity : color->a;
		cairo_pattern_add_color_stop_rgba (pattern, offset, color->r,
						   color->g, color->b, alpha);
	}
	return (opacity > 0.0);
}

//
// LinearGradientBrush
//

DependencyProperty* LinearGradientBrush::EndPointProperty;
DependencyProperty* LinearGradientBrush::StartPointProperty;

LinearGradientBrush*
linear_gradient_brush_new (void)
{
	return new LinearGradientBrush ();
}

Point*
linear_gradient_brush_get_end_point (LinearGradientBrush *brush)
{
	Value *value = brush->GetValue (LinearGradientBrush::EndPointProperty);
	return (value ? value->AsPoint() : NULL);
}

void
linear_gradient_brush_set_end_point (LinearGradientBrush *brush, Point *point)
{
	brush->SetValue (LinearGradientBrush::EndPointProperty, Value (*point));
}

Point*
linear_gradient_brush_get_start_point (LinearGradientBrush *brush)
{
	Value *value = brush->GetValue (LinearGradientBrush::StartPointProperty);
	return (value ? value->AsPoint() : NULL);
}

void
linear_gradient_brush_set_start_point (LinearGradientBrush *brush, Point *point)
{
	brush->SetValue (LinearGradientBrush::StartPointProperty, Value (*point));
}

bool
LinearGradientBrush::SetupBrush (cairo_t *cairo, UIElement *uielement)
{
	double w, h;
	
	if (uielement) {
		uielement->get_size_for_brush (cairo, &w, &h);
	} else {
		h = framework_element_get_height ((FrameworkElement *) uielement);
		w = framework_element_get_width ((FrameworkElement *) uielement);
	}
	
	Point *start = linear_gradient_brush_get_start_point (this);
	double x0 = start ? (start->x * w) : 0.0;
	double y0 = start ? (start->y * h) : 0.0;

	Point *end = linear_gradient_brush_get_end_point (this);
	double x1 = end ? (end->x * w) : w;
	double y1 = end ? (end->y * h) : h;

	cairo_pattern_t *pattern = cairo_pattern_create_linear (x0, y0, x1, y1);

	Transform *transform = brush_get_transform (this);
	if (transform) {
		cairo_matrix_t matrix;
		transform_get_transform (transform, &matrix);
		cairo_matrix_invert (&matrix);
		cairo_pattern_set_matrix (pattern, &matrix);
	}

	bool visible = GradientBrush::SetupGradient (pattern, uielement);

	cairo_set_source (cairo, pattern);
	cairo_pattern_destroy (pattern);

	return visible;
}

void 
LinearGradientBrush::OnPropertyChanged (DependencyProperty *prop)
{
	//
	// If any of our properties change, we have to notify our
	// owners that they must repaint (all of our properties have
	// a visible effect
	//
	if (prop->type == Type::LINEARGRADIENTBRUSH)
		NotifyAttacheesOfPropertyChange (prop);
	
	GradientBrush::OnPropertyChanged (prop);
}

//
// RadialGradientBrush
//

DependencyProperty* RadialGradientBrush::CenterProperty;
DependencyProperty* RadialGradientBrush::GradientOriginProperty;
DependencyProperty* RadialGradientBrush::RadiusXProperty;
DependencyProperty* RadialGradientBrush::RadiusYProperty;

RadialGradientBrush*
radial_gradient_brush_new (void)
{
	return new RadialGradientBrush ();
}

Point*
radial_gradient_brush_get_center (RadialGradientBrush *brush)
{
	Value *value = brush->GetValue (RadialGradientBrush::CenterProperty);
	return (value ? value->AsPoint() : NULL);
}

void
radial_gradient_brush_set_center (RadialGradientBrush *brush, Point *center)
{
	brush->SetValue (RadialGradientBrush::CenterProperty, Value (*center));
}

Point*
radial_gradient_brush_get_gradientorigin (RadialGradientBrush *brush)
{
	Value *value = brush->GetValue (RadialGradientBrush::GradientOriginProperty);
	return (value ? value->AsPoint() : NULL);
}

void
radial_gradient_brush_set_gradientorigin (RadialGradientBrush *brush, Point *origin)
{
	brush->SetValue (RadialGradientBrush::GradientOriginProperty, Value (*origin));
}

double
radial_gradient_brush_get_radius_x (RadialGradientBrush *brush)
{
	return brush->GetValue (RadialGradientBrush::RadiusXProperty)->AsDouble();
}

void
radial_gradient_brush_set_radius_x (RadialGradientBrush *brush, double radiusX)
{
	brush->SetValue (RadialGradientBrush::RadiusXProperty, Value (radiusX));
}

double
radial_gradient_brush_get_radius_y (RadialGradientBrush *brush)
{
	return brush->GetValue (RadialGradientBrush::RadiusYProperty)->AsDouble();
}

void
radial_gradient_brush_set_radius_y (RadialGradientBrush *brush, double radiusY)
{
	brush->SetValue (RadialGradientBrush::RadiusYProperty, Value (radiusY));
}

bool
RadialGradientBrush::SetupBrush (cairo_t *cairo, UIElement *uielement)
{
	Point *origin = radial_gradient_brush_get_gradientorigin (this);
	double ox = (origin ? origin->x : 0.5);
	double oy = (origin ? origin->y : 0.5);

	Point *center = radial_gradient_brush_get_center (this);
	double cx = (center ? center->x : 0.5);
	double cy = (center ? center->y : 0.5);

	double rx = radial_gradient_brush_get_radius_x (this);
	double ry = radial_gradient_brush_get_radius_y (this);

	cairo_pattern_t *pattern = cairo_pattern_create_radial (ox, oy, 0.0, cx, cy, ry);

	double x0, y0, x1, y1;
	cairo_stroke_extents (cairo, &x0, &y0, &x1, &y1);

	cairo_matrix_t matrix;
	cairo_matrix_init (&matrix, fabs (x1 - x0) * rx / ry, 0, 0, fabs (y1 - y0), x0, y0);

	Transform *transform = brush_get_transform (this);
	if (transform) {
		cairo_matrix_t tm;
		transform_get_transform (transform, &tm);
		// TODO - optimization, check for empty/identity matrix too ?
		cairo_matrix_multiply (&matrix, &matrix, &tm);
	}
	cairo_matrix_invert (&matrix);
	cairo_pattern_set_matrix (pattern, &matrix);

	bool visible = GradientBrush::SetupGradient (pattern, uielement);

	cairo_set_source (cairo, pattern);
	cairo_pattern_destroy (pattern);

	return visible;
}

//
// GradientStopCollection
//

GradientStopCollection*
gradient_stop_collection_new (void)
{
	return new GradientStopCollection ();
}

//
// GradientStop
//

DependencyProperty* GradientStop::ColorProperty;
DependencyProperty* GradientStop::OffsetProperty;

GradientStop*
gradient_stop_new (void)
{
	return new GradientStop ();
}

Color *
gradient_stop_get_color (GradientStop *stop)
{
	return stop->GetValue (GradientStop::ColorProperty)->AsColor();
}

void
gradient_stop_set_color (GradientStop *stop, Color *color)
{
	stop->SetValue (GradientStop::ColorProperty, Value (*color));
}

double
gradient_stop_get_offset (GradientStop *stop)
{
	return stop->GetValue (GradientStop::OffsetProperty)->AsDouble();
}

void
gradient_stop_set_offset (GradientStop *stop, double offset)
{
	stop->SetValue (GradientStop::OffsetProperty, Value (offset));
}

//
// TileBrush
//

DependencyProperty* TileBrush::AlignmentXProperty;
DependencyProperty* TileBrush::AlignmentYProperty;
DependencyProperty* TileBrush::StretchProperty;

TileBrush *
tile_brush_new (void)
{
	return new TileBrush ();
}

AlignmentX
tile_brush_get_alignment_x (TileBrush *brush)
{
	return (AlignmentX) brush->GetValue (TileBrush::AlignmentXProperty)->AsInt32();
}

void
tile_brush_set_alignment_x (TileBrush *brush, AlignmentX alignment)
{
	brush->SetValue (TileBrush::AlignmentXProperty, Value (alignment));
}

AlignmentY
tile_brush_get_alignment_y (TileBrush *brush)
{
	return (AlignmentY) brush->GetValue (TileBrush::AlignmentYProperty)->AsInt32();
}

void
tile_brush_set_alignment_y (TileBrush *brush, AlignmentY alignment)
{
	brush->SetValue (TileBrush::AlignmentYProperty, Value (alignment));
}

Stretch
tile_brush_get_stretch (TileBrush *brush)
{
	return (Stretch) brush->GetValue (TileBrush::StretchProperty)->AsInt32();
}

void
tile_brush_set_stretch (TileBrush *brush, Stretch stretch)
{
	brush->SetValue (TileBrush::StretchProperty, Value (stretch));
}

//
// ImageBrush
//

DependencyProperty* ImageBrush::DownloadProgressProperty;
DependencyProperty* ImageBrush::ImageSourceProperty;

ImageBrush *
image_brush_new (void)
{
	return new ImageBrush ();
}

double
image_brush_get_download_progress (ImageBrush *brush)
{
	return brush->GetValue (ImageBrush::DownloadProgressProperty)->AsDouble();
}

void
image_brush_set_download_progress (ImageBrush *brush, double progress)
{
	brush->SetValue (ImageBrush::DownloadProgressProperty, Value (progress));
}

char*
image_brush_get_image_source (ImageBrush *brush)
{
	Value *value = brush->GetValue (ImageBrush::ImageSourceProperty);
	return (value ? value->AsString() : NULL);
}

void
image_brush_set_image_source (ImageBrush *brush, const char* source)
{
	brush->SetValue (ImageBrush::ImageSourceProperty, Value (source));
}

ImageBrush::ImageBrush ()
{
	image = new Image ();
	image->brush = this;
}

ImageBrush::~ImageBrush ()
{
	image->brush = NULL;
	
	delete image;
}

void
ImageBrush::SetSource (DependencyObject *dl, char* PartName)
{
	image->SetSource (dl, PartName);
}

void
ImageBrush::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == ImageBrush::DownloadProgressProperty) {
		double progress = GetValue (ImageBrush::DownloadProgressProperty)->AsDouble();
		image->SetValue (Image::DownloadProgressProperty, Value (progress));

		NotifyAttacheesOfPropertyChange (Brush::ChangedProperty);
	} else if (prop == ImageBrush::ImageSourceProperty) {
		Value *value = GetValue (ImageBrush::ImageSourceProperty);
		char *source = value ? value->AsString () : NULL;
		image->SetValue (MediaBase::SourceProperty, Value (source));
		NotifyAttacheesOfPropertyChange (Brush::ChangedProperty);
	} else
		TileBrush::OnPropertyChanged (prop);
}

// ripped apart to be reusable for Image and VideoBrush classes
cairo_pattern_t*
image_brush_create_pattern (cairo_t *cairo, cairo_surface_t *surface, int sw, int sh, double opacity)
{
	cairo_pattern_t *pattern;

	if (opacity < 1.0) {
		cairo_surface_t *blending = cairo_surface_create_similar (cairo_get_target (cairo),
									  CAIRO_CONTENT_COLOR_ALPHA,
									  sw, sh);

		pattern = cairo_pattern_create_for_surface (surface);
		cairo_t *cr = cairo_create (blending);
		cairo_set_source (cr, pattern);
		cairo_paint_with_alpha (cr, opacity);
		cairo_destroy(cr);
		cairo_pattern_destroy (pattern);

		pattern = cairo_pattern_create_for_surface (blending);
	} else {
		pattern = cairo_pattern_create_for_surface (surface);
	}

	return pattern;
}

void
image_brush_compute_pattern_matrix (cairo_matrix_t *matrix, double width, double height, int sw, int sh, 
	Stretch stretch, AlignmentX align_x, AlignmentY align_y, Transform *transform)
{
	// scale required to "fit" for both axes
	double sx = sw / width;
	double sy = sh / height;

	// Fill is the simplest case because AlignementX and AlignmentY don't matter in this case
	if (stretch == StretchFill) {
		// fill extents in both axes
		cairo_matrix_init_scale (matrix, sx, sy);
	} else {
		double scale = 1.0;
		double dx = 0.0;
		double dy = 0.0;

		switch (stretch) {
		case StretchUniform:
			// fill without cuting the image, center the other axes
			scale = (sx < sy) ? sy : sx;
			break;
		case StretchUniformToFill:
			// fill by, potentially, cuting the image on one axe, center on both axes
			scale = (sx < sy) ? sx : sy;
			break;
		case StretchNone:
			break;
		default:
			g_warning ("Invalid Stretch value (%d).", stretch);
			break;
		}

		switch (align_x) {
		case AlignmentXLeft:
			dx = 0.0;
			break;
		case AlignmentXCenter:
			dx = (sw - (scale * width)) / 2;
			break;
		case AlignmentXRight:
			dx = (sw - (scale * width));
			break;
		}

		switch (align_y) {
		case AlignmentYTop:
			dy = 0.0;
			break;
		case AlignmentYCenter:
			dy = (sh - (scale * height)) / 2;
			break;
		case AlignmentYBottom:
			dy = (sh - (scale * height));
			break;
		}

		if (stretch == StretchNone) {
			// no strech, no scale
			cairo_matrix_init_translate (matrix, dx, dy);
		} else {
			// otherwise there's both a scale and translation to be done
			cairo_matrix_init (matrix, scale, 0, 0, scale, dx, dy);
		}
	}

	if (transform) {
		cairo_matrix_t tm;
		transform_get_transform (transform, &tm);
		cairo_matrix_invert (&tm);
		cairo_matrix_multiply (matrix, &tm, matrix);
	}
}

bool
ImageBrush::SetupBrush (cairo_t *cairo, UIElement *uielement)
{
	cairo_surface_t *surface = image->GetCairoSurface ();
	if (!surface) {
		// not yet available, draw gray-ish shadow where the brush should be applied
		cairo_set_source_rgba (cairo, 0.5, 0.5, 0.5, 0.5);
		return TRUE;
	}

// MS BUG ? the ImageBrush Opacity is ignored, only the Opacity from UIElement is considered
	double opacity = (uielement ? uielement->GetTotalOpacity () : 1.0);

	Stretch stretch = tile_brush_get_stretch (this);

	AlignmentX ax = tile_brush_get_alignment_x (this);
	AlignmentY ay = tile_brush_get_alignment_y (this);

	Transform *transform = brush_get_transform (this);
	
	double width, height;
	
	if (uielement) {
		uielement->get_size_for_brush (cairo, &width, &height);
	} else {
		double x1, y1, x2, y2;
		
		cairo_stroke_extents (cairo, &x1, &y1, &x2, &y2);
		
		height = fabs (y2 - y1);
		width = fabs (x2 - x1);
	}
	
	cairo_pattern_t *pattern = image_brush_create_pattern (cairo, surface, image->GetWidth (), image->GetHeight (), opacity);

	cairo_matrix_t matrix;
	image_brush_compute_pattern_matrix (&matrix, width, height, image->GetWidth (), image->GetHeight (), stretch, ax, ay, transform);
	cairo_pattern_set_matrix (pattern, &matrix);

	cairo_set_source (cairo, pattern);
	cairo_pattern_destroy (pattern);

	return (opacity > 0.0);
}

//
// VideoBrush
//

DependencyProperty *VideoBrush::SourceNameProperty;


VideoBrush::VideoBrush ()
{
	media = NULL;
}

VideoBrush::~VideoBrush ()
{
	if (media != NULL) {
		media->Detach (MediaElement::PositionProperty, this);
		media->unref ();
	}
}

bool
VideoBrush::SetupBrush (cairo_t *cairo, UIElement *uielement)
{
	MediaPlayer *mplayer = media ? media->mplayer : NULL;
	Transform *transform = brush_get_transform (this);
	AlignmentX ax = tile_brush_get_alignment_x (this);
	AlignmentY ay = tile_brush_get_alignment_y (this);
	Stretch stretch = tile_brush_get_stretch (this);
	double opacity, width, height;
	cairo_surface_t *surface;
	cairo_pattern_t *pattern;
	cairo_matrix_t matrix;
	
	if (media == NULL) {
		DependencyObject *obj;
		char *name;
		
		name = video_brush_get_source_name (this);
		
		if (name == NULL || *name == '\0')
			return false;
		
		if ((obj = FindName (name)) && obj->Is (Type::MEDIAELEMENT)) {
			obj->Attach (MediaElement::PositionProperty, this);
			media = (MediaElement *) obj;
			mplayer = media->mplayer;
			obj->ref ();
		} else if (obj == NULL) {
			printf ("could not find element `%s'\n", name);
		} else {
			printf ("obj %p is not of type MediaElement (it is %s)\n", obj,
				Type::Find (obj->GetObjectType ())->name);
		}
	}
	
	if (!mplayer || !(surface = mplayer->GetSurface ())) {
		// not yet available, draw gray-ish shadow where the brush should be applied
		cairo_set_source_rgba (cairo, 0.5, 0.5, 0.5, 0.5);
		return true;
	}
	
	if (uielement) {
		uielement->get_size_for_brush (cairo, &width, &height);
		opacity = uielement->GetTotalOpacity ();
	} else {
		double x1, y1, x2, y2;
		
		cairo_stroke_extents (cairo, &x1, &y1, &x2, &y2);
		height = fabs (y2 - y1);
		width = fabs (x2 - x1);
		
		opacity = 1.0;
	}
	
	pattern = image_brush_create_pattern (cairo, surface, mplayer->width, mplayer->height, opacity);
	image_brush_compute_pattern_matrix (&matrix, width, height, mplayer->width, mplayer->height, stretch, ax, ay, transform);
	cairo_pattern_set_matrix (pattern, &matrix);
	
	cairo_set_source (cairo, pattern);
	cairo_pattern_destroy (pattern);
	
	return (opacity > 0.0);
}

void
VideoBrush::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == VideoBrush::SourceNameProperty) {
		Value *value = GetValue (VideoBrush::SourceNameProperty);
		char *name = value ? value->AsString () : NULL;
		DependencyObject *obj;
		
		if (media != NULL) {
			media->Detach (MediaElement::PositionProperty, this);
			media->unref ();
			media = NULL;
		}
		
		if ((obj = FindName (name)) && obj->Is (Type::MEDIAELEMENT)) {
			obj->Attach (MediaElement::PositionProperty, this);
			media = (MediaElement *) obj;
			obj->ref ();
		} else {
			// Note: This may have failed because the parser hasn't set the
			// toplevel element yet, we'll try again in SetupBrush()
		}
		
		NotifyAttacheesOfPropertyChange (Brush::ChangedProperty);
	}
	
	TileBrush::OnPropertyChanged (prop);
}

void
VideoBrush::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (subprop == MediaElement::PositionProperty) {
		// We to changes in this MediaElement property so we
		// can notify whoever is using us to paint that they
		// need to redraw themselves.
		NotifyAttacheesOfPropertyChange (Brush::ChangedProperty);
	}
	
	TileBrush::OnSubPropertyChanged (prop, subprop);
}

VideoBrush *
video_brush_new (void)
{
	return new VideoBrush ();
}

char *
video_brush_get_source_name (VideoBrush *brush)
{
	Value *value = brush->GetValue (VideoBrush::SourceNameProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
video_brush_set_source_name (VideoBrush *brush, const char *value)
{
	brush->SetValue (VideoBrush::SourceNameProperty, Value (value));
}

//
//
//

void
brush_init (void)
{
	/* Brush fields */
	Brush::OpacityProperty = DependencyObject::Register (Type::BRUSH, "Opacity", new Value (1.0));
	Brush::RelativeTransformProperty = DependencyObject::Register (Type::BRUSH, "RelativeTransform", Type::TRANSFORMGROUP);
	Brush::TransformProperty = DependencyObject::Register (Type::BRUSH, "Transform", Type::TRANSFORMGROUP);
	Brush::ChangedProperty = DependencyObject::Register (Type::BRUSH, "Changed", Type::BOOL);
	
	/* SolidColorBrush fields */
	SolidColorBrush::ColorProperty = DependencyObject::Register (Type::SOLIDCOLORBRUSH, "Color", new Value (Color (0x00FFFFFF)));

	/* GradientBrush fields */
	GradientBrush::ColorInterpolationModeProperty = DependencyObject::Register (Type::GRADIENTBRUSH, "ColorInterpolationMode",  new Value (0));
	GradientBrush::GradientStopsProperty = DependencyObject::Register (Type::GRADIENTBRUSH, "GradientStops", Type::GRADIENTSTOP_COLLECTION);
	GradientBrush::MappingModeProperty = DependencyObject::Register (Type::GRADIENTBRUSH, "MappingMode",  new Value (0));
	GradientBrush::SpreadMethodProperty = DependencyObject::Register (Type::GRADIENTBRUSH, "SpreadMethod",  new Value (0));

	/* LinearGradientBrush fields */
	LinearGradientBrush::EndPointProperty = DependencyObject::Register (Type::LINEARGRADIENTBRUSH, "EndPoint", Type::POINT);
	LinearGradientBrush::StartPointProperty = DependencyObject::Register (Type::LINEARGRADIENTBRUSH, "StartPoint", Type::POINT);

	/* RadialGradientBrush fields */
	RadialGradientBrush::CenterProperty = DependencyObject::Register (Type::RADIALGRADIENTBRUSH, "Center", Type::POINT);
	RadialGradientBrush::GradientOriginProperty = DependencyObject::Register (Type::RADIALGRADIENTBRUSH, "GradientOrigin", Type::POINT);
	RadialGradientBrush::RadiusXProperty = DependencyObject::Register (Type::RADIALGRADIENTBRUSH, "RadiusX",  new Value (0.5));
	RadialGradientBrush::RadiusYProperty = DependencyObject::Register (Type::RADIALGRADIENTBRUSH, "RadiusY",  new Value (0.5));

	/* GradientStop fields */
	GradientStop::ColorProperty = DependencyObject::Register (Type::GRADIENTSTOP, "Color", new Value (Color (0x00FFFFFF)));
	GradientStop::OffsetProperty = DependencyObject::Register (Type::GRADIENTSTOP, "Offset", new Value (0.0));

	/* ImageBrush */
	ImageBrush::DownloadProgressProperty = DependencyObject::Register (Type::IMAGEBRUSH, "DownloadProgress", new Value (0.0));
	ImageBrush::ImageSourceProperty = DependencyObject::Register (Type::IMAGEBRUSH, "ImageSource", new Value (""));

	/* VideoBrush */
	VideoBrush::SourceNameProperty = DependencyObject::Register (Type::VIDEOBRUSH, "SourceName", new Value (""));

	/* TileBrush fields */
	TileBrush::AlignmentXProperty = DependencyObject::Register (Type::TILEBRUSH, "AlignmentX", new Value (AlignmentXCenter));
	TileBrush::AlignmentYProperty = DependencyObject::Register (Type::TILEBRUSH, "AlignmentY", new Value (AlignmentYCenter));
	TileBrush::StretchProperty = DependencyObject::Register (Type::TILEBRUSH, "Stretch", new Value (StretchFill));
}
