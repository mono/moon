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
#include "list.h"
#include "point.h"
#include "propertypath.h"
#include "moon-curves.h"
#include "applier.h"

// misc types
/* @Namespace=System.Windows.Media.Animation */
class KeySpline : public DependencyObject {
	moon_quadratic *quadraticsArray;
	
 protected:
	virtual ~KeySpline ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	KeySpline ();
	
	KeySpline (Point controlPoint1, Point controlPoint2);
	KeySpline (double x1, double y1, double x2, double y2);

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
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
};

/* @Namespace=System.Windows.Media.Animation */
/* @IncludeInKinds */
struct KeyTime {
 public:
	enum KeyTimeType {
		UNIFORM,
		PACED,
		PERCENT,
		TIMESPAN
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
class Animation;
class AnimationClock;
class AnimationStorage;

// internal WPF class gleaned from stack traces
class AnimationStorage {
 public:
	AnimationStorage (AnimationClock *clock, Animation/*Timeline*/ *timeline,
			  DependencyObject *targetobj, DependencyProperty *targetprop);
	~AnimationStorage ();
	
	void ResetPropertyValue ();
	void DetachUpdateHandler ();
	void ReAttachUpdateHandler ();
	void DetachTarget ();
	void FlagAsNonResetable ();
	void Float ();
	bool IsFloating () { return floating; };
	bool IsLonely () { return (targetobj == NULL); };
	bool IsCurrentStorage ();
	Value* GetResetValue ();
	void UpdatePropertyValueWith (Value *v);
	Value* GetStopValue (void);
	void DetachFromPrevStorage (void);

 private:
	void TargetObjectDestroyed ();
	static void target_object_destroyed (EventObject *sender, EventArgs *calldata, gpointer data);

	void UpdatePropertyValue ();
	static void update_property_value (EventObject *sender, EventArgs *calldata, gpointer data);

	AnimationClock *clock;
	Animation/*Timeline*/* timeline;
	DependencyObject *targetobj;
	DependencyProperty *targetprop;
	Value *baseValue;
	Value *stopValue;
	bool nonResetableFlag;
	bool floating;
	bool wasAttached;
};


class Animation/*Timeline*/;

/* @Namespace=None,ManagedDependencyProperties=None */
class AnimationClock : public Clock {
 protected:
	virtual ~AnimationClock ();

 public:
	AnimationClock (Animation/*Timeline*/ *timeline);
	virtual void ExtraRepeatAction ();
	virtual void OnSurfaceDetach ();
	virtual void OnSurfaceReAttach ();

	Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue);

	bool HookupStorage (DependencyObject *targetobj, DependencyProperty *targetprop);

	virtual void Stop ();
	virtual void Begin ();

 private:
	Animation/*Timeline*/ *timeline;
	AnimationStorage *storage;
};





/* this is called AnimationTimeline in wpf */
/* @Namespace=None */
class Animation/*Timeline*/ : public Timeline {
 protected:
	virtual ~Animation () {}

 public:

	Animation/*Timeline*/ () { };
	
	virtual Clock *AllocateClock ();

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);
	virtual Value *GetTargetValue (Value *defaultOriginValue);

	virtual Duration GetNaturalDurationCore (Clock* clock);


	virtual void Resolve () { };

	/* The kind of values this animation generates */
	virtual Type::Kind GetValueKind () { return Type::INVALID; };
};

/* @Namespace=System.Windows.Media.Animation */
class DoubleAnimation : public Animation/*Timeline*/ {
 protected:
	virtual ~DoubleAnimation () {}

 private:
	double *doubleToCached;
	double *doubleFromCached;
	double *doubleByCached;
	bool hasCached;

	void EnsureCache (void);

 public:
 	/* @PropertyType=double,Nullable,GenerateAccessors */
	const static int ByProperty;
	/* @PropertyType=double,Nullable,GenerateAccessors */
	const static int FromProperty;
	/* @PropertyType=double,Nullable,GenerateAccessors */
	const static int ToProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	DoubleAnimation ();
	
	virtual Type::Kind GetValueKind () { return Type::DOUBLE; };

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
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
};


/* @Namespace=System.Windows.Media.Animation */
class ColorAnimation : public Animation/*Timeline*/ {
 protected:
	virtual ~ColorAnimation () {}

 private:
	Color *colorToCached;
	Color *colorFromCached;
	Color *colorByCached;
	bool hasCached;

	void EnsureCache (void);

 public:
 	/* @PropertyType=Color,Nullable,GenerateAccessors */
	const static int ByProperty;
 	/* @PropertyType=Color,Nullable,GenerateAccessors */
	const static int FromProperty;
 	/* @PropertyType=Color,Nullable,GenerateAccessors */
	const static int ToProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ColorAnimation ();
	
