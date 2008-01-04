/*
 * clock.cpp: Clock management
 *
 * Author:
 *   Chris Toshok (toshok@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>

#include "clock.h"

#include "uielement.h"
#include "dirty.h"
#include "runtime.h"

//#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
//#endif

#define CLOCK_DEBUG 0
#define TIME_TICK 0
#define USE_SMOOTHING 1

#if TIME_TICK
#define STARTTICKTIMER(id,str) STARTTIMER(id,str)
#define ENDTICKTIMER(id,str) ENDTIMER(it,str)
#else
#define STARTTICKTIMER(id,str)
#define ENDTICKTIMER(id,str)
#endif


#define MINIMUM_FPS 5
#define DEFAULT_FPS 24
#define MAXIMUM_FPS 24

#define FPS_TO_DELAY(fps) (int)(((double)1/(fps)) * 1000)
#define DELAY_TO_FPS(delay) (1000.0 / delay)

extern guint32 moonlight_flags;

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

        if (gettimeofday (&tv, NULL) == 0) {
                res = (TimeSpan)(tv.tv_sec * 1000000 + tv.tv_usec) * 10;
                return res;
        }

	// XXX error
	return 0;
}


int TimeSource::TickEvent = -1;

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
	gtk_timeout = -1;
}

SystemTimeSource::~SystemTimeSource ()
{
	Stop();
}

void
SystemTimeSource::SetTimerFrequency (int timeout)
{
	bool running = false;
	if (gtk_timeout != -1)
		running = true;

	if (running)
		Stop ();

	frequency = timeout;

	if (running)
		Start ();
}

void
SystemTimeSource::Start ()
{
	if (gtk_timeout != -1)
		return;

	gtk_timeout = gtk_timeout_add (frequency, SystemTimeSource::tick_timeout, this);
}

void
SystemTimeSource::Stop ()
{
	if (gtk_timeout != -1){
		g_source_remove (gtk_timeout);
		gtk_timeout = -1;
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
	g_main_context_iteration (g_main_context_default(),
				  FALSE);
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

TimeManager* TimeManager::_instance = NULL;
int TimeManager::UpdateInputEvent = -1;
int TimeManager::RenderEvent = -1;

TimeManager::TimeManager ()
  : child_clocks (NULL),
    current_timeout (FPS_TO_DELAY (DEFAULT_FPS)),  /* something suitably small */
    max_fps (MAXIMUM_FPS),
    flags (TimeManagerOp (TIME_MANAGER_UPDATE_CLOCKS | TIME_MANAGER_RENDER | TIME_MANAGER_TICK_CALL /*| TIME_MANAGER_UPDATE_INPUT*/)),
    tick_calls (NULL)
{
	if (moonlight_flags & RUNTIME_INIT_MANUAL_TIMESOURCE)
		source = new ManualTimeSource();
	else
		source = new SystemTimeSource();

	start_time = source->GetNow ();
	start_time_usec = start_time / 10;
	source->AddHandler (TimeSource::TickEvent, tick_callback, this);

	tick_call_mutex = g_mutex_new ();
	registered_timeouts = NULL;
	first_tick = true;
}

TimeManager::~TimeManager ()
{
	g_mutex_free (tick_call_mutex);
	tick_call_mutex = NULL;
}

void
TimeManager::SetMaximumRefreshRate (int hz)
{
	max_fps = hz;
}

void
TimeManager::Start()
{
	last_global_time = current_global_time = source->GetNow();
	current_global_time_usec = current_global_time / 10;
	source->SetTimerFrequency (current_timeout);
	source->Start ();
}

void
TimeManager::Shutdown ()
{
	source->unref ();

        GList *node = child_clocks;
        GList *next;
	
	while (node != NULL) {
		next = node->next;
		((Clock*)node->data)->unref ();
		g_list_free_1 (node);
		node = next;
	}
	
	RemoveAllRegisteredTimeouts ();

	child_clocks = NULL;
	
	_instance->unref ();
	_instance = NULL;
}

void
TimeManager::tick_callback (EventObject *sender, gpointer calldata, gpointer closure)
{
	((TimeManager *) closure)->Tick ();
}

void
TimeManager::InvokeTickCall ()
{
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
}

