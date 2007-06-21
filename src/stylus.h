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


G_END_DECLS

#endif /* MOON_STYLUS_H */
