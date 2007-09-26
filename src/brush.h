/*
 * brush.h: Brushes
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

#ifndef __BRUSH__H__
#define __BRUSH__H__

G_BEGIN_DECLS

#include "enums.h"
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
	
	// internal property - generic brush property change
	static DependencyProperty *ChangedProperty;
	
	Brush () { }
	virtual Type::Kind GetObjectType () { return Type::BRUSH; };

	virtual bool SetupBrush (cairo_t *cairo, UIElement *uielement);
	virtual void OnPropertyChanged (DependencyProperty *prop);
	
	double GetTotalOpacity (UIElement *uielement);
};

Brush *		brush_new			(void);
double		brush_get_opacity		(Brush *brush);
void		brush_set_opacity		(Brush *brush, double opacity);
Transform	*brush_get_relative_transform	(Brush *brush);
void		brush_set_relative_transform	(Brush *brush, TransformGroup* transform);
Transform	*brush_get_transform		(Brush *brush);
void		brush_set_transform		(Brush *brush, TransformGroup* transform);


class SolidColorBrush : public Brush {
 public:
	static DependencyProperty* ColorProperty;

	SolidColorBrush () { };

	virtual Type::Kind GetObjectType () { return Type::SOLIDCOLORBRUSH; };

	virtual bool SetupBrush (cairo_t *cairo, UIElement *uielement);
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
	
	virtual Type::Kind GetObjectType () { return Type::GRADIENTBRUSH; }

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop);
	virtual bool SetupGradient (cairo_pattern_t *pattern, UIElement *uielement);
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

	virtual Type::Kind GetObjectType () { return Type::TILEBRUSH; }
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
	virtual ~ImageBrush ();
	
	virtual Type::Kind GetObjectType () { return Type::IMAGEBRUSH; }

	void SetSource (DependencyObject *dl, const char* PartName);
	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual bool SetupBrush (cairo_t *cairo, UIElement *uielement);
};

ImageBrush* image_brush_new (void);
double	image_brush_get_download_progress	(ImageBrush *brush);
void	image_brush_set_download_progress	(ImageBrush *brush, double progress);
char*	image_brush_get_image_source		(ImageBrush *brush);
void	image_brush_set_image_source		(ImageBrush *brush, const char* source);
cairo_surface_t *image_brush_create_similar     (cairo_t *, int width, int height);

class VideoBrush : public TileBrush {
	MediaElement *media;
public:
	static DependencyProperty *SourceNameProperty;
	
	VideoBrush ();
	virtual ~VideoBrush ();
	
	virtual Type::Kind GetObjectType () { return Type::VIDEOBRUSH; }
	
	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop);
	virtual bool SetupBrush (cairo_t *cairo, UIElement *uielement);
};

VideoBrush *video_brush_new (void);
char *video_brush_get_source_name (VideoBrush *brush);
void video_brush_set_source_name (VideoBrush *brush, const char *source);


class LinearGradientBrush : public GradientBrush {
 public:
	static DependencyProperty* EndPointProperty;
	static DependencyProperty* StartPointProperty;

	LinearGradientBrush () {};
	virtual Type::Kind GetObjectType () { return Type::LINEARGRADIENTBRUSH; }

	virtual bool SetupBrush (cairo_t *cairo, UIElement *uielement);
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
	virtual Type::Kind GetObjectType () { return Type::RADIALGRADIENTBRUSH; }

	virtual bool SetupBrush (cairo_t *cairo, UIElement *uielement);
	virtual void OnPropertyChanged (DependencyProperty *prop);
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
	virtual Type::Kind GetObjectType () { return Type::GRADIENTSTOP_COLLECTION; }

	virtual Type::Kind GetElementType() { return Type::GRADIENTSTOP; }
};

GradientStopCollection *gradient_stop_collection_new ();


class GradientStop : public DependencyObject {
 public:
	static DependencyProperty* ColorProperty;
	static DependencyProperty* OffsetProperty;

	virtual Type::Kind GetObjectType () { return Type::GRADIENTSTOP; }
};

GradientStop* gradient_stop_new (void);
Color	*gradient_stop_get_color	(GradientStop *stop);
void	gradient_stop_set_color		(GradientStop *stop, Color *color);
double	gradient_stop_get_offset	(GradientStop *stop);
void	gradient_stop_set_offset	(GradientStop *stop, double offset);


class VisualBrush : public TileBrush {
	cairo_surface_t *surface;

	static void update_brush (EventObject *, gpointer, gpointer closure);

 public:
	static DependencyProperty* VisualProperty;

	VisualBrush () { };

	virtual Type::Kind GetObjectType () { return Type::VISUALBRUSH; };

	virtual bool SetupBrush (cairo_t *cairo, UIElement *uielement);
	virtual void OnPropertyChanged (DependencyProperty *prop);
};

VisualBrush	*visual_brush_new (void);
Visual          *visual_brush_get_visual (VisualBrush *visual_brush);
void             visual_brush_set_visual (VisualBrush *visual_brush, Visual *visual);

void brush_init (void);

G_END_DECLS

#endif /* __BRUSH_H__ */