void
TimeManager::Tick ()
{
	STARTTICKTIMER (tick, "TimeManager::Tick");
	TimeSpan pre_tick = source->GetNow();
	if (flags & TIME_MANAGER_UPDATE_CLOCKS) {
		STARTTICKTIMER (tick_update_clocks, "TimeManager::Tick - UpdateClocks");
		current_global_time = source->GetNow();
		current_global_time_usec = current_global_time / 10;
		// loop over all toplevel clocks, updating their time (and
		// triggering them to queue up their events) using the
		// value of current_global_time...

		for (GList *l = child_clocks; l; l = l->next) {
			Clock *c = (Clock*)l->data;
			if (c->GetClockState() != Clock::Stopped) {
				c->Tick ();
			}
		}
	
		// ... then cause all clocks to raise the events they've queued up
		RaiseEnqueuedEvents ();
		ENDTICKTIMER (tick_update_clocks, "TimeManager::Tick - UpdateClocks");
	}

	if (flags & TIME_MANAGER_UPDATE_INPUT) {
		STARTTICKTIMER (tick_input, "TimeManager::Tick - Input");
		Emit (UpdateInputEvent);
		ENDTICKTIMER (tick_input, "TimeManager::Tick - Input");
	}

	if (flags & TIME_MANAGER_RENDER) {
	  //	  fprintf (stderr, "rendering\n"); fflush (stderr);
		STARTTICKTIMER (tick_render, "TimeManager::Tick - Render");
		STARTTIMER (tick_dirty, "TimeManager::Tick - Dirty");
		process_dirty_elements ();
		ENDTIMER (tick_dirty, "TimeManager::Tick - Dirty");

		Emit (RenderEvent);
		ENDTICKTIMER (tick_render, "TimeManager::Tick - Render");
	}

	if (flags & TIME_MANAGER_TICK_CALL) {
		STARTTICKTIMER (tick_call, "TimeManager::Tick - InvokeTickCall");
		InvokeTickCall ();
		ENDTICKTIMER (tick_call, "TimeManager::Tick - InvokeTickCall");
	}

	//time_manager_list_clocks ();

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
#define SMOOTHING_ALPHA 0.30 /* we probably want to play with this value some.. - toshok */


	TimeSpan xt = post_tick - pre_tick;

	/* the s(0) case */
	if (first_tick) {
		first_tick = false;
		previous_smoothed = xt;
		return;
	}

	/* the s(t) case */
	TimeSpan current_smoothed = (TimeSpan)(SMOOTHING_ALPHA * xt + (1 - SMOOTHING_ALPHA) * previous_smoothed);

	/* current_smoothed now contains the prediction for what our next delay should be */

	current_timeout = current_smoothed / 10000;
	if (current_timeout < FPS_TO_DELAY (max_fps))
		current_timeout = FPS_TO_DELAY (max_fps);
	else if (current_timeout > FPS_TO_DELAY (MINIMUM_FPS))
		current_timeout = FPS_TO_DELAY (MINIMUM_FPS);

	//	printf ("new timeout is %dms (%.2ffps)\n", current_timeout, DELAY_TO_FPS (current_timeout));

	source->SetTimerFrequency (current_timeout);

	previous_smoothed = current_smoothed;

#if SHOW_SMOOTHING_COST
	TimeSpan post_smooth = source->GetNow();

	printf ("for a clock tick of %lld, we spent %lld computing the smooth delay\n",
		xt, post_smooth - post_tick);
#endif
#endif

	last_global_time = current_global_time;
}

void
TimeManager::RaiseEnqueuedEvents ()
{
	clock_list_foreach (child_clocks, CallRaiseAccumulatedEvents);
}

