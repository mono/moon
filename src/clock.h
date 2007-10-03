/*
 * clock.h: Clock management
 *
 * Author:
 *   Chris Toshok (toshok@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_CLOCK_H
#define MOON_CLOCK_H

#include "collection.h"

G_BEGIN_DECLS

#define TIMESPANTICKS_IN_SECOND 10000000
#define TIMESPANTICKS_IN_SECOND_FLOAT 10000000.0

#define TimeSpan_FromSeconds(s)  ((TimeSpan)(s) * TIMESPANTICKS_IN_SECOND)
#define TimeSpan_ToSeconds(s)  ((TimeSpan)(s) / TIMESPANTICKS_IN_SECOND)

#define TimeSpan_FromSecondsFloat(s)  ((TimeSpan)((s) * TIMESPANTICKS_IN_SECOND_FLOAT))
#define TimeSpan_ToSecondsFloat(s)  (((TimeSpan)(s)) / TIMESPANTICKS_IN_SECOND_FLOAT)


// misc types
typedef gint32 FillBehavior;
enum {
	FillBehaviorHoldEnd,
	FillBehaviorStop
};


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

	gint32 ToSeconds ()
	{
		return TimeSpan_ToSeconds (timespan);
	}

	double ToSecondsFloat ()
	{
		return TimeSpan_ToSecondsFloat (timespan);
	}

	// This should live in a TimeSpan class, but oh well.. */
	static Duration FromSeconds (int seconds) { return Duration (TimeSpan_FromSeconds (seconds)); }
	static Duration FromSecondsFloat (double seconds) { return Duration (TimeSpan_FromSecondsFloat (seconds)); }

 private:
	DurationKind k;
	gint32 padding;
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

		/* not reached.  quiet g++ -Wall */
		return false;
	}

	double GetCount () { return count; }
	TimeSpan GetDuration() { return duration; }

	bool HasCount() { return k == COUNT; }
	bool HasDuration () { return k == DURATION; }

  private:
	RepeatKind k;
	gint32 padding;
	double count;
	TimeSpan duration;
};

// our root level time manager (basically the object that registers
// the gtk_timeout and drives all Clock objects
class Clock;

class TimeManager : public EventObject {
 public:
	void Start ();
	void Shutdown ();

	void Tick ();

	static TimeManager* Instance()
	{
		if (_instance == NULL) _instance = new TimeManager ();
		return _instance;
	}

	TimeSpan GetCurrentTime () { return current_global_time - start_time; }

	static TimeSpan GetCurrentGlobalTime () { return Instance()->current_global_time; }

	void AddChild (Clock *clock);
	void RemoveChild (Clock *clock);

	void AddTickCall (void (*func)(gpointer), gpointer tick_data);

	void SetMaximumRefreshRate (int hz);

	// Events you can AddHandler to
	static int UpdateInputEvent;
	static int RenderEvent;

	virtual Type::Kind GetObjectType () { return Type::TIMEMANAGER; };

	void ListClocks ();
 private:
	TimeManager ();

	void AddTimeout ();
	void RemoveTimeout ();

	void RaiseEnqueuedEvents ();

	void InvokeTickCall ();

	static TimeManager *_instance;

	GList *child_clocks; // XXX should we just have a ClockGroup?

	TimeSpan current_global_time;
	TimeSpan start_time;

	static gboolean tick_timeout (gpointer data);
	gint tick_id;
	int current_timeout;
	int max_fps;
	bool first_tick;

	TimeSpan previous_smoothed;

	enum TimeManagerOp {
		TIME_MANAGER_UPDATE_CLOCKS = 0x01,
		TIME_MANAGER_RENDER = 0x02,
		TIME_MANAGER_TICK_CALL = 0x04,
		TIME_MANAGER_UPDATE_INPUT = 0x08
	};

	TimeManagerOp flags;

	GMutex *tick_call_mutex;

	GList *tick_calls;
};

void time_manager_add_tick_call (void (*func)(gpointer), gpointer tick_data);

void time_manager_list_clocks ();


//
// Clocks and timelines
//

class Timeline;
class TimelineGroup;

/* our clock is a mixture of the WPF Clock and ClockController
   classes.  as such, all clocks are controllable */
class Clock : public DependencyObject {
 public:
	Clock (Timeline *timeline);
	virtual ~Clock () { };
	
	virtual Type::Kind GetObjectType () { return Type::CLOCK; };

	ClockGroup* GetParent ()          { return parent_clock; }
	double      GetCurrentProgress () { return current_progress; }
	TimeSpan    GetCurrentTime ()     { return current_time; }
	Timeline*   GetTimeline ()        { return timeline; }
	Duration    GetNaturalDuration () { return natural_duration; }
	bool        GetIsPaused ()        { return is_paused; }
	bool        GetHasStarted ()      { return has_started; }

	TimeSpan GetBeginTime ();

