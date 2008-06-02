/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * clock.cpp: Clock management
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "clock.h"

#include "uielement.h"
#include "dirty.h"
#include "runtime.h"

#define CLOCK_DEBUG 0
#define TIME_TICK 0
#define USE_SMOOTHING 1
//#define PUT_TIME_MANAGER_TO_SLEEP 1

#if TIME_TICK
#define STARTTICKTIMER(id,str) STARTTIMER(id,str)
#define ENDTICKTIMER(id,str) ENDTIMER(it,str)
#else
#define STARTTICKTIMER(id,str)
#define ENDTICKTIMER(id,str)
#endif


#define MINIMUM_FPS 5
#define DEFAULT_FPS 50
#define MAXIMUM_FPS 50

#define FPS_TO_DELAY(fps) (int)(((double)1/(fps)) * 1000)
#define DELAY_TO_FPS(delay) (1000.0 / delay)

struct ClockNode {
	ClockNode *next;
	Clock *clock;
};

typedef void (*ClockFunc)(Clock*);

void
clock_list_foreach (GList *clock_list, ClockFunc func)
{
	ClockNode *list, *tail, *next;
	GList *n;
	
	list = NULL;
	tail = (ClockNode *) &list;
	
	for (n = clock_list; n != NULL; n = n->next) {
		tail->next = g_new (ClockNode, 1);
		tail = tail->next;
		
		tail->clock = (Clock *) n->data;
		tail->clock->ref ();
		tail->next = NULL;
	}
	
	while (list != NULL) {
		func (list->clock);
		list->clock->unref ();
		next = list->next;
		g_free (list);
		list = next;
	}
}

static void
CallRaiseAccumulatedEvents (Clock *clock)
{
	clock->RaiseAccumulatedEvents ();
}

TimeSpan
get_now (void)
{
        struct timeval tv;
        TimeSpan res;
#ifdef CLOCK_MONOTONIC
	struct timespec tspec;
	if (clock_gettime (CLOCK_MONOTONIC, &tspec) == 0) {
		res = (TimeSpan)((gint64)tspec.tv_sec * 10000000 + tspec.tv_nsec / 100);
		return res;
	}
#endif

        if (gettimeofday (&tv, NULL) == 0) {
                res = (TimeSpan)(tv.tv_sec * 1000000 + tv.tv_usec) * 10;
                return res;
        }

	// XXX error
	return 0;
}


TimeSource::TimeSource ()
{
}

TimeSource::~TimeSource ()
{
}

void
TimeSource::Start ()
{
}

void
TimeSource::Stop ()
{
}

void
TimeSource::SetTimerFrequency (int frequency)
{
}

TimeSpan
TimeSource::GetNow ()
{
	return 0;
}

SystemTimeSource::SystemTimeSource ()
{
	timeout_id = 0;
	frequency = -1;
}

SystemTimeSource::~SystemTimeSource ()
{
	Stop();
}

void
SystemTimeSource::SetTimerFrequency (int timeout)
{
	bool running = timeout_id != 0;
	
	if (running)
		Stop ();

	frequency = timeout;

	if (running)
		Start ();
}

void
SystemTimeSource::Start ()
{
	if (timeout_id != 0)
		return;
	
	if (frequency == -1)
		g_warning ("SystemTimeSource::frequency uninitialized in ::Start()");
	
	timeout_id = g_timeout_add_full (G_PRIORITY_HIGH, frequency, SystemTimeSource::tick_timeout, this, NULL);
}

void
SystemTimeSource::Stop ()
{
	if (timeout_id != 0) {
		g_source_remove (timeout_id);
		timeout_id = 0;
	}
}

TimeSpan
SystemTimeSource::GetNow ()
{
	return get_now ();
}

gboolean
SystemTimeSource::tick_timeout (gpointer data)
{
	((SystemTimeSource*)data)->Emit (TimeSource::TickEvent);
	return TRUE;
}

ManualTimeSource::ManualTimeSource ()
{
	current_time = 0;
}

ManualTimeSource::~ManualTimeSource ()
{
}

void
ManualTimeSource::SetCurrentTime (TimeSpan current_time)
{
	this->current_time = current_time;
	g_main_context_iteration (g_main_context_default (), false);
	Emit (TimeSource::TickEvent);
	Emit (TimeSource::TickEvent);
	Emit (TimeSource::TickEvent);
}

TimeSpan
ManualTimeSource::GetNow ()
{
	return current_time;
}


typedef struct {
	void (*func)(gpointer);
	gpointer data;
} TickCall;

TimeManager::TimeManager ()
{
	if (moonlight_flags & RUNTIME_INIT_MANUAL_TIMESOURCE)
		source = new ManualTimeSource();
	else
		source = new SystemTimeSource();

	current_timeout = FPS_TO_DELAY (DEFAULT_FPS);  /* something suitably small */
	max_fps = MAXIMUM_FPS;
	flags = (TimeManagerOp) (TIME_MANAGER_UPDATE_CLOCKS | TIME_MANAGER_RENDER | TIME_MANAGER_TICK_CALL /*| TIME_MANAGER_UPDATE_INPUT*/);
	tick_calls = NULL;

	start_time = source->GetNow ();
	start_time_usec = start_time / 10;
	source->AddHandler (TimeSource::TickEvent, source_tick_callback, this);

	tick_call_mutex = g_mutex_new ();
	registered_timeouts = NULL;
	source_tick_pending = false;
	first_tick = true;

	timeline = new ParallelTimeline();
	timeline->SetDuration (Duration::Automatic);
	root_clock = new ClockGroup (timeline, true);
	char *name = g_strdup_printf ("Surface clock group for time manager (%p)", this);
	root_clock->SetValue(DependencyObject::NameProperty, name);
	g_free (name);
	root_clock->SetTimeManager (this);
}