	virtual Type::Kind GetValueKind () { return Type::COLOR; };

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
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
};


/* @Namespace=System.Windows.Media.Animation */
class PointAnimation : public Animation/*Timeline*/ {
 protected:
	virtual ~PointAnimation ();

 private:
	Point *pointToCached;
	Point *pointFromCached;
	Point *pointByCached;
	bool hasCached;

	void EnsureCache (void);

 public:
 	/* @PropertyType=Point,Nullable,GenerateAccessors */
	const static int ByProperty;
 	/* @PropertyType=Point,Nullable,GenerateAccessors */
	const static int FromProperty;
 	/* @PropertyType=Point,Nullable,GenerateAccessors */
	const static int ToProperty;
	
 	/* @GenerateCBinding,GeneratePInvoke */
 	PointAnimation ();
 	
	virtual Type::Kind GetValueKind () { return Type::POINT; };

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
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
};


/* @Namespace=None,ManagedDependencyProperties=None */
class KeyFrame : public DependencyObject {
 protected:
	virtual ~KeyFrame ();
	KeyFrame ();

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
};

/* @Namespace=None */
class KeyFrameCollection : public DependencyObjectCollection {
 protected:
	virtual bool AddedToCollection (Value *value, MoonError *error);
	virtual void RemovedFromCollection (Value *value);
	
	virtual ~KeyFrameCollection ();

 public:
	GPtrArray *sorted_list;
	bool resolved;
	
	/* @GenerateCBinding,GeneratePInvoke */
	KeyFrameCollection ();
	
	virtual Type::Kind GetElementType() { return Type::KEYFRAME; }
	
	virtual bool Clear ();
	
	KeyFrame *GetKeyFrameForTime (TimeSpan t, KeyFrame **previous_frame);

	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
};

/* @Namespace=System.Windows.Media.Animation */
class ColorKeyFrameCollection : public KeyFrameCollection {
 protected:
	virtual ~ColorKeyFrameCollection ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	ColorKeyFrameCollection ();
	
	virtual Type::Kind GetElementType() { return Type::COLORKEYFRAME; }
};

/* @Namespace=System.Windows.Media.Animation */
class DoubleKeyFrameCollection : public KeyFrameCollection {
 protected:
	virtual ~DoubleKeyFrameCollection ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	DoubleKeyFrameCollection ();
	
	virtual Type::Kind GetElementType() { return Type::DOUBLEKEYFRAME; }
};

/* @Namespace=System.Windows.Media.Animation */
class PointKeyFrameCollection : public KeyFrameCollection {
 protected:
	virtual ~PointKeyFrameCollection ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	PointKeyFrameCollection ();
	
	virtual Type::Kind GetElementType() { return Type::POINTKEYFRAME; }
};

/* @Namespace=System.Windows.Media.Animation */
class DoubleKeyFrame : public KeyFrame {
 protected:
	virtual ~DoubleKeyFrame ();

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
};

/* @Namespace=System.Windows.Media.Animation */
class ColorKeyFrame : public KeyFrame {
 protected:
	virtual ~ColorKeyFrame ();

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
};

/* @Namespace=System.Windows.Media.Animation */
class PointKeyFrame : public KeyFrame {
 protected:
	virtual ~PointKeyFrame ();
	
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
};



/* @Namespace=System.Windows.Media.Animation */
class DiscreteDoubleKeyFrame : public DoubleKeyFrame {
 protected:
	virtual ~DiscreteDoubleKeyFrame ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	DiscreteDoubleKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};




/* @Namespace=System.Windows.Media.Animation */
class DiscreteColorKeyFrame : public ColorKeyFrame {
 protected:
	virtual ~DiscreteColorKeyFrame ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	DiscreteColorKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};



/* @Namespace=System.Windows.Media.Animation */
class DiscretePointKeyFrame : public PointKeyFrame {
 protected:
	virtual ~DiscretePointKeyFrame ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	DiscretePointKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};


/* @Namespace=System.Windows.Media.Animation */
class LinearDoubleKeyFrame : public DoubleKeyFrame {
 protected:
	virtual ~LinearDoubleKeyFrame ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	LinearDoubleKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};



/* @Namespace=System.Windows.Media.Animation */
class LinearColorKeyFrame : public ColorKeyFrame {
 protected:
	virtual ~LinearColorKeyFrame ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	LinearColorKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};



/* @Namespace=System.Windows.Media.Animation */
class LinearPointKeyFrame : public PointKeyFrame {
 protected:
	virtual ~LinearPointKeyFrame ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	LinearPointKeyFrame ();
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};



/* @Namespace=System.Windows.Media.Animation */
class SplineDoubleKeyFrame : public DoubleKeyFrame {
 protected:
	virtual ~SplineDoubleKeyFrame ();
	
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
};



