/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * clock.h: Clock management
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_CLOCK_H
#define MOON_CLOCK_H

#include <glib.h>
#include "collection.h"

/*
 * Time units:
 *  TimeSpan: signed int64 value, 100-nanosecond units (10 000 000 ticks per second)
 *  Pts: unsigned int64 value, same units as TimeSpan
 *  Milliseconds
 *  Seconds
 */ 
 
typedef guint64 TimePts;

#define TIMESPANTICKS_IN_SECOND 10000000
#define TIMESPANTICKS_IN_SECOND_FLOAT 10000000.0

#define TimeSpan_FromSeconds(s)  ((TimeSpan)(s) * TIMESPANTICKS_IN_SECOND)
#define TimeSpan_ToSeconds(s)  ((TimeSpan)(s) / TIMESPANTICKS_IN_SECOND)

#define TimeSpan_FromSecondsFloat(s)  ((TimeSpan)((s) * TIMESPANTICKS_IN_SECOND_FLOAT))
#define TimeSpan_ToSecondsFloat(s)  (((TimeSpan)(s)) / TIMESPANTICKS_IN_SECOND_FLOAT)

#define TimeSpan_ToPts(s)	((guint64) (s))
#define TimeSpan_FromPts(s)	((TimeSpan) (s))

#define PTS_PER_MILLISECOND	10000

#define MilliSeconds_ToPts(s) ((guint64) (s) * PTS_PER_MILLISECOND)
#define MilliSeconds_FromPts(s) ((s) / PTS_PER_MILLISECOND)

/* @IncludeInKinds */
/* @Namespace=System.Windows */
struct Duration {
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


/* @IncludeInKinds */
/* @Namespace=System.Windows.Media.Animation */
struct RepeatBehavior {
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


//
// Clocks and timelines
//

class TimeManager;
class Timeline;
class TimelineGroup;

/* our clock is a mixture of the WPF Clock and ClockController
   classes.  as such, all clocks are controllable */
/* @Namespace=None,ManagedDependencyProperties=None,ManagedEvents=Manual */
class Clock : public DependencyObject {
public:
	Clock (Timeline *timeline);
	
	ClockGroup* GetParentClock ()     { return parent_clock; }
	double      GetCurrentProgress () { return progress; }
	virtual TimeSpan    GetCurrentTime ()     { return current_time; }
	Timeline*   GetTimeline ()        { return timeline; }
	Duration    GetNaturalDuration ();
	bool        GetIsPaused ()        { return is_paused; }
	bool        GetIsSeeking ()       { return is_seeking; }
	bool        GetHasStarted ()      { return has_started; }
	bool        GetWasStopped ()      { return was_stopped; }
	void        ClearHasStarted ()    { has_started = false; }
	TimeManager* GetTimeManager ()    { return time_manager; }

	TimeSpan begin_time;

	enum ClockState {
		Active,  /* time is progressing.  each tick results in a property value changing */
		Filling, /* time is progressing.  each tick results in NO property value changing */
		Stopped  /* time is no longer progressing */
	};
	ClockState GetClockState () { return state; }

	// ClockController methods
	virtual void Begin (TimeSpan parentTime);
	void Pause ();
	void Resume ();
	virtual void Seek (TimeSpan timespan);
	virtual void SeekAlignedToLastTick (TimeSpan timespan);
	virtual void SkipToFill ();
	virtual void Stop ();

	void BeginOnTick (bool begin = true);
	bool GetBeginOnTick () { return begin_on_tick; }

	void SetRootParentTime (TimeSpan parentTime);

	/* these shouldn't be used.  they're called by the TimeManager and parent Clocks */
	virtual void RaiseAccumulatedEvents ();
	virtual void RaiseAccumulatedCompleted ();
	virtual void ExtraRepeatAction () {};
	virtual bool UpdateFromParentTime (TimeSpan parentTime);
	void SetParentClock (ClockGroup *parent) { parent_clock = parent; }
	virtual void SetTimeManager (TimeManager *manager) { time_manager = manager; }
	virtual void Reset ();

	// Events you can AddHandler to
	const static int CurrentTimeInvalidatedEvent;
	const static int CurrentStateInvalidatedEvent;
	const static int CompletedEvent;

	virtual void Dispose ();

protected:
	virtual ~Clock ();

	TimeSpan ComputeNewTime ();

	void FillOnNextTick ();

	void SetClockState (ClockState state);
	void SetCurrentTime (TimeSpan ts);

	void CalculateFillTime ();

	void Completed ();
	
	// events to queue up
	enum {
		CURRENT_STATE_INVALIDATED        = 0x01,
		CURRENT_TIME_INVALIDATED         = 0x02
	};
	void QueueEvent (int event);

	bool calculated_natural_duration;
	Duration natural_duration;

	TimeSpan root_parent_time;
	bool begin_on_tick;

	// the start time of the current pause
	TimeSpan begin_pause_time;

	// the total amount of pause time we've accumulated
	TimeSpan accumulated_pause_time;

	ClockState state;

	double progress;

	TimeSpan current_time;

	TimeSpan seek_time;

private:
	bool emit_completed;
	bool has_completed;
	TimeManager *time_manager;
	ClockGroup *parent_clock;
	
	bool is_paused;
	bool is_seeking;

	bool has_started;
	bool was_stopped;
	Timeline *timeline;
	int queued_events;

	// for clocks with repeatbehavior that's not Forever and
	// durations that aren't forever, this represents the time at
	// which we'll hit our Fill.
	TimeSpan fillTime;
};


/* @Namespace=None,ManagedDependencyProperties=None */
class ClockGroup : public Clock {
public:
	ClockGroup (TimelineGroup *timeline, bool timeManagerClockGroup = false);

	void AddChild (Clock *clock);
	void RemoveChild (Clock *clock);

	virtual void SetTimeManager (TimeManager *manager);

	virtual void Begin (TimeSpan parentTime);
	virtual void SkipToFill ();
	virtual void Stop ();

	/* these shouldn't be used.  they're called by the TimeManager and parent Clocks */
	virtual void RaiseAccumulatedEvents ();
	virtual void RaiseAccumulatedCompleted ();

	virtual bool UpdateFromParentTime (TimeSpan parentTime);

	GList *child_clocks;

	virtual void Reset ();

	virtual void Dispose ();

protected:
	virtual ~ClockGroup ();

private:
	bool timemanager_clockgroup;
};

#endif /* MOON_CLOCK_H */