TimeManager::~TimeManager ()
{
	g_mutex_free (tick_call_mutex);
	tick_call_mutex = NULL;

	source->RemoveHandler (TimeSource::TickEvent, source_tick_callback, this);
	source->unref ();
	source = NULL;

	timeline->unref ();
	timeline = NULL;

	root_clock->unref ();
	root_clock = NULL;

	RemoveAllRegisteredTimeouts ();
}

void
TimeManager::SetMaximumRefreshRate (int hz)
{
	max_fps = hz;
	current_timeout = FPS_TO_DELAY (hz);
	source->SetTimerFrequency (current_timeout);
	first_tick = true;
}

void
TimeManager::Start()
{
	last_global_time = current_global_time = source->GetNow();
	current_global_time_usec = current_global_time / 10;
	source->SetTimerFrequency (current_timeout);
	source->Start ();
	source_tick_pending = true;
}

void
TimeManager::Stop ()
{
	source->Stop ();
}

void
TimeManager::Shutdown ()
{
	RemoveAllRegisteredTimeouts ();
	source->Stop ();
}

void
TimeManager::source_tick_callback (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((TimeManager *) closure)->SourceTick ();
}

bool
TimeManager::InvokeTickCall ()
{
	// Helgrind reports an issue here (reading from 'tick_calls' without locking)
	// This should be safe, given that tick_calls only goes from NULL -> something
	if (tick_calls) {
		g_mutex_lock (tick_call_mutex);

		TickCall *call = (TickCall*)tick_calls->data;

		// unlink the call first
		GList *new_tick_calls = tick_calls->next;
		g_list_free_1 (tick_calls);
		tick_calls = new_tick_calls;

		g_mutex_unlock (tick_call_mutex);

		// now invoke it
		call->func (call->data);
		g_free (call);
	}
	return tick_calls != NULL;
}

void
TimeManager::SourceTick ()
{
// 	printf ("TimeManager::SourceTick\n");
	STARTTICKTIMER (tick, "TimeManager::Tick");
	TimeSpan pre_tick = source->GetNow();

	TimeManagerOp current_flags = flags;

#if PUT_TIME_MANAGER_TO_SLEEP
	flags = (TimeManagerOp)0;
#endif

	source_tick_pending = false;

	if (current_flags & TIME_MANAGER_UPDATE_CLOCKS) {
		STARTTICKTIMER (tick_update_clocks, "TimeManager::Tick - UpdateClocks");
		current_global_time = source->GetNow();
		current_global_time_usec = current_global_time / 10;

		bool need_another_tick = root_clock->Tick ();
		if (need_another_tick)
			flags = (TimeManagerOp)(flags | TIME_MANAGER_UPDATE_CLOCKS);

		// ... then cause all clocks to raise the events they've queued up
		root_clock->RaiseAccumulatedEvents ();

#if PUT_TIME_MANAGER_TO_SLEEP
		// kind of a hack to make sure we render animation
		// changes in the same tick as when they happen.
		if (flags & TIME_MANAGER_RENDER) {
			current_flags = (TimeManagerOp)(current_flags | TIME_MANAGER_RENDER);
			flags = (TimeManagerOp)(flags & ~TIME_MANAGER_RENDER);
		}
#endif

		ENDTICKTIMER (tick_update_clocks, "TimeManager::Tick - UpdateClocks");
	}

	if (current_flags & TIME_MANAGER_UPDATE_INPUT) {
		STARTTICKTIMER (tick_input, "TimeManager::Tick - Input");
		Emit (UpdateInputEvent);
		ENDTICKTIMER (tick_input, "TimeManager::Tick - Input");
	}

	if (current_flags & TIME_MANAGER_RENDER) {
	  //	  fprintf (stderr, "rendering\n"); fflush (stderr);
		STARTTICKTIMER (tick_render, "TimeManager::Tick - Render");
		Emit (RenderEvent);
		ENDTICKTIMER (tick_render, "TimeManager::Tick - Render");
	}

	if (current_flags & TIME_MANAGER_TICK_CALL) {
		STARTTICKTIMER (tick_call, "TimeManager::Tick - InvokeTickCall");
		bool remaining_tick_calls = InvokeTickCall ();
		if (remaining_tick_calls)
			flags = (TimeManagerOp)(flags | TIME_MANAGER_TICK_CALL);
		ENDTICKTIMER (tick_call, "TimeManager::Tick - InvokeTickCall");
	}

#if CLOCK_DEBUG
	ListClocks ();
#endif

	ENDTICKTIMER (tick, "TimeManager::Tick");
	TimeSpan post_tick = source->GetNow();

	/* implement an exponential moving average by way of simple
	   exponential smoothing:

	   s(0) = x(0)
	   s(t) = alpha * x(t) + (1 - alpha) * s(t-1)

	   where 0 < alpha < 1.

	   see http://en.wikipedia.org/wiki/Exponential_smoothing.
	*/
#if USE_SMOOTHING
#define SMOOTHING_ALPHA 0.03 /* we probably want to play with this value some.. - toshok */
#define TIMEOUT_ERROR_DELTA 20 /* how far off of the current_timeout can we be */

	TimeSpan xt = post_tick - pre_tick;

	/* the s(0) case */
	if (first_tick) {
		first_tick = false;
		previous_smoothed = FPS_TO_DELAY (max_fps);
		return;
	}

	/* the s(t) case */
	TimeSpan current_smoothed = (TimeSpan)(SMOOTHING_ALPHA * xt + (1 - SMOOTHING_ALPHA) * previous_smoothed);

	/* current_smoothed now contains the prediction for what our next delay should be */
	
	
	int suggested_timeout = current_smoothed / 10000;
	if (suggested_timeout < FPS_TO_DELAY (max_fps)) {
		suggested_timeout = FPS_TO_DELAY (max_fps);
	}
	else if (suggested_timeout > FPS_TO_DELAY (MINIMUM_FPS)) {
		suggested_timeout = FPS_TO_DELAY (MINIMUM_FPS);
	}

	current_timeout = suggested_timeout;

#if PUT_TIME_MANAGER_TO_SLEEP
	// set up the next timeout here, but only if we need to
	if (flags || registered_timeouts) {
		source->SetTimerFrequency (MAX (0, current_timeout - xt / 10000));
		source->Start();
		source_tick_pending = true;
	}
	else {
		printf ("no work to do, TimeManager going to sleep\n");
		source->Stop ();
	}
#else
		source->SetTimerFrequency (MAX (0, current_timeout - xt / 10000));
#endif
	
	previous_smoothed = current_smoothed;

#if SHOW_SMOOTHING_COST
	TimeSpan post_smooth = source->GetNow();

	printf ("for a clock tick of %lld, we spent %lld computing the smooth delay\n",
		xt, post_smooth - post_tick);
#endif
#endif

	last_global_time = current_global_time;
}

