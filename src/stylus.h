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
#include "enums.h"
#include "canvas.h"
#include "collection.h"

namespace Moonlight {

/* @Namespace=System.Windows.Input */
class StylusInfo : public DependencyObject {
 protected:
	virtual ~StylusInfo () {}
	
 public:
	/* @PropertyType=TabletDeviceType,DefaultValue=TabletDeviceTypeMouse,GenerateAccessors */
	const static int DeviceTypeProperty;
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int IsInvertedProperty;
	
	/* @ManagedAccess=Internal,GeneratePInvoke */
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
/* @ManagedEvents=Manual */ // It's a managed struct
class StylusPoint : public DependencyObject {
 protected:
	virtual ~StylusPoint () {}
	
 public:
	/* @PropertyType=double,DefaultValue=0.5,GenerateAccessors */
	const static int PressureFactorProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int XProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int YProperty;
	
	/* @GeneratePInvoke,SkipFactories */
	StylusPoint () { SetObjectType (Type::STYLUSPOINT); }
	
	//
	// Property Accessors
	//
	/* @GeneratePInvoke */
	void SetPressureFactor (double factor);
	/* @GeneratePInvoke */
	double GetPressureFactor ();
	
	/* @GeneratePInvoke */
	void SetX (double x);
	/* @GeneratePInvoke */
	double GetX ();
	
	/* @GeneratePInvoke */
	void SetY (double y);
	/* @GeneratePInvoke */
	double GetY ();
};

/* @Namespace=System.Windows.Input */
class UnmanagedStylusPoint : public StylusPoint {

 protected:
	virtual ~UnmanagedStylusPoint () {}
	
 public:
	/* @GeneratePInvoke */
	UnmanagedStylusPoint () { SetObjectType (Type::UNMANAGEDSTYLUSPOINT); }
};

/* @Namespace=System.Windows.Input */
class StylusPointCollection : public DependencyObjectCollection {
 protected:
	virtual bool CanAdd (Value *value);
	
	virtual ~StylusPointCollection () {}
	
 public:
	/* @GeneratePInvoke */
	StylusPointCollection () { SetObjectType (Type::STYLUSPOINT_COLLECTION); }

	virtual Type::Kind GetElementType () { return Type::STYLUSPOINT; }
	
	/* @GeneratePInvoke */
	double AddStylusPoints (StylusPointCollection *stylusPointCollection);
	
	Rect GetBounds ();
};

/* @Namespace=System.Windows.Input */
class TouchDevice : public DependencyObject {
 protected:
	virtual ~TouchDevice () {}
	
 public:
	/* @PropertyType=UIElement,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int DirectlyOverProperty;
	/* @PropertyType=gint32,DefaultValue=0,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int IdProperty;
	
	/* @GeneratePInvoke */
	TouchDevice () { SetObjectType (Type::TOUCHDEVICE); }
	
	//
	// Property Accessors
	//
	
	/* @GeneratePInvoke */
	void SetDirectlyOver (UIElement *element);
	/* @GeneratePInvoke */
	UIElement *GetDirectlyOver ();
	
	/* @GeneratePInvoke */
	void SetId (int id);
	/* @GeneratePInvoke */
	int GetId ();
};

/* @Namespace=System.Windows.Input */
class TouchPoint : public DependencyObject {
 protected:
	virtual ~TouchPoint () {}
	
 public:
	/* @PropertyType=TouchAction,DefaultValue=0,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int ActionProperty;
	/* @PropertyType=Point,DefaultValue=Point(),ManagedSetterAccess=Internal,GenerateAccessors */
	const static int PositionProperty;
	/* @PropertyType=Size,DefaultValue=Size(),ManagedSetterAccess=Internal,GenerateAccessors */
	const static int SizeProperty;
	/* @PropertyType=TouchDevice,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int TouchDeviceProperty;
	
	/* @GeneratePInvoke */
	TouchPoint () { SetObjectType (Type::TOUCHPOINT); }
	
	//
	// Property Accessors
	//
	
