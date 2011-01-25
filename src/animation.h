/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * animation.h: Animation engine
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_ANIMATION_H
#define MOON_ANIMATION_H

#include <glib.h>

#include "trigger.h"
#include "collection.h"
#include "clock.h"
#include "timeline.h"
#include "list.h"
#include "point.h"
#include "propertypath.h"
#include "moon-curves.h"
#include "easing.h"
#include "applier.h"

namespace Moonlight {

/* @CBindingRequisite */
typedef void (*FlattenTimelinesCallback)(const Timeline *timeline, const DependencyObject *dep_ob, const DependencyProperty *prop);

// misc types
/* @Namespace=System.Windows.Media.Animation */
class KeySpline : public DependencyObject {
public:
	/* @SkipFactories */
	KeySpline (Point controlPoint1, Point controlPoint2);
	/* @SkipFactories */
	KeySpline (double x1, double y1, double x2, double y2);

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	void RegenerateQuadratics ();

	double GetSplineProgress (double linearProgress);
	/* @PropertyType=Point,ManagedPropertyType=Point,DefaultValue=Point (0\,0),ManagedFieldAccess=Internal,GenerateAccessors */
	const static int ControlPoint1Property;
	/* @PropertyType=Point,ManagedPropertyType=Point,DefaultValue=Point (1.0\, 1.0),ManagedFieldAccess=Internal,GenerateAccessors */
	const static int ControlPoint2Property;

	Point *GetControlPoint1 ();
	void SetControlPoint1 (Point *controlPoint1);

	Point *GetControlPoint2 ();
	void SetControlPoint2 (Point *controlPoint2);
	
protected:
	/* @GeneratePInvoke */
	KeySpline ();
	
	virtual ~KeySpline ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:
	moon_quadratic *quadraticsArray;
};

/* @Namespace=System.Windows.Media.Animation */
/* @IncludeInKinds */
struct KeyTime {
	enum KeyTimeType {
		UNIFORM,
		PERCENT,
		TIMESPAN,
		PACED
	};

	KeyTime (const KeyTime &keytime)
	{
		k = keytime.k;
		percent = keytime.percent;
		timespan = keytime.timespan;
		padding = keytime.padding;
	}

	KeyTime (double percent)
		: k (PERCENT),
		  padding(1),
		  percent (percent),
		  timespan (0)
	{
	}

	KeyTime (TimeSpan timeSpan)
		: k (TIMESPAN),
		  padding(1),
		  percent (0.0),
		  timespan (timeSpan)
	{
	}


	KeyTime (KeyTimeType kind)
		: k(kind),
		  padding (1),
		  percent (0.0),
		  timespan (0)
	{
	}

	// this ctor creates an invalid KeyTime.  it's used when coercing a null value to a keytime
	KeyTime ()
		: k (UNIFORM),
		  padding(0),
		  percent (0.0),
		  timespan (0)
	{
	}

	static KeyTime FromPercent (double percent) { return KeyTime (percent); }
	static KeyTime FromTimeSpan (TimeSpan timeSpan) { return KeyTime (timeSpan); }

	static KeyTime Paced;
	static KeyTime Uniform;

	bool operator!= (const KeyTime &v) const
	{
		return !(*this == v);
	}

	bool operator== (const KeyTime &v) const
	{
		if (v.k != k)
			return false;

		switch (k) {
		case PERCENT: return percent == v.percent;
		case TIMESPAN: return timespan == v.timespan;
		default: return true;
		}
	}

	bool HasPercent () { return k == PERCENT; }
	double GetPercent () { return percent; }

	bool HasTimeSpan () { return k == TIMESPAN; }
	TimeSpan GetTimeSpan () { return timespan; }

	bool IsValid () { return padding == 1; }

private:
	KeyTimeType k;
	gint32 padding;
	double percent;
	TimeSpan timespan;
};


//
// Animations (more specialized clocks and timelines) and their subclasses
//
class AnimationClock;


/* @Namespace=None */
class Animation : public Timeline {
public:

	Animation ();
	
	virtual Clock *AllocateClock ();

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);
	virtual Value *GetTargetValue (Value *defaultOriginValue);