/* @Namespace=System.Windows.Media.Animation */
class SplineColorKeyFrame : public ColorKeyFrame {
 protected:
	virtual ~SplineColorKeyFrame ();

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
};



/* @Namespace=System.Windows.Media.Animation */
class SplinePointKeyFrame : public PointKeyFrame {
 protected:
	virtual ~SplinePointKeyFrame ();
	
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
};


/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class DoubleAnimationUsingKeyFrames : public DoubleAnimation {
 protected:
	virtual ~DoubleAnimationUsingKeyFrames ();

 public:
 	/* @PropertyType=DoubleKeyFrameCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int KeyFramesProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	DoubleAnimationUsingKeyFrames ();
	
	void AddKeyFrame (DoubleKeyFrame *frame);
	void RemoveKeyFrame (DoubleKeyFrame *frame);
	
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);
	virtual void Resolve ();

	virtual Duration GetNaturalDurationCore (Clock* clock);

	virtual bool Validate ();
	
	//
	// Property Accessors
	//
	DoubleKeyFrameCollection *GetKeyFrames ();
	void SetKeyFrames (DoubleKeyFrameCollection* value);
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class ColorAnimationUsingKeyFrames : public ColorAnimation {
 protected:
	virtual ~ColorAnimationUsingKeyFrames ();

 public:
 	/* @PropertyType=ColorKeyFrameCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int KeyFramesProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ColorAnimationUsingKeyFrames ();
	
	void AddKeyFrame (ColorKeyFrame *frame);
	void RemoveKeyFrame (ColorKeyFrame *frame);
	
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);
	virtual void Resolve ();
	
	virtual Duration GetNaturalDurationCore (Clock* clock);
	
	virtual bool Validate ();
	
	//
	// Property Accessors
	//
	ColorKeyFrameCollection *GetKeyFrames ();
	void SetKeyFrames (ColorKeyFrameCollection* value);
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class PointAnimationUsingKeyFrames : public PointAnimation {
 protected:
	virtual ~PointAnimationUsingKeyFrames ();

 public:
 	/* @PropertyType=PointKeyFrameCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int KeyFramesProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	PointAnimationUsingKeyFrames ();
	
	void AddKeyFrame (PointKeyFrame *frame);
	void RemoveKeyFrame (PointKeyFrame *frame);
	
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);
	virtual void Resolve ();

	virtual Duration GetNaturalDurationCore (Clock *clock);

	virtual bool Validate ();
	
	//
	// Property Accessors
	//
	PointKeyFrameCollection *GetKeyFrames ();
	void SetKeyFrames (PointKeyFrameCollection* value);
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="Children" */
class Storyboard : public ParallelTimeline {
	static void storyboard_completed (EventObject *sender, EventArgs *calldata, gpointer data);
	static gboolean storyboard_tick (gpointer data);
	
	bool HookupAnimationsRecurse (Clock *clock, MoonError *error);
	void TeardownClockGroup ();
	gboolean Tick ();
	Clock *clock;
	Clock *root_clock;
	
 protected:
	virtual ~Storyboard ();

 public:
 	/* @PropertyType=string,Attached,GenerateAccessors,Validator=IsTimelineValidator */
	const static int TargetNameProperty;
 	/* @PropertyType=PropertyPath,Attached,GenerateAccessors,Validator=IsTimelineValidator */
	const static int TargetPropertyProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Storyboard ();
	
	bool Begin ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	bool BeginWithError (MoonError *error);

	void Pause ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void PauseWithError (MoonError *error);
	
	void Resume ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void ResumeWithError (MoonError *error);

	void Seek (TimeSpan timespan);

	/* @GenerateCBinding,GeneratePInvoke */
	void SeekWithError (TimeSpan timespan, MoonError *error);

	void Stop ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void StopWithError (MoonError *error);
	
	/* @GenerateCBinding,GeneratePInvoke */
	int GetCurrentState ();
	
	virtual Clock *AllocateClock ();

	static void SetTargetName (DependencyObject *o, const char *targetName);
	static const char *GetTargetName (DependencyObject *o);
	static void SetTargetProperty (DependencyObject *o, PropertyPath *targetProperty);
	static PropertyPath *GetTargetProperty (DependencyObject *o);
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="Storyboard" */
class BeginStoryboard : public TriggerAction {
 protected:
	virtual ~BeginStoryboard ();

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
};

G_BEGIN_DECLS

void key_spline_get_control_point_1 (KeySpline *ks, double *x, double *y);
void key_spline_set_control_point_1 (KeySpline *ks, double x, double y);

void key_spline_get_control_point_2 (KeySpline *ks, double *x, double *y);
void key_spline_set_control_point_2 (KeySpline *ks, double x, double y);

G_END_DECLS

#endif /* MOON_ANIMATION_H */