guint
TimeManager::AddTimeout (guint ms_interval, GSourceFunc func, gpointer tick_data)
{
	guint rv = gtk_timeout_add (ms_interval, func, tick_data);
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

guint
time_manager_add_timeout (guint ms_interval, GSourceFunc func, gpointer timeout_data)
{
	return TimeManager::Instance ()->AddTimeout (ms_interval, func, timeout_data);
}

void
time_manager_remove_timeout (guint timeout_id)
{
	return TimeManager::Instance ()->RemoveTimeout (timeout_id);
}

void
TimeManager::AddTickCall (void (*func)(gpointer), gpointer tick_data)
{
	TickCall *call = g_new (TickCall, 1);
	call->func = func;
	call->data = tick_data;
	g_mutex_lock (tick_call_mutex);
	tick_calls = g_list_append (tick_calls, call);
	g_mutex_unlock (tick_call_mutex);
}

void
time_manager_add_tick_call (void (*func)(gpointer), gpointer tick_data)
{
	TimeManager::Instance ()->AddTickCall (func, tick_data);
}

void
TimeManager::AddChild (Clock *child)
{
	child_clocks = g_list_append (child_clocks, child);
	child->ref ();
}

void
TimeManager::RemoveChild (Clock *child)
{
	child_clocks = g_list_remove (child_clocks, child);
	child->unref ();
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

	for (GList *l = child_clocks; l; l = l->next) {
// 		if (((Clock*)l->data)->GetClockState () != Clock::Filling)
			output_clock ((Clock*)l->data, 2);
	}
}

void
time_manager_list_clocks (void)
{
	TimeManager::Instance()->ListClocks();
}

int Clock::CurrentTimeInvalidatedEvent = -1;
int Clock::CurrentStateInvalidatedEvent = -1;
int Clock::CurrentGlobalSpeedInvalidatedEvent = -1;
int Clock::CompletedEvent = -1;


Clock::Clock (Timeline *tl)
  : natural_duration (Duration::Automatic),
    state (Clock::Stopped),
    progress (0.0),
    current_time (0), last_time (0),
    seeking (false), seek_time (0),
    speed (1.0),
    parent_clock (NULL),
    is_paused (false),
    has_started (false),
    timeline (tl),
    queued_events (0),
    forward (true)
{
	was_stopped = false;
	begin_time = -1;

	if (timeline->HasBeginTime ())
		begintime = timeline->GetBeginTime ();
	/* otherwise it's filled in when we Begin the clock */

	Duration *duration = timeline->GetDuration ();
	if (duration->HasTimeSpan ()) {
		natural_duration = *duration;
	}
	else {
		natural_duration = timeline->GetNaturalDuration (this);
	}
}

TimeSpan
Clock::GetParentTime ()
{
	if (parent_clock)
		return parent_clock->GetCurrentTime();
	else
		return TimeManager::Instance()->GetCurrentTime();
}

TimeSpan
Clock::GetLastParentTime ()
{
	if (parent_clock)
		return parent_clock->GetLastTime();
	else
		return TimeManager::Instance()->GetLastTime();
}

void
Clock::Tick ()
{
	last_time = current_time;
	SetCurrentTime (Clock::ComputeNewTime());

#if false
	if (GetClockState() == Clock::Active)
#endif
		ClampTime ();
	CalcProgress ();
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
		else if (state == Clock::Active)
			progress = (double)current_time / duration_timespan;
	}
}