guint
TimeManager::AddTimeout (guint ms_interval, GSourceFunc func, gpointer tick_data)
{
	guint rv = g_timeout_add (ms_interval, func, tick_data);
	registered_timeouts = g_list_prepend (registered_timeouts, GUINT_TO_POINTER (rv));
	return rv;
}

void
TimeManager::RemoveTimeout (guint timeout_id)
{
	g_source_remove (timeout_id);
	registered_timeouts = g_list_remove_all (registered_timeouts, GUINT_TO_POINTER (timeout_id));
}

void
TimeManager::RemoveAllRegisteredTimeouts ()
{
	GList *t;
	for (t = registered_timeouts; t; t = t->next)
		g_source_remove (GPOINTER_TO_UINT (t->data));

	g_list_free (registered_timeouts);
	registered_timeouts = NULL;
}

void
TimeManager::AddTickCall (void (*func)(gpointer), gpointer tick_data)
{
	TickCall *call = g_new (TickCall, 1);
	call->func = func;
	call->data = tick_data;
	g_mutex_lock (tick_call_mutex);
	tick_calls = g_list_append (tick_calls, call);

#if PUT_TIME_MANAGER_TO_SLEEP
	flags = (TimeManagerOp)(flags | TIME_MANAGER_TICK_CALL);
	if (!source_tick_pending) {
		source_tick_pending = true;
		source->SetTimerFrequency (0);
		source->Start();
	}
#endif

	g_mutex_unlock (tick_call_mutex);
}

void
TimeManager::NeedRedraw ()
{
#if PUT_TIME_MANAGER_TO_SLEEP
	flags = (TimeManagerOp)(flags | TIME_MANAGER_RENDER);
	if (!source_tick_pending) {
		source_tick_pending = true;
		source->SetTimerFrequency (0);
		source->Start();
	}
#endif
}

void
TimeManager::NeedClockTick ()
{
#if PUT_TIME_MANAGER_TO_SLEEP
	flags = (TimeManagerOp)(flags | TIME_MANAGER_UPDATE_CLOCKS);
	if (!source_tick_pending) {
		source_tick_pending = true;
		source->SetTimerFrequency (0);
		source->Start();
	}
#endif
}

void
time_manager_add_tick_call (TimeManager *manager, void (*func)(gpointer), gpointer tick_data)
{
	manager->AddTickCall (func, tick_data);
}

static void
spaces (int n)
{
	while (n--) putchar (' ');

}

static void
output_clock (Clock *clock, int level)
{
	spaces (level);
	printf (clock->Is(Type::CLOCKGROUP) ? "ClockGroup " : "Clock ");
	printf ("(%p) ", clock);
	if (clock->GetName ()) {
		printf ("'%s' ", clock->GetName());
	}

	printf ("%lld / %lld (%.2f) ", clock->GetCurrentTime(), clock->GetNaturalDuration().GetTimeSpan(), clock->GetCurrentProgress());

	printf ("%lld ", clock->GetBeginTime());

	switch (clock->GetClockState()) {
	case Clock::Active:
		printf ("A");
		break;
	case Clock::Filling:
		printf ("F");
		break;
	case Clock::Stopped:
		printf ("S");
		break;
	}

	if (clock->GetIsPaused())
		printf (" (paused)");

	if (clock->GetIsReversed())
		printf (" (rev)");

	printf ("\n");

	if (clock->Is(Type::CLOCKGROUP)) {
		ClockGroup *cg = (ClockGroup*)clock;
		level += 2;
		for (GList *l = cg->child_clocks; l; l = l->next) {
// 			if (((Clock*)l->data)->GetClockState () != Clock::Stopped)
				output_clock ((Clock*)l->data, level);
		}
	}
}

