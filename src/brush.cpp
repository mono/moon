/*
 * brush.cpp: Brushes
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *   Sebastien Pouliot (spouliot@novell.com)
 *   Stephane Delcroix (sdelcroix@novell.com)
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

Transform *
brush_get_relative_transform (Brush *brush)
{
	Value *value = brush->GetValue (Brush::RelativeTransformProperty);
	return value ? value->AsTransform() : NULL;
}

void
brush_set_relative_transform (Brush *brush, Transform* transform)
{
	brush->SetValue (Brush::RelativeTransformProperty, Value (transform));
}

Transform *
brush_get_transform (Brush *brush)
{
	Value *value = brush->GetValue (Brush::TransformProperty);
	return value ? value->AsTransform() : NULL;
}

void
brush_set_transform (Brush *brush, Transform* transform)
{
	brush->SetValue (Brush::TransformProperty, Value (transform));
}

void
transform_get_absolute_transform (Transform *relative_transform, double width, double height, cairo_matrix_t *result)
{
	cairo_matrix_init_scale (result, width, height);
	cairo_matrix_t tm;
	transform_get_transform (relative_transform, &tm);
	cairo_matrix_multiply (result, &tm, result);
	cairo_matrix_scale (result, 1.0/width, 1.0/height);
}

bool
Brush::IsOpaque ()
{
	return !IS_TRANSLUCENT (brush_get_opacity (this));
}

void
Brush::SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height)
{
	g_warning ("Brush:SetupBrush has been called. The derived class should have overridden it.");
}

void
Brush::SetupBrush (cairo_t *cr, UIElement *uielement)
{
	double x0, y0, x1, y1;
	double w, h;
	
	if (uielement) {
		uielement->GetSizeForBrush (cr, &w, &h);
	} else {
		cairo_stroke_extents (cr, &x0, &y0, &x1, &y1);
		
		h = fabs (y1 - y0);
		w = fabs (x1 - x0);
	}
	
	SetupBrush (cr, uielement, w, h);
}

//
// SolidColorBrush
//

DependencyProperty* SolidColorBrush::ColorProperty;

void
SolidColorBrush::SetupBrush (cairo_t *cr, UIElement *uielement)
{
	Color *color = solid_color_brush_get_color (this);
	double opacity = brush_get_opacity (this);

	cairo_set_source_rgba (cr, color->r, color->g, color->b, opacity * color->a);

	// [Relative]Transform do not apply to solid color brush
}

void
SolidColorBrush::SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height)
{
	// note: avoid computing width and height since it can be very expansive
	// (e.g. complex paths) and not required for a SolidColorBrush
	SetupBrush (cr, uielement);
}

Color *
SolidColorBrush::GetColor ()
{
	return GetValue (SolidColorBrush::ColorProperty)->AsColor();
}

void
SolidColorBrush::SetColor (Color *color)
{
	SetValue (SolidColorBrush::ColorProperty, Value (*color));
}

bool
SolidColorBrush::IsOpaque ()
{
	return Brush::IsOpaque() && !IS_TRANSLUCENT (GetColor()->a);
}

Color *
solid_color_brush_get_color (SolidColorBrush *solid_color_brush)
{
	return solid_color_brush->GetColor ();
}

void
solid_color_brush_set_color (SolidColorBrush *solid_color_brush, Color *color)
{
	solid_color_brush->SetColor (color);
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
	this->SetValue (GradientBrush::GradientStopsProperty, Value::CreateUnref (new GradientStopCollection ()));
}

void
GradientBrush::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args)
{
	// GeometryGroup only has one collection, so let's save the hash lookup
	//if (col == GetValue (GeometryGroup::ChildrenProperty)->AsGeometryCollection())
		NotifyListenersOfPropertyChange (GradientBrush::GradientStopsProperty);
}

void
GradientBrush::SetupGradient (cairo_pattern_t *pattern, UIElement *uielement, bool single)
{
	GradientStopCollection *children = GetValue (GradientBrush::GradientStopsProperty)->AsGradientStopCollection ();
	GradientSpreadMethod gsm = gradient_brush_get_spread (this);
	cairo_pattern_set_extend (pattern, convert_gradient_spread_method (gsm));
	Collection::Node *node;
	double opacity = brush_get_opacity (this);
	
	// TODO - ColorInterpolationModeProperty is ignored (map to ?)
	if (single) {
		// if a single color is shown (e.g. start == end point) Cairo will,
		// by default, use the start color while SL use the end color
		node = (Collection::Node *) children->list->Last ();
	} else {
		node = (Collection::Node *) children->list->First ();
	}

	GradientStop *negative_stop = NULL;	//the biggest negative stop
	double neg_offset = 0.0;		//the cached associated offset
	GradientStop *first_stop = NULL;	//the smallest positive stop
	double first_offset = 0.0;		//idem
	GradientStop *last_stop = NULL;		//the biggest stop <= 1
	double last_offset = 0.0;		//idem
	GradientStop *outofbounds_stop = NULL;	//the smallest stop > 1
	double out_offset = 0.0;		//idem

	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		GradientStop *stop = (GradientStop *) node->obj;
		double offset = gradient_stop_get_offset (stop);
		if (offset >= 0.0 && offset <= 1.0) {
			Color *color = gradient_stop_get_color (stop);
			cairo_pattern_add_color_stop_rgba (pattern, offset, color->r, color->g, color->b, color->a * opacity);
			if (!first_stop || (first_offset != 0.0 && offset < first_offset)) {
				first_offset = offset;
				first_stop = stop;
			}
			if (!last_stop || (last_offset != 1.0 && offset > last_offset)) {
				last_offset = offset;
				last_stop = stop;
			}
		} else if (offset < 0.0 && (!negative_stop || offset > neg_offset)) {
				negative_stop = stop;
				neg_offset = offset;
		} else if (offset > 1.0 && (!outofbounds_stop || offset < out_offset)) {
				outofbounds_stop = stop;
				out_offset = offset;
		}
	}

	if (negative_stop && first_stop && first_offset != 0.0) { //take care of the negative stop
		Color *neg_color = gradient_stop_get_color (negative_stop);
		Color *first_color = gradient_stop_get_color (first_stop);
		double ratio = neg_offset / (neg_offset - first_offset);
		cairo_pattern_add_color_stop_rgba (pattern, 0.0, 
			neg_color->r + ratio * (first_color->r - neg_color->r),
			neg_color->g + ratio * (first_color->g - neg_color->g),
			neg_color->b + ratio * (first_color->b - neg_color->b),
			(neg_color->a + ratio * (first_color->a - neg_color->a)) * opacity);
	}
	if (outofbounds_stop && last_stop && last_offset != 1.0) { //take care of the >1 stop
		Color *last_color = gradient_stop_get_color (last_stop);
		Color *out_color = gradient_stop_get_color (outofbounds_stop);
		double ratio = (1.0 - last_offset) / (out_offset - last_offset);
		cairo_pattern_add_color_stop_rgba (pattern, 1.0, 
			last_color->r + ratio * (out_color->r - last_color->r),
			last_color->g + ratio * (out_color->g - last_color->g),
			last_color->b + ratio * (out_color->b - last_color->b),
			(last_color->a + ratio * (out_color->a - last_color->a)) * opacity);	
	}

	if (negative_stop && outofbounds_stop && !first_stop && !last_stop) { //only 2 stops, one < 0, the other > 1
		Color *neg_color = gradient_stop_get_color (negative_stop);
		Color *out_color = gradient_stop_get_color (outofbounds_stop);
		double ratio = neg_offset / (neg_offset - out_offset);
		cairo_pattern_add_color_stop_rgba (pattern, 0.0, 
			neg_color->r + ratio * (out_color->r - neg_color->r),
			neg_color->g + ratio * (out_color->g - neg_color->g),
			neg_color->b + ratio * (out_color->b - neg_color->b),
			(neg_color->a + ratio * (out_color->a - neg_color->a)) * opacity);
		ratio = (1.0 - neg_offset) / (out_offset - neg_offset);
		cairo_pattern_add_color_stop_rgba (pattern, 1.0, 
			neg_color->r + ratio * (out_color->r - neg_color->r),
			neg_color->g + ratio * (out_color->g - neg_color->g),
			neg_color->b + ratio * (out_color->b - neg_color->b),
			(neg_color->a + ratio * (out_color->a - neg_color->a)) * opacity);	
	}

	if (negative_stop && !outofbounds_stop && !first_stop && !last_stop) { //only negative stops
		Color *color = gradient_stop_get_color (negative_stop);
		cairo_pattern_add_color_stop_rgba (pattern, 0.0, color->r, color->g, color->b, color->a * opacity);	
	}

	if (outofbounds_stop && !negative_stop && !first_stop && !last_stop) { //only > 1 stops
		Color *color = gradient_stop_get_color (outofbounds_stop);
		cairo_pattern_add_color_stop_rgba (pattern, 1.0, color->r, color->g, color->b, color->a * opacity);	
	}

	

}

bool
GradientBrush::IsOpaque ()
{
	if (!Brush::IsOpaque())
		return false;

	GradientStopCollection *stops = gradient_brush_get_gradient_stops (this);

	if (stops->list->Length() > 0) {
		Collection::Node *cn;
		for (cn = (Collection::Node *) stops->list->First ();
		     cn != NULL;
		     cn = (Collection::Node *) cn->next) {

			GradientStop *stop = (GradientStop*)cn->obj;
			Color* c = gradient_stop_get_color (stop);
			if (IS_TRANSLUCENT (c->a))
				return false;
		}
	}

	return true;
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

void
LinearGradientBrush::SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height)
{
	Point *start = linear_gradient_brush_get_start_point (this);
	Point *end = linear_gradient_brush_get_end_point (this);
	double x0, y0, x1, y1;
	cairo_matrix_t offset_matrix; 
	Point p = uielement->GetOriginPoint ();

	switch (gradient_brush_get_mapping_mode (this)) {
	case BrushMappingModeAbsolute:
		y0 = start ? start->y : 0.0;
		x0 = start ? start->x : 0.0;
		y1 = end ? end->y : height;
		x1 = end ? end->x : width;
		break;
	case BrushMappingModeRelativeToBoundingBox:
	default:
		y0 = start ? (start->y * height) : 0.0;
		x0 = start ? (start->x * width) : 0.0;
		y1 = end ? (end->y * height) : height;
		x1 = end ? (end->x * width) : width;
		break;	
	}

	cairo_pattern_t *pattern = cairo_pattern_create_linear (x0, y0, x1, y1);
	
	cairo_matrix_t matrix;
	cairo_matrix_init_identity (&matrix);

	Transform *transform = brush_get_transform (this);
	if (transform) {
		cairo_matrix_t tm;
		transform_get_transform (transform, &tm);
		// TODO - optimization, check for empty/identity matrix too ?
		cairo_matrix_multiply (&matrix, &matrix, &tm);
	}
	
	Transform *relative_transform = brush_get_relative_transform (this);
	if (relative_transform) {
		cairo_matrix_t tm;
		transform_get_absolute_transform (relative_transform, width, height, &tm);
		cairo_matrix_multiply (&matrix, &matrix, &tm);
	}

	if (p.x != 0.0 && p.y != 0.0) {
		cairo_matrix_init_translate (&offset_matrix, p.x, p.y);
		cairo_matrix_multiply (&matrix, &matrix, &offset_matrix);
	}

	cairo_matrix_invert (&matrix);
	cairo_pattern_set_matrix (pattern, &matrix);
	
	bool only_start = (x0 == x1 && y0 == y1);
	GradientBrush::SetupGradient (pattern, uielement, only_start);
	
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
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

void
RadialGradientBrush::SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height)
{
	Point offset = uielement->GetOriginPoint ();
	Point *origin = radial_gradient_brush_get_gradientorigin (this);
	double ox = (origin ? origin->x : 0.5);
	double oy = (origin ? origin->y : 0.5);
	cairo_matrix_t offset_matrix; 
	
	Point *center = radial_gradient_brush_get_center (this);
	double cx = (center ? center->x : 0.5);
	double cy = (center ? center->y : 0.5);
	
	double rx = radial_gradient_brush_get_radius_x (this);
	double ry = radial_gradient_brush_get_radius_y (this);
	
	cairo_pattern_t *pattern = cairo_pattern_create_radial (ox/rx, oy/ry, 0.0, cx/rx, cy/ry, 1);

	cairo_matrix_t matrix;
	switch (gradient_brush_get_mapping_mode (this)) {
	case BrushMappingModeAbsolute:
		cairo_matrix_init_translate (&matrix, cx, cy);
		cairo_matrix_scale (&matrix, rx, ry);
		cairo_matrix_translate (&matrix, -cx/rx, -cy/ry);
		break;
	case BrushMappingModeRelativeToBoundingBox:
		cairo_matrix_init_translate (&matrix, cx * width, cy * height);
		cairo_matrix_scale (&matrix, width * rx, height * ry );
		cairo_matrix_translate (&matrix, -cx/rx, -cy/ry);
		break;
	}
	
	Transform *transform = brush_get_transform (this);
	if (transform) {
		cairo_matrix_t tm;
		transform_get_transform (transform, &tm);
		// TODO - optimization, check for empty/identity matrix too ?
		cairo_matrix_multiply (&matrix, &matrix, &tm);
	}

	Transform *relative_transform = brush_get_relative_transform (this);
	if (relative_transform) {
		cairo_matrix_t tm;
		transform_get_absolute_transform (relative_transform, width, height, &tm);
		// TODO - optimization, check for empty/identity matrix too ?
		cairo_matrix_multiply (&matrix, &matrix, &tm);
	}

	if (offset.x != 0.0 || offset.y != 0.0) {
		cairo_matrix_init_translate (&offset_matrix, offset.x, offset.y);
		cairo_matrix_multiply (&matrix, &matrix, &offset_matrix);
	}

	cairo_status_t status = cairo_matrix_invert (&matrix);
	if (status != CAIRO_STATUS_SUCCESS) {
		printf ("Moonlight: Error inverting matrix falling back\n");
		cairo_matrix_init_identity (&matrix);
	}
	cairo_pattern_set_matrix (pattern, &matrix);
	GradientBrush::SetupGradient (pattern, uielement);

	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
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

int ImageBrush::DownloadProgressChangedEvent = -1;
int ImageBrush::ImageFailedEvent = -1;

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

void
image_brush_set_source (ImageBrush *brush, Downloader *downloader, const char *PartName)
{
	brush->SetSource (downloader, PartName);
}

void
ImageBrush::image_progress_changed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	ImageBrush *brush = (ImageBrush*)closure;
	double progress = brush->image->GetValue (Image::DownloadProgressProperty)->AsDouble();
	brush->SetValue (ImageBrush::DownloadProgressProperty, Value (progress));
	brush->Emit (ImageBrush::DownloadProgressChangedEvent);
}

void
ImageBrush::image_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((ImageBrush*)closure)->Emit (ImageBrush::ImageFailedEvent);
}

ImageBrush::ImageBrush ()
{
	image = new Image ();

	image->AddHandler (MediaBase::DownloadProgressChangedEvent, image_progress_changed, this);
	image->AddHandler (Image::ImageFailedEvent, image_failed, this);

	image->brush = this;
}

ImageBrush::~ImageBrush ()
{
	image->brush = NULL;
	image->unref ();
}

void
ImageBrush::SetSource (Downloader *downloader, const char *PartName)
{
	image->SetSource (downloader, PartName);
}

void
ImageBrush::SetSurface (Surface *surface)
{
	image->SetSurface (surface);
	DependencyObject::SetSurface (surface);
}

void
ImageBrush::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::IMAGEBRUSH) {
		TileBrush::OnPropertyChanged (args);
		return;
	}

	if (args->property == ImageBrush::DownloadProgressProperty) {
		image->SetValue (Image::DownloadProgressProperty, args->new_value);
	}
	else if (args->property == ImageBrush::ImageSourceProperty) {
		image->SetValue (MediaBase::SourceProperty, args->new_value);
	}

	NotifyListenersOfPropertyChange (args);
}

bool
ImageBrush::IsOpaque ()
{
	// XXX punt for now and return false here.
	return false;
}

cairo_surface_t *image_brush_create_similar (cairo_t *cairo, int width, int height)
{
#if USE_OPT_IMAGE_ONLY
	return cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
#else
	return cairo_surface_create_similar (cairo_get_target (cairo),
					     CAIRO_CONTENT_COLOR_ALPHA,
					     width,
					     height);
#endif
}

void
image_brush_compute_pattern_matrix (cairo_matrix_t *matrix, double width, double height, int sw, int sh, 
	Stretch stretch, AlignmentX align_x, AlignmentY align_y, Transform *transform, Transform *relative_transform)
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

	if (transform || relative_transform) {
		if (transform) {
			cairo_matrix_t tm;
			transform_get_transform (transform, &tm);
			cairo_matrix_invert (&tm);
			cairo_matrix_multiply (matrix, &tm, matrix);
		}
		if (relative_transform) {
			cairo_matrix_t tm;
			transform_get_absolute_transform (relative_transform, width, height, &tm);
			cairo_matrix_invert (&tm);
			cairo_matrix_multiply (matrix, &tm, matrix);
		}
	}
}

void
ImageBrush::SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height)
{
	cairo_surface_t *surface = image->GetCairoSurface ();
	if (!surface) {
		// not yet available, draw nothing
		// XXX Removing this _source_set at all?
		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);
		return;
	}
	
	Stretch stretch = tile_brush_get_stretch (this);
	
	AlignmentX ax = tile_brush_get_alignment_x (this);
	AlignmentY ay = tile_brush_get_alignment_y (this);
	
	Transform *transform = brush_get_transform (this);
	Transform *relative_transform = brush_get_relative_transform (this);
	cairo_matrix_t matrix;

	cairo_pattern_t *pattern = cairo_pattern_create_for_surface (surface);

	image_brush_compute_pattern_matrix (&matrix, width, height, image->GetWidth (), image->GetHeight (), stretch, ax, ay, transform, relative_transform);
	Point offset = uielement->GetOriginPoint ();
	cairo_matrix_translate (&matrix, -offset.x, -offset.y);
	cairo_pattern_set_matrix (pattern, &matrix);
	
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
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
		media->RemovePropertyChangeListener (this);
		media->unref ();
	}
}

void
VideoBrush::SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height)
{
	MediaPlayer *mplayer = media ? media->GetMediaPlayer () : NULL;
	Transform *transform = brush_get_transform (this);
	Transform *relative_transform = brush_get_relative_transform (this);
	AlignmentX ax = tile_brush_get_alignment_x (this);
	AlignmentY ay = tile_brush_get_alignment_y (this);
	Stretch stretch = tile_brush_get_stretch (this);
	cairo_surface_t *surface;
	cairo_pattern_t *pattern;
	cairo_matrix_t matrix;
	
	if (media == NULL) {
		DependencyObject *obj;
		char *name;
		
		name = video_brush_get_source_name (this);
		
		if (name == NULL || *name == '\0')
			return;
		
		if ((obj = FindName (name)) && obj->Is (Type::MEDIAELEMENT)) {
			obj->AddPropertyChangeListener (this);
			media = (MediaElement *) obj;
			mplayer = media->GetMediaPlayer ();
			obj->ref ();
		} else if (obj == NULL) {
			printf ("could not find element `%s'\n", name);
		} else {
			printf ("obj %p is not of type MediaElement (it is %s)\n", obj,
				Type::Find (obj->GetObjectType ())->name);
		}
	}
	
	if (!mplayer || !(surface = mplayer->GetCairoSurface ())) {
		// not yet available, draw gray-ish shadow where the brush should be applied
		cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 0.5);
		return;
	}
	
	pattern = cairo_pattern_create_for_surface (surface);

	image_brush_compute_pattern_matrix (&matrix, width, height, mplayer->GetWidth (), mplayer->GetHeight (), stretch, ax, ay, transform, relative_transform);
	Point offset = uielement->GetOriginPoint ();
	cairo_matrix_translate (&matrix, -offset.x, -offset.y);
	cairo_pattern_set_matrix (pattern, &matrix);
	
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
}

void
VideoBrush::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::VIDEOBRUSH) {
		TileBrush::OnPropertyChanged (args);
		return;
	}

	if (args->property == VideoBrush::SourceNameProperty) {
		char *name = args->new_value ? args->new_value->AsString () : NULL;
		DependencyObject *obj;
		
		if (media != NULL) {
			media->RemovePropertyChangeListener (this);
			media->unref ();
			media = NULL;
		}
		
		if ((obj = FindName (name)) && obj->Is (Type::MEDIAELEMENT)) {
			obj->AddPropertyChangeListener (this);
			media = (MediaElement *) obj;
			obj->ref ();
		} else {
			// Note: This may have failed because the parser hasn't set the
			// toplevel element yet, we'll try again in SetupBrush()
		}
	}

	NotifyListenersOfPropertyChange (args);
}

void
VideoBrush::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (subobj_args->property == MediaElement::PositionProperty) {
		// We to changes in this MediaElement property so we
		// can notify whoever is using us to paint that they
		// need to redraw themselves.
		NotifyListenersOfPropertyChange (Brush::ChangedProperty);
	}
	
	TileBrush::OnSubPropertyChanged (prop, obj, subobj_args);
}

bool
VideoBrush::IsOpaque ()
{
	// XXX punt for now and return false here.
	return false;
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
// VisualBrush
//

DependencyProperty* VisualBrush::VisualProperty;

void
VisualBrush::SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height)
{
	UIElement *ui = (UIElement*)GetValue (VisualBrush::VisualProperty)->AsVisual ();
	if (!ui) {
		// not yet available, draw gray-ish shadow where the brush should be applied
		cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 0.5);
		return;
	}
	
	// XXX we should cache the surface so that it can be
	// used multiple times without having to re-render each time.
	Rect bounds = ui->GetSubtreeBounds().RoundOut ();

	surface = image_brush_create_similar (cr,
					      (int)bounds.w, 
					      (int)bounds.h);

	cairo_t *surface_cr = cairo_create (surface);
	ui->Render (surface_cr, 0, 0, (int)bounds.w , (int)bounds.h);
	cairo_destroy (surface_cr);
	
	Stretch stretch = tile_brush_get_stretch (this);
	
	AlignmentX ax = tile_brush_get_alignment_x (this);
	AlignmentY ay = tile_brush_get_alignment_y (this);
	
	Transform *transform = brush_get_transform (this);
	Transform *relative_transform = brush_get_relative_transform (this);
	
 	cairo_pattern_t *pattern = cairo_pattern_create_for_surface (surface);
	cairo_matrix_t matrix;
 	image_brush_compute_pattern_matrix (&matrix, width, height,
					    (int)bounds.w, (int)bounds.h,
					    stretch, ax, ay, transform, relative_transform);

	Point offset = uielement->GetOriginPoint ();
	cairo_matrix_translate (&matrix, -offset.x, -offset.y);
 	cairo_pattern_set_matrix (pattern, &matrix);

 	cairo_set_source (cr, pattern);
 	cairo_pattern_destroy (pattern);

	cairo_surface_destroy (surface);
}

void
VisualBrush::update_brush (EventObject *, EventArgs *, gpointer closure)
{
	VisualBrush *b = (VisualBrush*)closure;
	b->NotifyListenersOfPropertyChange (Brush::ChangedProperty);
}

void
VisualBrush::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::VISUALBRUSH) {
		TileBrush::OnPropertyChanged (args);
		return;
	}

	if (args->property == VisualBrush::VisualProperty) {
		// XXX we really need a way to disconnect from the preview visual
		Visual *v = args->new_value->AsVisual();
		v->AddHandler (((UIElement*)v)->InvalidatedEvent, update_brush, this);
	}

	NotifyListenersOfPropertyChange (args);
}

bool
VisualBrush::IsOpaque ()
{
	// XXX punt for now and return false here.
	return false;
}


Visual *
visual_brush_get_visual (VisualBrush *visual_brush)
{
	return visual_brush->GetValue (VisualBrush::VisualProperty)->AsVisual();
}

void
visualbrush_set_color (VisualBrush *visual_brush, Visual *visual)
{
	visual_brush->SetValue (VisualBrush::VisualProperty, Value (visual));
}

VisualBrush*
visual_brush_new (void)
{
	return new VisualBrush ();
}

//
//
//

void
brush_init (void)
{
	/* Brush fields */
	Brush::OpacityProperty = DependencyObject::Register (Type::BRUSH, "Opacity", new Value (1.0));
	Brush::RelativeTransformProperty = DependencyObject::Register (Type::BRUSH, "RelativeTransform", Type::TRANSFORM);
	Brush::TransformProperty = DependencyObject::Register (Type::BRUSH, "Transform", Type::TRANSFORM);
	Brush::ChangedProperty = DependencyObject::Register (Type::BRUSH, "Changed", Type::BOOL);
	
	/* SolidColorBrush fields */
	SolidColorBrush::ColorProperty = DependencyObject::Register (Type::SOLIDCOLORBRUSH, "Color", new Value (Color (0x00000000)));

	/* GradientBrush fields */
	GradientBrush::ColorInterpolationModeProperty = DependencyObject::Register (Type::GRADIENTBRUSH, "ColorInterpolationMode",  new Value (ColorInterpolationModeSRgbLinearInterpolation));
	GradientBrush::GradientStopsProperty = DependencyObject::Register (Type::GRADIENTBRUSH, "GradientStops", Type::GRADIENTSTOP_COLLECTION);
	GradientBrush::MappingModeProperty = DependencyObject::Register (Type::GRADIENTBRUSH, "MappingMode",  new Value (BrushMappingModeRelativeToBoundingBox));
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
	GradientStop::ColorProperty = DependencyObject::Register (Type::GRADIENTSTOP, "Color", new Value (Color (0x00000000)));
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

	/* VisualBrush */
	VisualBrush::VisualProperty = DependencyObject::Register (Type::VISUALBRUSH, "Visual", new Value (Type::VISUAL));

	/* lookup events */
	Type *t = Type::Find (Type::IMAGEBRUSH);
	ImageBrush::DownloadProgressChangedEvent = t->LookupEvent ("DownloadProgressChanged");
	ImageBrush::ImageFailedEvent = t->LookupEvent ("ImageFailed");
}
