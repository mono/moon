#ifndef MOON_ANIMATION_H
#define MOON_ANIMATION_H

#include "runtime.h"
#include "clock.h"

G_BEGIN_DECLS

// misc types
class KeySpline : public DependencyObject {
 public:
	KeySpline ();
	KeySpline (Point controlPoint1, Point controlPoint2);
	KeySpline (double x1, double y1, double x2, double y2);

	Value::Kind GetObjectType () { return Value::KEYSPLINE; }

	double GetSplineProgress (double linearProgress);

	Point GetControlPoint1 ();
	void SetControlPoint1 (Point controlPoint1);

	Point GetControlPoint2 ();
	void SetControlPoint2 (Point controlPoint2);

 private:
	Point controlPoint1;
	Point controlPoint2;
};

KeySpline *key_spline_new (void);


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
	    timespan (0),
            percent (percent) { }

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

	double GetPercent () { return percent; }
	TimeSpan GetTimeSpan () { return timespan; }

  private:
	KeyTimeType k;
	double percent;
	TimeSpan timespan;
};


//
// Animations (more specialized clocks and timelines) and their subclasses
//
class Animation;
class AnimationClock;

// internal WPF class gleaned from stack traces
class AnimationStorage {
 public:
	AnimationStorage (AnimationClock *clock, Animation/*Timeline*/ *timeline,
			  DependencyObject *targetobj, DependencyProperty *targetprop);
	~AnimationStorage ();
	
 private:
	void UpdatePropertyValue ();
	static void update_property_value (gpointer data);

	AnimationClock *clock;
	Animation/*Timeline*/* timeline;
	DependencyObject *targetobj;
	DependencyProperty *targetprop;
	Value *baseValue;
};





class Animation/*Timeline*/;

class AnimationClock : public Clock {
 public:
	AnimationClock (Animation/*Timeline*/ *timeline);
	~AnimationClock ();
	virtual Value::Kind GetObjectType () { return Value::ANIMATIONCLOCK; };

	Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue);

	void HookupStorage (DependencyObject *targetobj, DependencyProperty *targetprop);
 private:
	Animation/*Timeline*/ *timeline;
	AnimationStorage *storage;
};





/* this is called AnimationTimeline in wpf */
class Animation/*Timeline*/ : public Timeline {
 public:

	Animation/*Timeline*/ () { };
	virtual Value::Kind GetObjectType () { return Value::ANIMATION; };

	virtual Clock *AllocateClock () { return new AnimationClock (this); }

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual Duration GetNaturalDurationCore (Clock* clock);
};


#define NULLABLE_GETSET_DECL(prop, t) \
void Set##prop (t v); \
void Set##prop (t* pv); \
t* Get##prop ()

#define SET_NULLABLE_FUNC(t) \
static void SetNullable##t##Prop (DependencyObject *obj, DependencyProperty *prop, t *pv) \
{ \
  if (!pv) \
    obj->SetValue (prop, NULL); \
  else \
    obj->SetValue (prop, Value(*pv)); \
}

#define NULLABLE_GETSET_IMPL(klass,prop,t,T) \
void klass::Set##prop (t v) { Set##prop (&v); } \
void klass::Set##prop (t *pv) { SetNullable##t##Prop (this, klass::prop##Property, pv); } \
t* klass::Get##prop () { Value* v = this->DependencyObject::GetValue (klass::prop##Property);  return v ? v->As##T () : NULL; }

#define NULLABLE_PRIM_GETSET_IMPL(klass,prop,t,T) \
void klass::Set##prop (t v) { Set##prop (&v); } \
void klass::Set##prop (t *pv) { SetNullable##t##Prop (this, klass::prop##Property, pv); } \
t* klass::Get##prop () { Value* v = this->DependencyObject::GetValue (klass::prop##Property);  return v ? v->AsNullable##T () : NULL; }


class DoubleAnimation : public Animation/*Timeline*/ {
 public:
	DoubleAnimation ();
	virtual Value::Kind GetObjectType () { return Value::DOUBLEANIMATION; };

	static DependencyProperty* ByProperty;
	static DependencyProperty* FromProperty;
	static DependencyProperty* ToProperty;

	NULLABLE_GETSET_DECL(By, double);
	NULLABLE_GETSET_DECL(From, double);
	NULLABLE_GETSET_DECL(To, double);

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);
};

DoubleAnimation *double_animation_new (void);



