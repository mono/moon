#ifndef MOON_STYLUS_H
#define MOON_STYLUS_H

#include "runtime.h"

G_BEGIN_DECLS

class StylusInfo : public DependencyObject {
 public:
	StylusInfo () { }
	~StylusInfo () { };
	
	virtual Type::Kind GetObjectType () { return Type::STYLUSINFO; };

	static DependencyProperty* DeviceTypeProperty;
	static DependencyProperty* IsInvertedProperty;
};

StylusInfo* stylus_info_new ();

class StylusPoint : public DependencyObject {
 public:
	StylusPoint () { }
	~StylusPoint () { };
	
	virtual Type::Kind GetObjectType () { return Type::STYLUSPOINT; };

	static DependencyProperty* PressureFactorProperty;
	static DependencyProperty* XProperty;
	static DependencyProperty* YProperty;
};

StylusPoint* stylus_point_new ();

class Stroke : public DependencyObject {
 public:
	Stroke () { }
	~Stroke () { };
	
	virtual Type::Kind GetObjectType () { return Type::STROKE; };

	static DependencyProperty* DrawingAttributesProperty;
	static DependencyProperty* StylusPointsProperty;
};

Stroke* stroke_new ();

class DrawingAttributes : public DependencyObject {
 public:
	DrawingAttributes () { }
	~DrawingAttributes () { };
	
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
	~InkPresenter () { };
	
	virtual Type::Kind GetObjectType () { return Type::INKPRESENTER; };

	static DependencyProperty* StrokesProperty;
};

InkPresenter* ink_presenter_new ();

G_END_DECLS

#endif /* MOON_STYLUS_H */
