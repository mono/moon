#ifndef __BRUSH__H__
#define __BRUSH__H__

G_BEGIN_DECLS

#include "runtime.h"


class Brush : public DependencyObject {
 public:
	static DependencyProperty* OpacityProperty;
	static DependencyProperty* RelativeTransformProperty;
	static DependencyProperty* TransformProperty;

	Brush ()
	{
	}
	Value::Kind GetObjectType () { return Value::BRUSH; };
	virtual void SetupBrush (cairo_t *cairo, UIElement *uielement) = 0;
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

class GradientBrush : public Brush {
 public:
	GradientBrush ();
	Value::Kind GetObjectType () { return Value::GRADIENTBRUSH; }
	// ColorInterpolationMode{ScRgbLinearInterpolation, SRgbLinearInterpolation} mode;
	// GradientStopCollection stops
	// BrushMappingMode{Absolute,RelativeToBoundingBox} MappingMode
	// GradientSpreadMethod{Pad, Reflect, Repeat} SpreadMethod
	virtual void SetupBrush (cairo_t *cairo);
};

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
	Value::Kind GetObjectType () { return Value::LINEARGRADIENTBRUSH; }
};

class RadialGradientBrush : public GradientBrush {
 public:
	static DependencyProperty* CenterProperty;
	static DependencyProperty* GradientOriginProperty;
	static DependencyProperty* RadiusXProperty;
	static DependencyProperty* RadiusYProperty;

	Value::Kind GetObjectType () { return Value::RADIALGRADIENTBRUSH; }
};
RadialGradientBrush *radial_gradient_brush_new ();
Point*	radial_gradient_brush_get_center		(RadialGradientBrush *brush);
void	radial_gradient_brush_set_center		(RadialGradientBrush *brush, Point *center);
Point*	radial_gradient_brush_get_gradientorigin	(RadialGradientBrush *brush);
void	radial_gradient_brush_set_gradientorigin	(RadialGradientBrush *brush, Point *origin);
double	radial_gradient_brush_get_radius_x		(RadialGradientBrush *brush);
void	radial_gradient_brush_set_radius_x		(RadialGradientBrush *brush, double radiusX);
double	radial_gradient_brush_get_radius_y		(RadialGradientBrush *brush);
void	radial_gradient_brush_set_radius_y		(RadialGradientBrush *brush, double radiusX);


class GradientStop : public DependencyObject {
	Value::Kind GetObjectType () { return Value::GRADIENTSTOP; }
};

G_END_DECLS

#endif
