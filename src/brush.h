/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * brush.h: Brushes
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __BRUSH__H__
#define __BRUSH__H__

#include <glib.h>

G_BEGIN_DECLS

#include "enums.h"
#include "collection.h"
#include "uielement.h"

class MediaElement;

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
 protected:
	virtual ~Brush () {}

 public:
	static DependencyProperty *OpacityProperty;
	static DependencyProperty *RelativeTransformProperty;
	static DependencyProperty *TransformProperty;
	
	// internal property - generic brush property change
	static DependencyProperty *ChangedProperty;
	
	Brush () { }
	virtual Type::Kind GetObjectType () { return Type::BRUSH; };
	
	virtual void SetupBrush (cairo_t *cr, UIElement *uielement);
	virtual void SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height);
	
	// returns true if OpacityProperty == 1.0.
	// subclasses override this to deal with their local coloring
	virtual bool IsOpaque ();
	
	// property accessors
	void SetOpacity (double opacity);
	double GetOpacity ();
	
	void SetRelativeTransform (Transform *transform);
	Transform *GetRelativeTransform ();
	
	void SetTransform (Transform *transform);
	Transform *GetTransform ();
};

Brush     *brush_new                    (void);
double     brush_get_opacity            (Brush *brush);
void       brush_set_opacity            (Brush *brush, double opacity);
Transform *brush_get_relative_transform (Brush *brush);
void       brush_set_relative_transform (Brush *brush, Transform *transform);
Transform *brush_get_transform          (Brush *brush);
void       brush_set_transform          (Brush *brush, Transform *transform);


class SolidColorBrush : public Brush {
 protected:
	virtual ~SolidColorBrush () {}
	
 public:
	static DependencyProperty *ColorProperty;
	
	SolidColorBrush () { };
	SolidColorBrush (const char *color);
	
	virtual Type::Kind GetObjectType () { return Type::SOLIDCOLORBRUSH; };

	virtual void SetupBrush (cairo_t *cr, UIElement *uielement);
	virtual void SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height);
	
	virtual bool IsOpaque ();
	
	// property accessors
	void SetColor (Color *color);
	Color *GetColor ();
};

SolidColorBrush	*solid_color_brush_new       (void);
Color           *solid_color_brush_get_color (SolidColorBrush *solid_color_brush);
void             solid_color_brush_set_color (SolidColorBrush *solid_color_brush, Color *color);


class GradientStopCollection : public Collection {
 protected:
	virtual ~GradientStopCollection () {}

 public:
	GradientStopCollection () {}
	virtual Type::Kind GetObjectType () { return Type::GRADIENTSTOP_COLLECTION; }

	virtual Type::Kind GetElementType() { return Type::GRADIENTSTOP; }
};

GradientStopCollection *gradient_stop_collection_new (void);


class GradientStop : public DependencyObject {
 protected:
	virtual ~GradientStop () {}
	
 public:
	static DependencyProperty *ColorProperty;
	static DependencyProperty *OffsetProperty;
	
	virtual Type::Kind GetObjectType () { return Type::GRADIENTSTOP; }
	
	// property accessors
	void SetColor (Color *color);
	Color *GetColor ();
	
	void SetOffset (double offset);
	double GetOffset ();
};

GradientStop *gradient_stop_new        (void);
Color        *gradient_stop_get_color  (GradientStop *stop);
void          gradient_stop_set_color  (GradientStop *stop, Color *color);
double        gradient_stop_get_offset (GradientStop *stop);
void          gradient_stop_set_offset (GradientStop *stop, double offset);


// note: abstract in C#
/* @ContentProperty="GradientStops" */
class GradientBrush : public Brush {
 protected:
	virtual ~GradientBrush () {}

 public:
	static DependencyProperty *ColorInterpolationModeProperty;
	static DependencyProperty *GradientStopsProperty;
	static DependencyProperty *MappingModeProperty;
	static DependencyProperty *SpreadMethodProperty;
	
	GradientBrush ();
	
	virtual Type::Kind GetObjectType () { return Type::GRADIENTBRUSH; }
	
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args);
	virtual void SetupGradient (cairo_pattern_t *pattern, UIElement *uielement, bool single = false);
	
	virtual bool IsOpaque ();
	
	// property accessors
	void SetColorInterpolationMode (ColorInterpolationMode mode);
	ColorInterpolationMode GetColorInterpolationMode ();
	
	void SetGradientStops (GradientStopCollection *collection);
	GradientStopCollection *GetGradientStops ();
	
	void SetMappingMode (BrushMappingMode mode);
	BrushMappingMode GetMappingMode ();
	
	void SetSpreadMethod (GradientSpreadMethod method);
	GradientSpreadMethod GetSpreadMethod ();
};

GradientBrush          *gradient_brush_new (void);
ColorInterpolationMode  gradient_brush_get_color_interpolation_mode (GradientBrush *brush);
void                    gradient_brush_set_color_interpolation_mode (GradientBrush *brush, ColorInterpolationMode mode);
GradientStopCollection *gradient_brush_get_gradient_stops           (GradientBrush *brush);
void                    gradient_brush_set_gradient_stops           (GradientBrush *brush, GradientStopCollection *collection);
BrushMappingMode        gradient_brush_get_mapping_mode             (GradientBrush *brush);
void                    gradient_brush_set_mapping_mode             (GradientBrush *brush, BrushMappingMode mode);
GradientSpreadMethod    gradient_brush_get_spread_method            (GradientBrush *brush);
void                    gradient_brush_set_spread_method            (GradientBrush *brush, GradientSpreadMethod method);


class LinearGradientBrush : public GradientBrush {
 protected:
	virtual ~LinearGradientBrush () {}

