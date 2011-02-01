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

// misc types
/* @Namespace=System.Windows.Media.Animation */
class KeySpline : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	KeySpline ();
	
	KeySpline (Point controlPoint1, Point controlPoint2);
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
	virtual ~KeySpline ();


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
	}

	KeyTime (double percent)
	  : k (PERCENT),
	    percent (percent),
	    timespan (0) { }

	KeyTime (TimeSpan timeSpan)
	  : k (TIMESPAN),
            timespan (timeSpan) { }


	KeyTime (KeyTimeType kind) : k(kind) { }

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

	Animation () { };
	
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

	/* @GenerateCBinding,GeneratePInvoke */
	DoubleAnimation ();
	
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
	virtual ~DoubleAnimation () {}

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
	
	/* @GenerateCBinding,GeneratePInvoke */
	ColorAnimation ();
	
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
	virtual ~ColorAnimation () {}

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
	
 	/* @GenerateCBinding,GeneratePInvoke */
 	PointAnimation ();
 	
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
	virtual ~PointAnimation ();

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
	virtual void SetKeyTime (KeyTime keytime) = 0;
	virtual void SetKeyTime (KeyTime *keytime) = 0;

protected:
	virtual ~KeyFrame ();
	KeyFrame ();
};

/* @Namespace=None */
class KeyFrameCollection : public DependencyObjectCollection {
public:
	GPtrArray *sorted_list;
	bool resolved;
	
	/* @GenerateCBinding,GeneratePInvoke */
	KeyFrameCollection ();
	
	virtual Type::Kind GetElementType() { return Type::KEYFRAME; }
	
	virtual bool Clear ();
	
	KeyFrame *GetKeyFrameForTime (TimeSpan t, KeyFrame **previous_frame);

	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);

protected:
	virtual bool AddedToCollection (Value *value, MoonError *error);
	virtual void RemovedFromCollection (Value *value);
	
	virtual ~KeyFrameCollection ();
};

/* @Namespace=System.Windows.Media.Animation */
class ColorKeyFrameCollection : public KeyFrameCollection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	ColorKeyFrameCollection ();
	
	virtual Type::Kind GetElementType() { return Type::COLORKEYFRAME; }

protected:
	virtual ~ColorKeyFrameCollection ();
};

/* @Namespace=System.Windows.Media.Animation */
class DoubleKeyFrameCollection : public KeyFrameCollection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	DoubleKeyFrameCollection ();
	
	virtual Type::Kind GetElementType() { return Type::DOUBLEKEYFRAME; }

protected:
	virtual ~DoubleKeyFrameCollection ();
};

/* @Namespace=System.Windows.Media.Animation */
class PointKeyFrameCollection : public KeyFrameCollection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	PointKeyFrameCollection ();
	
	virtual Type::Kind GetElementType() { return Type::POINTKEYFRAME; }

protected:
	virtual ~PointKeyFrameCollection ();
};

/* @Version=2,Namespace=System.Windows.Media.Animation */
class ObjectKeyFrameCollection : public KeyFrameCollection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	ObjectKeyFrameCollection ();

	virtual Type::Kind GetElementType() { return Type::OBJECTKEYFRAME; }

protected:
	virtual ~ObjectKeyFrameCollection ();
};

/* @Namespace=System.Windows.Media.Animation */
class DoubleKeyFrame : public KeyFrame {
public:
 	/* @PropertyType=double,Nullable,ManagedPropertyType=double,GenerateAccessors */
	const static int ValueProperty;
	/* @PropertyType=KeyTime,Nullable,ManagedPropertyType=KeyTime,GenerateAccessors */
	const static int KeyTimeProperty;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	DoubleKeyFrame ();
	
	//
	// Property Accessors
	//
	double *GetValue ();
	void    SetValue (double *pv);
	void    SetValue (double v);

	virtual KeyTime *GetKeyTime ();
	virtual void SetKeyTime (KeyTime keytime);
	virtual void SetKeyTime (KeyTime *keytime);

protected:
	virtual ~DoubleKeyFrame ();
};

