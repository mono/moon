#include "runtime.h"

// misc types
struct Duration {
 public:
	enum DurationKind {
		TIMESPAN,
		AUTOMATIC,
		FOREVER
	};

	Duration (guint64/*TimeSpan*/ duration)
	  : k (TIMESPAN),
	    timespan (duration) { }

	Duration (const Duration &duration)
	{
		k = duration.k;
		timespan = duration.timespan;
	}

	Duration (DurationKind kind) : k(kind) { };

	bool HasTimeSpan () { return k == TIMESPAN; }
	guint64/*TimeSpan*/ GetTimeSpan() { return timespan; }

	static Duration Automatic;
	static Duration Forever;

	// XXX tons more operators here
	bool operator!= (const Duration &v) const
	{
		return !(*this == v);
	}

	bool operator== (const Duration &v) const
	{
		if (v.k != k)
			return false;

		if (v.k == TIMESPAN)
			return timespan == v.timespan;

		return true;
	}


	// This should live in a TimeSpan class, but oh well.. */
	static Duration FromSeconds (int seconds) { return (guint64)seconds * 1000000; }

	DurationKind k;
 private:
	guint64/*TimeSpan*/ timespan;
};





struct RepeatBehavior {
  public:
	enum RepeatKind {
		COUNT,
		DURATION,
		FOREVER
	};

	RepeatBehavior (const RepeatBehavior &repeat)
	{
		k = repeat.k;
		duration = repeat.duration;
		count = repeat.count;
	}

	RepeatBehavior (double count)
	  : k (COUNT),
	    count (count) { }

	RepeatBehavior (RepeatKind kind) : k(kind) { }

	RepeatBehavior (guint64/*TimeSpan*/ duration)
	  : k (DURATION),
	    duration (duration)
	{
	}

	static RepeatBehavior Forever;

	bool operator!= (const RepeatBehavior &v) const
	{
		return !(*this == v);
	}

	bool operator== (const RepeatBehavior &v) const
	{
		if (v.k != k)
			return false;

		switch (k) {
		case DURATION: return duration == v.duration;
		case COUNT: return count == v.count;
		case FOREVER: return true;
		}
	}

	double GetCount () { return count; }
	guint64/*TimeSpan*/ GetDuration() { return duration; }

	bool HasCount() { return k == COUNT; }
	bool HasDuration () { return k == DURATION; }

	RepeatKind k;
  private:
	double count;
	guint64/*TimeSpan*/ duration;
};





// our root level time manager (basically the object that registers
// the gtk_timeout and drives all Clock objects
class Clock;

class TimeManager {
 public:
	void Start ();

	void Tick ();

	static TimeManager* Instance()
	{
		if (_instance == NULL) _instance = new TimeManager ();
		return _instance;
	}

	static guint64 GetCurrentGlobalTime () { return Instance()->current_global_time; }

	void AddChild (Clock *clock);
	void RemoveChild (Clock *clock);

 private:
	TimeManager ();

	void RaiseEnqueuedEvents ();

	static TimeManager *_instance;

	GList *child_clocks; // XXX should we just have a ClockGroup?

	guint64 current_global_time;
	static gboolean tick_timeout (gpointer data);
	gint tick_id;
};





//
// Clocks and timelines
//


class Timeline;
class TimelineGroup;

class Clock : public DependencyObject {
 public:
	// events to queue up
	enum {
		CURRENT_GLOBAL_SPEED_INVALIDATED = 0x01,
		CURRENT_STATE_INVALIDATED        = 0x02,
		CURRENT_TIME_INVALIDATED         = 0x04,
		REMOVE_REQUESTED                 = 0x08,
		SPEED_CHANGED                    = 0x10
	};

	// states
	enum ClockState {
		RUNNING = 0x00,
		STOPPED = 0x01,

		PAUSED  = 0x80
	};

	Clock (Timeline *timeline);

	virtual void SpeedChanged () { };

	virtual void Begin (guint64 parent_time);
	void Pause ();
	void Remove ();
	void Resume ();
	void Seek (guint64/*TimeSpan*/ timespan);
	void SeekAlignedToLastTick ();
	void SkipToFill ();
	void Stop ();

	double GetCurrentProgress ();
	guint64 GetCurrentTime ();
	Timeline *GetTimeline () { return timeline; }

	virtual void RaiseAccumulatedEvents ();
	virtual void TimeUpdated (guint64 parent_time);

 protected:
	double current_progress;
	guint64/*TimeSpan*/ current_time;
	guint64/*TimeSpan*/ start_time; /* the time we actually started.  used for computing CurrentTime */

	void QueueEvent (int event);

 private:
	Timeline *timeline;
	int queued_events;

	bool reversed;  // if we're presently working our way from 1.0 progress to 0.0
	bool autoreverse;
	int remaining_iterations;
	double completed_iterations;
	Duration *duration;

	ClockState current_state;
};





class ClockGroup : public Clock {
 public:
	ClockGroup (TimelineGroup *timeline);

	virtual void Begin (guint64 parent_time);

	void AddChild (Clock *clock);
	void RemoveChild (Clock *clock);

	virtual void RaiseAccumulatedEvents ();
	virtual void TimeUpdated (guint64 parent_time);

	GList *child_clocks;

