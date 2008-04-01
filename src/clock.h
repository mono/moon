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

#include <stdint.h>
#include "collection.h"

G_BEGIN_DECLS

/*
 * Time units:
 *  TimeSpan: signed int64 value, 100-nanosecond units (10 000 000 ticks per second)
 *  Pts: unsigned int64 value, same units as TimeSpan
 *  Milliseconds
 *  Seconds
 */ 
 
typedef uint64_t TimePts;

#define TIMESPANTICKS_IN_SECOND 10000000
#define TIMESPANTICKS_IN_SECOND_FLOAT 10000000.0

#define TimeSpan_FromSeconds(s)  ((TimeSpan)(s) * TIMESPANTICKS_IN_SECOND)
#define TimeSpan_ToSeconds(s)  ((TimeSpan)(s) / TIMESPANTICKS_IN_SECOND)

#define TimeSpan_FromSecondsFloat(s)  ((TimeSpan)((s) * TIMESPANTICKS_IN_SECOND_FLOAT))
#define TimeSpan_ToSecondsFloat(s)  (((TimeSpan)(s)) / TIMESPANTICKS_IN_SECOND_FLOAT)

#define TimeSpan_ToPts(s)	((uint64_t) (s))
#define TimeSpan_FromPts(s)	((TimeSpan) (s))

#define MilliSeconds_ToPts(s) ((uint64_t) (s) * 10000)
#define MilliSeconds_FromPts(s) ((s) / 10000)

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

	bool IsAutomatic () { return k == AUTOMATIC; }
	bool IsForever () { return k == FOREVER; }

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

	gint32 ToSeconds () { return TimeSpan_ToSeconds (timespan); }

	double ToSecondsFloat () { return TimeSpan_ToSecondsFloat (timespan); }

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

	bool IsForever () { return k == FOREVER; }

  private:
	RepeatKind k;
	gint32 padding;
	double count;
	TimeSpan duration;
};

class TimeSource : public EventObject {
 protected:
	virtual ~TimeSource ();

 public:
	TimeSource ();

	virtual void Start ();
	virtual void Stop ();
	virtual void SetTimerFrequency (int timeout);

	virtual TimeSpan GetNow ();

	static int TickEvent;

	virtual Type::Kind GetObjectType () { return Type::TIMESOURCE; };
};

class SystemTimeSource : public TimeSource {
 protected:
	virtual ~SystemTimeSource ();

 public:
	SystemTimeSource ();

	virtual void Start ();
	virtual void Stop ();
	virtual void SetTimerFrequency (int timeout);

	virtual TimeSpan GetNow ();

	virtual Type::Kind GetObjectType () { return Type::SYSTEMTIMESOURCE; };

 private:
	int gtk_timeout;
	int frequency;
	static gboolean tick_timeout (gpointer data);
};

class ManualTimeSource : public TimeSource {
 protected:
	virtual ~ManualTimeSource ();

 public:
	ManualTimeSource ();

	virtual TimeSpan GetNow ();

	void SetCurrentTime (TimeSpan current_time);

	virtual Type::Kind GetObjectType () { return Type::MANUALTIMESOURCE; };

 private:
	TimeSpan current_time;
};

//
// Clocks and timelines
//

class TimeManager;
class Timeline;
class TimelineGroup;

/* our clock is a mixture of the WPF Clock and ClockController
   classes.  as such, all clocks are controllable */
class Clock : public DependencyObject {
 protected:
	virtual ~Clock () {};

 public:
	Clock (Timeline *timeline);
	
	virtual Type::Kind GetObjectType () { return Type::CLOCK; };

	ClockGroup* GetParent ()          { return parent_clock; }
	double      GetCurrentProgress () { return progress; }
	virtual TimeSpan    GetCurrentTime ()     { return current_time; }
	virtual TimeSpan    GetLastTime ()        { return last_time; }
	Timeline*   GetTimeline ()        { return timeline; }
	Duration    GetNaturalDuration ();
	bool        GetIsPaused ()        { return is_paused; }
	bool        GetHasStarted ()      { return has_started; }
	bool        GetWasStopped ()      { return was_stopped; }
	void        ClearHasStarted ()    { has_started = false; }
	bool        GetIsReversed ()      { return !forward; }
	TimeSpan    GetBeginTime ()       { return begin_time; }
	TimeManager* GetTimeManager ()    { return time_manager; }