class ColorAnimation : public Animation/*Timeline*/ {
 public:
	ColorAnimation ();
	virtual Value::Kind GetObjectType () { return Value::COLORANIMATION; };

	static DependencyProperty* ByProperty;
	static DependencyProperty* FromProperty;
	static DependencyProperty* ToProperty;

	NULLABLE_GETSET_DECL(By, Color);
	NULLABLE_GETSET_DECL(From, Color);
	NULLABLE_GETSET_DECL(To, Color);

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);
};

ColorAnimation *color_animation_new (void);



class PointAnimation : public Animation/*Timeline*/ {
 public:
	PointAnimation () {};
	virtual Value::Kind GetObjectType () { return Value::POINTANIMATION; };

	static DependencyProperty* ByProperty;
	static DependencyProperty* FromProperty;
	static DependencyProperty* ToProperty;

	NULLABLE_GETSET_DECL(By, Point);
	NULLABLE_GETSET_DECL(From, Point);
	NULLABLE_GETSET_DECL(To, Point);

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);
};

PointAnimation *point_animation_new (void);



class KeyFrame : public DependencyObject {
 public:
	KeyFrame ();
	virtual Value::Kind GetObjectType () { return Value::KEYFRAME; };

	KeyTime *GetKeyTime();
	void SetKeyTime (KeyTime keytime);

	static DependencyProperty *KeyTimeProperty;

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress) = 0;
};



class KeyFrameCollection : public Collection {
 public:
	KeyFrameCollection () : sorted_list(NULL) {}

	virtual Value::Kind GetObjectType() { return Value::KEYFRAME_COLLECTION; }
	virtual Value::Kind GetElementType() { return Value::KEYFRAME; }

	virtual void Add (DependencyObject *obj);
	virtual void Remove (DependencyObject *obj);

	KeyFrame *GetKeyFrameForTime (TimeSpan t, KeyFrame **previous_frame);

 private:
	void Resort ();
	GSList *sorted_list;
};

KeyFrameCollection *key_frame_collection_new (void);


class DoubleKeyFrame : public KeyFrame {
 public:
	DoubleKeyFrame ();
	virtual Value::Kind GetObjectType() { return Value::DOUBLEKEYFRAME; };
	NULLABLE_GETSET_DECL (Value, double);

	static DependencyProperty *ValueProperty;
};



class ColorKeyFrame : public KeyFrame {
 public:
	ColorKeyFrame ();
	virtual Value::Kind GetObjectType () { return Value::COLORKEYFRAME; };

	NULLABLE_GETSET_DECL(Value, Color);

	static DependencyProperty *ValueProperty;
};


class PointKeyFrame : public KeyFrame {
 public:
	PointKeyFrame ();
	virtual Value::Kind GetObjectType () { return Value::POINTKEYFRAME; };

	NULLABLE_GETSET_DECL(Value, Point);

	static DependencyProperty *ValueProperty;
};




class DiscreteDoubleKeyFrame : public DoubleKeyFrame {
 public:
	DiscreteDoubleKeyFrame () { }
	virtual Value::Kind GetObjectType () { return Value::DISCRETEDOUBLEKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};

DiscreteDoubleKeyFrame *discrete_double_key_frame_new (void);


class DiscreteColorKeyFrame : public ColorKeyFrame {
 public:
	DiscreteColorKeyFrame () { }
	virtual Value::Kind GetObjectType () { return Value::DISCRETECOLORKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};

DiscreteColorKeyFrame *discrete_color_key_frame_new (void);


class DiscretePointKeyFrame : public PointKeyFrame {
 public:
	DiscretePointKeyFrame () { }
	virtual Value::Kind GetObjectType () { return Value::DISCRETEPOINTKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};

DiscretePointKeyFrame *discrete_point_key_frame_new (void);


class LinearDoubleKeyFrame : public DoubleKeyFrame {
 public:
	LinearDoubleKeyFrame () { }
	virtual Value::Kind GetObjectType () { return Value::LINEARDOUBLEKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};

LinearDoubleKeyFrame *linear_double_key_frame_new (void);


class LinearColorKeyFrame : public ColorKeyFrame {
 public:
	LinearColorKeyFrame () { }
	virtual Value::Kind GetObjectType () { return Value::LINEARCOLORKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};

LinearColorKeyFrame *linear_color_key_frame_new (void);


class LinearPointKeyFrame : public PointKeyFrame {
 public:
	LinearPointKeyFrame () { }
	virtual Value::Kind GetObjectType () { return Value::LINEARPOINTKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);
};

LinearPointKeyFrame *linear_point_key_frame_new (void);


class SplineDoubleKeyFrame : public DoubleKeyFrame {
 public:
	SplineDoubleKeyFrame () { }
	virtual Value::Kind GetObjectType () { return Value::SPLINEDOUBLEKEYFRAME; };