 public:
	static DependencyProperty *EndPointProperty;
	static DependencyProperty *StartPointProperty;

	LinearGradientBrush () {};
	virtual Type::Kind GetObjectType () { return Type::LINEARGRADIENTBRUSH; }

	virtual void SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height);
	
	// property accessors
	void SetEndPoint (Point *point);
	Point *GetEndPoint ();
	
	void SetStartPoint (Point *point);
	Point *GetStartPoint ();
};

LinearGradientBrush *linear_gradient_brush_new  (void);
Point	*linear_gradient_brush_get_end_point 	(LinearGradientBrush *brush);
void	linear_gradient_brush_set_end_point	(LinearGradientBrush *brush, Point *point);
Point	*linear_gradient_brush_get_start_point	(LinearGradientBrush *brush);
void	linear_gradient_brush_set_start_point	(LinearGradientBrush *brush, Point *point);


class RadialGradientBrush : public GradientBrush {
 protected:
	virtual ~RadialGradientBrush () {}

 public:
	static DependencyProperty *CenterProperty;
	static DependencyProperty *GradientOriginProperty;
	static DependencyProperty *RadiusXProperty;
	static DependencyProperty *RadiusYProperty;
	
	RadialGradientBrush () {};
	virtual Type::Kind GetObjectType () { return Type::RADIALGRADIENTBRUSH; }
	
	virtual void SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height);
	
	// property accessors
	void SetCenter (Point *center);
	Point *GetCenter ();
	
	void SetGradientOrigin (Point *origin);
	Point *GetGradientOrigin ();
	
	void SetRadiusX (double radius);
	double GetRadiusX ();
	
	void SetRadiusY (double radius);
	double GetRadiusY ();
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


class TileBrush : public Brush {
 protected:
	virtual ~TileBrush () {}

 public:
	static DependencyProperty *AlignmentXProperty;
	static DependencyProperty *AlignmentYProperty;
	static DependencyProperty *StretchProperty;
	
	virtual Type::Kind GetObjectType () { return Type::TILEBRUSH; }
	
	// property accessors
	void SetAlignmentX (AlignmentX alignment);
	AlignmentX GetAlignmentX ();
	
	void SetAlignmentY (AlignmentY alignment);
	AlignmentY GetAlignmentY ();
	
	void SetStretch (Stretch stretch);
	Stretch GetStretch ();
};

TileBrush*	tile_brush_new			(void);
AlignmentX	tile_brush_get_alignment_x	(TileBrush *brush);
void		tile_brush_set_alignment_x	(TileBrush *brush, AlignmentX alignment);
AlignmentY	tile_brush_get_alignment_y	(TileBrush *brush);
void		tile_brush_set_alignment_y	(TileBrush *brush, AlignmentY alignment);
Stretch		tile_brush_get_stretch		(TileBrush *brush);
void		tile_brush_set_stretch		(TileBrush *brush, Stretch stretch);


class ImageBrush : public TileBrush {
	static void image_progress_changed (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void image_failed (EventObject *sender, EventArgs *calldata, gpointer closure);

	Image *image;

 protected:
	virtual ~ImageBrush ();

 public:
	static DependencyProperty *DownloadProgressProperty;
	static DependencyProperty *ImageSourceProperty;
	
	const static int DownloadProgressChangedEvent;
	const static int ImageFailedEvent;
	
	ImageBrush ();
	
	virtual Type::Kind GetObjectType () { return Type::IMAGEBRUSH; }
	
	void SetSource (Downloader *downloader, const char *PartName);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height);
	virtual void SetSurface (Surface *surface);
	
	virtual bool IsOpaque ();
	
	// property accessors
	void SetDownloadProgress (double progress);
	double GetDownloadProgress ();
	
	void SetImageSource (const char *source);
	const char *GetImageSource ();
};

ImageBrush* image_brush_new (void);
double	image_brush_get_download_progress	(ImageBrush *brush);
void	image_brush_set_download_progress	(ImageBrush *brush, double progress);
const char *image_brush_get_image_source       	(ImageBrush *brush);
void	image_brush_set_image_source		(ImageBrush *brush, const char *source);
void	image_brush_set_source			(ImageBrush *brush, Downloader *downloader, const char *PartName);
cairo_surface_t *image_brush_create_similar     (cairo_t *cr, int width, int height);


class VideoBrush : public TileBrush {
private:
	MediaElement *media;

protected:
	virtual ~VideoBrush ();

public:
	static DependencyProperty *SourceNameProperty;
	
	VideoBrush ();
	
	virtual Type::Kind GetObjectType () { return Type::VIDEOBRUSH; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height);

	virtual bool IsOpaque ();
	
	// property accessors
	void SetSourceName (const char *name);
	const char *GetSourceName ();
};

VideoBrush *video_brush_new (void);
const char *video_brush_get_source_name (VideoBrush *brush);
void video_brush_set_source_name (VideoBrush *brush, const char *name);


class VisualBrush : public TileBrush {
 private:
	cairo_surface_t *surface;

	static void update_brush (EventObject *, EventArgs *, gpointer closure);

 protected:
	virtual ~VisualBrush () {}

 public:
	static DependencyProperty *VisualProperty;

	VisualBrush () { };

	virtual Type::Kind GetObjectType () { return Type::VISUALBRUSH; };

	virtual void SetupBrush (cairo_t *cr, UIElement *uielement, double width, double height);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	virtual bool IsOpaque ();
	
	void SetVisual (Visual *visual);
	Visual *GetVisual ();
};

VisualBrush	*visual_brush_new (void);
Visual          *visual_brush_get_visual (VisualBrush *brush);
void             visual_brush_set_visual (VisualBrush *brush, Visual *visual);


void brush_init (void);

G_END_DECLS

#endif /* __BRUSH_H__ */