/* @Namespace=System.Windows.Media.Animation */
class ColorKeyFrame : public KeyFrame {
public:
 	/* @PropertyType=Color,Nullable,ManagedPropertyType=Color,GenerateAccessors */
	const static int ValueProperty;
	/* @PropertyType=KeyTime,Nullable,ManagedPropertyType=KeyTime,GenerateAccessors */
	const static int KeyTimeProperty;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	ColorKeyFrame ();
	
	//
	// Property Accessors
	//
	Color *GetValue ();
	void   SetValue (Color *pv);
	void   SetValue (Color v);

	virtual KeyTime *GetKeyTime ();
	virtual void SetKeyTime (KeyTime keytime);
	virtual void SetKeyTime (KeyTime *keytime);

protected:
	virtual ~ColorKeyFrame ();
};

/* @Version=2,Namespace=System.Windows.Media.Animation */
class ObjectKeyFrame : public KeyFrame /* The managed class derives directly from DependencyObject */ {
public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	ObjectKeyFrame ();
	
	/* @PropertyType=object,GenerateAccessors,GenerateManagedAccessors=false,ManagedFieldAccess=Internal */
	const static int ConvertedValueProperty;
	/* @PropertyType=object,ManagedPropertyType=object */
	const static int ValueProperty;
	/* @PropertyType=KeyTime,Nullable,ManagedPropertyType=KeyTime,GenerateAccessors */
	const static int KeyTimeProperty;

	Value *GetConvertedValue ();
	void SetConvertedValue (Value *value);
	Value *GetValue ();

	virtual KeyTime *GetKeyTime ();
	virtual void SetKeyTime (KeyTime keytime);
	virtual void SetKeyTime (KeyTime *keytime);

protected:
	virtual ~ObjectKeyFrame ();
};

/* @Namespace=System.Windows.Media.Animation */
class PointKeyFrame : public KeyFrame {
public:
 	/* @PropertyType=Point,Nullable,ManagedPropertyType=Point,GenerateAccessors */
	const static int ValueProperty;
	/* @PropertyType=KeyTime,Nullable,ManagedPropertyType=KeyTime,GenerateAccessors */
	const static int KeyTimeProperty;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	PointKeyFrame ();
	
	//
	// Property Accessors
	//
	Point *GetValue ();
	void   SetValue (Point *pv);
	void   SetValue (Point v);

	virtual KeyTime *GetKeyTime ();
	virtual void SetKeyTime (KeyTime keytime);
	virtual void SetKeyTime (KeyTime *keytime);

protected:
	virtual ~PointKeyFrame ();
};



/* @Namespace=System.Windows.Media.Animation */
class DiscreteDoubleKeyFrame : public DoubleKeyFrame {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	DiscreteDoubleKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	virtual ~DiscreteDoubleKeyFrame ();
};




/* @Namespace=System.Windows.Media.Animation */
class DiscreteColorKeyFrame : public ColorKeyFrame {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	DiscreteColorKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	virtual ~DiscreteColorKeyFrame ();
};


/* @Version=2,Namespace=System.Windows.Media.Animation */
class DiscreteObjectKeyFrame : public ObjectKeyFrame {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	DiscreteObjectKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	virtual ~DiscreteObjectKeyFrame ();
};

/* @Namespace=System.Windows.Media.Animation */
class DiscretePointKeyFrame : public PointKeyFrame {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	DiscretePointKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	virtual ~DiscretePointKeyFrame ();
};


/* @Namespace=System.Windows.Media.Animation */
class LinearDoubleKeyFrame : public DoubleKeyFrame {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	LinearDoubleKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	virtual ~LinearDoubleKeyFrame ();
};



/* @Namespace=System.Windows.Media.Animation */
class LinearColorKeyFrame : public ColorKeyFrame {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	LinearColorKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	virtual ~LinearColorKeyFrame ();
};



/* @Namespace=System.Windows.Media.Animation */
class LinearPointKeyFrame : public PointKeyFrame {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	LinearPointKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

protected:
	virtual ~LinearPointKeyFrame ();
};



/* @Namespace=System.Windows.Media.Animation */
class SplineDoubleKeyFrame : public DoubleKeyFrame {
public:
 	/* @PropertyType=KeySpline,AutoCreateValue,GenerateAccessors */
	const static int KeySplineProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	SplineDoubleKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	KeySpline *GetKeySpline ();
	void SetKeySpline (KeySpline* value);

protected:
	virtual ~SplineDoubleKeyFrame ();
};



