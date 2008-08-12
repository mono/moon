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
#include "moon-curves.h"

// misc types
/* @Namespace=System.Windows.Media.Animation */
class KeySpline : public DependencyObject {
	moon_quadratic quadraticsArray [16];
	Point controlPoint1;
	Point controlPoint2;
	
 protected:
	virtual ~KeySpline () {}
	
 public:
	/* @GenerateCBinding */
	KeySpline ();
	KeySpline (Point controlPoint1, Point controlPoint2);
	KeySpline (double x1, double y1, double x2, double y2);

	virtual Type::Kind GetObjectType () { return Type::KEYSPLINE; }

	double GetSplineProgress (double linearProgress);

	Point GetControlPoint1 ();
	void SetControlPoint1 (Point controlPoint1);

	Point GetControlPoint2 ();
	void SetControlPoint2 (Point controlPoint2);
};

/* @IncludeInKinds */
/* @Namespace=System.Windows.Media.Animation */
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
	bool IsCurrentStorage ();
	Value* GetResetValue ();
	void UpdatePropertyValueWith (Value *v);
	Value* GetStopValue (void);

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
};


class Animation/*Timeline*/;

class AnimationClock : public Clock {
 protected:
	virtual ~AnimationClock ();

 public:
	AnimationClock (Animation/*Timeline*/ *timeline);
	virtual Type::Kind GetObjectType () { return Type::ANIMATIONCLOCK; };
	virtual void ExtraRepeatAction ();
	virtual void OnSurfaceDetach ();
	virtual void OnSurfaceReAttach ();

	Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue);

	bool HookupStorage (DependencyObject *targetobj, DependencyProperty *targetprop);

	virtual void Stop ();

 private:
	Animation/*Timeline*/ *timeline;
	AnimationStorage *storage;
};





/* this is called AnimationTimeline in wpf */
/* @Namespace=System.Windows.Media.Animation */
class Animation/*Timeline*/ : public Timeline {
 protected:
	virtual ~Animation () {}

 public:

	Animation/*Timeline*/ () { };
	virtual Type::Kind GetObjectType () { return Type::ANIMATION; };
	
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

 public:
 	/* @PropertyType=double,Nullable */
	static DependencyProperty *ByProperty;
	/* @PropertyType=double,Nullable */
	static DependencyProperty *FromProperty;
	/* @PropertyType=double,Nullable */
	static DependencyProperty *ToProperty;
	
	/* @GenerateCBinding */
	DoubleAnimation ();
	virtual Type::Kind GetObjectType () { return Type::DOUBLEANIMATION; };
	virtual Type::Kind GetValueKind () { return Type::DOUBLE; };
	
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

 public:
 	/* @PropertyType=Color,Nullable */
	static DependencyProperty *ByProperty;
 	/* @PropertyType=Color,Nullable */
	static DependencyProperty *FromProperty;
 	/* @PropertyType=Color,Nullable */
	static DependencyProperty *ToProperty;
	
	/* @GenerateCBinding */
	ColorAnimation ();
	virtual Type::Kind GetObjectType () { return Type::COLORANIMATION; };
	virtual Type::Kind GetValueKind () { return Type::COLOR; };
	
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
	virtual ~PointAnimation () {}

 public:
 	/* @PropertyType=Point,Nullable */
	static DependencyProperty *ByProperty;
 	/* @PropertyType=Point,Nullable */
	static DependencyProperty *FromProperty;
 	/* @PropertyType=Point,Nullable */
	static DependencyProperty *ToProperty;
	
 	/* @GenerateCBinding */
 	PointAnimation () {}
 	
	virtual Type::Kind GetObjectType () { return Type::POINTANIMATION; };
	virtual Type::Kind GetValueKind () { return Type::POINT; };
	
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


/* @Namespace=System.Windows.Media.Animation */
class KeyFrame : public DependencyObject {
 protected:
	virtual ~KeyFrame () {}

 public:
 	/* @PropertyType=KeyTime,Nullable */
	static DependencyProperty *KeyTimeProperty;
	TimeSpan resolved_keytime;
	bool resolved;
	
