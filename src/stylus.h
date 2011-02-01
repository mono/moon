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
#include "moonbuild.h"
#include "canvas.h"
#include "collection.h"

enum TabletDeviceType {
	TabletDeviceTypeMouse,
	TabletDeviceTypeStylus,
	TabletDeviceTypeTouch
};

enum TouchAction {
	TouchActionDown = 1,
	TouchActionMove = 2,
	TouchActionUp   = 3
};

/* @Namespace=System.Windows.Input */
class MOON_API StylusInfo : public DependencyObject {
 protected:
	virtual ~StylusInfo () {}
	
 public:
	/* @PropertyType=TabletDeviceType,DefaultValue=TabletDeviceTypeMouse,GenerateAccessors */
	const static int DeviceTypeProperty;
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int IsInvertedProperty;
	
	/* @ManagedAccess=Internal,GenerateCBinding,GeneratePInvoke */
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
	
	/* @GenerateCBinding,GeneratePInvoke */
	StylusPoint () { SetObjectType (Type::STYLUSPOINT); }
	
	//
	// Property Accessors
	//
	/* @GenerateCBinding,GeneratePInvoke */
	void SetPressureFactor (double factor);
	/* @GenerateCBinding,GeneratePInvoke */
	double GetPressureFactor ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetX (double x);
	/* @GenerateCBinding,GeneratePInvoke */
	double GetX ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetY (double y);
	/* @GenerateCBinding,GeneratePInvoke */
	double GetY ();
};


/* @Namespace=System.Windows.Input */
class MOON_API StylusPointCollection : public DependencyObjectCollection {
 protected:
	virtual bool CanAdd (Value *value);
	
	virtual ~StylusPointCollection () {}
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	StylusPointCollection () { SetObjectType (Type::STYLUSPOINT_COLLECTION); }

	virtual Type::Kind GetElementType () { return Type::STYLUSPOINT; }
	
	/* @GenerateCBinding,GeneratePInvoke */
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
	/* @PropertyType=gint32,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int IdProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	TouchDevice () { SetObjectType (Type::TOUCHDEVICE); }
	
	//
	// Property Accessors
	//
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetDirectlyOver (UIElement *element);
	/* @GenerateCBinding,GeneratePInvoke */
	UIElement *GetDirectlyOver ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetId (int id);
	/* @GenerateCBinding,GeneratePInvoke */
	int GetId ();
};

/* @Namespace=System.Windows.Input */
class TouchPoint : public DependencyObject {
 protected:
	virtual ~TouchPoint () {}
	
 public:
	/* @PropertyType=TouchAction,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int ActionProperty;
	/* @PropertyType=Point,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int PositionProperty;
	/* @PropertyType=Size,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int SizeProperty;
	/* @PropertyType=TouchDevice,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int TouchDeviceProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	TouchPoint () { SetObjectType (Type::TOUCHPOINT); }
	
	//
	// Property Accessors
	//
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetAction (TouchAction action);
	/* @GenerateCBinding,GeneratePInvoke */
	TouchAction GetAction ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetPosition (Point *position);
	/* @GenerateCBinding,GeneratePInvoke */
	Point *GetPosition ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetSize (Size *size);
	/* @GenerateCBinding,GeneratePInvoke */
	Size *GetSize ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetTouchDevice (TouchDevice *device);
	/* @GenerateCBinding,GeneratePInvoke */
	TouchDevice *GetTouchDevice ();
};


/* @Namespace=System.Windows.Input */
class TouchPointCollection : public DependencyObjectCollection {
 protected:
	virtual ~TouchPointCollection () {}
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
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
class MOON_API Stroke : public DependencyObject {
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
class MOON_API StrokeCollection : public DependencyObjectCollection {
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

	virtual void PostRender (cairo_t *cr, Region *region, bool front_to_back);

 public:
	/* @PropertyType=StrokeCollection,AutoCreateValue,GenerateAccessors */
	const static int StrokesProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
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
void stroke_get_bounds (Stroke *stroke, /* @MarshalAs=Rect,IsRef */ Rect *bounds) MOON_API;
/* @GeneratePInvoke */
void stroke_collection_get_bounds (StrokeCollection *collection, /* @MarshalAs=Rect,IsRef */ Rect *bounds) MOON_API;

G_END_DECLS

#endif /* __STYLUS_H__ */
