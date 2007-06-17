#ifndef __BRUSH__H__
#define __BRUSH__H__

G_BEGIN_DECLS

#include "runtime.h"
#include "media.h"

enum AlignmentX {
	AlignmentXLeft,
	AlignmentXCenter,
	AlignmentXRight
};

enum AlignmentY {
	AlignmentYTop,
	AlignmentYCenter,
	AlignmentYBottom
};

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
	virtual Value::Kind GetObjectType () { return Value::BRUSH; };

	virtual void SetupBrush (cairo_t *cairo, UIElement *uielement);
	virtual void OnPropertyChanged (DependencyProperty *prop);

	double GetTotalOpacity (UIElement *uielement);
};

Brush *		brush_new			(void);
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

	virtual Value::Kind GetObjectType () { return Value::SOLIDCOLORBRUSH; };

	virtual void SetupBrush (cairo_t *cairo, UIElement *uielement);
	virtual void OnPropertyChanged (DependencyProperty *prop);
};

SolidColorBrush	*solid_color_brush_new (void);
Color		*solid_color_brush_get_color (SolidColorBrush *solid_color_brush);
void		solid_color_brush_set_color (SolidColorBrush *solid_color_brush, Color *color);


// note: abstract in C#
class GradientBrush : public Brush {
 public:
	static DependencyProperty* ColorInterpolationModeProperty;
	static DependencyProperty* GradientStopsProperty;
	static DependencyProperty* MappingModeProperty;
	static DependencyProperty* SpreadMethodProperty;

	GradientBrush ();
	~GradientBrush ();
	
	virtual Value::Kind GetObjectType () { return Value::GRADIENTBRUSH; }

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void SetupGradient (cairo_pattern_t *pattern, UIElement *uielement);
};

GradientBrush* gradient_brush_new (void);
ColorInterpolationMode gradient_brush_get_color_interpolation_mode (GradientBrush *brush);
void gradient_brush_set_color_interpolation_mode (GradientBrush *brush, ColorInterpolationMode mode);
GradientStopCollection *gradient_brush_get_gradient_stops (GradientBrush *brush);
void gradient_brush_set_gradient_stops (GradientBrush *brush, GradientStopCollection* collection);
BrushMappingMode gradient_brush_get_mapping_mode (GradientBrush *brush);
void gradient_brush_set_mapping_mode (GradientBrush *brush, BrushMappingMode mode);
GradientSpreadMethod gradient_brush_get_spread (GradientBrush *brush);
void gradient_brush_set_spread (GradientBrush *brush, GradientSpreadMethod method);


class TileBrush : public Brush {
 public:
	static DependencyProperty* AlignmentXProperty;
	static DependencyProperty* AlignmentYProperty;
	static DependencyProperty* StretchProperty;

	virtual Value::Kind GetObjectType () { return Value::TILEBRUSH; }
};

TileBrush*	tile_brush_new			();
AlignmentX	tile_brush_get_alignment_x	(TileBrush *brush);
void		tile_brush_set_alignment_x	(TileBrush *brush, AlignmentX alignment);
AlignmentY	tile_brush_get_alignment_y	(TileBrush *brush);
void		tile_brush_set_alignment_y	(TileBrush *brush, AlignmentY alignment);
Stretch		tile_brush_get_stretch		(TileBrush *brush);
void		tile_brush_set_stretch		(TileBrush *brush, Stretch stretch);


class ImageBrush : public TileBrush {
	Image *image;
 public:
	static DependencyProperty *DownloadProgressProperty;
	static DependencyProperty *ImageSourceProperty;

	ImageBrush ();
	~ImageBrush ();
	
	virtual Value::Kind GetObjectType () { return Value::IMAGEBRUSH; }

	void SetSource (DependencyObject *dl, char* PartName);
	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void SetupBrush (cairo_t *cairo, UIElement *uielement);
};

ImageBrush* image_brush_new (void);
double	image_brush_get_download_progress	(ImageBrush *brush);
void	image_brush_set_download_progress	(ImageBrush *brush, double progress);
char*	image_brush_get_image_source		(ImageBrush *brush);
void	image_brush_set_image_source		(ImageBrush *brush, const char* source);


class VideoBrush : public TileBrush {
 public:
	static DependencyProperty *SourceNameProperty;
	
	MediaPlayer *mplayer;
	guint timeout_id;
	
	VideoBrush ();
	~VideoBrush ();
	
	virtual Value::Kind GetObjectType () { return Value::VIDEOBRUSH; }
	
	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void SetupBrush (cairo_t *cairo, UIElement *uielement);
};

VideoBrush *video_brush_new (void);
char *video_brush_get_source_name (VideoBrush *brush);
void video_brush_set_source_name (VideoBrush *brush, const char *source);


class LinearGradientBrush : public GradientBrush {
 public:
	static DependencyProperty* EndPointProperty;
	static DependencyProperty* StartPointProperty;

	LinearGradientBrush () {};
	virtual Value::Kind GetObjectType () { return Value::LINEARGRADIENTBRUSH; }

	virtual void SetupBrush (cairo_t *cairo, UIElement *uielement);
	virtual void OnPropertyChanged (DependencyProperty *prop);
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
	virtual Value::Kind GetObjectType () { return Value::RADIALGRADIENTBRUSH; }

	virtual void SetupBrush (cairo_t *cairo, UIElement *uielement);
};

RadialGradientBrush *radial_gradient_brush_new (void);
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

	virtual Value::Kind GetObjectType () { return Value::GRADIENTSTOP; }
};

GradientStop* gradient_stop_new (void);
Color	*gradient_stop_get_color	(GradientStop *stop);
void	gradient_stop_set_color		(GradientStop *stop, Color *color);
double	gradient_stop_get_offset	(GradientStop *stop);
void	gradient_stop_set_offset	(GradientStop *stop, double offset);


G_END_DECLS

#endif /* __BRUSH_H__ */