	TimeSpan begin_time;

	enum ClockState {
		Active,  /* time is progressing.  each tick results in a property value changing */
		Filling, /* time is progressing.  each tick results in NO property value changing */
		Stopped  /* time is no longer progressing */
	};
	ClockState GetClockState () { return state; }

	TimeSpan GetParentTime ();
	TimeSpan GetLastParentTime ();

	virtual void SpeedChanged () { }

	// ClockController methods
	virtual void Begin ();
	void Pause ();
	void Remove ();
	void Resume ();
	virtual void Seek (TimeSpan timespan);
	void SeekAlignedToLastTick ();
	virtual void SkipToFill ();
	virtual void Stop ();

	void BeginOnTick (bool begin = true) { this->begin_on_tick = begin; }
	bool GetBeginOnTick () { return begin_on_tick; }

	virtual void ComputeBeginTime ();

	/* these shouldn't be used.  they're called by the TimeManager and parent Clocks */
	virtual void RaiseAccumulatedEvents ();
	virtual bool Tick ();
	void SetParent (ClockGroup *parent) { parent_clock = parent; }
	virtual void SetTimeManager (TimeManager *manager) { time_manager = manager; }

	// Events you can AddHandler to
	static int CurrentTimeInvalidatedEvent;
	static int CurrentStateInvalidatedEvent;
	static int CurrentGlobalSpeedInvalidatedEvent;
	static int CompletedEvent;

 protected:
	TimeSpan ComputeNewTime ();
	void ClampTime ();
	void CalcProgress ();
	virtual void DoRepeat ();

	void SetClockState (ClockState state) { this->state = state; QueueEvent (CURRENT_STATE_INVALIDATED); }
	void SetCurrentTime (TimeSpan ts) { this->current_time = ts; QueueEvent (CURRENT_TIME_INVALIDATED); }
	void SetSpeed (double speed) { this->speed = speed; QueueEvent (CURRENT_GLOBAL_SPEED_INVALIDATED); }

	// events to queue up
	enum {
		CURRENT_GLOBAL_SPEED_INVALIDATED = 0x01,
		CURRENT_STATE_INVALIDATED        = 0x02,
		CURRENT_TIME_INVALIDATED         = 0x04,
		REMOVE_REQUESTED                 = 0x08,
	};
	void QueueEvent (int event) { queued_events |= event; }

	bool calculated_natural_duration;
	Duration natural_duration;

	TimeSpan begintime;
	bool begin_on_tick;

	ClockState state;

	double progress;

	TimeSpan current_time;
	TimeSpan last_time;

	bool seeking;
	TimeSpan seek_time;

	double speed;

	double repeat_count;
	TimeSpan repeat_time;

 private:

	TimeManager *time_manager;
	ClockGroup *parent_clock;

	bool is_paused;
	bool has_started;
	bool was_stopped;
	Timeline *timeline;
	int queued_events;

	bool forward;  // if we're presently working our way from 0.0 progress to 1.0.  false if reversed
};





class ClockGroup : public Clock {
 protected:
	virtual ~ClockGroup ();

 public:
	ClockGroup (TimelineGroup *timeline);
	virtual Type::Kind GetObjectType () { return Type::CLOCKGROUP; };

	void AddChild (Clock *clock);
	void RemoveChild (Clock *clock);
	bool IsIdle () { return idle_hint; };

	virtual void SetTimeManager (TimeManager *manager);

	virtual void Begin ();
	virtual void Seek (TimeSpan timespan);
	virtual void SkipToFill ();
	virtual void Stop ();

	virtual void ComputeBeginTime ();

	/* these shouldn't be used.  they're called by the TimeManager and parent Clocks */
	virtual void RaiseAccumulatedEvents ();
	virtual bool Tick ();

