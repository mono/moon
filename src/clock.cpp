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


#define DESIRED_FPS 20

#define FPS_TO_DELAY(fps) (int)(((double)1/(fps)) * 1000)

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
                res = (TimeSpan)tv.tv_sec * 1000000 + tv.tv_usec;
                return res;
        }

	// XXX error
	return 0;
}


TimeManager* TimeManager::_instance = NULL;

TimeManager::TimeManager ()
  : child_clocks (NULL),
    tick_id (-1),
    current_timeout (FPS_TO_DELAY (DESIRED_FPS)),  /* something suitably small */
    tick_calls (NULL),
    flags (TimeManagerOp (TIME_MANAGER_UPDATE_CLOCKS | TIME_MANAGER_RENDER | TIME_MANAGER_TICK_CALL))
{
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
	GList *node = child_clocks;
	GList *next;
	
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
		TickCall *call = (TickCall*)tick_calls->data;

		// unlink the call first
		GList *new_tick_calls = tick_calls->next;
		g_list_free_1 (tick_calls);
		tick_calls = new_tick_calls;

		// now invoke it
		call->func (call->data);
		g_free (call);
	}
}

void
TimeManager::Tick ()
{
	STARTTICKTIMER (tick, "TimeManager::Tick");
	if (flags & TIME_MANAGER_UPDATE_CLOCKS) {
		STARTTICKTIMER (tick_update_clocks, "TimeManager::Tick - UpdateClocks");
		current_global_time = get_now ();
		// loop over all toplevel clocks, updating their time (and
		// triggering them to queue up their events) using the
		// value of current_global_time...

		for (GList *l = child_clocks; l; l = l->next)
			((Clock*)l->data)->TimeUpdated (current_global_time);

	
		// ... then cause all clocks to raise the events they've queued up
		RaiseEnqueuedEvents ();
		ENDTICKTIMER (tick_update_clocks, "TimeManager::Tick - UpdateClocks");
	}

	if (flags & TIME_MANAGER_RENDER) {
		STARTTICKTIMER (tick_render, "TimeManager::Tick - Render");
		Emit (NULL/* XXX */, "render");
		ENDTICKTIMER (tick_render, "TimeManager::Tick - Render");
	}

	if (flags & TIME_MANAGER_TICK_CALL) {
		STARTTICKTIMER (tick_call, "TimeManager::Tick - InvokeTick");
		InvokeTickCall ();
		ENDTICKTIMER (tick_call, "TimeManager::Tick - InvokeTick");
	}

	ENDTICKTIMER (tick, "TimeManager::Tick");
}

void
TimeManager::RaiseEnqueuedEvents ()
{
	for (GList *l = child_clocks; l; l = l->next)
		((Clock*)l->data)->RaiseAccumulatedEvents ();
}

void
TimeManager::AddTickCall (void (*func)(gpointer), gpointer tick_data)
{
	TickCall *call = g_new (TickCall, 1);
	call->func = func;
	call->data = tick_data;
	tick_calls = g_list_append (tick_calls, call);
}

void
time_manager_add_tick_call (void (*func)(gpointer), gpointer tick_data)
{
	TimeManager::Instance ()->AddTickCall (func, tick_data);
}

void
TimeManager::AddChild (Clock *child)
{
	child_clocks = g_list_prepend (child_clocks, child);
	child->ref ();
}

void
TimeManager::RemoveChild (Clock *child)
{
	child_clocks = g_list_remove (child_clocks, child);
	child->unref ();
}





Clock::Clock (Timeline *tl)
  : timeline (tl),
    current_state (STOPPED),
    queued_events (0),
    natural_duration (Duration::Automatic)
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

	autoreverse = timeline->GetAutoReverse ();

	duration = timeline->GetDuration ();
	if (duration->HasTimeSpan ()) {
#if CLOCK_DEBUG
	  printf ("Clock %p has a timespan based duration\n", this);
#endif
		natural_duration = *duration;
	}
	else {
#if CLOCK_DEBUG
	  printf ("Clock %p has NON-timespan based duration\n", this);
#endif
		natural_duration = timeline->GetNaturalDuration (this);
	}
	current_progress = 0.0;
	current_time = 0;
	reversed = false;
	iter_start = 0;
	parent_offset = 0;
}

