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


/* @Namespace=System.Windows.Media */
class Brush : public DependencyObject {
 protected:
	virtual ~Brush () {}

 public:
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	static DependencyProperty *OpacityProperty;
	/* @PropertyType=Transform,GenerateAccessors */
	static DependencyProperty *RelativeTransformProperty;
	/* @PropertyType=Transform,GenerateAccessors */
	static DependencyProperty *TransformProperty;
	
	// internal property - generic brush property change
	// used only for notifying attachees
	/* @PropertyType=bool,Access=Internal */
	static DependencyProperty *ChangedProperty;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Brush () { }
	
	virtual Type::Kind GetObjectType () { return Type::BRUSH; };
	
	virtual void SetupBrush (cairo_t *cr, const Rect &area);

	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);

	virtual void Fill (cairo_t *cr, bool preserve = FALSE);
	virtual void Stroke (cairo_t *cr, bool preserve = FALSE);

	// returns true if OpacityProperty == 1.0.
	// subclasses override this to deal with their local coloring
	virtual bool IsOpaque ();

	// returns true if the brush is expected to change each render
	// pass. subclasses should override this to handle
	virtual bool IsAnimating ();

	//
	// Property Accessors
	//
	void SetOpacity (double opacity);
	double GetOpacity ();
	
	void SetRelativeTransform (Transform *transform);
	Transform *GetRelativeTransform ();
	
	void SetTransform (Transform *transform);
	Transform *GetTransform ();
};


/* @Namespace=System.Windows.Media */
class SolidColorBrush : public Brush {
 protected:
	virtual ~SolidColorBrush () {}
	
 public:
	/* @PropertyType=Color,DefaultValue=Color (0x00000000),GenerateAccessors */
	static DependencyProperty *ColorProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	SolidColorBrush () { }
	SolidColorBrush (const char *color);
	
	virtual Type::Kind GetObjectType () { return Type::SOLIDCOLORBRUSH; };

	virtual void SetupBrush (cairo_t *cr, const Rect &area);
	
	virtual bool IsOpaque ();
	
	//
	// Property Accessors
	//
	void SetColor (Color *color);
	Color *GetColor ();
};


/* @Namespace=System.Windows.Media */
class GradientStopCollection : public DependencyObjectCollection {
 protected:
	virtual ~GradientStopCollection () {}

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	GradientStopCollection () { }
	
	virtual Type::Kind GetObjectType () { return Type::GRADIENTSTOP_COLLECTION; }
	virtual Type::Kind GetElementType() { return Type::GRADIENTSTOP; }
};


/* @Namespace=System.Windows.Media */
class GradientStop : public DependencyObject {
 protected:
	virtual ~GradientStop () {}
	
 public:
	/* @PropertyType=Color,DefaultValue=Color (0x00000000),GenerateAccessors */
	static DependencyProperty *ColorProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *OffsetProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	GradientStop () { }
	
	virtual Type::Kind GetObjectType () { return Type::GRADIENTSTOP; }
	
	//
	// Property Accessors
	//
	void SetColor (Color *color);
	Color *GetColor ();
	
	void SetOffset (double offset);
	double GetOffset ();
};


// note: abstract in C#
/* @Namespace=System.Windows.Media */
/* @ContentProperty="GradientStops" */
class GradientBrush : public Brush {
 protected:
	virtual ~GradientBrush () {}

 public:
	/* @PropertyType=ColorInterpolationMode,DefaultValue=ColorInterpolationModeSRgbLinearInterpolation,GenerateAccessors */
	static DependencyProperty *ColorInterpolationModeProperty;
	/* @PropertyType=GradientStopCollection,GenerateAccessors */
	static DependencyProperty *GradientStopsProperty;
	/* @PropertyType=BrushMappingMode,DefaultValue=BrushMappingModeRelativeToBoundingBox,GenerateAccessors */
	static DependencyProperty *MappingModeProperty;
	/* @PropertyType=GradientSpreadMethod,DefaultValue=GradientSpreadMethodPad,GenerateAccessors */
	static DependencyProperty *SpreadMethodProperty;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	GradientBrush ();
	
	virtual Type::Kind GetObjectType () { return Type::GRADIENTBRUSH; }
	
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void SetupGradient (cairo_pattern_t *pattern, const Rect &area, bool single = false);
	
	virtual bool IsOpaque ();
	
	//
	// Property Accessors
	//
	void SetColorInterpolationMode (ColorInterpolationMode mode);
	ColorInterpolationMode GetColorInterpolationMode ();
	
	void SetGradientStops (GradientStopCollection *collection);
	GradientStopCollection *GetGradientStops ();
	
	void SetMappingMode (BrushMappingMode mode);
	BrushMappingMode GetMappingMode ();
	
	void SetSpreadMethod (GradientSpreadMethod method);
	GradientSpreadMethod GetSpreadMethod ();
};


/* @Namespace=System.Windows.Media */
class LinearGradientBrush : public GradientBrush {
 protected:
	virtual ~LinearGradientBrush () {}

 public:
	/* @PropertyType=Point,GenerateAccessors */
	static DependencyProperty *EndPointProperty;
	/* @PropertyType=Point,GenerateAccessors */
	static DependencyProperty *StartPointProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	LinearGradientBrush () { }
	
	virtual Type::Kind GetObjectType () { return Type::LINEARGRADIENTBRUSH; }

	virtual void SetupBrush (cairo_t *cr, const Rect &area);
	
	//
	// Property Accessors
	//
	void SetEndPoint (Point *point);
	Point *GetEndPoint ();
	