	enum ClockState {
		Active,  /* time is progressing.  each tick results in a property value changing */
		Filling, /* time is progressing.  each tick results in NO property value changing */
		Stopped  /* time is no longer progressing */
	};
	ClockState GetClockState () { return current_state; }
	ClockState GetNewClockState () { return new_state; }

	virtual void SpeedChanged () { }

	// ClockController methods
	void Begin ();
	void Pause ();
	void Remove ();
	void Resume ();
	virtual void Seek (TimeSpan timespan);
	void SeekAlignedToLastTick ();
	virtual void SkipToFill ();
	virtual void Stop ();


	/* these shouldn't be used.  they're called by the TimeManager and parent Clocks */
	virtual void RaiseAccumulatedEvents ();
	virtual void Tick ();
	void SetParent (ClockGroup *parent) { parent_clock = parent; }

	// Events you can AddHandler to
	static int CurrentTimeInvalidatedEvent;
	static int CurrentStateInvalidatedEvent;
	static int CurrentGlobalSpeedInvalidatedEvent;
	static int CompletedEvent;

 protected:
	// events to queue up
	enum {
		CURRENT_GLOBAL_SPEED_INVALIDATED = 0x01,
		CURRENT_STATE_INVALIDATED        = 0x02,
		CURRENT_TIME_INVALIDATED         = 0x04,
		REMOVE_REQUESTED                 = 0x08,
	};
	void QueueEvent (int event) { queued_events |= event; }

	Duration *duration;
	Duration natural_duration;

	TimeSpan begintime;

	ClockState current_state;
	ClockState new_state;

	double current_progress;
	double new_progress;

	TimeSpan current_time;
	TimeSpan new_time;

	bool seeking;
	TimeSpan seek_time;

	double current_speed;
	double new_speed;

 private:

	ClockGroup *parent_clock;
	TimeSpan last_parent_time;

	bool is_paused;
	bool has_started;
	Timeline *timeline;
	int queued_events;

	bool is_reversed;  // if we're presently working our way from 1.0 progress to 0.0
	bool autoreverse;
	int remaining_iterations;
};





class ClockGroup : public Clock {
 public:
	ClockGroup (TimelineGroup *timeline);
	virtual ~ClockGroup ();
	virtual Type::Kind GetObjectType () { return Type::CLOCKGROUP; };

	void AddChild (Clock *clock);
	void RemoveChild (Clock *clock);

	virtual void Begin ();
	virtual void Seek (TimeSpan timespan);
	virtual void SkipToFill ();
	virtual void Stop ();

	/* these shouldn't be used.  they're called by the TimeManager and parent Clocks */
	virtual void RaiseAccumulatedEvents ();
	virtual void Tick ();

	GList *child_clocks;

 private:
	TimelineGroup *timeline;
};





class Timeline : public DependencyObject {
 public:
	Timeline ();
	virtual Type::Kind GetObjectType () { return Type::TIMELINE; };

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

	TimeSpan GetBeginTime ();
	bool HasBeginTime ();

	void SetSpeedRatio (double ratio);
	double GetSpeedRatio ();
	
	Duration GetNaturalDuration (Clock *clock);
	virtual Duration GetNaturalDurationCore (Clock *clock);

	FillBehavior GetFillBehavior ();

	virtual Clock* AllocateClock () { return new Clock (this); }
};



class TimelineCollection : public Collection {
 public:
	TimelineCollection () {}
	virtual Type::Kind GetObjectType() { return Type::TIMELINE_COLLECTION; }

	virtual Type::Kind GetElementType() { return Type::TIMELINE; }
};

TimelineCollection * timeline_collection_new ();

class TimelineGroup : public Timeline {
 public:
	TimelineGroup ();
	virtual ~TimelineGroup ();
	
	virtual Type::Kind GetObjectType () { return Type::TIMELINEGROUP; };

	static DependencyProperty* ChildrenProperty;

	virtual Clock *AllocateClock () { return new ClockGroup (this); }

	ClockGroup *CreateClock ();

	void AddChild (Timeline *child);
	void RemoveChild (Timeline *child);

	virtual void OnPropertyChanged (DependencyProperty *prop);
};

TimelineGroup * timeline_group_new ();



class ParallelTimeline : public TimelineGroup {
 public:
	ParallelTimeline () { }
	virtual Type::Kind GetObjectType () { return Type::PARALLELTIMELINE; };

	virtual Duration GetNaturalDurationCore (Clock *clock);
};

ParallelTimeline * parallel_timeline_new ();

class TimelineMarker : public DependencyObject {
 public:
	TimelineMarker () {}
	virtual Type::Kind GetObjectType () { return Type::TIMELINEMARKER; };

	static DependencyProperty* TextProperty;
	static DependencyProperty* TimeProperty;
	static DependencyProperty* TypeProperty;
};

TimelineMarker* timeline_marker_new ();


/* useful for timing things */
TimeSpan get_now (void);

void clock_init (void);

G_END_DECLS

#endif /* MOON_CLOCK_H */
