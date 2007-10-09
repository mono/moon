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

//#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
//#endif

#define CLOCK_DEBUG 0
#define TIME_TICK 0

#if TIME_TICK
#define STARTTICKTIMER(id,str) STARTTIMER(id,str)
#define ENDTICKTIMER(id,str) ENDTIMER(it,str)
#else
#define STARTTICKTIMER(id,str)
#define ENDTICKTIMER(id,str)
#endif


#define MINIMUM_FPS 5
#define DEFAULT_FPS 60
#define MAXIMUM_FPS 60

#define FPS_TO_DELAY(fps) (int)(((double)1/(fps)) * 1000)
#define DELAY_TO_FPS(delay) (1000.0 / delay)

typedef struct {
  void (*func)(gpointer);
  gpointer data;
} TickCall;

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


TimeManager* TimeManager::_instance = NULL;
int TimeManager::UpdateInputEvent = -1;
int TimeManager::RenderEvent = -1;

TimeManager::TimeManager ()
  : child_clocks (NULL),
    tick_id (-1),
    current_timeout (FPS_TO_DELAY (DEFAULT_FPS)),  /* something suitably small */
    max_fps (MAXIMUM_FPS),
    flags (TimeManagerOp (TIME_MANAGER_UPDATE_CLOCKS | TIME_MANAGER_RENDER | TIME_MANAGER_TICK_CALL /*| TIME_MANAGER_UPDATE_INPUT*/)),
    tick_calls (NULL)
{
	start_time = get_now ();

	tick_call_mutex = g_mutex_new ();
	first_tick = true;
}

void
TimeManager::SetMaximumRefreshRate (int hz)
{
	max_fps = hz;
}

void
TimeManager::Start()
{
	current_global_time = get_now ();
	AddTimeout ();
}

void
TimeManager::AddTimeout ()
{
	if (tick_id != -1)
		return;

	tick_id = gtk_timeout_add (current_timeout, TimeManager::tick_timeout, this);
}

void
TimeManager::RemoveTimeout ()
{
	if (tick_id != -1){
		g_source_remove (tick_id);
		tick_id = -1;
	}
}

void
TimeManager::Shutdown ()
{
	RemoveTimeout ();

        GList *node = child_clocks;
        GList *next;
	
	while (node != NULL) {
		next = node->next;
		((Base *) node->data)->unref ();
		g_list_free_1 (node);
		node = next;
	}
	
	child_clocks = NULL;
	
	_instance->unref ();
	_instance = NULL;
}

gboolean
TimeManager::tick_timeout (gpointer data)
{
	((TimeManager*)data)->Tick ();
	return TRUE;
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
	TimeSpan pre_tick = get_now();
	if (flags & TIME_MANAGER_UPDATE_CLOCKS) {
		STARTTICKTIMER (tick_update_clocks, "TimeManager::Tick - UpdateClocks");
		current_global_time = get_now ();
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
		process_dirty_elements ();

		Emit (RenderEvent);
		ENDTICKTIMER (tick_render, "TimeManager::Tick - Render");
	}

	if (flags & TIME_MANAGER_TICK_CALL) {
		STARTTICKTIMER (tick_call, "TimeManager::Tick - InvokeTick");
		InvokeTickCall ();
		ENDTICKTIMER (tick_call, "TimeManager::Tick - InvokeTick");
	}

	ENDTICKTIMER (tick, "TimeManager::Tick");
	// for some reason this code completely breaks the airlines demo...
	// we need to figure out what the hell it's doing.

	TimeSpan post_tick = get_now();

	/* implement an exponential moving average by way of simple
	   exponential smoothing:

	   s(0) = x(0)
	   s(t) = alpha * x(t) + (1 - alpha) * s(t-1)

	   where 0 < alpha < 1.

	   see http://en.wikipedia.org/wiki/Exponential_smoothing.
	*/

#define SMOOTHING_ALPHA 0.60 /* we probably want to play with this value some.. - toshok */


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
	RemoveTimeout();
	AddTimeout();

	previous_smoothed = current_smoothed;
}

