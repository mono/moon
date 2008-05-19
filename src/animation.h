/*
 * animation.h: Animation engine
 *
 * Author:
 *   Chris Toshok (toshok@novell.com)
 *   Michael Dominic K. <mdk@mdk.am>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_ANIMATION_H
#define MOON_ANIMATION_H

#include "trigger.h"
#include "collection.h"
#include "clock.h"
#include "list.h"
#include "point.h"

// misc types
class KeySpline : public DependencyObject {
 protected:
	virtual ~KeySpline () {}

 public:
	KeySpline ();
	KeySpline (Point controlPoint1, Point controlPoint2);
	KeySpline (double x1, double y1, double x2, double y2);

	virtual Type::Kind GetObjectType () { return Type::KEYSPLINE; }

	double GetSplineProgress (double linearProgress);

	Point GetControlPoint1 ();
	void SetControlPoint1 (Point controlPoint1);

	Point GetControlPoint2 ();
	void SetControlPoint2 (Point controlPoint2);

 private:
	Point controlPoint1;
	Point controlPoint2;
};

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

	Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue);

	bool HookupStorage (DependencyObject *targetobj, DependencyProperty *targetprop);

	virtual void Stop ();

 private:
	Animation/*Timeline*/ *timeline;
	AnimationStorage *storage;
};





/* this is called AnimationTimeline in wpf */
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

class DoubleAnimation : public Animation/*Timeline*/ {
 protected:
	virtual ~DoubleAnimation () {}

 public:
	DoubleAnimation ();
	virtual Type::Kind GetObjectType () { return Type::DOUBLEANIMATION; };

	static DependencyProperty* ByProperty;
	static DependencyProperty* FromProperty;
	static DependencyProperty* ToProperty;

	double *GetBy ();
	void    SetBy (double* pv);
	void    SetBy (double v);

	double *GetFrom ();
	void    SetFrom (double* pv);
	void    SetFrom (double v);

	double *GetTo ();
	void    SetTo (double* pv);
	void    SetTo (double v);

	virtual Value *GetTargetValue (Value *defaultOriginValue);
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual Type::Kind GetValueKind () { return Type::DOUBLE; };
};


class ColorAnimation : public Animation/*Timeline*/ {
 protected:
	virtual ~ColorAnimation () {}

 public:
	ColorAnimation ();
	virtual Type::Kind GetObjectType () { return Type::COLORANIMATION; };

	static DependencyProperty* ByProperty;
	static DependencyProperty* FromProperty;
	static DependencyProperty* ToProperty;

	Color *GetBy ();
	void   SetBy (Color* pv);
	void   SetBy (Color v);

	Color *GetFrom ();
	void   SetFrom (Color* pv);
	void   SetFrom (Color v);

	Color *GetTo ();
	void   SetTo (Color* pv);
	void   SetTo (Color v);

	virtual Value *GetTargetValue (Value *defaultOriginValue);
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual Type::Kind GetValueKind () { return Type::COLOR; };
};


class PointAnimation : public Animation/*Timeline*/ {
 protected:
	virtual ~PointAnimation () {}

 public:
	virtual Type::Kind GetObjectType () { return Type::POINTANIMATION; };

	static DependencyProperty* ByProperty;
	static DependencyProperty* FromProperty;
	static DependencyProperty* ToProperty;

	Point *GetBy ();
	void   SetBy (Point* pv);
	void   SetBy (Point v);

	Point *GetFrom ();
	void   SetFrom (Point* pv);
	void   SetFrom (Point v);

	Point *GetTo ();
	void   SetTo (Point* pv);
	void   SetTo (Point v);

	virtual Value *GetTargetValue (Value *defaultOriginValue);
	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual Type::Kind GetValueKind () { return Type::POINT; };
};


class KeyFrame : public DependencyObject {
 protected:
	virtual ~KeyFrame () {}

 public:
	KeyFrame ();
	virtual Type::Kind GetObjectType () { return Type::KEYFRAME; };

	KeyTime *GetKeyTime();
	void SetKeyTime (KeyTime keytime);

	static DependencyProperty *KeyTimeProperty;

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

	bool resolved;
	TimeSpan resolved_keytime;
};

class KeyFrameCollection : public Collection {
 protected:
	virtual ~KeyFrameCollection ();

 public:

	KeyFrameCollection ();

	virtual Type::Kind GetObjectType() { return Type::KEYFRAME_COLLECTION; }
	virtual Type::Kind GetElementType() { return Type::KEYFRAME; }

	virtual int  Add (DependencyObject *obj);
	virtual bool Remove (DependencyObject *obj);
	virtual bool Insert (int index, DependencyObject *data);
	virtual void Clear ();

	GPtrArray *sorted_list;

	bool resolved;

	KeyFrame *GetKeyFrameForTime (TimeSpan t, KeyFrame **previous_frame);

	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
};

class ColorKeyFrameCollection : public KeyFrameCollection {
 protected:
	virtual ~ColorKeyFrameCollection () {}

 public:
	ColorKeyFrameCollection () { }