	virtual Duration GetNaturalDurationCore (Clock* clock);


	virtual bool Resolve (DependencyObject *target, DependencyProperty *property) { return true; };

	/* The kind of values this animation generates */
	virtual Type::Kind GetValueKind () { return Type::INVALID; };

protected:
	virtual ~Animation () {}
};

/* @Namespace=System.Windows.Media.Animation */
class DoubleAnimation : public Animation {
public:
 	/* @PropertyType=double,Nullable,GenerateAccessors */
	const static int ByProperty;
	/* @PropertyType=double,Nullable,GenerateAccessors */
	const static int FromProperty;
	/* @PropertyType=double,Nullable,GenerateAccessors */
	const static int ToProperty;
	/* @PropertyType=EasingFunctionBase,ManagedPropertyType=IEasingFunction,GenerateAccessors */
	const static int EasingFunctionProperty;

	virtual Type::Kind GetValueKind () { return Type::DOUBLE; };

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	virtual Value *GetTargetValue (Value *defaultOriginValue);
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);
	
	//
	// Property Accessors
	//
	double *GetBy ();
	void    SetBy (double *pv);
	void    SetBy (double v);
	
	double *GetFrom ();
	void    SetFrom (double *pv);
	void    SetFrom (double v);
	
	double *GetTo ();
	void    SetTo (double *pv);
	void    SetTo (double v);

	EasingFunctionBase* GetEasingFunction ();
	void SetEasingFunction (EasingFunctionBase* value);

protected:
	/* @GeneratePInvoke */
	DoubleAnimation ();
	
	virtual ~DoubleAnimation () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:
	double *doubleToCached;
	double *doubleFromCached;
	double *doubleByCached;
	bool hasCached;

	void EnsureCache (void);

};


/* @Namespace=System.Windows.Media.Animation */
class ColorAnimation : public Animation {
 public:
 	/* @PropertyType=Color,Nullable,GenerateAccessors */
	const static int ByProperty;
 	/* @PropertyType=Color,Nullable,GenerateAccessors */
	const static int FromProperty;
 	/* @PropertyType=Color,Nullable,GenerateAccessors */
	const static int ToProperty;
	/* @PropertyType=EasingFunctionBase,ManagedPropertyType=IEasingFunction,GenerateAccessors */
	const static int EasingFunctionProperty;
	
	virtual Type::Kind GetValueKind () { return Type::COLOR; };

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	virtual Value *GetTargetValue (Value *defaultOriginValue);
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);
	
	//
	// Property Accessors
	//
	Color *GetBy ();
	void   SetBy (Color *pv);
	void   SetBy (Color v);

	Color *GetFrom ();
	void   SetFrom (Color *pv);
	void   SetFrom (Color v);

	Color *GetTo ();
	void   SetTo (Color *pv);
	void   SetTo (Color v);

	EasingFunctionBase* GetEasingFunction ();
	void SetEasingFunction (EasingFunctionBase* value);

protected:
	/* @GeneratePInvoke */
	ColorAnimation ();

	virtual ~ColorAnimation () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:
	Color *colorToCached;
	Color *colorFromCached;
	Color *colorByCached;
	bool hasCached;

	void EnsureCache (void);
};


/* @Namespace=System.Windows.Media.Animation */
class PointAnimation : public Animation {
public:
 	/* @PropertyType=Point,Nullable,GenerateAccessors */
	const static int ByProperty;
 	/* @PropertyType=Point,Nullable,GenerateAccessors */
	const static int FromProperty;
 	/* @PropertyType=Point,Nullable,GenerateAccessors */
	const static int ToProperty;
	/* @PropertyType=EasingFunctionBase,ManagedPropertyType=IEasingFunction,GenerateAccessors */
	const static int EasingFunctionProperty;
	
	virtual Type::Kind GetValueKind () { return Type::POINT; };

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	virtual Value *GetTargetValue (Value *defaultOriginValue);
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);
	
	//
	// Property Accessors
	//
	Point *GetBy ();
	void   SetBy (Point *pv);
	void   SetBy (Point v);

	Point *GetFrom ();
	void   SetFrom (Point *pv);
	void   SetFrom (Point v);

	Point *GetTo ();
	void   SetTo (Point *pv);
	void   SetTo (Point v);

	EasingFunctionBase* GetEasingFunction ();
	void SetEasingFunction (EasingFunctionBase* value);

