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

#include "enums.h"
#include "collection.h"
#include "imagesource.h"

namespace Moonlight {

class MediaElement;

/* @Namespace=System.Windows.Media */
class Brush : public DependencyObject {
 protected:
	/* @GeneratePInvoke,ManagedAccess=Protected */
	Brush (): DependencyObject (Type::BRUSH) { }

	/* @SkipFactories */
	Brush (Type::Kind object_type) : DependencyObject (object_type) { }

	virtual ~Brush () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int OpacityProperty;
	/* @PropertyType=Transform,AutoCreator=Brush::CreateDefaultMatrixTransform,HiddenDefaultValue,GenerateAccessors */
	const static int RelativeTransformProperty;
	/* @PropertyType=Transform,AutoCreator=Brush::CreateDefaultMatrixTransform,HiddenDefaultValue,GenerateAccessors */
	const static int TransformProperty;
	
	// internal property - generic brush property change
	// used only for notifying attachees
	/* @PropertyType=bool,Access=Internal */
	const static int ChangedProperty;
	
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
	
	static Value *CreateDefaultMatrixTransform (Type::Kind kind, DependencyProperty *property, DependencyObject *forObj);
};


/* @Namespace=System.Windows.Media */
/* @ContentProperty="Color" */
class SolidColorBrush : public Brush {
 protected:
	/* @GeneratePInvoke */
	SolidColorBrush ();

	virtual ~SolidColorBrush () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=Color,DefaultValue=Color (0x00000000),GenerateAccessors */
	const static int ColorProperty;
	
	/* @SkipFactories */
	SolidColorBrush (const char *color);
	
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
	/* @GeneratePInvoke */
	GradientStopCollection ();

	virtual ~GradientStopCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	virtual Type::Kind GetElementType() { return Type::GRADIENTSTOP; }
};


/* @Namespace=System.Windows.Media */
class GradientStop : public DependencyObject {
 protected:
	/* @GeneratePInvoke */
	GradientStop ();
	
	virtual ~GradientStop ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=Color,DefaultValue=Color (0x00000000),GenerateAccessors */
	const static int ColorProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int OffsetProperty;
	
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
	/* @GeneratePInvoke,ManagedAccess=Protected */
	GradientBrush ();
	
	virtual ~GradientBrush ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=ColorInterpolationMode,DefaultValue=ColorInterpolationModeSRgbLinearInterpolation,GenerateAccessors */
	const static int ColorInterpolationModeProperty;
	/* @PropertyType=GradientStopCollection,AutoCreateValue,HiddenDefaultValue,GenerateAccessors */
	const static int GradientStopsProperty;
	/* @PropertyType=BrushMappingMode,DefaultValue=BrushMappingModeRelativeToBoundingBox,GenerateAccessors */
	const static int MappingModeProperty;
	/* @PropertyType=GradientSpreadMethod,DefaultValue=GradientSpreadMethodPad,GenerateAccessors */
	const static int SpreadMethodProperty;
	
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
	/* @GeneratePInvoke */
	LinearGradientBrush ();

	virtual ~LinearGradientBrush ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=Point,DefaultValue=Point(1\,1),GenerateAccessors */
	const static int EndPointProperty;
	/* @PropertyType=Point,DefaultValue=Point(),GenerateAccessors */
	const static int StartPointProperty;

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
	/* @GeneratePInvoke */
	RadialGradientBrush ();
	
	virtual ~RadialGradientBrush ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=Point,DefaultValue=Point (0.5\, 0.5),GenerateAccessors */
	const static int CenterProperty;
	/* @PropertyType=Point,DefaultValue=Point (0.5\, 0.5),GenerateAccessors */
	const static int GradientOriginProperty;
	/* @PropertyType=double,DefaultValue=0.5,GenerateAccessors */
	const static int RadiusXProperty;
	/* @PropertyType=double,DefaultValue=0.5,GenerateAccessors */
	const static int RadiusYProperty;
	
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
	/* @GeneratePInvoke,ManagedAccess=Protected */
	TileBrush () : Brush (Type::TILEBRUSH) {}

	virtual ~TileBrush ();

	/* @SkipFactories */
	TileBrush (Type::Kind object_type) : Brush (object_type) { }

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=AlignmentX,DefaultValue=AlignmentXCenter,GenerateAccessors */
	const static int AlignmentXProperty;
	/* @PropertyType=AlignmentY,DefaultValue=AlignmentYCenter,GenerateAccessors */
	const static int AlignmentYProperty;
	/* @PropertyType=Stretch,DefaultValue=StretchFill,GenerateAccessors */
	const static int StretchProperty;
	
	virtual void Fill (cairo_t *cr, bool preserve);
	virtual void Stroke (cairo_t *cr, bool preserve);
	
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
class ImplicitInputBrush : public Brush {
protected:
	/* @GeneratePInvoke */
	ImplicitInputBrush () : Brush (Type::IMPLICITINPUTBRUSH) { }