/* @Namespace=System.Windows.Media.Animation */
class SplineColorKeyFrame : public ColorKeyFrame {
public:
 	/* @PropertyType=KeySpline,AutoCreateValue,GenerateAccessors */
	const static int KeySplineProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	SplineColorKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	KeySpline *GetKeySpline ();
	void SetKeySpline (KeySpline* value);

protected:
	virtual ~SplineColorKeyFrame ();
};



/* @Namespace=System.Windows.Media.Animation */
class SplinePointKeyFrame : public PointKeyFrame {
public:
 	/* @PropertyType=KeySpline,AutoCreateValue,GenerateAccessors */
	const static int KeySplineProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	SplinePointKeyFrame ();

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	KeySpline *GetKeySpline ();
	void SetKeySpline (KeySpline* value);

protected:
	virtual ~SplinePointKeyFrame ();
};

/* @Namespace=System.Windows.Media.Animation */
class EasingColorKeyFrame : public ColorKeyFrame {
public:
 	/* @PropertyType=EasingFunctionBase,ManagedPropertyType=IEasingFunction,GenerateAccessors */
	const static int EasingFunctionProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	EasingColorKeyFrame ();

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	EasingFunctionBase *GetEasingFunction ();
	void SetEasingFunction (EasingFunctionBase* value);

protected:
	virtual ~EasingColorKeyFrame ();
};

/* @Namespace=System.Windows.Media.Animation */
class EasingDoubleKeyFrame : public DoubleKeyFrame {
public:
 	/* @PropertyType=EasingFunctionBase,ManagedPropertyType=IEasingFunction,GenerateAccessors */
	const static int EasingFunctionProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	EasingDoubleKeyFrame ();

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	EasingFunctionBase *GetEasingFunction ();
	void SetEasingFunction (EasingFunctionBase* value);

protected:
	virtual ~EasingDoubleKeyFrame ();
};