	virtual Type::Kind GetObjectType() { return Type::COLORKEYFRAME_COLLECTION; }
	virtual Type::Kind GetElementType() { return Type::COLORKEYFRAME; }
};

class DoubleKeyFrameCollection : public KeyFrameCollection {
 protected:
	virtual ~DoubleKeyFrameCollection () {}

 public:
	DoubleKeyFrameCollection () { }

	virtual Type::Kind GetObjectType() { return Type::DOUBLEKEYFRAME_COLLECTION; }
	virtual Type::Kind GetElementType() { return Type::DOUBLEKEYFRAME; }
};

class PointKeyFrameCollection : public KeyFrameCollection {
 protected:
	virtual ~PointKeyFrameCollection () {}

 public:
	PointKeyFrameCollection () { }

	virtual Type::Kind GetObjectType() { return Type::POINTKEYFRAME_COLLECTION; }
	virtual Type::Kind GetElementType() { return Type::POINTKEYFRAME; }
};

class DoubleKeyFrame : public KeyFrame {
 protected:
	virtual ~DoubleKeyFrame () {}

 public:
	DoubleKeyFrame ();
	virtual Type::Kind GetObjectType() { return Type::DOUBLEKEYFRAME; };

	double *GetValue ();
	void    SetValue (double* pv);
	void    SetValue (double v);

	static DependencyProperty *ValueProperty;
};

class ColorKeyFrame : public KeyFrame {
 protected:
	virtual ~ColorKeyFrame () {}

 public:
	ColorKeyFrame ();
	virtual Type::Kind GetObjectType () { return Type::COLORKEYFRAME; };

	Color *GetValue ();
	void   SetValue (Color* pv);
	void   SetValue (Color v);

	static DependencyProperty *ValueProperty;
};

class PointKeyFrame : public KeyFrame {
 protected:
	virtual ~PointKeyFrame () {}

 public:
	PointKeyFrame ();
	virtual Type::Kind GetObjectType () { return Type::POINTKEYFRAME; };

	Point *GetValue ();
	void   SetValue (Point* pv);
	void   SetValue (Point v);

	static DependencyProperty *ValueProperty;
};



class DiscreteDoubleKeyFrame : public DoubleKeyFrame {
 protected:
	virtual ~DiscreteDoubleKeyFrame () {}

 public:
	DiscreteDoubleKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::DISCRETEDOUBLEKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};




class DiscreteColorKeyFrame : public ColorKeyFrame {
 protected:
	virtual ~DiscreteColorKeyFrame () {}

 public:
	DiscreteColorKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::DISCRETECOLORKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};



class DiscretePointKeyFrame : public PointKeyFrame {
 protected:
	virtual ~DiscretePointKeyFrame () {}

 public:
	DiscretePointKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::DISCRETEPOINTKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};


class LinearDoubleKeyFrame : public DoubleKeyFrame {
 protected:
	virtual ~LinearDoubleKeyFrame () {}

 public:
	LinearDoubleKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::LINEARDOUBLEKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};



class LinearColorKeyFrame : public ColorKeyFrame {
 protected:
	virtual ~LinearColorKeyFrame () {}

 public:
	LinearColorKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::LINEARCOLORKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};



class LinearPointKeyFrame : public PointKeyFrame {
 protected:
	virtual ~LinearPointKeyFrame () {}

 public:
	LinearPointKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::LINEARPOINTKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};



class SplineDoubleKeyFrame : public DoubleKeyFrame {
 protected:
	virtual ~SplineDoubleKeyFrame () {}

 public:
	SplineDoubleKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::SPLINEDOUBLEKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

	KeySpline* GetKeySpline ();

	static DependencyProperty *KeySplineProperty;
};



class SplineColorKeyFrame : public ColorKeyFrame {
 protected:
	virtual ~SplineColorKeyFrame () {}

 public:
	SplineColorKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::SPLINECOLORKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

	KeySpline* GetKeySpline ();

	static DependencyProperty *KeySplineProperty;
};



class SplinePointKeyFrame : public PointKeyFrame {
 protected:
	virtual ~SplinePointKeyFrame () {}

 public:
	SplinePointKeyFrame () { }
	virtual Type::Kind GetObjectType () { return Type::SPLINEPOINTKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

	KeySpline* GetKeySpline ();

	static DependencyProperty *KeySplineProperty;
};


/* @ContentProperty="KeyFrames" */
class DoubleAnimationUsingKeyFrames : public DoubleAnimation {
 protected:
	virtual ~DoubleAnimationUsingKeyFrames ();

 public:
	DoubleAnimationUsingKeyFrames ();
	virtual Type::Kind GetObjectType () { return Type::DOUBLEANIMATIONUSINGKEYFRAMES; };

	void AddKeyFrame (DoubleKeyFrame *frame);
	void RemoveKeyFrame (DoubleKeyFrame *frame);

	static DependencyProperty *KeyFramesProperty;

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual void Resolve ();

	virtual Duration GetNaturalDurationCore (Clock* clock);