	/* @GenerateCBinding */
	KeyFrame ();
	virtual Type::Kind GetObjectType () { return Type::KEYFRAME; };
	
	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	KeyTime *GetKeyTime();
	void SetKeyTime (KeyTime keytime);
};

class KeyFrameCollection : public DependencyObjectCollection {
 protected:
	virtual void AddedToCollection (Value *value);
	virtual void RemovedFromCollection (Value *value);
	
	virtual ~KeyFrameCollection ();

 public:
	GPtrArray *sorted_list;
	bool resolved;
	
	/* @GenerateCBinding */
	KeyFrameCollection ();

	virtual Type::Kind GetObjectType() { return Type::KEYFRAME_COLLECTION; }
	virtual Type::Kind GetElementType() { return Type::KEYFRAME; }
	
	virtual void Clear ();
	
	KeyFrame *GetKeyFrameForTime (TimeSpan t, KeyFrame **previous_frame);

	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
};

/* @Namespace=System.Windows.Media.Animation */
class ColorKeyFrameCollection : public KeyFrameCollection {
 protected:
	virtual ~ColorKeyFrameCollection () {}

 public:
	/* @GenerateCBinding */
	ColorKeyFrameCollection () { }

	virtual Type::Kind GetObjectType() { return Type::COLORKEYFRAME_COLLECTION; }
	virtual Type::Kind GetElementType() { return Type::COLORKEYFRAME; }
};

/* @Namespace=System.Windows.Media.Animation */
class DoubleKeyFrameCollection : public KeyFrameCollection {
 protected:
	virtual ~DoubleKeyFrameCollection () {}

 public:
	/* @GenerateCBinding */
	DoubleKeyFrameCollection () { }

	virtual Type::Kind GetObjectType() { return Type::DOUBLEKEYFRAME_COLLECTION; }
	virtual Type::Kind GetElementType() { return Type::DOUBLEKEYFRAME; }
};

/* @Namespace=System.Windows.Media.Animation */
class PointKeyFrameCollection : public KeyFrameCollection {
 protected:
	virtual ~PointKeyFrameCollection () {}

 public:
	/* @GenerateCBinding */
	PointKeyFrameCollection () { }

	virtual Type::Kind GetObjectType() { return Type::POINTKEYFRAME_COLLECTION; }
	virtual Type::Kind GetElementType() { return Type::POINTKEYFRAME; }
};

/* @Namespace=System.Windows.Media.Animation */
class DoubleKeyFrame : public KeyFrame {
 protected:
	virtual ~DoubleKeyFrame () {}

 public:
 	/* @PropertyType=double,Nullable */
	static DependencyProperty *ValueProperty;
	
	/* @GenerateCBinding */
	DoubleKeyFrame ();
	virtual Type::Kind GetObjectType() { return Type::DOUBLEKEYFRAME; };
	
	//
	// Property Accessors
	//
	double *GetValue ();
	void    SetValue (double *pv);
	void    SetValue (double v);
};

/* @Namespace=System.Windows.Media.Animation */
class ColorKeyFrame : public KeyFrame {
 protected:
	virtual ~ColorKeyFrame () {}

 public:
 	/* @PropertyType=Color,Nullable */
	static DependencyProperty *ValueProperty;
	
	/* @GenerateCBinding */
	ColorKeyFrame ();
	virtual Type::Kind GetObjectType () { return Type::COLORKEYFRAME; };
	
	//
	// Property Accessors
	//
	Color *GetValue ();
	void   SetValue (Color *pv);
	void   SetValue (Color v);
};

/* @Namespace=System.Windows.Media.Animation */
class PointKeyFrame : public KeyFrame {
 protected:
	virtual ~PointKeyFrame () {}

 public:
 	/* @PropertyType=Point,Nullable */
	static DependencyProperty *ValueProperty;
	
	/* @GenerateCBinding */
	PointKeyFrame ();
	virtual Type::Kind GetObjectType () { return Type::POINTKEYFRAME; };
	
	//
	// Property Accessors
	//
	Point *GetValue ();
	void   SetValue (Point *pv);
	void   SetValue (Point v);
};



/* @Namespace=System.Windows.Media.Animation */
class DiscreteDoubleKeyFrame : public DoubleKeyFrame {
 protected:
	virtual ~DiscreteDoubleKeyFrame () {}