void
Clock::TimeUpdated (TimeSpan parent_clock_time)
{
	if ((current_state & STOPPED) != 0) {
		return;
	}

	if ((current_state & PAUSED) != 0) {
		current_time_while_paused = parent_clock_time - iter_start - parent_offset;
		return;
	}

	current_time = parent_clock_time - iter_start - parent_offset;

#if CLOCK_DEBUG
	printf ("Clock %p updated to time %lld\n", this, current_time);
#endif

	if (natural_duration.HasTimeSpan ()) {
		TimeSpan duration_timespan = natural_duration.GetTimeSpan();

#if CLOCK_DEBUG
		printf ("+ clock %p duration is %lld\n", this, duration_timespan);
#endif
		if (reversed)
			current_time = duration_timespan - (current_time - duration_timespan);

		if (current_time >= duration_timespan) {
#if CLOCK_DEBUG
		  printf ("+ clock %p hit its duration\n", this);
#endif
			// we've hit the end point of the clock's timeline
			if (autoreverse) {
				/* since autoreverse is true we need
				   to turn around and head back to
				   0.0 */
				reversed = true;
				current_time = duration_timespan - (current_time - duration_timespan);
			}
			else {
				/* but autoreverse is false. Decrement
				   the number of remaining iterations.

				   If we're done, Stop().  If we're
				   not done, force current_time to 0
				   so we start counting from there
				   again. */
				if (remaining_iterations > 0) {
					remaining_iterations --;
#if CLOCK_DEBUG
					printf (" + clock %p remaining iterations = %f\n", this, remaining_iterations);
#endif
				}

				if (remaining_iterations == 0) {
#if CLOCK_DEBUG
					printf (" + clock %p should STOP\n", this, remaining_iterations);
#endif
					current_time = duration_timespan;
					Stop ();
				}
				else {
					current_time -= duration_timespan;
					iter_start = parent_clock_time - current_time;
				}
			}
		}
		else if (current_time <= 0) {
			reversed = false;
			current_time = -current_time;

			if (remaining_iterations > 0 && *duration != Duration::Automatic)
				remaining_iterations --;

			if (remaining_iterations == 0) {
				current_time = 0;
				Stop ();
			}
			else {
				iter_start = parent_clock_time - current_time;
			}
		}

		current_progress = (double)current_time / duration_timespan;
	}

	QueueEvent (CURRENT_TIME_INVALIDATED);
}

void
Clock::RaiseAccumulatedEvents ()
{
	if ((queued_events & CURRENT_TIME_INVALIDATED) != 0) {
		events->Emit (this, "CurrentTimeInvalidated");
	}

	if ((queued_events & CURRENT_STATE_INVALIDATED) != 0) {
		current_state = new_state;
		events->Emit (this, "CurrentStateInvalidated");
	}

	if ((queued_events & SPEED_CHANGED) != 0)
		SpeedChanged ();

	/* XXX more events here, i assume */

	queued_events = 0;
}

void
Clock::QueueEvent (int event)
{
	queued_events |= event;
}

TimeSpan
Clock::GetCurrentTime ()
{
	return current_time;
}

double
Clock::GetCurrentProgress ()
{
	return current_progress;
}

void
Clock::Begin (TimeSpan start_time)

{
	this->start_time = start_time;
	this->iter_start = start_time;
	current_state = new_state = RUNNING; /* should we invalidate the state here? */
	QueueEvent (CURRENT_STATE_INVALIDATED);
}

void
Clock::Pause ()
{
	if ((new_state & PAUSED) == PAUSED)
		return;

	new_state = (ClockState)(new_state | PAUSED);
	QueueEvent (CURRENT_STATE_INVALIDATED);

	pause_time = current_time;
}

void
Clock::Remove ()
{
}

void
Clock::Resume ()
{
	if ((new_state & PAUSED) == 0)
		return;

	new_state = (ClockState)(new_state & ~PAUSED);
	QueueEvent (CURRENT_STATE_INVALIDATED);

	parent_offset += current_time_while_paused - pause_time;
}

void
Clock::Seek (TimeSpan timespan)
{
}

void
Clock::SeekAlignedToLastTick ()
{
}

void
Clock::SkipToFill ()
{
}

void
Clock::Stop ()
{
#if CLOCK_DEBUG
	printf ("stopping clock %p after this tick\n", this);
#endif
	new_state = STOPPED;
	QueueEvent (CURRENT_STATE_INVALIDATED);
}



ClockGroup::ClockGroup (TimelineGroup *timeline)
  : timeline (timeline),
    Clock (timeline),
    child_clocks (NULL)
{
}

void
ClockGroup::AddChild (Clock *clock)
{
	child_clocks = g_list_append (child_clocks, clock);
	clock->ref ();
}

void
ClockGroup::RemoveChild (Clock *clock)
{
	child_clocks = g_list_remove (child_clocks, clock);
	clock->unref ();
}

void
ClockGroup::Begin (TimeSpan start_time)
{
	this->Clock::Begin (start_time);

	for (GList *l = child_clocks; l; l = l->next) {
		((Clock*)l->data)->Begin (current_time);
	}
}