TimeSpan
Clock::ComputeNewTime ()
{
	TimeSpan our_delta = (TimeSpan)((GetParentTime() - GetLastParentTime()) * speed);
	TimeSpan ret_time = current_time;

	/* serious voodoo here... */
	if ((timeline->GetSpeedRatio() > 1 && our_delta > 0)
	    || (timeline->GetSpeedRatio() < 1)) {

		our_delta = (TimeSpan)(our_delta * timeline->GetSpeedRatio());
	}

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

		ret_time = current_time + (forward ? our_delta : -our_delta);
	}

	/* if our duration is automatic or forever, we're done. */
	if (!natural_duration.HasTimeSpan())
		return ret_time;

	// XXX there are a number of missing 'else's in the following
	// block of code.  it would be nice to figure out if they need
	// to be there...

	TimeSpan duration_timespan = natural_duration.GetTimeSpan();

	if (our_delta > 0) {
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
				}
				else {
					DoRepeat ();
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
			forward = true;
			ret_time = -ret_time;

			if (repeat_count > 0) {
				repeat_count --;
				if (repeat_count < 0)
					repeat_count = 0;
			}

			if (repeat_count == 0) {
				SkipToFill ();
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
Clock::DoRepeat ()
{
	if (natural_duration.HasTimeSpan ()) {
		SetCurrentTime (current_time - natural_duration.GetTimeSpan());
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
	SetCurrentTime (GetParentTime() - GetBeginTime ());
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
}

void
Clock::ComputeBeginTime ()
{
	TimeSpan offset = 0;

	if (timeline->HasBeginTime ())
		offset = timeline->GetBeginTime ();

	begin_time = GetParentTime() + offset;
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
	seeking = true;

	/* calculate our resulting time based on our
	   duration/repeatbehavior/etc */
	//int active_segments = timespan % duration;

	seek_time = timespan;
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



ClockGroup::ClockGroup (TimelineGroup *timeline)
  : Clock (timeline),
    child_clocks (NULL),
    timeline (timeline),
    emitted_complete (false)
{
}

void
ClockGroup::AddChild (Clock *clock)
{
	child_clocks = g_list_append (child_clocks, clock);
	clock->ref ();
	clock->SetParent (this);
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
	emitted_complete = false;

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
	Clock::ComputeBeginTime ();
	for (GList *l = child_clocks; l; l = l->next) {
		Clock *c = (Clock*)l->data;
		c->ComputeBeginTime ();
	}
}

void
ClockGroup::SkipToFill ()
{
	if (child_clocks == NULL)
		Stop ();
	else
		Clock::SkipToFill ();
}

void
ClockGroup::Seek (TimeSpan timespan)
{
	Clock::Seek (timespan);

	for (GList *l = child_clocks; l; l = l->next)
		((Clock*)l->data)->Seek (seek_time);
}

void
ClockGroup::Stop ()
{
	for (GList *l = child_clocks; l; l = l->next)
		((Clock*)l->data)->Stop ();

	Clock::Stop ();
}

void
ClockGroup::Tick ()
{
	last_time = current_time;

	SetCurrentTime (Clock::ComputeNewTime());

	for (GList *l = child_clocks; l; l = l->next) {
		/* start the child clock here if we need to,
		   otherwise just call its Tick
		   method */
		Clock *c = (Clock*)l->data;
		if (c->GetClockState() != Clock::Stopped) {
			c->Tick ();
		}
		else if (!c->GetHasStarted() && !c->GetWasStopped() && c->GetBeginTime () <= current_time) {
			c->Begin ();
		}
	}

#if false
	if (GetClockState() == Clock::Active)
#endif
		ClampTime ();

	CalcProgress ();

	if (GetClockState() == Clock::Stopped)
		return;

	/* if we're automatic and no child clocks are still running, we need to stop.

	   XXX we should probably do this by attaching to
	   CurrentStateInvalidated on the child clocks instead of
	   looping over them at the end of each Tick call.
	*/
 	if (timeline->GetDuration()->IsAutomatic()) {
		for (GList *l = child_clocks; l; l = l->next) {
			Clock *c = (Clock*)l->data;
			if (!c->GetHasStarted () || c->GetClockState() == Clock::Active)
				return;
		}

		if (repeat_count > 0)
			repeat_count --;

		if (repeat_count == 0) {
			Clock::Stop ();
		}
		else {
			DoRepeat ();
		}
	}
}

void
ClockGroup::DoRepeat ()
{
	Clock::DoRepeat ();
	
	for (GList *l = child_clocks; l; l = l->next) {
		Clock *c = (Clock*)l->data;

		// restart all non-active clocks
		if (c->GetClockState() != Clock::Active) {
			// XXX this is wrong.  begin
			// time's are only figured into
			// the initial starting.  not
			// repeated iterations.
			c->ComputeBeginTime ();
			c->Begin ();
		}
	}
}

void
ClockGroup::RaiseAccumulatedEvents ()
{
	/* raise our events */
	this->Clock::RaiseAccumulatedEvents ();
	
	/* now cause our children to raise theirs */
	clock_list_foreach (child_clocks, CallRaiseAccumulatedEvents);
	
	if (GetHasStarted() && state == Clock::Stopped && !emitted_complete) {
		Emit (CompletedEvent);
		emitted_complete = true;
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

DependencyProperty* Timeline::AutoReverseProperty;
DependencyProperty* Timeline::BeginTimeProperty;
DependencyProperty* Timeline::DurationProperty;
DependencyProperty* Timeline::FillBehaviorProperty;
DependencyProperty* Timeline::RepeatBehaviorProperty;
DependencyProperty* Timeline::SpeedRatioProperty;

Timeline::Timeline ()
{
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

DependencyProperty* TimelineGroup::ChildrenProperty;

TimelineGroup::TimelineGroup ()
{
	this->SetValue (TimelineGroup::ChildrenProperty, Value::CreateUnref (new TimelineCollection ()));
}

TimelineGroup::~TimelineGroup ()
{
}

void
TimelineGroup::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::TIMELINEGROUP) {
		Timeline::OnPropertyChanged (prop);
		return;
	}

	if (prop == TimelineGroup::ChildrenProperty) {
		TimelineCollection *newcol = GetValue (prop)->AsTimelineCollection();

		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	}

	NotifyAttachersOfPropertyChange (prop);
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
timeline_group_new ()
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
		} else if (repeat->HasDuration ()) {
			if (span > repeat->GetDuration ())
				span = repeat->GetDuration ();
		}

		if (timeline->GetAutoReverse ())
			span *= 2;

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

DependencyProperty* TimelineMarker::TextProperty;
DependencyProperty* TimelineMarker::TimeProperty;
DependencyProperty* TimelineMarker::TypeProperty;

TimelineMarker *
timeline_marker_new (void)
{
	return new TimelineMarker ();
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

	/* lookup events */

	Type *t = Type::Find (Type::TIMEMANAGER);
	TimeManager::UpdateInputEvent = t->LookupEvent ("update-input");
	TimeManager::RenderEvent = t->LookupEvent ("render");

	t = Type::Find (Type::CLOCK);
	Clock::CurrentTimeInvalidatedEvent = t->LookupEvent ("CurrentTimeInvalidated");
	Clock::CurrentStateInvalidatedEvent = t->LookupEvent ("CurrentStateInvalidated");
	Clock::CurrentGlobalSpeedInvalidatedEvent = t->LookupEvent ("CurrentGlobalSpeedInvalidated");
	Clock::CompletedEvent = t->LookupEvent ("Completed");

	t = Type::Find (Type::TIMESOURCE);
	TimeSource::TickEvent = t->LookupEvent ("Tick");
}