 public:
	/* @GenerateCBinding */
	DiscreteDoubleKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::DISCRETEDOUBLEKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};




/* @Namespace=System.Windows.Media.Animation */
class DiscreteColorKeyFrame : public ColorKeyFrame {
 protected:
	virtual ~DiscreteColorKeyFrame () {}

 public:
	/* @GenerateCBinding */
	DiscreteColorKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::DISCRETECOLORKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};



/* @Namespace=System.Windows.Media.Animation */
class DiscretePointKeyFrame : public PointKeyFrame {
 protected:
	virtual ~DiscretePointKeyFrame () {}

 public:
	/* @GenerateCBinding */
	DiscretePointKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::DISCRETEPOINTKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};


/* @Namespace=System.Windows.Media.Animation */
class LinearDoubleKeyFrame : public DoubleKeyFrame {
 protected:
	virtual ~LinearDoubleKeyFrame () {}

 public:
	/* @GenerateCBinding */
	LinearDoubleKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::LINEARDOUBLEKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};



/* @Namespace=System.Windows.Media.Animation */
class LinearColorKeyFrame : public ColorKeyFrame {
 protected:
	virtual ~LinearColorKeyFrame () {}

 public:
	/* @GenerateCBinding */
	LinearColorKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::LINEARCOLORKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};



/* @Namespace=System.Windows.Media.Animation */
class LinearPointKeyFrame : public PointKeyFrame {
 protected:
	virtual ~LinearPointKeyFrame () {}

 public:
	/* @GenerateCBinding */
	LinearPointKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::LINEARPOINTKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};



/* @Namespace=System.Windows.Media.Animation */
class SplineDoubleKeyFrame : public DoubleKeyFrame {
 protected:
	virtual ~SplineDoubleKeyFrame () {}

 public:
 	/* @PropertyType=KeySpline */
	static DependencyProperty *KeySplineProperty;
	
	/* @GenerateCBinding */
	SplineDoubleKeyFrame ();
	virtual Type::Kind GetObjectType () { return Type::SPLINEDOUBLEKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	KeySpline *GetKeySpline ();
};



/* @Namespace=System.Windows.Media.Animation */
class SplineColorKeyFrame : public ColorKeyFrame {
 protected:
	virtual ~SplineColorKeyFrame () {}

 public:
 	/* @PropertyType=KeySpline */
	static DependencyProperty *KeySplineProperty;
	
	/* @GenerateCBinding */
	SplineColorKeyFrame ();
	virtual Type::Kind GetObjectType () { return Type::SPLINECOLORKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	KeySpline *GetKeySpline ();
};



/* @Namespace=System.Windows.Media.Animation */
class SplinePointKeyFrame : public PointKeyFrame {
 protected:
	virtual ~SplinePointKeyFrame () {}

 public:
 	/* @PropertyType=KeySpline */
	static DependencyProperty *KeySplineProperty;
	