void
TimeManager::ListClocks()
{
	printf ("Currently registered clocks:\n");
	printf ("============================\n");

	output_clock (root_clock, 2);

	printf ("============================\n");
}

void
time_manager_list_clocks (TimeManager *manager)
{
	manager->ListClocks();
}

Clock::Clock (Timeline *tl)
  : natural_duration (Duration::Automatic)
{
	calculated_natural_duration = false;
	state = Clock::Stopped;
	progress = 0.0;
	current_time = 0;
	last_time = 0;
	seeking = false;
	seek_time = 0;
	speed = 1.0;
	time_manager = NULL;
	parent_clock = NULL;
	is_paused = false;
	has_started = false;
	timeline = tl;
	queued_events = 0;
	forward = true;

	was_stopped = false;
	begin_time = -1;
	begin_on_tick = false;

	if (timeline->HasBeginTime ())
		begintime = timeline->GetBeginTime ();
	/* otherwise it's filled in when we Begin the clock */
}

TimeSpan
Clock::GetParentTime ()
{
	return parent_clock ? parent_clock->GetCurrentTime() : time_manager ? time_manager->GetCurrentTime () : 0LL;
}

TimeSpan
Clock::GetLastParentTime ()
{
	return parent_clock ? parent_clock->GetLastTime() : time_manager ? time_manager->GetLastTime () : 0LL;
}

Duration
Clock::GetNaturalDuration ()
{
	if (!calculated_natural_duration) {
		calculated_natural_duration = true;

		Duration *duration = timeline->GetDuration ();
		if (duration->HasTimeSpan ()) {
			natural_duration = *duration;
		}
		else {
			natural_duration = timeline->GetNaturalDuration (this);
		}
	}

	return natural_duration;
}

bool
Clock::Tick ()
{
	last_time = current_time;

	// Save the old state as ComputeNewTime () changes state
	// and the ClampTime/CalcProgress decision depends on our state NOW

	int old_state = GetClockState ();
	SetCurrentTime (Clock::ComputeNewTime());

	if (old_state == Clock::Active || GetClockState () == Clock::Active) {
		ClampTime ();
		CalcProgress ();
	}

	return state != Clock::Stopped;
}

void
Clock::ClampTime ()
{
	if (natural_duration.HasTimeSpan()) {
		if (current_time > natural_duration.GetTimeSpan())
			SetCurrentTime (natural_duration.GetTimeSpan());
	}
	if (current_time < 0)
		SetCurrentTime (0);
}

void
Clock::CalcProgress ()
{
	if (natural_duration.HasTimeSpan()) {
		TimeSpan duration_timespan = natural_duration.GetTimeSpan();
		/* calculate our progress based on our time */
		if (duration_timespan == 0)
			progress = 1.0;
		else if (current_time >= duration_timespan)
			progress = 1.0;
		else if (GetClockState () != Clock::Stopped)
			progress = (double)current_time / duration_timespan;
	}
}

TimeSpan
Clock::ComputeNewTime ()
{
	TimeSpan our_delta = ceil ((TimeSpan)((GetParentTime() - GetLastParentTime()) * speed));
	TimeSpan ret_time = current_time;

	our_delta = (TimeSpan) ceil ((our_delta * timeline->GetSpeedRatio()));

	if (! forward)
		our_delta = - our_delta;

	if (seeking) {
		/* MS starts up animations if you seek them */
		if (state != Clock::Active)
			SetClockState (Clock::Active);

		ret_time = seek_time;

		seeking = false;
	}
	else {
		if (state == Clock::Stopped)
			return ret_time;

		ret_time = current_time + our_delta;
	}

	/* if our duration is automatic or forever, we're done. */
	if (!natural_duration.HasTimeSpan())
		return ret_time;

	// XXX there are a number of missing 'else's in the following
	// block of code.  it would be nice to figure out if they need
	// to be there...

	TimeSpan duration_timespan = natural_duration.GetTimeSpan();

	if (our_delta >= 0) {
		// time is progressing in the normal way

		if (ret_time >= duration_timespan) {
			// we've hit the end point of the clock's timeline
			if (timeline->GetAutoReverse ()) {
				/* since autoreverse is true we need
				   to turn around and head back to
				   0.0 */
				forward = false;
				ret_time = duration_timespan - (ret_time - duration_timespan);
			}
			else {
				/* but autoreverse is false. Decrement
				   the number of remaining iterations.

				   If we're done, Stop().  If we're
				   not done, force new_time to 0
				   so we start counting from there
				   again. */
				if (repeat_count > 0) {
					repeat_count --;
					if (repeat_count < 0)
						repeat_count = 0;
				}

				if (repeat_count == 0) {
					SkipToFill ();
					Completed ();
				}
				else {
					DoRepeat (ret_time);
					ret_time = current_time;
				}
			}
		}
		else if (ret_time >= 0 && GetClockState() != Clock::Active) {
			SetClockState (Clock::Active);
		}
	}
	else {
		// we're moving backward in time.

		if (ret_time <= 0) {
			// if this timeline isn't autoreversed it means the parent is, so don't flip ourselves to forward
			if (timeline->GetAutoReverse ()) {
				forward = true;
				ret_time = -ret_time;
			}
			
			// Here we check the repeat count even is we're auto-reversed. 
			// Ie. a auto-reversed storyboard has to stop here.
			if (repeat_count > 0) {
				repeat_count --;
				if (repeat_count < 0)
					repeat_count = 0;
			}

			if (repeat_count == 0) {
				ret_time = 0;
				SkipToFill ();
				Completed ();
			}
		}
		else if (ret_time <= duration_timespan && GetClockState() != Clock::Active) {
			SetClockState (Clock::Active);
		}
	}

	/* once we've calculated our time, make sure we don't
	   go over our repeat time (if there is one) */
	if (repeat_time >= 0 && repeat_time <= ret_time)
		SkipToFill ();

	return ret_time;

#if false
	// XXX I think we only need to check repeat_count in
	// the forward direction.
	if (forward) {
		if (repeat_count > 0 && repeat_count <= new_progress) {
			repeat_count = 0;
			SkipToFill ();
		}
	}
#endif
}

