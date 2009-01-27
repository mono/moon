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

#ifndef __STYLUS_H__
#define __STYLUS_H__

#include <glib.h>
#include "canvas.h"
#include "collection.h"

enum TabletDeviceType {
	TabletDeviceTypeMouse,
	TabletDeviceTypeStylus,
	TabletDeviceTypeTouch
};


/* @Namespace=None,ManagedDependencyProperties=None */
class StylusInfo : public DependencyObject {
 protected:
	virtual ~StylusInfo () {}
	
 public:
	/* @PropertyType=TabletDeviceType,DefaultValue=TabletDeviceTypeMouse,GenerateAccessors */
	static DependencyProperty *DeviceTypeProperty;
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	static DependencyProperty *IsInvertedProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	StylusInfo () { SetObjectType (Type::STYLUSINFO); }
	
	//
	// Property Accessors
	//
	void SetDeviceType (TabletDeviceType type);
	TabletDeviceType GetDeviceType ();
	
	void SetIsInverted (bool inverted);
	bool GetIsInverted ();
};


/* @Namespace=None */
/* @ManagedDependencyProperties=Manual */ // It's a managed struct
class StylusPoint : public DependencyObject {
 protected:
	virtual ~StylusPoint () {}
	
 public:
	/* @PropertyType=double,DefaultValue=0.5,GenerateAccessors */
	static DependencyProperty *PressureFactorProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *XProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *YProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	StylusPoint () { SetObjectType (Type::STYLUSPOINT); }
	
	//
	// Property Accessors
	//
	void SetPressureFactor (double factor);
	double GetPressureFactor ();
	
	void SetX (double x);
	double GetX ();
	
	void SetY (double y);
	double GetY ();
};


/* @Namespace=System.Windows.Input */
class StylusPointCollection : public DependencyObjectCollection {
 protected:
	virtual bool CanAdd (Value *value);
	
	virtual ~StylusPointCollection () {}
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	StylusPointCollection () { SetObjectType (Type::STYLUSPOINT_COLLECTION); }

	virtual Type::Kind GetElementType () { return Type::STYLUSPOINT; }
	
	double AddStylusPoints (StylusPointCollection *stylusPointCollection);
	
	Rect GetBounds ();
};


/* @Namespace=System.Windows.Ink */
class DrawingAttributes : public DependencyObject {
 protected:
	virtual ~DrawingAttributes () {}

 public:
	/* @PropertyType=Color,DefaultValue=Color (0xFF000000),ManagedFieldAccess=Private,GenerateAccessors */
	static DependencyProperty *ColorProperty;
	/* @PropertyType=Color,DefaultValue=Color (0x00000000),ManagedFieldAccess=Private,GenerateAccessors */
	static DependencyProperty *OutlineColorProperty;
	/* @PropertyType=double,DefaultValue=3.0,ManagedFieldAccess=Private,GenerateAccessors */
	static DependencyProperty *HeightProperty;
	/* @PropertyType=double,DefaultValue=3.0,ManagedFieldAccess=Private,GenerateAccessors */
	static DependencyProperty *WidthProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	DrawingAttributes () { SetObjectType (Type::DRAWINGATTRIBUTES); }
	
	void Render (cairo_t *cr, StylusPointCollection *collection);
	static void RenderWithoutDrawingAttributes (cairo_t *cr, StylusPointCollection *collection);
	
	//
	// Property Accessors
	//
	void SetColor (Color *color);
	Color *GetColor ();
	
	void SetOutlineColor (Color *color);
	Color *GetOutlineColor ();
	
	void SetHeight (double height);
	double GetHeight ();
	
	void SetWidth (double width);
	double GetWidth ();
};


/* @Namespace=System.Windows.Ink */
class Stroke : public DependencyObject {
	Rect old_bounds;
	Rect bounds;
	Rect dirty;
	
	Rect AddStylusPointToBounds (StylusPoint *stylus_point, const Rect &bounds);
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
	/* @PropertyType=DrawingAttributes,ManagedFieldAccess=Private,GenerateAccessors */
	static DependencyProperty *DrawingAttributesProperty;
	/* @PropertyType=StylusPointCollection,ManagedFieldAccess=Private,GenerateAccessors */
	static DependencyProperty *StylusPointsProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Stroke ();

	/* @GenerateCBinding,GeneratePInvoke */
	bool HitTest (StylusPointCollection *stylusPoints);
	
	Rect GetOldBounds () { return old_bounds; }
	Rect GetBounds () { return bounds; }
	
	void ResetDirty () { dirty = Rect (); }
	Rect GetDirty () { return dirty; }
	
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	
	//
	// Property Accessors
	//
	void SetDrawingAttributes (DrawingAttributes *attrs);
	DrawingAttributes *GetDrawingAttributes ();
	
	void SetStylusPoints (StylusPointCollection *points);
	StylusPointCollection *GetStylusPoints ();
};


/* @Namespace=System.Windows.Ink */
class StrokeCollection : public DependencyObjectCollection {
 protected:
	virtual bool CanAdd (Value *value);
	
	virtual ~StrokeCollection () {}
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	StrokeCollection () { SetObjectType (Type::STROKE_COLLECTION); }
	
	virtual Type::Kind GetElementType () { return Type::STROKE; }
	
	virtual bool AddedToCollection (Value *value, MoonError *error);
	
	/* @GenerateCBinding,GeneratePInvoke */
	StrokeCollection *HitTest (StylusPointCollection *stylusPoints);
	
	Rect GetBounds ();
};


/* @Namespace=System.Windows.Controls */
class InkPresenter : public Canvas {
	Rect render_bounds;
	
 protected:
	virtual ~InkPresenter () {}

 public:
	/* @PropertyType=StrokeCollection,GenerateAccessors */
	static DependencyProperty *StrokesProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	InkPresenter ();
	
	virtual void PostRender (cairo_t *cr, Region *region, bool front_to_back);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	virtual void ComputeBounds ();

	virtual Rect GetRenderBounds ();

	virtual void ShiftPosition (Point p);
	
	//
	// Property Accessors
	//
	void SetStrokes (StrokeCollection *strokes);
	StrokeCollection *GetStrokes ();
};


G_BEGIN_DECLS

void stroke_get_bounds (Stroke *stroke, Rect *bounds);
void stroke_collection_get_bounds (StrokeCollection *collection, Rect *bounds);

G_END_DECLS

#endif /* __STYLUS_H__ */