/* @Namespace=System.Windows.Media.Animation */
class EasingPointKeyFrame : public PointKeyFrame {
public:
 	/* @PropertyType=EasingFunctionBase,ManagedPropertyType=IEasingFunction,GenerateAccessors */
	const static int EasingFunctionProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	EasingPointKeyFrame ();

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	EasingFunctionBase *GetEasingFunction ();
	void SetEasingFunction (EasingFunctionBase* value);

protected:
	virtual ~EasingPointKeyFrame ();
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class DoubleAnimationUsingKeyFrames : public DoubleAnimation {
public:
 	/* @PropertyType=DoubleKeyFrameCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int KeyFramesProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	DoubleAnimationUsingKeyFrames ();
	
	void AddKeyFrame (DoubleKeyFrame *frame);
	void RemoveKeyFrame (DoubleKeyFrame *frame);
	
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);
	virtual bool Resolve (DependencyObject *target, DependencyProperty *property);

	virtual Duration GetNaturalDurationCore (Clock* clock);

	virtual bool Validate ();
	
	//
	// Property Accessors
	//
	DoubleKeyFrameCollection *GetKeyFrames ();
	void SetKeyFrames (DoubleKeyFrameCollection* value);

protected:
	virtual ~DoubleAnimationUsingKeyFrames ();
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class ColorAnimationUsingKeyFrames : public ColorAnimation {
public:
 	/* @PropertyType=ColorKeyFrameCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int KeyFramesProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ColorAnimationUsingKeyFrames ();
	
	void AddKeyFrame (ColorKeyFrame *frame);
	void RemoveKeyFrame (ColorKeyFrame *frame);
	
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);
	virtual bool Resolve (DependencyObject *target, DependencyProperty *property);
	
	virtual Duration GetNaturalDurationCore (Clock* clock);
	
	virtual bool Validate ();
	
	//
	// Property Accessors
	//
	ColorKeyFrameCollection *GetKeyFrames ();
	void SetKeyFrames (ColorKeyFrameCollection* value);

protected:
	virtual ~ColorAnimationUsingKeyFrames ();
};

/* @Version=2 */
/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class ObjectAnimationUsingKeyFrames : public /*Object*/Animation {
public:
 	/* @PropertyType=ObjectKeyFrameCollection,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors,AutoCreateValue */
	const static int KeyFramesProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ObjectAnimationUsingKeyFrames ();

	void AddKeyFrame (ObjectKeyFrame *frame);
	void RemoveKeyFrame (ObjectKeyFrame *frame);

	// Property accessors
	ObjectKeyFrameCollection *GetKeyFrames ();
	void SetKeyFrames (ObjectKeyFrameCollection* value);

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual bool Resolve (DependencyObject *target, DependencyProperty *property);

	virtual Duration GetNaturalDurationCore (Clock* clock);
	virtual bool Validate ();

	virtual Type::Kind GetValueKind () { return Type::INVALID; };

protected:
	virtual ~ObjectAnimationUsingKeyFrames ();
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class PointAnimationUsingKeyFrames : public PointAnimation {
public:
 	/* @PropertyType=PointKeyFrameCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int KeyFramesProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	PointAnimationUsingKeyFrames ();
	
	void AddKeyFrame (PointKeyFrame *frame);
	void RemoveKeyFrame (PointKeyFrame *frame);
	
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);
	virtual bool Resolve (DependencyObject *target, DependencyProperty *property);

	virtual Duration GetNaturalDurationCore (Clock *clock);

	virtual bool Validate ();
	
	//
	// Property Accessors
	//
	PointKeyFrameCollection *GetKeyFrames ();
	void SetKeyFrames (PointKeyFrameCollection* value);

protected:
	virtual ~PointAnimationUsingKeyFrames ();
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="Children" */
class MOON_API Storyboard : public ParallelTimeline {
public:
 	/* @PropertyType=string,Attached,GenerateAccessors,Validator=IsTimelineValidator */
	const static int TargetNameProperty;
 	/* @PropertyType=PropertyPath,Attached,GenerateAccessors,Validator=StoryboardTargetPropertyValidator */
	const static int TargetPropertyProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Storyboard ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	bool BeginWithError (MoonError *error);

	/* @GenerateCBinding,GeneratePInvoke */
	void PauseWithError (MoonError *error);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void ResumeWithError (MoonError *error);

	/* @GenerateCBinding,GeneratePInvoke */
	void SeekWithError (TimeSpan timespan, MoonError *error);

	/* @GenerateCBinding,GeneratePInvoke */
	void SeekAlignedToLastTickWithError (TimeSpan timespan, MoonError *error);

	/* @GenerateCBinding,GeneratePInvoke */
	void SkipToFillWithError (MoonError *error);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void StopWithError (MoonError *error);

	/* @GenerateCBinding,GeneratePInvoke */
	TimeSpan GetCurrentTime ();

	/* @GenerateCBinding,GeneratePInvoke */
	int GetCurrentState ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	DependencyProperty *GetTargetDependencyProperty ();
	
	static void SetTargetName (DependencyObject *o, const char *targetName);
	static const char *GetTargetName (DependencyObject *o);
	static void SetTargetProperty (DependencyObject *o, PropertyPath *targetProperty);
	static PropertyPath *GetTargetProperty (DependencyObject *o);

protected:
	virtual ~Storyboard ();

private:
	bool HookupAnimationsRecurse (Clock *clock,
				      DependencyObject *targetObject, PropertyPath *targetPropertyPath,
				      GHashTable *promoted_values,
				      MoonError *error);
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="Storyboard" */
class BeginStoryboard : public TriggerAction {
public:
 	/* @PropertyType=Storyboard,GenerateAccessors */
	const static int StoryboardProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	BeginStoryboard ();
	
	virtual void Fire ();
	
	//
	// Property Accessors
	//
	void SetStoryboard (Storyboard *sb);
	Storyboard *GetStoryboard ();

protected:
	virtual ~BeginStoryboard ();
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

	Value* GetResetValue ();
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

protected:
	virtual ~AnimationClock ();

private:
	Animation *timeline;
	AnimationStorage *storage;
};

#endif /* MOON_ANIMATION_H */