void
Clock::DoRepeat (TimeSpan time)
{
	if (natural_duration.HasTimeSpan ()) {
		if (natural_duration.GetTimeSpan () != 0)
			SetCurrentTime (time % natural_duration.GetTimeSpan());
		else
			SetCurrentTime (0);

		last_time = current_time;
	}
}

void
Clock::RaiseAccumulatedEvents ()
{
	if ((queued_events & CURRENT_TIME_INVALIDATED) != 0) {
		Emit (CurrentTimeInvalidatedEvent);
	}

	if ((queued_events & CURRENT_STATE_INVALIDATED) != 0) {
		if (state == Clock::Active)
			has_started = true;
		Emit (CurrentStateInvalidatedEvent);
	}

	if ((queued_events & CURRENT_GLOBAL_SPEED_INVALIDATED) != 0) {
		SpeedChanged ();
		Emit (CurrentGlobalSpeedInvalidatedEvent); /* this probably happens in SpeedChanged () */
	}

	/* XXX more events here, i assume */

	queued_events = 0;
}

void
Clock::Begin ()
{
	seeking = false;
	has_started = false;
	was_stopped = false;
	is_paused = false;
	forward = true;

	/* we're starting.  initialize our current_time field */
	SetCurrentTime ((GetParentTime() - GetBeginTime ()) * timeline->GetSpeedRatio ());
	last_time = current_time;

	if (natural_duration.HasTimeSpan ()) {
		if (natural_duration.GetTimeSpan() == 0) {
			progress = 1.0;
		}
		else {
			progress = (double)current_time / natural_duration.GetTimeSpan();
			if (progress > 1.0)
				progress = 1.0;
		}
	}
	else
		progress = 0.0;

	RepeatBehavior *repeat = timeline->GetRepeatBehavior ();
	if (repeat->HasCount ()) {

		/* XXX I'd love to be able to do away with the 2
		   repeat_* variables altogether by just converting
		   them both to a timespan, but for now it's not
		   working in the general case.  It *does* work,
		   however, if the count is < 1, so deal with that
		   tnow. */
		if (natural_duration.HasTimeSpan() && repeat->GetCount() < 1) {
			repeat_count = -1;
			repeat_time = (TimeSpan)(natural_duration.GetTimeSpan() *
						 (timeline->GetAutoReverse () ? 2 : 1) *
						 repeat->GetCount());
		}
		else {
			repeat_count = repeat->GetCount ();
			repeat_time = -1;
		}
	}
	else if (repeat->HasDuration ()) {
		repeat_count = -1;
		repeat_time = repeat->GetDuration();
	}
	else {
		repeat_count = -1;
		repeat_time = -1;
	}

	forward = true;
	SetClockState (Clock::Active);

	// force the time manager to tick the clock hierarchy to wake it up
	time_manager->NeedClockTick ();
}

void
Clock::ComputeBeginTime ()
{
	begin_time = timeline->HasBeginTime() ? timeline->GetBeginTime() : 0;
}

void
Clock::Pause ()
{
	if (is_paused)
		return;

	is_paused = true;

	SetSpeed (0.0);
}

void
Clock::SoftStop ()
{
	SetClockState (Clock::Stopped);
	has_started = false;
	was_stopped = false;
}

void
Clock::Remove ()
{
}

void
Clock::Resume ()
{
	if (!is_paused)
		return;

	is_paused = false;

	SetSpeed (1.0);
}

void
Clock::Seek (TimeSpan timespan)
{
	seek_time = timespan;

	// Start the clock if seeking into it's timespan
	if (!GetHasStarted() && !GetWasStopped() && (GetBeginOnTick() || GetBeginTime () <= seek_time)) {
		if (GetBeginOnTick()) {
			BeginOnTick (false);
			ComputeBeginTime ();
		}
		Begin ();
		seek_time = seek_time - GetBeginTime ();
	}

	seeking = true;
}

void
Clock::SeekAlignedToLastTick ()
{
}