	/* @GenerateCBinding */
	SplinePointKeyFrame ();
	virtual Type::Kind GetObjectType () { return Type::SPLINEPOINTKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
	
	//
	// Property Accessors
	//
	KeySpline *GetKeySpline ();
};


/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class DoubleAnimationUsingKeyFrames : public DoubleAnimation {
 protected:
	virtual ~DoubleAnimationUsingKeyFrames ();

 public:
 	/* @PropertyType=DoubleKeyFrameCollection */
	static DependencyProperty *KeyFramesProperty;
	
	/* @GenerateCBinding */
	DoubleAnimationUsingKeyFrames ();
	virtual Type::Kind GetObjectType () { return Type::DOUBLEANIMATIONUSINGKEYFRAMES; };

	void AddKeyFrame (DoubleKeyFrame *frame);
	void RemoveKeyFrame (DoubleKeyFrame *frame);
	
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual void Resolve ();

	virtual Duration GetNaturalDurationCore (Clock* clock);

	virtual bool Validate ();
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class ColorAnimationUsingKeyFrames : public ColorAnimation {
 protected:
	virtual ~ColorAnimationUsingKeyFrames ();

 public:
 	/* @PropertyType=ColorKeyFrameCollection */
	static DependencyProperty *KeyFramesProperty;
	
	/* @GenerateCBinding */
	ColorAnimationUsingKeyFrames ();
	virtual Type::Kind GetObjectType () { return Type::COLORANIMATIONUSINGKEYFRAMES; };

	void AddKeyFrame (ColorKeyFrame *frame);
	void RemoveKeyFrame (ColorKeyFrame *frame);

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual void Resolve ();

	virtual Duration GetNaturalDurationCore (Clock* clock);

	virtual bool Validate ();
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="KeyFrames" */
class PointAnimationUsingKeyFrames : public PointAnimation {
 protected:
	virtual ~PointAnimationUsingKeyFrames ();

 public:
 	/* @PropertyType=PointKeyFrameCollection */
	static DependencyProperty *KeyFramesProperty;
	
	/* @GenerateCBinding */
	PointAnimationUsingKeyFrames ();
	virtual Type::Kind GetObjectType () { return Type::POINTANIMATIONUSINGKEYFRAMES; };

	void AddKeyFrame (PointKeyFrame *frame);
	void RemoveKeyFrame (PointKeyFrame *frame);
	
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock *animationClock);

	virtual void Resolve ();

	virtual Duration GetNaturalDurationCore (Clock *clock);

	virtual bool Validate ();
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="Children" */
class Storyboard : public ParallelTimeline {
	static void storyboard_completed (EventObject *sender, EventArgs *calldata, gpointer data);
	static gboolean storyboard_tick (gpointer data);
	
	void HookupAnimationsRecurse (Clock *clock);
	void TeardownClockGroup ();
	gboolean Tick ();
	
	Clock *root_clock;
	
 protected:
	virtual ~Storyboard ();

 public:
 	/* @PropertyType=char*,Attached */
	static DependencyProperty *TargetNameProperty;
 	/* @PropertyType=char*,Attached */
	static DependencyProperty *TargetPropertyProperty;
	
	/* @GenerateCBinding */
	Storyboard ();
	virtual Type::Kind GetObjectType () { return Type::STORYBOARD; };
	
	virtual void SetSurface (Surface *surface);
	
	bool Begin ();
	void Pause ();
	void Resume ();
	void Seek (TimeSpan timespan);
	void Stop ();
	
	// XXX event Completed

	static void SetTargetName (DependencyObject *o, const char *targetName);
	static char *GetTargetName (DependencyObject *o);
	static void SetTargetProperty (DependencyObject *o, const char *targetProperty);
	static char *GetTargetProperty (DependencyObject *o);

	// events you can AddHandler to
	const static int CompletedEvent;
};

/* @Namespace=System.Windows.Media.Animation */
/* @ContentProperty="Storyboard" */
class BeginStoryboard : public TriggerAction {
 protected:
	virtual ~BeginStoryboard ();

 public:
 	/* @PropertyType=Storyboard*/
	static DependencyProperty *StoryboardProperty;
	
	/* @GenerateCBinding */
	BeginStoryboard () { }
	
	virtual Type::Kind GetObjectType () { return Type::BEGINSTORYBOARD; };
	
	void Fire ();
	
	//
	// Property Accessors
	//
	void SetStoryboard (Storyboard *sb);
	Storyboard *GetStoryboard ();
};

G_BEGIN_DECLS

void key_spline_set_control_point_1 (KeySpline *k, double x, double y);
void key_spline_set_control_point_2 (KeySpline *k, double x, double y);
void key_spline_get_control_point_1 (KeySpline *k, double *x, double *y);
void key_spline_get_control_point_2 (KeySpline *k, double *x, double *y);

void        storyboard_begin  (Storyboard *sb);
void        storyboard_pause  (Storyboard *sb);
void        storyboard_resume (Storyboard *sb);
void        storyboard_seek   (Storyboard *sb, TimeSpan ts);
void        storyboard_stop   (Storyboard *sb);

void animation_shutdown (void);

G_END_DECLS

#endif /* MOON_ANIMATION_H */