 private:
	TimelineGroup *timeline;
};





class Timeline : public DependencyObject {
 public:
	Timeline ();

	static DependencyProperty* AutoReverseProperty;
	static DependencyProperty* BeginTimeProperty;
	static DependencyProperty* DurationProperty;
	static DependencyProperty* FillBehaviorProperty;
	static DependencyProperty* RepeatBehaviorProperty;
	static DependencyProperty* SpeedRatioProperty;

	void SetRepeatBehavior (RepeatBehavior behavior);
	RepeatBehavior *GetRepeatBehavior ();

	void SetAutoReverse (bool autoreverse);
	bool GetAutoReverse ();

	void SetDuration (Duration duration);
	Duration* GetDuration ();

	virtual Clock* AllocateClock () { return new Clock (this); }
};





class TimelineGroup : public Timeline {
 public:
	TimelineGroup ();

	virtual Clock *AllocateClock () { return new ClockGroup (this); }

	ClockGroup *CreateClock ();

	/* we use these dependency properties:
	   Timeline Children - XXX shouldn't that be TimelineCollection?
	*/
	void AddChild (Timeline *child);
	void RemoveChild (Timeline *child);

	GList *child_timelines;
};

void timeline_group_add_child (TimelineGroup *group, Timeline *child);
void timeline_group_remove_child (TimelineGroup *group, Timeline *child);






class ParallelTimeline : public TimelineGroup {
 public:
	ParallelTimeline () { }
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

	virtual Clock *AllocateClock () { return new AnimationClock (this); }

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);

	//	Duration GetNaturalDurationCore (Clock* clock);
};



template<class T>
class Nullable {
  bool is_null;
  T value;
 public:
  Nullable (T v) : value(v) { is_null = false; }
  Nullable (T* p) { if (!p) is_null = true; else /* XXX */; }
  
  bool IsNull () { return is_null; }

  T GetValue () { return value; }
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

#define NULLABLE_GETSET_IMPL(klass,prop,t,T,umem) \
void klass::Set##prop (t v) { Set##prop (&v); } \
void klass::Set##prop (t *pv) { SetNullable##t##Prop (this, klass::prop##Property, pv); } \
t* klass::Get##prop () { Value* v = GetValue (klass::prop##Property);  return v ? (umem) : NULL; }


class DoubleAnimation : public Animation/*Timeline*/ {
 public:

	DoubleAnimation ();

	static DependencyProperty* ByProperty;
	static DependencyProperty* FromProperty;
	static DependencyProperty* ToProperty;

	NULLABLE_GETSET_DECL(By, double);
	NULLABLE_GETSET_DECL(From, double);
	NULLABLE_GETSET_DECL(To, double);

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);
};

DoubleAnimation * double_animation_new ();





class ColorAnimation : public Animation/*Timeline*/ {
 public:

	ColorAnimation ();

	static DependencyProperty* ByProperty;
	static DependencyProperty* FromProperty;
	static DependencyProperty* ToProperty;

	NULLABLE_GETSET_DECL(By, Color);
	NULLABLE_GETSET_DECL(From, Color);
	NULLABLE_GETSET_DECL(To, Color);

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);
};

ColorAnimation * color_animation_new ();





class PointAnimation : public Animation/*Timeline*/ {
 public:

	PointAnimation ();

	static DependencyProperty* ByProperty;
	static DependencyProperty* FromProperty;
	static DependencyProperty* ToProperty;

	NULLABLE_GETSET_DECL(By, Point);
	NULLABLE_GETSET_DECL(From, Point);
	NULLABLE_GETSET_DECL(To, Point);

	virtual Value *GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					AnimationClock* animationClock);
};

PointAnimation * point_animation_new ();





class Storyboard : public ParallelTimeline {
 public:
	Storyboard ();

	void Begin ();
	void Pause ();
	void Resume ();
	void Seek (/*Timespan*/guint64 timespan);
	void Stop ();

	static DependencyProperty* TargetNameProperty;
	static DependencyProperty* TargetPropertyProperty;

	// XXX event Completed

/* 	virtual void ClockTimeChanged (); */

 private:
	void HookupAnimationsRecurse (Clock *clock);
	Clock *root_clock;

	gboolean Tick ();
	static gboolean storyboard_tick (gpointer data);
};

Storyboard *storyboard_new ();
void storyboard_begin (Storyboard *sb);
void storyboard_pause (Storyboard *sb);
void storyboard_resume (Storyboard *sb);
void storyboard_seek (Storyboard *sb, /*Timespan*/guint64 timespan);
void storyboard_stop (Storyboard *sb);

void storyboard_child_set_target_property (Storyboard *sb,
					   DependencyObject *o,
					   char *targetProperty);

char* storyboard_child_get_target_property (Storyboard *sb,
					    DependencyObject *o);

void storyboard_child_set_target_name (Storyboard *sb,
				       DependencyObject *o,
				       char *targetName);

char* storyboard_child_get_target_name (Storyboard *sb,
					DependencyObject *o);





class BeginStoryboard : public TriggerAction {

 public:
	BeginStoryboard ()
	{
		SetObjectType (Value::BEGINSTORYBOARD);
	}

	static DependencyProperty* StoryboardProperty;
};

BeginStoryboard *begin_storyboard_new ();