	/* @GeneratePInvoke */
	void SetAction (TouchAction action);
	/* @GeneratePInvoke */
	TouchAction GetAction ();
	
	/* @GeneratePInvoke */
	void SetPosition (Point *position);
	/* @GeneratePInvoke */
	Point *GetPosition ();
	
	/* @GeneratePInvoke */
	void SetSize (Size *size);
	/* @GeneratePInvoke */
	Size *GetSize ();
	
	/* @GeneratePInvoke */
	void SetTouchDevice (TouchDevice *device);
	/* @GeneratePInvoke */
	TouchDevice *GetTouchDevice ();
};


/* @Namespace=System.Windows.Input */
class TouchPointCollection : public DependencyObjectCollection {
 protected:
	virtual ~TouchPointCollection () {}
	
 public:
	/* @GeneratePInvoke,ManagedAccess=Internal */
	TouchPointCollection () { SetObjectType (Type::TOUCHPOINT_COLLECTION); }

	virtual Type::Kind GetElementType () { return Type::TOUCHPOINT; }
};


/* @Namespace=System.Windows.Ink */
class DrawingAttributes : public DependencyObject {
 protected:
	virtual ~DrawingAttributes () {}

 public:
	/* @PropertyType=Color,DefaultValue=Color (0xFF000000),ManagedFieldAccess=Private,GenerateAccessors */
	const static int ColorProperty;
	/* @PropertyType=Color,DefaultValue=Color (0x00000000),ManagedFieldAccess=Private,GenerateAccessors */
	const static int OutlineColorProperty;
	/* @PropertyType=double,DefaultValue=3.0,ManagedFieldAccess=Private,GenerateAccessors */
	const static int HeightProperty;
	/* @PropertyType=double,DefaultValue=3.0,ManagedFieldAccess=Private,GenerateAccessors */
	const static int WidthProperty;
	
	/* @GeneratePInvoke */
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
	/* @PropertyType=DrawingAttributes,AutoCreateValue,ManagedFieldAccess=Private,GenerateAccessors */
	const static int DrawingAttributesProperty;
	/* @PropertyType=StylusPointCollection,AutoCreateValue,ManagedFieldAccess=Private,GenerateAccessors */
	const static int StylusPointsProperty;
	
	/* @GeneratePInvoke */
	Stroke ();
	/* @GeneratePInvoke */
	bool HitTest (StylusPointCollection *stylusPoints);
	
	Rect GetOldBounds () { return old_bounds; }
	Rect GetBounds () { return bounds; }
	
	void ResetDirty () { dirty = Rect (); }
	Rect GetDirty () { return dirty; }
	
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
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
	/* @GeneratePInvoke */
	StrokeCollection () { SetObjectType (Type::STROKE_COLLECTION); }
	
	virtual Type::Kind GetElementType () { return Type::STROKE; }
	
	virtual bool AddedToCollection (Value *value, MoonError *error);
	
	/* @GeneratePInvoke */
	StrokeCollection *HitTest (StylusPointCollection *stylusPoints);
	
	Rect GetBounds ();
};


/* @Namespace=System.Windows.Controls */
class InkPresenter : public Canvas {
	Rect render_bounds;
	
 protected:
	virtual ~InkPresenter () {}

	virtual void PostRender (Context *ctx, Region *region, bool skip_children);

 public:
	/* @PropertyType=StrokeCollection,AutoCreateValue,HiddenDefaultValue,GenerateAccessors */
	const static int StrokesProperty;
	
	/* @GeneratePInvoke */
	InkPresenter ();
	
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
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

/* @GeneratePInvoke */
MOON_API void stroke_get_bounds (Stroke *stroke, /* @MarshalAs=Rect,IsRef */ Rect *bounds);
/* @GeneratePInvoke */
MOON_API void stroke_collection_get_bounds (StrokeCollection *collection, /* @MarshalAs=Rect,IsRef */ Rect *bounds);

G_END_DECLS

};
#endif /* __STYLUS_H__ */