void
ClockGroup::TimeUpdated (TimeSpan parent_clock_time)
{
	if ((current_state & STOPPED) != 0) {
		return;
	}

	/* recompute our current_time */
	this->Clock::TimeUpdated (parent_clock_time);

	if ((current_state & STOPPED) == 0) {
		for (GList *l = child_clocks; l; l = l->next) {
			((Clock*)l->data)->TimeUpdated (current_time);
		}
	}

	/* if we're automatic and no child clocks are still running, we need to stop.

	   XXX we should probably do this by attaching to
	   CurrentStateInvalidated on the child clocks instead of
	   looping over them at the end of each TimeUpdated call.
	*/
	if (*duration == Duration::Automatic) {
		bool stop = true;
		for (GList *l = child_clocks; l; l = l->next) {
			if ((((Clock*)l->data)->current_state & Clock::STOPPED) == 0) {
				stop = false;
				break;
			}
		}

		if (stop) {
			Stop ();
		}
	}
}

void
ClockGroup::RaiseAccumulatedEvents ()
{
	/* if we're changing from running to stopped and our progress is 1.0, we need to emit Completed */
	bool need_completed = false;
	if (current_progress == 1.0 && (new_state & STOPPED) != 0 && (current_state & STOPPED) == 0)
		need_completed = true;

	/* raise our events */
	this->Clock::RaiseAccumulatedEvents ();

	/* now cause our children to raise theirs*/
	for (GList *l = child_clocks; l; l = l->next) {
		((Clock*)l->data)->RaiseAccumulatedEvents ();
	}

	if (need_completed) {
		events->Emit (this, "Completed");
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

/* timeline group */

DependencyProperty* TimelineGroup::ChildrenProperty;

TimelineGroup::TimelineGroup ()
{
	this->SetValue (TimelineGroup::ChildrenProperty, Value (new TimelineCollection ()));
}

TimelineGroup::~TimelineGroup ()
{
}

void
TimelineGroup::OnPropertyChanged (DependencyProperty *prop)
{
	Timeline::OnPropertyChanged (prop);

	if (prop == ChildrenProperty) {
		TimelineCollection *newcol = GetValue (prop)->AsTimelineCollection();

		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	}
}

ClockGroup*
TimelineGroup::CreateClock ()
{
	TimelineCollection *child_timelines = GetValue (ChildrenProperty)->AsTimelineCollection();
	ClockGroup* group = new ClockGroup (this);
	for (GList *l = child_timelines->list; l ; l = l->next) {
		group->AddChild (((Timeline*)l->data)->AllocateClock ());
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
	TimelineCollection *child_timelines = GetValue (ChildrenProperty)->AsTimelineCollection();

	if (!child_timelines->list)
		return Duration (0);

	Duration d = Duration::Automatic;
	TimeSpan duration_span = 0;

	for (GList *l = child_timelines->list; l ; l = l->next) {
		Timeline *child_timeline = (Timeline*)l->data;

		Duration child_duration = child_timeline->GetNaturalDuration (clock);
		if (child_duration == Duration::Automatic)
			continue;

		if (child_duration == Duration::Forever)
			return Duration::Forever;

		TimeSpan child_span = child_duration.GetTimeSpan();

		RepeatBehavior *child_repeat = child_timeline->GetRepeatBehavior ();
		if (*child_repeat == RepeatBehavior::Forever) {
			return Duration::Forever;
		}
		else if (child_repeat->HasCount ()) {
			child_span *= child_repeat->GetCount ();
		}
		else if (child_repeat->HasDuration ()) {
			if (child_span > child_repeat->GetDuration ())
				child_span = child_repeat->GetDuration ();
		}

		Value *v = child_timeline->GetValue (Timeline::BeginTimeProperty);
		if (v)
			child_span += v->AsInt64();
		
		if (duration_span < child_span) {
			duration_span = child_span;
			d = Duration (duration_span);
		}
	}

	return d;
}

ParallelTimeline *
parallel_timeline_new ()
{
	return new ParallelTimeline ();
}

TimelineCollection *
timeline_collection_new ()
{
	return new TimelineCollection ();
}


void
clock_init ()
{
	/* Timeline properties */
	Timeline::AutoReverseProperty = DependencyObject::Register (Type::TIMELINE, "AutoReverse", new Value (false));
	Timeline::BeginTimeProperty = DependencyObject::RegisterNullable (Type::TIMELINE, "BeginTime", Type::INT64);
	Timeline::DurationProperty = DependencyObject::Register (Type::TIMELINE, "Duration", new Value (Duration::Automatic));
	Timeline::RepeatBehaviorProperty = DependencyObject::Register (Type::TIMELINE, "RepeatBehavior", new Value (RepeatBehavior ((double)1)));
	Timeline::FillBehaviorProperty = DependencyObject::Register (Type::TIMELINE, "FillBehavior", new Value ((int)FillBehaviorHoldEnd));
	Timeline::SpeedRatioProperty = DependencyObject::Register (Type::TIMELINE, "SpeedRatio", new Value (1.0));

	/* TimelineGroup properties */
	TimelineGroup::ChildrenProperty = DependencyObject::Register (Type::TIMELINEGROUP, "Children", Type::TIMELINE_COLLECTION);
}
