/*
 * stylus.h
 *
 * Author:
 *   Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_STYLUS_H
#define MOON_STYLUS_H

#include "canvas.h"

G_BEGIN_DECLS

class StylusInfo : public DependencyObject {
 public:
	StylusInfo () { }
	virtual ~StylusInfo () { };
	
	virtual Type::Kind GetObjectType () { return Type::STYLUSINFO; };

	static DependencyProperty* DeviceTypeProperty;
	static DependencyProperty* IsInvertedProperty;
};

StylusInfo* stylus_info_new ();

class StylusPoint : public DependencyObject {
 public:
	StylusPoint () { }
	virtual ~StylusPoint () { };
	
	virtual Type::Kind GetObjectType () { return Type::STYLUSPOINT; };

	static DependencyProperty* PressureFactorProperty;
	static DependencyProperty* XProperty;
	static DependencyProperty* YProperty;
};

StylusPoint* stylus_point_new ();

class Stroke : public DependencyObject {
 public:
	Stroke () { }
	virtual ~Stroke () { };
	
	virtual Type::Kind GetObjectType () { return Type::STROKE; };

	static DependencyProperty* DrawingAttributesProperty;
	static DependencyProperty* StylusPointsProperty;
};

Stroke* stroke_new ();

class DrawingAttributes : public DependencyObject {
 public:
	DrawingAttributes () { }
	virtual ~DrawingAttributes () { };
	
	virtual Type::Kind GetObjectType () { return Type::DRAWINGATTRIBUTES; };

	static DependencyProperty* ColorProperty;
	static DependencyProperty* OutlineColorProperty;
	static DependencyProperty* HeightProperty;
	static DependencyProperty* WidthProperty;
};

DrawingAttributes* drawing_attributes_new ();

class InkPresenter : public Canvas {
 public:
	InkPresenter () { }
	virtual ~InkPresenter () { };
	
	virtual Type::Kind GetObjectType () { return Type::INKPRESENTER; };

	static DependencyProperty* StrokesProperty;
};

InkPresenter* ink_presenter_new ();

void stylus_init (void);

G_END_DECLS

#endif /* MOON_STYLUS_H */
