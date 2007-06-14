#ifndef MOON_TIME_H
#define MOON_TIME_H

#include "runtime.h"

G_BEGIN_DECLS

// misc types
enum {
  FillBehaviorHoldEnd,
  FillBehaviorStop
};


typedef gint64 TimeSpan;

struct Duration {
 public:
	enum DurationKind {
		TIMESPAN,
		AUTOMATIC,
		FOREVER
	};

	Duration (TimeSpan duration)
	  : k (TIMESPAN),
	    timespan (duration) { }

	Duration (const Duration &duration)
	{
		k = duration.k;
		timespan = duration.timespan;
	}

	Duration (DurationKind kind) : k(kind) { };

	bool HasTimeSpan () { return k == TIMESPAN; }
	TimeSpan GetTimeSpan() { return timespan; }

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
	static Duration FromSeconds (int seconds) { return Duration ((TimeSpan)seconds * 1000000); }

	DurationKind k;
 private:
	TimeSpan timespan;
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

	RepeatBehavior (TimeSpan duration)
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
	TimeSpan GetDuration() { return duration; }

	bool HasCount() { return k == COUNT; }
	bool HasDuration () { return k == DURATION; }

	RepeatKind k;
  private:
	double count;
	TimeSpan duration;
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

	static TimeSpan GetCurrentGlobalTime () { return Instance()->current_global_time; }

	void AddChild (Clock *clock);
	void RemoveChild (Clock *clock);

 private:
	TimeManager ();

	void RaiseEnqueuedEvents ();

	static TimeManager *_instance;

	GList *child_clocks; // XXX should we just have a ClockGroup?

	TimeSpan current_global_time;
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
	~Clock () {  }
	
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
	virtual Value::Kind GetObjectType () { return Value::CLOCK; };

	virtual void SpeedChanged () { };

	virtual void Begin (TimeSpan parent_time);
	void Pause ();
	void Remove ();
	void Resume ();
	void Seek (TimeSpan timespan);
	void SeekAlignedToLastTick ();
	void SkipToFill ();
	void Stop ();

	double GetCurrentProgress ();
	TimeSpan GetCurrentTime ();
	Timeline *GetTimeline () { return timeline; }

	virtual void RaiseAccumulatedEvents ();
	virtual void TimeUpdated (TimeSpan parent_time);

	ClockState current_state;

 protected:
	double current_progress;
	TimeSpan current_time;
	TimeSpan current_time_while_paused;
	TimeSpan start_time; /* the time we actually started.  used for computing CurrentTime */
	TimeSpan iter_start; /* the time we started the current iteration */
	TimeSpan pause_time;
	TimeSpan parent_offset; /* the amount we need to subtract from the parent clock's time to get our time */

	void QueueEvent (int event);

 protected:
	Duration *duration;
	Duration natural_duration;

 private:
	Timeline *timeline;
	int queued_events;

	bool reversed;  // if we're presently working our way from 1.0 progress to 0.0
	bool autoreverse;
	int remaining_iterations;
};





class ClockGroup : public Clock {
 public:
	ClockGroup (TimelineGroup *timeline);
	~ClockGroup ();
	virtual Value::Kind GetObjectType () { return Value::CLOCKGROUP; };

	virtual void Begin (TimeSpan parent_time);

	void AddChild (Clock *clock);
	void RemoveChild (Clock *clock);

	virtual void RaiseAccumulatedEvents ();
	virtual void TimeUpdated (TimeSpan parent_time);

	GList *child_clocks;

 private:
	TimelineGroup *timeline;
};





class Timeline : public DependencyObject {
 public:
	Timeline ();
	virtual Value::Kind GetObjectType () { return Value::TIMELINE; };

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

	Duration GetNaturalDuration (Clock *clock);
	virtual Duration GetNaturalDurationCore (Clock *clock);

	virtual Clock* AllocateClock () { return new Clock (this); }
};



class TimelineCollection : public Collection {
 public:
	TimelineCollection () {}
	virtual Value::Kind GetObjectType() { return Value::TIMELINE_COLLECTION; }

	virtual Value::Kind GetElementType() { return Value::TIMELINE; }
};

TimelineCollection * timeline_collection_new ();

class TimelineGroup : public Timeline {
 public:
	TimelineGroup ();
	~TimelineGroup ();
	
	virtual Value::Kind GetObjectType () { return Value::TIMELINEGROUP; };

	static DependencyProperty* ChildrenProperty;

	virtual Clock *AllocateClock () { return new ClockGroup (this); }

	ClockGroup *CreateClock ();

	void AddChild (Timeline *child);
	void RemoveChild (Timeline *child);

	virtual void OnPropertyChanged (DependencyProperty *prop);

 protected:
	TimelineCollection *child_timelines;
};

TimelineGroup * timeline_group_new ();



class ParallelTimeline : public TimelineGroup {
 public:
	ParallelTimeline () { }
	virtual Value::Kind GetObjectType () { return Value::PARALLELTIMELINE; };

	virtual Duration GetNaturalDurationCore (Clock *clock);
};

ParallelTimeline * parallel_timeline_new ();


G_END_DECLS

#endif /* MOON_TIME_H */
