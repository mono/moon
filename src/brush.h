#ifndef __BRUSH__H__
#define __BRUSH__H__

G_BEGIN_DECLS

#include "runtime.h"

enum BrushMappingMode {
	BrushMappingModeAbsolute,
	BrushMappingModeRelativeToBoundingBox
};

enum ColorInterpolationMode {
	ColorInterpolationModeScRgbLinearInterpolation,
	ColorInterpolationModeSRgbLinearInterpolation
};

enum GradientSpreadMethod {
	GradientSpreadMethodPad,
	GradientSpreadMethodReflect,
	GradientSpreadMethodRepeat
};


class Brush : public DependencyObject {
 public:
	static DependencyProperty* OpacityProperty;
	static DependencyProperty* RelativeTransformProperty;
	static DependencyProperty* TransformProperty;

	Brush () { }
	Value::Kind GetObjectType () { return Value::BRUSH; };

	virtual void SetupBrush (cairo_t *cairo, UIElement *uielement) = 0;
	virtual void SetupPattern (cairo_pattern_t *pattern);
};

double		brush_get_opacity		(Brush *brush);
void		brush_set_opacity		(Brush *brush, double opacity);
TransformGroup	*brush_get_relative_transform	(Brush *brush);
void		brush_set_relative_transform	(Brush *brush, TransformGroup* transform_group);
TransformGroup	*brush_get_transform		(Brush *brush);
void		brush_set_transform		(Brush *brush, TransformGroup* transform_group);


class SolidColorBrush : public Brush {
 public:
	static DependencyProperty* ColorProperty;

	SolidColorBrush () { };

	Value::Kind GetObjectType () { return Value::SOLIDCOLORBRUSH; };

	virtual void SetupBrush (cairo_t *cairo, UIElement *uielement);
};

SolidColorBrush	*solid_color_brush_new ();
Color		*solid_color_brush_get_color (SolidColorBrush *solid_color_brush);
void		solid_color_brush_set_color (SolidColorBrush *solid_color_brush, Color *color);

// note: abstract in C#
class GradientBrush : public Brush {
 public:
	static DependencyProperty* ColorInterpolationModeProperty;
	static DependencyProperty* GradientStopsProperty;
	static DependencyProperty* MappingModeProperty;
	static DependencyProperty* SpreadMethodProperty;

	GradientStopCollection *children;

	GradientBrush ();
	Value::Kind GetObjectType () { return Value::GRADIENTBRUSH; }

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void SetupPattern (cairo_pattern_t *pattern);
};

ColorInterpolationMode gradient_brush_get_color_interpolation_mode (GradientBrush *brush);
void gradient_brush_set_color_interpolation_mode (GradientBrush *brush, ColorInterpolationMode mode);
GradientStopCollection *gradient_brush_get_gradient_stops (GradientBrush *brush);
void gradient_brush_set_gradient_stops (GradientBrush *brush, GradientStopCollection* collection);
BrushMappingMode gradient_brush_get_mapping_mode (GradientBrush *brush);
void gradient_brush_set_mapping_mode (GradientBrush *brush, BrushMappingMode mode);
GradientSpreadMethod gradient_brush_get_spread (GradientBrush *brush);
void gradient_brush_set_spread (GradientBrush *brush, GradientSpreadMethod method);

class TileBrush : public Brush {
	Value::Kind GetObjectType () { return Value::TILEBRUSH; }
};

class ImageBrush : public TileBrush {
	Value::Kind GetObjectType () { return Value::IMAGEBRUSH; }
};

class VideoBrush : public TileBrush {
	Value::Kind GetObjectType () { return Value::VIDEOBRUSH; }
};

class LinearGradientBrush : public GradientBrush {
 public:
	static DependencyProperty* EndPointProperty;
	static DependencyProperty* StartPointProperty;

	LinearGradientBrush () {};
	Value::Kind GetObjectType () { return Value::LINEARGRADIENTBRUSH; }

	virtual void SetupBrush (cairo_t *cairo, UIElement *uielement);
};

LinearGradientBrush *linear_gradient_brush_new ();
Point	*linear_gradient_brush_get_end_point 	(LinearGradientBrush *brush);
void	linear_gradient_brush_set_end_point	(LinearGradientBrush *brush, Point *point);
Point	*linear_gradient_brush_get_start_point	(LinearGradientBrush *brush);
void	linear_gradient_brush_set_start_point	(LinearGradientBrush *brush, Point *point);

class RadialGradientBrush : public GradientBrush {
 public:
	static DependencyProperty* CenterProperty;
	static DependencyProperty* GradientOriginProperty;
	static DependencyProperty* RadiusXProperty;
	static DependencyProperty* RadiusYProperty;

	RadialGradientBrush () {};
	Value::Kind GetObjectType () { return Value::RADIALGRADIENTBRUSH; }

	virtual void SetupBrush (cairo_t *cairo, UIElement *uielement);
};

RadialGradientBrush *radial_gradient_brush_new ();
Point*	radial_gradient_brush_get_center		(RadialGradientBrush *brush);
void	radial_gradient_brush_set_center		(RadialGradientBrush *brush, Point *center);
Point*	radial_gradient_brush_get_gradientorigin	(RadialGradientBrush *brush);
void	radial_gradient_brush_set_gradientorigin	(RadialGradientBrush *brush, Point *origin);
double	radial_gradient_brush_get_radius_x		(RadialGradientBrush *brush);
void	radial_gradient_brush_set_radius_x		(RadialGradientBrush *brush, double radiusX);
double	radial_gradient_brush_get_radius_y		(RadialGradientBrush *brush);
void	radial_gradient_brush_set_radius_y		(RadialGradientBrush *brush, double radiusY);

class GradientStopCollection : public Collection {
 public:
	GradientStopCollection () {}
	virtual Value::Kind GetObjectType () { return Value::GRADIENTSTOP_COLLECTION; }

	virtual Value::Kind GetElementType() { return Value::GRADIENTSTOP; }
};

GradientStopCollection *gradient_stop_collection_new ();

class GradientStop : public DependencyObject {
 public:
	static DependencyProperty* ColorProperty;
	static DependencyProperty* OffsetProperty;

	Value::Kind GetObjectType () { return Value::GRADIENTSTOP; }
};

GradientStop* gradient_stop_new ();
Color	*gradient_stop_get_color	(GradientStop *stop);
void	gradient_stop_set_color		(GradientStop *stop, Color *color);
double	gradient_stop_get_offset	(GradientStop *stop);
void	gradient_stop_set_offset	(GradientStop *stop, double offset);


G_END_DECLS

#endif