protected:
 	/* @GeneratePInvoke */
 	PointAnimation ();
 	
	virtual ~PointAnimation ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:
	Point *pointToCached;
	Point *pointFromCached;
	Point *pointByCached;
	bool hasCached;

	void EnsureCache (void);

};


/* @Namespace=None,ManagedDependencyProperties=None */
class KeyFrame : public DependencyObject {
public:
	TimeSpan resolved_keytime;
	bool resolved;
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	virtual KeyTime *GetKeyTime () = 0;
	void SetKeyTime (KeyTime keytime) { SetKeyTime (&keytime); }
	virtual void SetKeyTime (KeyTime *keytime) = 0;

	static bool CoerceKeyTime (DependencyObject *obj, DependencyProperty *p, Value *value, Value **coerced, MoonError *error);

protected:
	virtual ~KeyFrame ();
	/* @SkipFactories */
	KeyFrame ();
};

/* @Namespace=None */
class KeyFrameCollection : public DependencyObjectCollection {
public:
	GPtrArray *sorted_list;
	bool resolved;
	
	virtual Type::Kind GetElementType() { return Type::KEYFRAME; }
	
	virtual bool Clear ();
	
	KeyFrame *GetKeyFrameForTime (TimeSpan t, KeyFrame **previous_frame);

	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);