	virtual bool Validate ();
};


/* ContentProperty="KeyFrames" */
class ColorAnimationUsingKeyFrames : public ColorAnimation {
 protected:
	virtual ~ColorAnimationUsingKeyFrames ();

 public:
	ColorAnimationUsingKeyFrames ();
	virtual Type::Kind GetObjectType () { return Type::COLORANIMATIONUSINGKEYFRAMES; };

	void AddKeyFrame (ColorKeyFrame *frame);
	void RemoveKeyFrame (ColorKeyFrame *frame);

	static DependencyProperty *KeyFramesProperty;

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual void Resolve ();

	virtual Duration GetNaturalDurationCore (Clock* clock);

	virtual bool Validate ();
};

/* @ContentProperty="KeyFrames" */
class PointAnimationUsingKeyFrames : public PointAnimation {
 protected:
	virtual ~PointAnimationUsingKeyFrames ();

 public:
	PointAnimationUsingKeyFrames ();
	virtual Type::Kind GetObjectType () { return Type::POINTANIMATIONUSINGKEYFRAMES; };

	void AddKeyFrame (PointKeyFrame *frame);
	void RemoveKeyFrame (PointKeyFrame *frame);

	static DependencyProperty *KeyFramesProperty;

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual void Resolve ();

	virtual Duration GetNaturalDurationCore (Clock* clock);

	virtual bool Validate ();
};

/* @ContentProperty="Children" */
class Storyboard : public ParallelTimeline {
 protected:
	virtual ~Storyboard ();

 public:
	Storyboard ();
	virtual Type::Kind GetObjectType () { return Type::STORYBOARD; };

	bool Begin ();
	void Pause ();
	void Resume ();
	void Seek (TimeSpan timespan);
	void Stop ();
	
	virtual void SetSurface (Surface *surface);

	static DependencyProperty* TargetNameProperty;
	static DependencyProperty* TargetPropertyProperty;

	// XXX event Completed

	static void SetTargetName (DependencyObject *o, const char *targetName);
	static char* GetTargetName (DependencyObject *o);
	static void SetTargetProperty (DependencyObject *o, const char *targetProperty);
	static char* GetTargetProperty (DependencyObject *o);

	// events you can AddHandler to
	const static int CompletedEvent;

 private:
	void HookupAnimationsRecurse (Clock *clock);
	Clock *root_clock;

	gboolean Tick ();
	static gboolean storyboard_tick (gpointer data);
	static void invoke_completed (EventObject *sender, EventArgs *calldata, gpointer data);

	void TeardownClockGroup ();
	static void teardown_clockgroup (EventObject *sender, EventArgs *calldata, gpointer data);
};

/* @ContentProperty="Storyboard" */
class BeginStoryboard : public TriggerAction {
 protected:
	virtual ~BeginStoryboard ();

 public:
	BeginStoryboard () { }
	
	virtual Type::Kind GetObjectType () { return Type::BEGINSTORYBOARD; };

	
	void Fire ();

	void SetStoryboard (Storyboard *sb);
	Storyboard *GetStoryboard ();

	static DependencyProperty* StoryboardProperty;
};

G_BEGIN_DECLS

KeySpline *key_spline_new (void);

DoubleAnimation* double_animation_new (void);
ColorAnimation*  color_animation_new (void);
PointAnimation*  point_animation_new (void);

KeyFrame* key_frame_new (void);

ColorKeyFrameCollection *color_key_frame_collection_new (void);
DoubleKeyFrameCollection *double_key_frame_collection_new (void);
PointKeyFrameCollection *point_key_frame_collection_new (void);

DoubleKeyFrame* double_key_frame_new (void);
ColorKeyFrame*  color_key_frame_new (void);
PointKeyFrame*  point_key_frame_new (void);

DiscreteDoubleKeyFrame* discrete_double_key_frame_new (void);
DiscreteColorKeyFrame*  discrete_color_key_frame_new (void);
DiscretePointKeyFrame*  discrete_point_key_frame_new (void);

LinearDoubleKeyFrame* linear_double_key_frame_new (void);
LinearColorKeyFrame*  linear_color_key_frame_new (void);
LinearPointKeyFrame*  linear_point_key_frame_new (void);

SplineDoubleKeyFrame* spline_double_key_frame_new (void);
SplineColorKeyFrame*  spline_color_key_frame_new (void);
SplinePointKeyFrame*  spline_point_key_frame_new (void);

DoubleAnimationUsingKeyFrames* double_animation_using_key_frames_new (void);
ColorAnimationUsingKeyFrames*  color_animation_using_key_frames_new (void);
PointAnimationUsingKeyFrames*  point_animation_using_key_frames_new (void);


BeginStoryboard *begin_storyboard_new (void);

Storyboard *storyboard_new    (void);
void        storyboard_begin  (Storyboard *sb);
void        storyboard_pause  (Storyboard *sb);
void        storyboard_resume (Storyboard *sb);
void        storyboard_seek   (Storyboard *sb, TimeSpan ts);
void        storyboard_stop   (Storyboard *sb);


void animation_init (void);
void animation_shutdown (void);

G_END_DECLS

#endif /* MOON_ANIMATION_H */