void
Clock::SkipToFill ()
{
#if CLOCK_DEBUG
	printf ("filling clock %p after this tick\n", this);
#endif
	switch (timeline->GetFillBehavior()) {

		case FillBehaviorHoldEnd:
			SetClockState (Clock::Filling);
			break;

		case FillBehaviorStop:
			Stop ();
			break;
	}
}

void
Clock::Stop ()
{
#if CLOCK_DEBUG
	printf ("stopping clock %p after this tick\n", this);
#endif
	SetClockState (Clock::Stopped);
	was_stopped = true;
}

ClockGroup::ClockGroup (TimelineGroup *timeline, bool never_f)
  : Clock (timeline)
{
	child_clocks = NULL;
	this->timeline = timeline;
	emit_completed = false;
	idle_hint = false;
	never_fill = never_f;
}

void
ClockGroup::AddChild (Clock *clock)
{
	clock->GetNaturalDuration(); // ugh

	child_clocks = g_list_append (child_clocks, clock);
	clock->ref ();
	clock->SetParent (this);
	clock->SetTimeManager (GetTimeManager());

	idle_hint = false;
}

void
ClockGroup::SetTimeManager (TimeManager *manager)
{
	Clock::SetTimeManager (manager);
	for (GList *l = child_clocks; l; l = l->next) {
		Clock *c = (Clock*)l->data;
		c->SetTimeManager (manager);
	}
}

void
ClockGroup::RemoveChild (Clock *clock)
{
	child_clocks = g_list_remove (child_clocks, clock);
	clock->SetParent (NULL);
	clock->unref ();
}

void
ClockGroup::Begin ()
{
	emit_completed = false;
	idle_hint = false;
	
	Clock::Begin ();

	for (GList *l = child_clocks; l; l = l->next) {
		Clock *c = (Clock*)l->data;
		c->ClearHasStarted ();
		c->ComputeBeginTime ();

		/* start any clocks that need starting immediately */
		if (c->GetBeginTime() <= current_time) {
			c->Begin ();
		}
	}
}

void
ClockGroup::ComputeBeginTime ()
{
	begin_time = GetParentTime() + (timeline->HasBeginTime() ? timeline->GetBeginTime() : 0);
	for (GList *l = child_clocks; l; l = l->next) {
		Clock *c = (Clock*)l->data;
		c->ComputeBeginTime ();
	}
}

void
ClockGroup::SkipToFill ()
{
	idle_hint = true;

	if (child_clocks == NULL)
		Stop ();
	else
		Clock::SkipToFill ();
}

void
ClockGroup::Seek (TimeSpan timespan)
{
	Clock::Seek (timespan);
}

void
ClockGroup::Stop ()
{
	for (GList *l = child_clocks; l; l = l->next)
		((Clock*)l->data)->Stop ();

	Clock::Stop ();
}

bool
ClockGroup::Tick ()
{
	bool child_running = false;

	last_time = current_time;

	SetCurrentTime (Clock::ComputeNewTime());
	ClampTime ();

	for (GList *l = child_clocks; l; l = l->next) {
		/* start the child clock here if we need to,
		   otherwise just call its Tick
		   method */
		Clock *c = (Clock*)l->data;
		if (c->GetClockState() != Clock::Stopped) {
			if (c->GetObjectType () < Type::CLOCKGROUP || ! ((ClockGroup *) c)->IsIdle ())
				child_running |= c->Tick ();
		}
		else if (!c->GetHasStarted() && !c->GetWasStopped() && (c->GetBeginOnTick() || c->GetBeginTime () <= current_time)) {
			if (c->GetBeginOnTick()) {
				c->BeginOnTick (false);
				c->ComputeBeginTime ();
			}
			c->Begin ();
			child_running = true;
		}
	}

	if (GetClockState() == Clock::Active)
		CalcProgress ();

	if (GetClockState() == Clock::Stopped)
		return false;

	/*
	   XXX we should probably do this by attaching to
	   CurrentStateInvalidated on the child clocks instead of
	   looping over them at the end of each Tick call.
	*/
 	if (timeline->GetDuration()->IsAutomatic()) {
		for (GList *l = child_clocks; l; l = l->next) {
			Clock *c = (Clock*)l->data;
			if (!c->GetHasStarted () || c->GetClockState() == Clock::Active)
				return child_running;
		}

		if (repeat_count > 0)
			repeat_count --;

		if (repeat_count == 0) {
			/* 
			   Setting the idle hint is an optimization -- the ClockGroup 
			   will not tick anymore but it'll remain in the proper state. 
			   SL seems to do something similiar. We never fill ie. the
			   main (time manager) clock group.
			*/
			idle_hint = true;
			if (! never_fill)
				SkipToFill ();
		} else
			DoRepeat (current_time);
	}

	if (state == Clock::Stopped || idle_hint == true)
		return false;
	else
		return true;
}

void
ClockGroup::DoRepeat (TimeSpan time)
{
	Clock::DoRepeat (time);

	BeginOnTick (true);
	
	for (GList *l = child_clocks; l; l = l->next) {
		Clock *c = (Clock*)l->data;
		c->ExtraRepeatAction ();
		c->ComputeBeginTime ();
		c->SoftStop ();
	}
}

void
ClockGroup::RaiseAccumulatedEvents ()
{
	/* raise our events */
	Clock::RaiseAccumulatedEvents ();
	
	/* now cause our children to raise theirs */
	clock_list_foreach (child_clocks, CallRaiseAccumulatedEvents);
	
	if (emit_completed && (state == Clock::Stopped || state == Clock::Filling)) {
		emit_completed = false;
		Emit (CompletedEvent);
	}
}