	virtual ~ImplicitInputBrush () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media */
class ImageBrush : public TileBrush {
private:
	void DownloadProgress ();
	void ImageOpened (RoutedEventArgs *args);
	void ImageFailed (ImageErrorEventArgs *args);
	void SourcePixelDataChanged ();

	static void download_progress (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void image_opened (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void image_failed (EventObject *sender, EventArgs *calldata, gpointer closure);
	static void source_pixel_data_changed (EventObject *sender, EventArgs *calldata, gpointer closure);

 protected:
	/* @GeneratePInvoke */
	ImageBrush ();

	virtual ~ImageBrush ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=double,DefaultValue=0.0,ManagedAccess=Private,GenerateAccessors */
	const static int DownloadProgressProperty;
 	/* @PropertyType=ImageSource,AutoCreator=ImageBrush::CreateDefaultImageSource,HiddenDefaultValue,GenerateAccessors */
	const static int ImageSourceProperty;
	
	/* @GenerateManagedEvent=false */
	const static int DownloadProgressChangedEvent;
	/* @DelegateType=EventHandler<ExceptionRoutedEventArgs> */
	const static int ImageFailedEvent;
	/* @DelegateType=EventHandler<RoutedEventArgs> */
	const static int ImageOpenedEvent;
	
	virtual void Dispose ();
	
	void SetSource (Downloader *downloader, const char *PartName);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void SetupBrush (cairo_t *cr, const Rect &area);
	
	virtual bool IsOpaque ();
	
	//
	// Property Accessors
	//
	void SetDownloadProgress (double progress);
	double GetDownloadProgress ();
	
	void SetImageSource (ImageSource *source);
	ImageSource *GetImageSource ();

	static Value *CreateDefaultImageSource (Type::Kind kind, DependencyProperty *property, DependencyObject *forObj);
};

cairo_surface_t *image_brush_create_similar     (cairo_t *cr, int width, int height);


/* @Namespace=System.Windows.Media */
class VideoBrush : public TileBrush {
	WeakRef<DependencyObject> source;
	
	static void update_brush (EventObject *, EventArgs *, gpointer closure);
	static void video_format_changed (EventObject *, EventArgs *, gpointer closure);

	void VideoFormatChanged (VideoFormatChangedEventArgs *args);
	
	void SetupBrushFromMediaElement (cairo_t *cr, const Rect &area);
	void SetupBrushFromCaptureSource (cairo_t *cr, const Rect &area);

	VideoFormat *video_format;

 protected:
	/* @GeneratePInvoke */
	VideoBrush ();
	
	virtual ~VideoBrush ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	/* @PropertyType=string,DefaultValue=\"\",GenerateAccessors */
	const static int SourceNameProperty;
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void SetupBrush (cairo_t *cr, const Rect &area);

	virtual bool IsOpaque ();
	virtual bool IsAnimating ();
	
	//
	// Methods
	//
	/* @GeneratePInvoke */
	void SetSource (DependencyObject *source);
	
	//
	// Property Accessors
	//
	void SetSourceName (const char *name);
	const char *GetSourceName ();

	const static void *SourceWeakRef;
};


/* @Namespace=None */
/* @ManagedDependencyProperties=None */
/* @ManagedEvents=None */
class VisualBrush : public TileBrush {
	cairo_surface_t *surface;

	static void update_brush (EventObject *, EventArgs *, gpointer closure);

 protected:
	/* @GeneratePInvoke */
	VisualBrush ();

	virtual ~VisualBrush ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* @PropertyType=UIElement,GenerateAccessors */
	const static int VisualProperty;
	
	virtual void SetupBrush (cairo_t *cr, const Rect &area);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	virtual bool IsOpaque ();
	
	//
	// Property Accessors
	//
	void SetVisual (UIElement *visual);
	UIElement *GetVisual ();
};

/* @Namespace=System.Windows.Controls */
class WebBrowserBrush : public TileBrush {
 public:
	/* @PropertyType=string,DefaultValue=\"\",GenerateAccessors */
	const static int SourceNameProperty;

	void SetSourceName (const char *name);
	const char * GetSourceName ();

 protected:
	/* @GeneratePInvoke */
	WebBrowserBrush () { SetObjectType (Type::WEBBROWSERBRUSH); }

	virtual ~WebBrowserBrush () { }

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

G_BEGIN_DECLS

void image_brush_compute_pattern_matrix (cairo_matrix_t *matrix, double width, double height, int sw, int sh, 
					 Stretch stretch, AlignmentX align_x, AlignmentY align_y, Transform *transform,
					 Transform *relative_transform);
					 
G_END_DECLS

};
#endif /* __BRUSH_H__ */
