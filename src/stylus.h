/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * stylus.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_STYLUS_H
#define MOON_STYLUS_H

#include <glib.h>
#include "canvas.h"
#include "collection.h"

G_BEGIN_DECLS

enum TabletDeviceType {
	TabletDeviceTypeMouse,
	TabletDeviceTypeStylus,
	TabletDeviceTypeTouch
};


class StylusInfo : public DependencyObject {
 protected:
	virtual ~StylusInfo () {}
	
 public:
	/* @PropertyType=gint32,DefaultValue=TabletDeviceTypeMouse */
	static DependencyProperty *DeviceTypeProperty;
	/* @PropertyType=bool,DefaultValue=false */
	static DependencyProperty *IsInvertedProperty;
	
	/* @GenerateCBinding */
	StylusInfo () { }
	
	virtual Type::Kind GetObjectType () { return Type::STYLUSINFO; };
};

TabletDeviceType stylus_info_get_device_type (StylusInfo *stylus_info);
void	stylus_info_set_device_type	(StylusInfo *stylus_info, TabletDeviceType type);

bool	stylus_info_get_inverted	(StylusInfo *stylus_info);
void	stylus_info_set_inverted	(StylusInfo *stylus_info, bool inverted);


class StylusPoint : public DependencyObject {
 protected:
	virtual ~StylusPoint () {}
	
 public:
	/* @PropertyType=double,DefaultValue=0.5 */
	static DependencyProperty *PressureFactorProperty;
	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *XProperty;
	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *YProperty;
	
	/* @GenerateCBinding */
	StylusPoint () { }
	
	virtual Type::Kind GetObjectType () { return Type::STYLUSPOINT; };
};

double	stylus_point_get_x (StylusPoint *stylus_point);
void	stylus_point_set_x (StylusPoint *stylus_point, double x);

double	stylus_point_get_y (StylusPoint *stylus_point);
void	stylus_point_set_y (StylusPoint *stylus_point, double y);

double	stylus_point_get_pressure_factor (StylusPoint *stylus_point);
void	stylus_point_set_pressure_factor (StylusPoint *stylus_point, double pressure);


class StylusPointCollection : public DependencyObjectCollection {
 protected:
	virtual bool CanAdd (Value *value) { return !Contains (value); }
	
	virtual ~StylusPointCollection () {}
	
 public:
	/* @GenerateCBinding */
	StylusPointCollection () {}
	
	virtual Type::Kind GetObjectType () { return Type::STYLUSPOINT_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::STYLUSPOINT; }
	
	double AddStylusPoints (StylusPointCollection *stylusPointCollection);
	
	Rect GetBounds ();
};

double stylus_point_collection_add_stylus_points (StylusPointCollection *col, StylusPointCollection *stylusPointCollection);


class DrawingAttributes : public DependencyObject {
 protected:
	virtual ~DrawingAttributes () {}

 public:
	/* @PropertyType=Color,DefaultValue=Color (0xFF000000) */
	static DependencyProperty *ColorProperty;
	/* @PropertyType=Color,DefaultValue=Color (0x00000000) */
	static DependencyProperty *OutlineColorProperty;
	/* @PropertyType=double,DefaultValue=3.0 */
	static DependencyProperty *HeightProperty;
	/* @PropertyType=double,DefaultValue=3.0 */
	static DependencyProperty *WidthProperty;
	
	/* @GenerateCBinding */
	DrawingAttributes () { }
	
	virtual Type::Kind GetObjectType () { return Type::DRAWINGATTRIBUTES; };

	void Render (cairo_t *cr, StylusPointCollection* collection);
	static void RenderWithoutDrawingAttributes (cairo_t *cr, StylusPointCollection* collection);
};

Color  *drawing_attributes_get_color (DrawingAttributes *da);
void	drawing_attributes_set_color (DrawingAttributes *da, Color *color);