	virtual Value *InterpolateValue (Value *baseValue, double keyFrameProgress);

	KeySpline* GetKeySpline ();

	static DependencyProperty *KeySplineProperty;
};

SplineDoubleKeyFrame *spline_double_key_frame_new (void);


class DoubleAnimationUsingKeyFrames : public DoubleAnimation {
 public:
	DoubleAnimationUsingKeyFrames ();
	~DoubleAnimationUsingKeyFrames ();
	virtual Value::Kind GetObjectType () { return Value::DOUBLEANIMATIONUSINGKEYFRAMES; };

	void AddKeyFrame (DoubleKeyFrame *frame);
	void RemoveKeyFrame (DoubleKeyFrame *frame);

	static DependencyProperty *KeyFramesProperty;

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual void OnPropertyChanged (DependencyProperty *prop);

	virtual Duration GetNaturalDurationCore (Clock* clock);
};

DoubleAnimationUsingKeyFrames *double_animation_using_key_frames_new (void);


class ColorAnimationUsingKeyFrames : public ColorAnimation {
 public:
	ColorAnimationUsingKeyFrames ();
	~ColorAnimationUsingKeyFrames();
	virtual Value::Kind GetObjectType () { return Value::COLORANIMATIONUSINGKEYFRAMES; };

	void AddKeyFrame (ColorKeyFrame *frame);
	void RemoveKeyFrame (ColorKeyFrame *frame);

	static DependencyProperty *KeyFramesProperty;

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual void OnPropertyChanged (DependencyProperty *prop);

	virtual Duration GetNaturalDurationCore (Clock* clock);
};

ColorAnimationUsingKeyFrames *color_animation_using_key_frames_new (void);


class PointAnimationUsingKeyFrames : public PointAnimation {
 public:
	PointAnimationUsingKeyFrames ();
	~PointAnimationUsingKeyFrames();
	virtual Value::Kind GetObjectType () { return Value::POINTANIMATIONUSINGKEYFRAMES; };

	void AddKeyFrame (PointKeyFrame *frame);
	void RemoveKeyFrame (PointKeyFrame *frame);

	static DependencyProperty *KeyFramesProperty;

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	virtual void OnPropertyChanged (DependencyProperty *prop);

	virtual Duration GetNaturalDurationCore (Clock* clock);
};

PointAnimationUsingKeyFrames *point_animation_using_key_frames_new (void);


class Storyboard : public ParallelTimeline {
 public:
	Storyboard ();
	virtual Value::Kind GetObjectType () { return Value::STORYBOARD; };

	void Begin ();
	void Pause ();
	void Resume ();
	void Seek (TimeSpan timespan);
	void Stop ();
	~Storyboard ();

	static DependencyProperty* TargetNameProperty;
	static DependencyProperty* TargetPropertyProperty;

	// XXX event Completed

	static void SetTargetName (DependencyObject *o, char *targetName);
	static char* GetTargetName (DependencyObject *o);
	static void SetTargetProperty (DependencyObject *o, char *targetProperty);
	static char* GetTargetProperty (DependencyObject *o);

 private:
	void HookupAnimationsRecurse (Clock *clock);
	Clock *root_clock;

	gboolean Tick ();
	static gboolean storyboard_tick (gpointer data);
};

Storyboard *storyboard_new (void);
void storyboard_begin  (Storyboard *sb);
void storyboard_pause  (Storyboard *sb);
void storyboard_resume (Storyboard *sb);
void storyboard_seek   (Storyboard *sb, TimeSpan ts);
void storyboard_stop   (Storyboard *sb);



class BeginStoryboard : public TriggerAction {
 public:
	BeginStoryboard () { }
	~BeginStoryboard ();
	
	virtual Value::Kind GetObjectType () { return Value::BEGINSTORYBOARD; };

	
	void Fire ();

	void SetStoryboard (Storyboard *sb);
	Storyboard *GetStoryboard ();

	static DependencyProperty* StoryboardProperty;
};

BeginStoryboard *begin_storyboard_new (void);

G_END_DECLS

#endif /* MOON_ANIMATION_H */