protected:
	virtual bool AddedToCollection (Value *value, MoonError *error);
	virtual void RemovedFromCollection (Value *value, bool is_value_safe);
	
	/* @GeneratePInvoke */
	KeyFrameCollection ();
	
	virtual ~KeyFrameCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class ColorKeyFrameCollection : public KeyFrameCollection {
public:
	virtual Type::Kind GetElementType() { return Type::COLORKEYFRAME; }

protected:
	/* @GeneratePInvoke */
	ColorKeyFrameCollection ();
	
	virtual ~ColorKeyFrameCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class DoubleKeyFrameCollection : public KeyFrameCollection {
public:
	virtual Type::Kind GetElementType() { return Type::DOUBLEKEYFRAME; }

protected:
	/* @GeneratePInvoke */
	DoubleKeyFrameCollection ();
	
	virtual ~DoubleKeyFrameCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class PointKeyFrameCollection : public KeyFrameCollection {
public:
	virtual Type::Kind GetElementType() { return Type::POINTKEYFRAME; }

protected:
	/* @GeneratePInvoke */
	PointKeyFrameCollection ();

	virtual ~PointKeyFrameCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class ObjectKeyFrameCollection : public KeyFrameCollection {
public:
	virtual Type::Kind GetElementType() { return Type::OBJECTKEYFRAME; }

protected:
	/* @GeneratePInvoke */
	ObjectKeyFrameCollection ();

	virtual ~ObjectKeyFrameCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class DoubleKeyFrame : public KeyFrame {
public:
 	/* @PropertyType=double,DefaultValue=0.0,ManagedPropertyType=double,GenerateAccessors */
	const static int ValueProperty;
	/* @PropertyType=KeyTime,DefaultValue=KeyTime(KeyTime::UNIFORM),ManagedPropertyType=KeyTime,GenerateAccessors,Coercer=KeyFrame::CoerceKeyTime */
	const static int KeyTimeProperty;
	
	//
	// Property Accessors
	//
	double GetValue ();
	void SetValue (double v);

	virtual KeyTime *GetKeyTime ();
	void SetKeyTime (KeyTime keytime) { SetKeyTime (&keytime); }
	virtual void SetKeyTime (KeyTime *keytime);

protected:
	/* @GeneratePInvoke,ManagedAccess=Protected */
	DoubleKeyFrame ();
	
	virtual ~DoubleKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class ColorKeyFrame : public KeyFrame {
public:
 	/* @PropertyType=Color,DefaultValue=Color(0.0\,0.0\,0.0\,1.0),ManagedPropertyType=Color,GenerateAccessors */
	const static int ValueProperty;
	/* @PropertyType=KeyTime,DefaultValue=KeyTime(KeyTime::UNIFORM),ManagedPropertyType=KeyTime,GenerateAccessors,Coercer=KeyFrame::CoerceKeyTime */
	const static int KeyTimeProperty;
	
	//
	// Property Accessors
	//
	Color *GetValue ();
	void SetValue (Color v) { SetValue (&v); }
	void SetValue (Color *v);

	virtual KeyTime *GetKeyTime ();
	void SetKeyTime (KeyTime keytime) { SetKeyTime (&keytime); }
	virtual void SetKeyTime (KeyTime *keytime);

protected:
	/* @GeneratePInvoke,ManagedAccess=Protected */
	ColorKeyFrame ();
	
	virtual ~ColorKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class ObjectKeyFrame : public KeyFrame /* The managed class derives directly from DependencyObject */ {
public:
	/* @PropertyType=object,GenerateAccessors,GenerateManagedAccessors=false,ManagedFieldAccess=Internal */
	const static int ConvertedValueProperty;
	/* @PropertyType=object,ManagedPropertyType=object */
	const static int ValueProperty;
	/* @PropertyType=KeyTime,DefaultValue=KeyTime(KeyTime::UNIFORM),ManagedPropertyType=KeyTime,GenerateAccessors,Coercer=KeyFrame::CoerceKeyTime */
	const static int KeyTimeProperty;

	Value *GetConvertedValue ();
	void SetConvertedValue (Value *value);
	Value *GetValue ();

	virtual KeyTime *GetKeyTime ();
	void SetKeyTime (KeyTime keytime) { SetKeyTime (&keytime); }
	virtual void SetKeyTime (KeyTime *keytime);

protected:
	/* @GeneratePInvoke,ManagedAccess=Protected */
	ObjectKeyFrame ();
	
	virtual ~ObjectKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class PointKeyFrame : public KeyFrame {
public:
	/* @PropertyType=Point,DefaultValue=Point(),ManagedPropertyType=Point,GenerateAccessors */
	const static int ValueProperty;
	/* @PropertyType=KeyTime,DefaultValue=KeyTime(KeyTime::UNIFORM),ManagedPropertyType=KeyTime,GenerateAccessors,Coercer=KeyFrame::CoerceKeyTime */
	const static int KeyTimeProperty;
	
	//
	// Property Accessors
	//
	Point *GetValue ();
	void SetValue (Point v) { SetValue (&v); }
	void SetValue (Point *v);

	virtual KeyTime *GetKeyTime ();
	void SetKeyTime (KeyTime keytime) { SetKeyTime (&keytime); }
	virtual void SetKeyTime (KeyTime *keytime);

protected:
	/* @GeneratePInvoke,ManagedAccess=Protected */
	PointKeyFrame ();
	
	virtual ~PointKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};



/* @Namespace=System.Windows.Media.Animation */
class DiscreteDoubleKeyFrame : public DoubleKeyFrame {
public:
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	/* @GeneratePInvoke */
	DiscreteDoubleKeyFrame ();
	
	virtual ~DiscreteDoubleKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};




/* @Namespace=System.Windows.Media.Animation */
class DiscreteColorKeyFrame : public ColorKeyFrame {
public:
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	/* @GeneratePInvoke */
	DiscreteColorKeyFrame ();
	
	virtual ~DiscreteColorKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};


/* @Namespace=System.Windows.Media.Animation */
class DiscreteObjectKeyFrame : public ObjectKeyFrame {
public:
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	/* @GeneratePInvoke */
	DiscreteObjectKeyFrame ();
	
	virtual ~DiscreteObjectKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class DiscretePointKeyFrame : public PointKeyFrame {
public:
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	/* @GeneratePInvoke */
	DiscretePointKeyFrame ();
	
	virtual ~DiscretePointKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};


/* @Namespace=System.Windows.Media.Animation */
class LinearDoubleKeyFrame : public DoubleKeyFrame {
public:
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	/* @GeneratePInvoke */
	LinearDoubleKeyFrame ();
	
	virtual ~LinearDoubleKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};



/* @Namespace=System.Windows.Media.Animation */
class LinearColorKeyFrame : public ColorKeyFrame {
public:
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	/* @GeneratePInvoke */
	LinearColorKeyFrame ();
	
	virtual ~LinearColorKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};



/* @Namespace=System.Windows.Media.Animation */
class LinearPointKeyFrame : public PointKeyFrame {
public:
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	/* @GeneratePInvoke */
	LinearPointKeyFrame ();
	
	virtual ~LinearPointKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};



/* @Namespace=System.Windows.Media.Animation */
class SplineDoubleKeyFrame : public DoubleKeyFrame {
public:
 	/* @PropertyType=KeySpline,AutoCreateValue,GenerateAccessors */
	const static int KeySplineProperty;
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	KeySpline *GetKeySpline ();
	void SetKeySpline (KeySpline* value);

protected:
	/* @GeneratePInvoke */
	SplineDoubleKeyFrame ();
	
	virtual ~SplineDoubleKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};



/* @Namespace=System.Windows.Media.Animation */
class SplineColorKeyFrame : public ColorKeyFrame {
public:
 	/* @PropertyType=KeySpline,AutoCreateValue,GenerateAccessors */
	const static int KeySplineProperty;
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	KeySpline *GetKeySpline ();
	void SetKeySpline (KeySpline* value);

protected:
	/* @GeneratePInvoke */
	SplineColorKeyFrame ();
	
	virtual ~SplineColorKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};



/* @Namespace=System.Windows.Media.Animation */
class SplinePointKeyFrame : public PointKeyFrame {
public:
 	/* @PropertyType=KeySpline,AutoCreateValue,GenerateAccessors */
	const static int KeySplineProperty;
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	KeySpline *GetKeySpline ();
	void SetKeySpline (KeySpline* value);

protected:
	/* @GeneratePInvoke */
	SplinePointKeyFrame ();

	virtual ~SplinePointKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class EasingColorKeyFrame : public ColorKeyFrame {
public:
 	/* @PropertyType=EasingFunctionBase,ManagedPropertyType=IEasingFunction,GenerateAccessors */
	const static int EasingFunctionProperty;
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	EasingFunctionBase *GetEasingFunction ();
	void SetEasingFunction (EasingFunctionBase* value);

protected:
	/* @GeneratePInvoke */
	EasingColorKeyFrame ();

	virtual ~EasingColorKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class EasingDoubleKeyFrame : public DoubleKeyFrame {
public:
 	/* @PropertyType=EasingFunctionBase,ManagedPropertyType=IEasingFunction,GenerateAccessors */
	const static int EasingFunctionProperty;
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	EasingFunctionBase *GetEasingFunction ();
	void SetEasingFunction (EasingFunctionBase* value);

protected:
	/* @GeneratePInvoke */
	EasingDoubleKeyFrame ();

	virtual ~EasingDoubleKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
class EasingPointKeyFrame : public PointKeyFrame {
public:
 	/* @PropertyType=EasingFunctionBase,ManagedPropertyType=IEasingFunction,GenerateAccessors */
	const static int EasingFunctionProperty;
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	EasingFunctionBase *GetEasingFunction ();
	void SetEasingFunction (EasingFunctionBase* value);

protected:
	/* @GeneratePInvoke */
	EasingPointKeyFrame ();

	virtual ~EasingPointKeyFrame ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class DoubleAnimationUsingKeyFrames : public DoubleAnimation {
public:
 	/* @PropertyType=DoubleKeyFrameCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int KeyFramesProperty;
	
	void AddKeyFrame (DoubleKeyFrame *frame);
	void RemoveKeyFrame (DoubleKeyFrame *frame);
	
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);
	virtual bool Resolve (DependencyObject *target, DependencyProperty *property);

	virtual Duration GetNaturalDurationCore (Clock* clock);

	virtual bool Validate (MoonError *error);
	
	//
	// Property Accessors
	//
	DoubleKeyFrameCollection *GetKeyFrames ();
	void SetKeyFrames (DoubleKeyFrameCollection* value);

protected:
	/* @GeneratePInvoke */
	DoubleAnimationUsingKeyFrames ();
	
	virtual ~DoubleAnimationUsingKeyFrames ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class ColorAnimationUsingKeyFrames : public ColorAnimation {
public:
 	/* @PropertyType=ColorKeyFrameCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int KeyFramesProperty;
	
	void AddKeyFrame (ColorKeyFrame *frame);
	void RemoveKeyFrame (ColorKeyFrame *frame);
	
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);
	virtual bool Resolve (DependencyObject *target, DependencyProperty *property);
	
	virtual Duration GetNaturalDurationCore (Clock* clock);
	
	virtual bool Validate (MoonError *error);
	
	//
	// Property Accessors
	//
	ColorKeyFrameCollection *GetKeyFrames ();
	void SetKeyFrames (ColorKeyFrameCollection* value);

protected:
	/* @GeneratePInvoke */
	ColorAnimationUsingKeyFrames ();
	
	virtual ~ColorAnimationUsingKeyFrames ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class ObjectAnimationUsingKeyFrames : public /*Object*/Animation {
public:
 	/* @PropertyType=ObjectKeyFrameCollection,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors,AutoCreateValue */
	const static int KeyFramesProperty;
	
	void AddKeyFrame (ObjectKeyFrame *frame);
	void RemoveKeyFrame (ObjectKeyFrame *frame);

	// Property accessors
	ObjectKeyFrameCollection *GetKeyFrames ();
	void SetKeyFrames (ObjectKeyFrameCollection* value);

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual bool Resolve (DependencyObject *target, DependencyProperty *property);

	virtual Duration GetNaturalDurationCore (Clock* clock);
	virtual bool Validate (MoonError *error);

	virtual Type::Kind GetValueKind () { return Type::INVALID; };

protected:
	/* @GeneratePInvoke */
	ObjectAnimationUsingKeyFrames ();

	virtual ~ObjectAnimationUsingKeyFrames ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class PointAnimationUsingKeyFrames : public PointAnimation {
public:
 	/* @PropertyType=PointKeyFrameCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int KeyFramesProperty;
	
	void AddKeyFrame (PointKeyFrame *frame);
	void RemoveKeyFrame (PointKeyFrame *frame);
	
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);
	virtual bool Resolve (DependencyObject *target, DependencyProperty *property);

	virtual Duration GetNaturalDurationCore (Clock *clock);

	virtual bool Validate (MoonError *error);
	
	//
	// Property Accessors
	//
	PointKeyFrameCollection *GetKeyFrames ();
	void SetKeyFrames (PointKeyFrameCollection* value);

protected:
	/* @GeneratePInvoke */
	PointAnimationUsingKeyFrames ();
	
	virtual ~PointAnimationUsingKeyFrames ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="Children" */
class Storyboard : public ParallelTimeline {
public:
 	/* @PropertyType=string,Attached,GenerateAccessors,Validator=IsTimelineValidator */
	const static int TargetNameProperty;
 	/* @PropertyType=PropertyPath,Attached,GenerateAccessors,Validator=StoryboardTargetPropertyValidator */
	const static int TargetPropertyProperty;
	
	/* @GeneratePInvoke */
	bool BeginWithError (MoonError *error);

	/* @GeneratePInvoke */
	void FlattenTimelines (FlattenTimelinesCallback callback);
	static void FlattenTimelines (FlattenTimelinesCallback callback, Timeline *timeline, DependencyObject *targetObject, PropertyPath *targetPropertyPath);

	/* @GeneratePInvoke */
	void PauseWithError (MoonError *error);
	
	/* @GeneratePInvoke */
	void ResumeWithError (MoonError *error);

	/* @GeneratePInvoke */
	void SeekWithError (TimeSpan timespan, MoonError *error);

	/* @GeneratePInvoke */
	void SeekAlignedToLastTickWithError (TimeSpan timespan, MoonError *error);

	/* @GeneratePInvoke */
	void SkipToFillWithError (MoonError *error);
	
	/* @GeneratePInvoke */
	void StopWithError (MoonError *error);

	/* @GeneratePInvoke */
	TimeSpan GetCurrentTime ();

	/* @GeneratePInvoke */
	int GetCurrentState ();
	
	/* @GeneratePInvoke */
	DependencyProperty *GetTargetDependencyProperty ();
	
	static void SetTargetName (DependencyObject *o, const char *targetName);
	static const char *GetTargetName (DependencyObject *o);
	static void SetTargetProperty (DependencyObject *o, PropertyPath *targetProperty);
	static PropertyPath *GetTargetProperty (DependencyObject *o);

protected:
	/* @GeneratePInvoke */
	Storyboard ();

	virtual void ClearClock ();
	virtual void Dispose ();

	virtual ~Storyboard ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
private:
	bool HookupAnimationsRecurse (Clock *clock,
				      DependencyObject *targetObject, PropertyPath *targetPropertyPath,
				      GHashTable *promoted_values,
				      List *animated_properties,
				      MoonError *error);

	static void clock_statechanged (EventObject *sender, EventArgs *calldata, gpointer closure);
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="Storyboard" */
class BeginStoryboard : public TriggerAction {
public:
 	/* @PropertyType=Storyboard,GenerateAccessors */
	const static int StoryboardProperty;
	
	virtual void Fire ();
	
	//
	// Property Accessors
	//
	void SetStoryboard (Storyboard *sb);
	Storyboard *GetStoryboard ();

protected:
	/* @GeneratePInvoke */
	BeginStoryboard ();
	
	virtual ~BeginStoryboard ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};



class AnimationNode : public List::Node {
 public:
	DependencyProperty *property;
	DependencyObject *target;
	AnimationNode (DependencyObject *target, DependencyProperty *property)
	{
		this->target = target;
		this->property = property;
	}

	static bool AnimationNodeComparer (Node *node, void *data)
	{
		AnimationNode *left = (AnimationNode *) node;
		AnimationNode *right = (AnimationNode *) data;

		return left->property == right->property &&
				left->target == right->target;
	}
};

// internal WPF class gleaned from stack traces
class AnimationStorage {
public:

	class Node : public List::Node {
	public:
		AnimationStorage *storage;
		DependencyProperty *key;

		Node (DependencyProperty *key, AnimationStorage *storage) {
			this->key = key;
			this->storage = storage;
		}

		Node *Clone () {
			return new Node (key, storage);
		}
	};

	AnimationStorage (AnimationClock *clock, Animation *timeline,
			  DependencyObject *targetobj, DependencyProperty *targetprop);
	~AnimationStorage ();
	
	void SwitchTarget (DependencyObject *target);
	void Enable ();
	void Disable ();
	void Stop ();
	bool IsCurrentStorage ();

	Value* GetStopValue ();
	void SetStopValue (Value *value);

	AnimationClock *GetClock ();
	Animation *GetTimeline ();

private:

	void ResetPropertyValue ();
	void UpdatePropertyValue ();
	static void update_property_value (EventObject *sender, EventArgs *calldata, gpointer data);

	void TargetObjectDestroyed ();
	static void target_object_destroyed (EventObject *sender, EventArgs *calldata, gpointer data);

	void AttachUpdateHandler ();
	void DetachUpdateHandler ();
	void AttachTargetHandler ();
	void DetachTargetHandler ();
	void DetachFromProperty ();

	AnimationClock *clock;
	Animation* timeline;
	DependencyObject *targetobj;
	DependencyProperty *targetprop;
	Value *baseValue;
	Value *stopValue;
	bool disabled;
};

/* @Namespace=None,ManagedDependencyProperties=None */
class AnimationClock : public Clock {
public:
	AnimationClock (Animation *timeline);

	Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue);

	AnimationStorage *HookupStorage (DependencyObject *targetobj, DependencyProperty *targetprop);
	void DetachStorage ();

	virtual void Stop ();
	virtual void Begin (TimeSpan parentTime);

	const static void *TimelineWeakRef;

protected:
	virtual ~AnimationClock ();

private:
	WeakRef<Animation> timeline;
	AnimationStorage *storage;
};

};

#endif /* MOON_ANIMATION_H */