void
TimeManager::RaiseEnqueuedEvents ()
{
	GList *copy = g_list_copy (child_clocks);
	g_list_foreach (copy, (GFunc)base_ref, NULL);
	for (GList *l = copy; l; l = l->next) {
		Clock *c = (Clock*)l->data;
		c->RaiseAccumulatedEvents ();
	}
	g_list_foreach (copy, (GFunc)base_unref, NULL);
	g_list_free (copy);
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

	printf ("%lld (%.2f) ", clock->GetCurrentTime(), clock->GetCurrentProgress());

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

	printf ("\n");

	if (clock->Is(Type::CLOCKGROUP)) {
		ClockGroup *cg = (ClockGroup*)clock;
		level += 2;
		for (GList *l = cg->child_clocks; l; l = l->next) {
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
		output_clock ((Clock*)l->data, 2);
	}
}

void
time_manager_list_clocks ()
{
	TimeManager::Instance()->ListClocks();
}

int Clock::CurrentTimeInvalidatedEvent = -1;
int Clock::CurrentStateInvalidatedEvent = -1;
int Clock::CurrentGlobalSpeedInvalidatedEvent = -1;
int Clock::CompletedEvent = -1;


Clock::Clock (Timeline *tl)
  : natural_duration (Duration::Automatic),
    current_state (Clock::Stopped), new_state (Clock::Stopped),
    current_progress (0.0), new_progress (0.0),
    current_time (0), new_time (0),
    seeking (false), seek_time (0),
    current_speed (1.0), new_speed (1.0),
    parent_clock (NULL),
    is_paused (false),
    has_started (false),
    timeline (tl),
    queued_events (0),
    is_reversed (false)
{
	RepeatBehavior *repeat = timeline->GetRepeatBehavior ();
	if (repeat->HasCount ()) {
		// remaining_iterations is an int.. GetCount returns a double.  badness?
		remaining_iterations = (int)repeat->GetCount ();
	}
	else {
		// XXX treat both Forever and Duration as Forever.
		remaining_iterations = -1;
	}

	if (timeline->HasBeginTime ())
		begintime = timeline->GetBeginTime ();
	/* otherwise it's filled in when we Begin the clock */

	autoreverse = timeline->GetAutoReverse ();

	duration = timeline->GetDuration ();
	if (duration->HasTimeSpan ()) {
		natural_duration = *duration;
#if CLOCK_DEBUG
		printf ("Clock %p (%s) has a timespan based duration of %llu\n", this, Type::Find(timeline->GetObjectType())->name, natural_duration.GetTimeSpan ());
#endif
	}
	else {
		natural_duration = timeline->GetNaturalDuration (this);
#if CLOCK_DEBUG
	  printf ("Clock %p (%s) has NON-timespan based duration, computed to be %llu\n", this, Type::Find(timeline->GetObjectType())->name, natural_duration.GetTimeSpan ());
#endif
	}
}

void
Clock::Tick ()
{
	TimeSpan new_parent_time = parent_clock == NULL ? TimeManager::Instance()->GetCurrentTime() : parent_clock->GetCurrentTime();

	TimeSpan our_delta = (TimeSpan)((new_parent_time - last_parent_time) * current_speed * timeline->GetSpeedRatio());

	last_parent_time = new_parent_time;

	if (seeking) {
		if (current_state != Clock::Active) {
			new_state = Clock::Active;
			QueueEvent (CURRENT_STATE_INVALIDATED);
		}

		new_time = seek_time;

		seeking = false;
	}
	else {
		if (current_state == Clock::Stopped)
			return;

		new_time = current_time;

		if (is_reversed)
			new_time -= our_delta;
		else
			new_time += our_delta;
	}

	/* if we were filling and ended up back in our Active period,
	   switch our state and return. */
	if (current_state == Clock::Filling) {
		if (natural_duration.HasTimeSpan ())
			current_progress = new_progress = (double)new_time / natural_duration.GetTimeSpan();
		else
			current_progress = new_progress = 0.0;

		new_state = Clock::Active;
		QueueEvent (CURRENT_STATE_INVALIDATED | CURRENT_TIME_INVALIDATED);
	}


#if CLOCK_DEBUG
	printf ("Clock %p updated to time %lld\n", this, new_time);
#endif

	if (natural_duration.HasTimeSpan ()) {
		TimeSpan duration_timespan = natural_duration.GetTimeSpan();

#if CLOCK_DEBUG
		printf ("+ clock %p (%s) duration is %lld, and new_time == %lld\n", this, timeline->GetName(), duration_timespan, new_time);
#endif
		if (!is_reversed && new_time >= duration_timespan) {
#if CLOCK_DEBUG
			printf ("+ clock %p (%s) hit its duration\n", this, timeline->GetName());
#endif
			// we've hit the end point of the clock's timeline
			if (autoreverse) {
				/* since autoreverse is true we need
				   to turn around and head back to
				   0.0 */
				is_reversed = true;
				new_time = duration_timespan - (new_time - duration_timespan);
			}
			else {
				/* but autoreverse is false. Decrement
				   the number of remaining iterations.

				   If we're done, Stop().  If we're
				   not done, force new_time to 0
				   so we start counting from there
				   again. */
				if (remaining_iterations > 0) {
					remaining_iterations --;
#if CLOCK_DEBUG
					printf (" + clock %p remaining iterations = %d\n", this, remaining_iterations);
#endif
				}

				if (remaining_iterations == 0) {
#if CLOCK_DEBUG
					printf (" + clock %p should fill\n", this);
#endif
					new_time = duration_timespan;
					SkipToFill ();
				}
				else {
					new_time -= duration_timespan;
				}
			}
		}
		else if (is_reversed && new_time <= 0) {
			is_reversed = false;
			new_time = -new_time;

			if (remaining_iterations > 0 && *duration != Duration::Automatic)
				remaining_iterations --;

			if (remaining_iterations == 0) {
				new_time = 0;
				SkipToFill ();
			}
		}

		if (duration_timespan == 0)
			new_progress = 1.0;
		else
			new_progress = (double)new_time / duration_timespan;
	}

	QueueEvent (CURRENT_TIME_INVALIDATED);
}

void
Clock::RaiseAccumulatedEvents ()
{
	if ((queued_events & CURRENT_TIME_INVALIDATED) != 0) {
		current_time = new_time;
		current_progress = new_progress;

		Emit (CurrentTimeInvalidatedEvent);
	}

	if ((queued_events & CURRENT_STATE_INVALIDATED) != 0) {
		current_state = new_state;
		if (current_state == Clock::Active)
			has_started = true;
		Emit (CurrentStateInvalidatedEvent);
	}

	if ((queued_events & CURRENT_GLOBAL_SPEED_INVALIDATED) != 0) {
		current_speed = new_speed;
		SpeedChanged ();
		Emit (CurrentGlobalSpeedInvalidatedEvent); /* this probably happens in SpeedChanged () */
	}

	/* XXX more events here, i assume */

	queued_events = 0;
}

TimeSpan
Clock::GetBeginTime ()
{
	if (timeline->HasBeginTime ()) {
		return timeline->GetBeginTime ();
	}
	else {
		return parent_clock == NULL ? TimeManager::Instance()->GetCurrentTime() : parent_clock->GetCurrentTime();
	}
}

void
Clock::Begin ()
{
	if (current_state != Clock::Stopped)
		return;

#if CLOCK_DEBUG
	printf ("Starting clock %p (%s) on the next tick\n", this, timeline->GetName());
#endif

	/* we're starting.  initialize our last_parent_time and current_time fields */
	last_parent_time = parent_clock == NULL ? TimeManager::Instance()->GetCurrentTime() : parent_clock->GetCurrentTime();

	if (seeking) {
		current_time = new_time = seek_time;
	}
	else {
		current_time = new_time = last_parent_time - GetBeginTime ();
	}

#if CLOCK_DEBUG
	printf (" + it's current time at that point will be %lld\n", current_time);
#endif
	if (natural_duration.HasTimeSpan ())
		current_progress = new_progress = (double)current_time / natural_duration.GetTimeSpan();
	else
		current_progress = new_progress = 0.0;

	new_state = Clock::Active;
	QueueEvent (CURRENT_STATE_INVALIDATED | CURRENT_TIME_INVALIDATED);
}

void
Clock::Pause ()
{
	if (is_paused)
		return;

	// XXX this likely should be updated in RaiseAccumulatedEvents like current_speed is
	is_paused = true;

	new_speed = 0.0;
	QueueEvent (CURRENT_GLOBAL_SPEED_INVALIDATED);
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

	// XXX this likely should be updated in RaiseAccumulatedEvents like current_speed is
	is_paused = false;

	new_speed = 1.0;
	QueueEvent (CURRENT_GLOBAL_SPEED_INVALIDATED);
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
		new_state = Clock::Filling;
		QueueEvent (CURRENT_STATE_INVALIDATED);		
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
	new_state = Clock::Stopped;
	QueueEvent (CURRENT_STATE_INVALIDATED);
}



ClockGroup::ClockGroup (TimelineGroup *timeline)
  : Clock (timeline),
    child_clocks (NULL),
    timeline (timeline)
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
	Clock::Begin ();

	if (current_state == Clock::Stopped && new_state == Clock::Active) {
		/* start any clocks that need starting immediately */
		for (GList *l = child_clocks; l; l = l->next) {
			Clock *c = (Clock*)l->data;
			if ((c->GetClockState() == Clock::Stopped) && (!c->GetHasStarted() && c->GetBeginTime() <= current_time)) {
				c->Begin ();
			}
		}
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

	for (GList *l = child_clocks; l; l = l->next) {
		Clock *c = (Clock*)l->data;
		c->Seek (seek_time);
	}
}

void
ClockGroup::Stop ()
{
	for (GList *l = child_clocks; l; l = l->next) {
		Clock *c = (Clock*)l->data;
		c->Stop ();
	}
}

void
ClockGroup::Tick ()
{
	/* recompute our current_time */
	this->Clock::Tick ();

	if (GetClockState() == Clock::Stopped)
		return;

	for (GList *l = child_clocks; l; l = l->next) {
		/* start the child clock here if we need to,
		   otherwise just call its Tick
		   method */
		Clock *c = (Clock*)l->data;
		if (c->GetClockState() != Clock::Stopped && c->GetNewClockState() != Clock::Stopped) {
			c->Tick ();
		}
		else if ((c->GetClockState() == Clock::Stopped) && (!c->GetHasStarted() && c->GetBeginTime () <= current_time)) {
			c->Begin ();
		}
	}

	/* if we're automatic and no child clocks are still running, we need to stop.

	   XXX we should probably do this by attaching to
	   CurrentStateInvalidated on the child clocks instead of
	   looping over them at the end of each Tick call.
	*/
	if (*duration == Duration::Automatic) {
		for (GList *l = child_clocks; l; l = l->next) {
			Clock *c = (Clock*)l->data;
			if (!c->GetHasStarted () || c->GetClockState() == Clock::Active || c->GetNewClockState() == Clock::Active)
				return;
		}

		Clock::Stop ();
	}
}

void
ClockGroup::RaiseAccumulatedEvents ()
{
	bool need_completed = false;
	
	if (new_state == Clock::Stopped && current_state != Clock::Stopped) {
		need_completed = true;
	}

	/* raise our events */
	this->Clock::RaiseAccumulatedEvents ();

	/* now cause our children to raise theirs*/
	GList *copy = g_list_copy (child_clocks);
	for (GList *l = copy; l; l = l->next) {
		((Clock*)l->data)->RaiseAccumulatedEvents ();
	}
	g_list_free (copy);

	if (need_completed) {
		Emit (CompletedEvent);
	}
}

ClockGroup::~ClockGroup ()
{
	GList *node = child_clocks;
	GList *next;
	
	while (node != NULL) {
		((Base *) node->data)->unref ();
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

	if (prop == ChildrenProperty) {
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
	TimelineCollection *collection = GetValue (ChildrenProperty)->AsTimelineCollection();
	ClockGroup *group = new ClockGroup (this);
	Collection::Node *node;
	
	node = (Collection::Node *) collection->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		Clock* clock = ((Timeline *) node->obj)->AllocateClock ();
		group->AddChild (clock);
		clock->unref ();
	}
	
	return group;
}

void
TimelineGroup::AddChild (Timeline *child)
{
	GetValue (ChildrenProperty)->AsTimelineCollection()->Add (child);
}

void
TimelineGroup::RemoveChild (Timeline *child)
{
	GetValue (ChildrenProperty)->AsTimelineCollection()->Remove (child);
}

TimelineGroup *
timeline_group_new ()
{
	return new TimelineGroup ();
}

Duration
ParallelTimeline::GetNaturalDurationCore (Clock *clock)
{
	TimelineCollection *collection = GetValue (ChildrenProperty)->AsTimelineCollection();
	Collection::Node *node = (Collection::Node *) collection->list->First ();
	Duration d = Duration::Automatic;
	TimeSpan duration_span = 0;
	
	if (!node)
		return Duration::FromSeconds (0);
	
	for ( ; node != NULL; node = (Collection::Node *) node->next) {
		Timeline *timeline = (Timeline *) node->obj;

		Duration duration = timeline->GetNaturalDuration (clock);
		if (duration == Duration::Automatic)
			continue;

		if (duration == Duration::Forever)
			return Duration::Forever;
		
		TimeSpan span = duration.GetTimeSpan ();
		
		RepeatBehavior *repeat = timeline->GetRepeatBehavior ();
		if (*repeat == RepeatBehavior::Forever)
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

TimelineMarker*
timeline_marker_new ()
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
}