	void SetStartPoint (Point *point);
	Point *GetStartPoint ();
};


/* @Namespace=System.Windows.Media */
class RadialGradientBrush : public GradientBrush {
 protected:
	virtual ~RadialGradientBrush () {}

 public:
	/* @PropertyType=Point,GenerateAccessors */
	static DependencyProperty *CenterProperty;
	/* @PropertyType=Point,GenerateAccessors */
	static DependencyProperty *GradientOriginProperty;
	/* @PropertyType=double,DefaultValue=0.5,GenerateAccessors */
	static DependencyProperty *RadiusXProperty;
	/* @PropertyType=double,DefaultValue=0.5,GenerateAccessors */
	static DependencyProperty *RadiusYProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	RadialGradientBrush () { }
	
	virtual Type::Kind GetObjectType () { return Type::RADIALGRADIENTBRUSH; }
	
	virtual void SetupBrush (cairo_t *cr, const Rect &area);
	
	//
	// Property Accessors
	//
	void SetCenter (Point *center);
	Point *GetCenter ();
	
	void SetGradientOrigin (Point *origin);
	Point *GetGradientOrigin ();
	
	void SetRadiusX (double radius);
	double GetRadiusX ();
	
	void SetRadiusY (double radius);
	double GetRadiusY ();
};


/* @Namespace=System.Windows.Media */
class TileBrush : public Brush {
 protected:
	virtual ~TileBrush () {}

 public:
	/* @PropertyType=AlignmentX,DefaultValue=AlignmentXCenter,GenerateAccessors */
	static DependencyProperty *AlignmentXProperty;
	/* @PropertyType=AlignmentY,DefaultValue=AlignmentYCenter,GenerateAccessors */
	static DependencyProperty *AlignmentYProperty;
	/* @PropertyType=Stretch,DefaultValue=StretchFill,GenerateAccessors */
	static DependencyProperty *StretchProperty;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	TileBrush () { }
	virtual void Fill (cairo_t *cr, bool preserve);
	virtual void Stroke (cairo_t *cr, bool preserve);
	
	virtual Type::Kind GetObjectType () { return Type::TILEBRUSH; }

	//
	// Property Accessors
	//
	void SetAlignmentX (AlignmentX alignment);
	AlignmentX GetAlignmentX ();
	
	void SetAlignmentY (AlignmentY alignment);
	AlignmentY GetAlignmentY ();
	
	void SetStretch (Stretch stretch);
	Stretch GetStretch ();
};


/* @Namespace=System.Windows.Media */
class ImageBrush : public TileBrush {
	static void image_progress_changed (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void image_failed (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void target_unloaded (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void target_loaded (EventObject *sender, EventArgs *calldata, gpointer closure);
	
	void TargetUnloaded ();
	void TargetLoaded ();
	
	int loaded_count;
	Image *image;
	
 protected:
	virtual ~ImageBrush ();
	
 public:
	/* @PropertyType=double,DefaultValue=0.0,ManagedAccess=Private,GenerateAccessors */
	static DependencyProperty *DownloadProgressProperty;
 	/* @PropertyType=string,DefaultValue=\"\",ManagedPropertyType=ImageSource,GenerateAccessors */
	static DependencyProperty *ImageSourceProperty;
	
	const static int DownloadProgressChangedEvent;
	const static int ImageFailedEvent;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ImageBrush ();
	
	virtual Type::Kind GetObjectType () { return Type::IMAGEBRUSH; }
	
	void SetSource (Downloader *downloader, const char *PartName);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void SetupBrush (cairo_t *cr, const Rect &area);
	virtual void RemoveTarget (DependencyObject *obj);
	virtual void AddTarget (DependencyObject *obj);
	virtual void SetSurface (Surface *surface);
	
	virtual bool IsOpaque ();
	
	//
	// Property Accessors
	//
	void SetDownloadProgress (double progress);
	double GetDownloadProgress ();
	
	void SetImageSource (const char *source);
	const char *GetImageSource ();
};

cairo_surface_t *image_brush_create_similar     (cairo_t *cr, int width, int height);


/* @Namespace=System.Windows.Media */
class VideoBrush : public TileBrush {
	MediaElement *media;
	
 protected:
	virtual ~VideoBrush ();
	
 public:
	/* @PropertyType=string,DefaultValue=\"\",GenerateAccessors */
	static DependencyProperty *SourceNameProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	VideoBrush ();
	
	virtual Type::Kind GetObjectType () { return Type::VIDEOBRUSH; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void SetupBrush (cairo_t *cr, const Rect &area);

	virtual bool IsOpaque ();
	virtual bool IsAnimating ();
	
	//
	// Property Accessors
	//
	void SetSourceName (const char *name);
	const char *GetSourceName ();
};


/* @Namespace=None */
/* @ManagedDependencyProperties=None */
class VisualBrush : public TileBrush {
	cairo_surface_t *surface;

	static void update_brush (EventObject *, EventArgs *, gpointer closure);

 protected:
	virtual ~VisualBrush () {}

 public:
	/* @PropertyType=UIElement,GenerateAccessors */
	static DependencyProperty *VisualProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	VisualBrush () { }
	
	virtual Type::Kind GetObjectType () { return Type::VISUALBRUSH; };

	virtual void SetupBrush (cairo_t *cr, const Rect &area);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	virtual bool IsOpaque ();
	
	//
	// Property Accessors
	//
	void SetVisual (UIElement *visual);
	UIElement *GetVisual ();
};

G_END_DECLS

#endif /* __BRUSH_H__ */