ClockGroup::~ClockGroup ()
{
	GList *node = child_clocks;
	GList *next;
	
	while (node != NULL) {
		Clock *clock = (Clock *) node->data;
		clock->SetParent (NULL);
		clock->unref ();

		next = node->next;
		g_list_free_1 (node);
		node = next;
	}
}

/* timeline */

DependencyProperty *Timeline::AutoReverseProperty;
DependencyProperty *Timeline::BeginTimeProperty;
DependencyProperty *Timeline::DurationProperty;
DependencyProperty *Timeline::FillBehaviorProperty;
DependencyProperty *Timeline::RepeatBehaviorProperty;
DependencyProperty *Timeline::SpeedRatioProperty;

Timeline::Timeline ()
{
}

bool
Timeline::Validate ()
{
	RepeatBehavior *repeat = GetRepeatBehavior ();
	Duration *duration = GetDuration ();

	if (repeat->HasDuration () && repeat->GetDuration () == 0) {
		delete repeat;
		repeat = new RepeatBehavior (1.0);
	}

	if (duration->HasTimeSpan () && duration->GetTimeSpan () == 0 && 
	    (GetFillBehavior () == FillBehaviorStop || (repeat->HasCount () && repeat->GetCount () > 1.0)))
		return false;

	return true;
}

void
Timeline::SetRepeatBehavior (RepeatBehavior behavior)
{
	SetValue (Timeline::RepeatBehaviorProperty, Value(behavior));
}

RepeatBehavior *
Timeline::GetRepeatBehavior ()
{
	return GetValue (Timeline::RepeatBehaviorProperty)->AsRepeatBehavior();
}

void
Timeline::SetAutoReverse (bool autoreverse)
{
	SetValue (Timeline::AutoReverseProperty, Value(autoreverse));
}

bool
Timeline::GetAutoReverse ()
{
	return GetValue (Timeline::AutoReverseProperty)->AsBool();
}

void
Timeline::SetDuration (Duration duration)
{
	SetValue (Timeline::DurationProperty, Value(duration));
}

Duration*
Timeline::GetDuration ()
{
	return GetValue (Timeline::DurationProperty)->AsDuration();
}

void
Timeline::SetSpeedRatio (double ratio)
{
	SetValue (Timeline::SpeedRatioProperty, Value(ratio));
}

double
Timeline::GetSpeedRatio ()
{
	return GetValue (Timeline::SpeedRatioProperty)->AsDouble();
}

FillBehavior
Timeline::GetFillBehavior ()
{
	return (FillBehavior)GetValue (Timeline::FillBehaviorProperty)->AsInt32();
}

Duration
Timeline::GetNaturalDuration (Clock *clock)
{
	Duration* d = GetDuration ();
	if (*d == Duration::Automatic) {
//  		printf ("automatic duration, we need to calculate it\n");
		Duration cd = GetNaturalDurationCore (clock);
// 		if (cd.HasTimeSpan ())
//  			printf (" + duration (%lld timespan)\n", cd.GetTimeSpan ());
// 		else if (cd == Duration::Automatic)
// 			printf (" + automatic\n");
// 		else if (cd == Duration::Forever)
// 			printf (" + forever\n");
		return cd;
	}
	else {
		return *d;
	}
}

Duration
Timeline::GetNaturalDurationCore (Clock *clock)
{
	return Duration::Automatic;
}

bool
Timeline::HasBeginTime ()
{
	return GetValue (Timeline::BeginTimeProperty) != NULL;
}

TimeSpan
Timeline::GetBeginTime ()
{
	Value *v = GetValue (Timeline::BeginTimeProperty);
	return v == NULL ? 0LL : v->AsTimeSpan();
}


/* timeline group */

DependencyProperty *TimelineGroup::ChildrenProperty;

TimelineGroup::TimelineGroup ()
{
	SetValue (TimelineGroup::ChildrenProperty, Value::CreateUnref (new TimelineCollection ()));
}

TimelineGroup::~TimelineGroup ()
{
}

ClockGroup *
TimelineGroup::CreateClock ()
{
	TimelineCollection *collection = GetValue (TimelineGroup::ChildrenProperty)->AsTimelineCollection();
	ClockGroup *group = new ClockGroup (this);
	Collection::Node *node;
	
	node = (Collection::Node *) collection->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		Clock *clock = ((Timeline *) node->obj)->AllocateClock ();
		group->AddChild (clock);
		clock->unref ();
	}
	
	return group;
}

// Validate this TimelineGroup by validating all of it's children
bool
TimelineGroup::Validate ()
{
	TimelineCollection *collection = GetValue (TimelineGroup::ChildrenProperty)->AsTimelineCollection();
	Collection::Node *node = (Collection::Node *) collection->list->First ();

	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		Timeline *timeline = (Timeline *) node->obj;
		if (! timeline->Validate ())
			return false;
	}

	if (Timeline::Validate ())
		return true;
	else
		return false;
}

void
TimelineGroup::AddChild (Timeline *child)
{
	GetValue (TimelineGroup::ChildrenProperty)->AsTimelineCollection()->Add (child);
}