	GList *child_clocks;

 protected:
	virtual void DoRepeat ();

 private:
	TimelineGroup *timeline;
	bool emitted_complete;
	bool idle_hint;
};


// our root level time manager (basically the object that registers
// the gtk_timeout and drives all Clock objects
class TimeManager : public EventObject {
 public:
	TimeManager ();

	void Start ();

	TimeSource *GetSource() { return source; }
	ClockGroup *GetRootClock() { return root_clock; }

	virtual TimeSpan GetCurrentTime ()     { return current_global_time - start_time; }
	virtual TimeSpan GetLastTime ()        { return last_global_time - start_time; }
	TimeSpan GetCurrentTimeUsec () { return current_global_time_usec - start_time_usec; }

	void AddTickCall (void (*func)(gpointer), gpointer tick_data);
	void NeedRedraw ();
	void NeedClockTick ();

	guint AddTimeout (guint ms_interval, GSourceFunc func, gpointer timeout_data);
	void RemoveTimeout (guint timeout_id);

	void SetMaximumRefreshRate (int hz);

	// Events you can AddHandler to
	static int UpdateInputEvent;
	static int RenderEvent;

	virtual Type::Kind GetObjectType () { return Type::TIMEMANAGER; };

	void ListClocks ();

	static void InvokeOnMainThread (GSourceFunc func, gpointer data);
 protected:
	~TimeManager ();

 private:

	TimelineGroup *timeline;
	ClockGroup *root_clock;

	void SourceTick ();

	void RemoveAllRegisteredTimeouts ();

	bool InvokeTickCall ();

	TimeSpan current_global_time;
	TimeSpan last_global_time;
	TimeSpan start_time;

	TimeSpan current_global_time_usec;
	TimeSpan start_time_usec;

	static void source_tick_callback (EventObject *sender, EventArgs *calldata, gpointer closure);
	bool source_tick_pending;
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

	TimeSource *source;

	GMutex *tick_call_mutex;

	GList *tick_calls;

	GList *registered_timeouts;
};

void time_manager_add_tick_call (TimeManager *manager, void (*func)(gpointer), gpointer tick_data);
void time_manager_list_clocks   (TimeManager *manager);





class Timeline : public DependencyObject {
 protected:
	virtual ~Timeline () {}

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
 protected:
	virtual ~TimelineCollection () {}

 public:
	TimelineCollection () {}
	virtual Type::Kind GetObjectType() { return Type::TIMELINE_COLLECTION; }

	virtual Type::Kind GetElementType() { return Type::TIMELINE; }
};

TimelineCollection *timeline_collection_new (void);

class TimelineGroup : public Timeline {
 protected:
	virtual ~TimelineGroup ();

 public:
	TimelineGroup ();
	
	virtual Type::Kind GetObjectType () { return Type::TIMELINEGROUP; };

	static DependencyProperty* ChildrenProperty;

	virtual Clock *AllocateClock () { return new ClockGroup (this); }

	ClockGroup *CreateClock ();

	void AddChild (Timeline *child);
	void RemoveChild (Timeline *child);
};

TimelineGroup *timeline_group_new (void);



class ParallelTimeline : public TimelineGroup {
 protected:
	virtual ~ParallelTimeline () {}

 public:
	ParallelTimeline () { }
	virtual Type::Kind GetObjectType () { return Type::PARALLELTIMELINE; };

	virtual Duration GetNaturalDurationCore (Clock *clock);
};

ParallelTimeline * parallel_timeline_new ();

class TimelineMarker : public DependencyObject {
 protected:
	virtual ~TimelineMarker () {}

 public:
	TimelineMarker () {}
	virtual Type::Kind GetObjectType () { return Type::TIMELINEMARKER; };

	static DependencyProperty* TextProperty;
	static DependencyProperty* TimeProperty;
	static DependencyProperty* TypeProperty;
};

TimelineMarker *timeline_marker_new (void);


/* useful for timing things */
TimeSpan get_now (void);

void clock_init (void);

G_END_DECLS

#endif /* MOON_CLOCK_H */