Color  *drawing_attributes_get_outline_color (DrawingAttributes *da);
void	drawing_attributes_set_outline_color (DrawingAttributes *da, Color *color);

double	drawing_attributes_get_height (DrawingAttributes *da);
void	drawing_attributes_set_height (DrawingAttributes *da, double height);

double	drawing_attributes_get_width (DrawingAttributes *da);
void	drawing_attributes_set_width (DrawingAttributes *da, double width);


class Stroke : public DependencyObject {
	Rect old_bounds;
	Rect bounds;
	
	void AddStylusPointToBounds (StylusPoint *stylus_point);
	void ComputeBounds ();

	bool HitTestEndcapSegment (Point c, double w, double h, Point p1, Point p2);
	bool HitTestEndcapPoint (Point c, double w, double h, Point p1);
	bool HitTestEndcap (Point p, double w, double h, StylusPointCollection *stylusPoints);

	bool HitTestSegmentSegment (Point stroke_p1, Point stroke_p2, double w, double h, Point p1, Point p2);
	bool HitTestSegmentPoint (Point stroke_p1, Point stroke_p2, double w, double h, Point p1);
	bool HitTestSegment (Point stroke_p1, Point stroke_p2, double w, double h, StylusPointCollection *stylusPoints);
	
 protected:
	virtual ~Stroke () {}
	
 public:
	/* @PropertyType=DrawingAttributes */
	static DependencyProperty *DrawingAttributesProperty;
	/* @PropertyType=StylusPointCollection */
	static DependencyProperty *StylusPointsProperty;
	
	/* @GenerateCBinding */
	Stroke ();

	virtual Type::Kind GetObjectType () { return Type::STROKE; };

	Rect GetBounds ();
	Rect GetOldBounds ();
	bool HitTest (StylusPointCollection *stylusPoints);
	
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
};

DrawingAttributes     *stroke_get_drawing_attributes (Stroke *stroke);
void                   stroke_set_drawing_attributes (Stroke *stroke, DrawingAttributes *attributes);

StylusPointCollection *stroke_get_stylus_points (Stroke *stroke);
void                   stroke_set_stylus_points (Stroke *stroke, StylusPointCollection *collection);

void                   stroke_get_bounds (Stroke *stroke, Rect *bounds);
bool                   stroke_hit_test (Stroke *stroke, StylusPointCollection *stylusPointCollection);


class StrokeCollection : public DependencyObjectCollection {
 protected:
	virtual bool CanAdd (Value *value) { return !Contains (value); }
	
	virtual ~StrokeCollection () {}
	
 public:
	/* @GenerateCBinding */
	StrokeCollection () {}
	virtual Type::Kind GetObjectType () { return Type::STROKE_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::STROKE; }
	
	Rect GetBounds ();
	StrokeCollection *HitTest (StylusPointCollection *stylusPoints);
};

void              stroke_collection_get_bounds (StrokeCollection *col, Rect *bounds);
StrokeCollection *stroke_collection_hit_test (StrokeCollection *col, StylusPointCollection *stylusPointCollection);


class InkPresenter : public Canvas {
	Rect render_bounds;
	
 protected:
	virtual ~InkPresenter () {}

 public:
	/* @PropertyType=StrokeCollection */
	static DependencyProperty *StrokesProperty;
	
	/* @GenerateCBinding */
	InkPresenter ();
	
	virtual Type::Kind GetObjectType () { return Type::INKPRESENTER; };

	virtual void PostRender (cairo_t *cr, Region *region, bool front_to_back);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	virtual void ComputeBounds ();

	virtual Rect GetRenderBounds ();

	virtual void ShiftPosition (Point p);
};

StrokeCollection *ink_presenter_get_strokes (InkPresenter *ink_presenter);
void ink_presenter_set_strokes (InkPresenter *ink_presenter, StrokeCollection *collection);

G_END_DECLS

#endif /* MOON_STYLUS_H */