void
TimelineGroup::RemoveChild (Timeline *child)
{
	GetValue (TimelineGroup::ChildrenProperty)->AsTimelineCollection()->Remove (child);
}

TimelineGroup *
timeline_group_new (void)
{
	return new TimelineGroup ();
}

Duration
ParallelTimeline::GetNaturalDurationCore (Clock *clock)
{
	TimelineCollection *collection = GetValue (TimelineGroup::ChildrenProperty)->AsTimelineCollection();
	Collection::Node *node = (Collection::Node *) collection->list->First ();
	Duration d = Duration::Automatic;
	TimeSpan duration_span = 0;
	
	if (!node)
		return Duration::FromSeconds (0);
	
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		Timeline *timeline = (Timeline *) node->obj;

		Duration duration = timeline->GetNaturalDuration (clock);
		if (duration.IsAutomatic())
			continue;

		if (duration.IsForever())
			return Duration::Forever;
		
		TimeSpan span = duration.GetTimeSpan ();
		
		RepeatBehavior *repeat = timeline->GetRepeatBehavior ();
		if (repeat->IsForever())
			return Duration::Forever;
		
		if (repeat->HasCount ()) {
			span = (TimeSpan) (span * repeat->GetCount ());
		}

		if (timeline->GetAutoReverse ())
			span *= 2;

		// If we have duration-base repeat behavior, 
		// clamp/up our span to that.
		if (repeat->HasDuration ()) {
			span = repeat->GetDuration ();
		}

		span = (TimeSpan)(span / timeline->GetSpeedRatio());

		span += timeline->GetBeginTime ();

		if (duration_span < span) {
			duration_span = span;
			d = Duration (duration_span);
		}
	}

	return d;
}

ParallelTimeline *
parallel_timeline_new (void)
{
	return new ParallelTimeline ();
}

TimelineCollection *
timeline_collection_new (void)
{
	return new TimelineCollection ();
}

/*
	TimelineMarker
*/

DependencyProperty *TimelineMarker::TextProperty;
DependencyProperty *TimelineMarker::TimeProperty;
DependencyProperty *TimelineMarker::TypeProperty;

void
TimelineMarker::SetTime (TimeSpan time)
{
	SetValue (TimelineMarker::TimeProperty, Value (time, Type::TIMESPAN));
}

TimeSpan
TimelineMarker::GetTime ()
{
	Value *value = GetValue (TimelineMarker::TimeProperty);
	
	return value ? value->AsTimeSpan () : 0;
}

void
TimelineMarker::SetText (const char *text)
{
	SetValue (TimelineMarker::TextProperty, Value (text));
}

const char *
TimelineMarker::GetText ()
{
	Value *value = GetValue (TimelineMarker::TextProperty);
	
	return value ? value->AsString () : NULL;
}

void
TimelineMarker::SetType (const char *type)
{
	SetValue (TimelineMarker::TypeProperty, Value (type));
}

const char *
TimelineMarker::GetType ()
{
	Value *value = GetValue (TimelineMarker::TypeProperty);
	
	return value ? value->AsString () : NULL;
}

TimelineMarker *
timeline_marker_new (void)
{
	return new TimelineMarker ();
}

void
timeline_marker_set_text (TimelineMarker *marker, const char *text)
{
	marker->SetText (text);
}

const char *
timeline_marker_get_text (TimelineMarker *marker)
{
	return marker->GetText ();
}

void
timeline_marker_set_type (TimelineMarker *marker, const char *type)
{
	marker->SetType (type);
}

const char *
timeline_marker_get_type (TimelineMarker *marker)
{
	return marker->GetType ();
}

void
timeline_marker_set_time (TimelineMarker *marker, TimeSpan time)
{
	marker->SetTime (time);
}

TimeSpan
timeline_marker_get_time (TimelineMarker *marker)
{
	return marker->GetTime ();
}


void
clock_init (void)
{
	/* Timeline properties */
	Timeline::AutoReverseProperty = DependencyObject::Register (Type::TIMELINE, "AutoReverse", new Value (false));
	Timeline::BeginTimeProperty = DependencyObject::RegisterNullable (Type::TIMELINE, "BeginTime", Type::TIMESPAN);
	Timeline::DurationProperty = DependencyObject::Register (Type::TIMELINE, "Duration", new Value (Duration::Automatic));
	Timeline::RepeatBehaviorProperty = DependencyObject::Register (Type::TIMELINE, "RepeatBehavior", new Value (RepeatBehavior ((double)1)));
	Timeline::FillBehaviorProperty = DependencyObject::Register (Type::TIMELINE, "FillBehavior", new Value ((int)FillBehaviorHoldEnd));
	Timeline::SpeedRatioProperty = DependencyObject::Register (Type::TIMELINE, "SpeedRatio", new Value (1.0));

	/* TimelineGroup properties */
	TimelineGroup::ChildrenProperty = DependencyObject::Register (Type::TIMELINEGROUP, "Children", Type::TIMELINE_COLLECTION);

	/* TimelineMarker properties */
	TimelineMarker::TextProperty = DependencyObject::Register (Type::TIMELINEMARKER, "Text", Type::STRING);
	TimelineMarker::TimeProperty = DependencyObject::Register (Type::TIMELINEMARKER, "Time", Type::TIMESPAN);
	TimelineMarker::TypeProperty = DependencyObject::Register (Type::TIMELINEMARKER, "Type", Type::STRING);
}
