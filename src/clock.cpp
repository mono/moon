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



static TimeSpan
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
  : child_clocks (NULL)
{
}

void
TimeManager::Start()
{
	current_global_time = get_now ();
	tick_id = gtk_timeout_add (60 /* something suitably small */, TimeManager::tick_timeout, this);
}

gboolean
TimeManager::tick_timeout (gpointer data)
{
	((TimeManager*)data)->Tick ();
	return TRUE;
}

void
TimeManager::Tick ()
{
	current_global_time = get_now ();

	//printf ("TimeManager::Tick\n");

	// loop over all toplevel clocks, updating their time (and
	// triggering them to queue up their events) using the
	// value of current_global_time...

	for (GList *l = child_clocks; l; l = l->next)
		((Clock*)l->data)->TimeUpdated (current_global_time);

	
	// ... then cause all clocks to raise the events they've queued up
	RaiseEnqueuedEvents ();
}

void
TimeManager::RaiseEnqueuedEvents ()
{
	for (GList *l = child_clocks; l; l = l->next)
		((Clock*)l->data)->RaiseAccumulatedEvents ();
}

void
TimeManager::AddChild (Clock *child)
{
	child_clocks = g_list_prepend (child_clocks, child);
}

void
TimeManager::RemoveChild (Clock *child)
{
	child_clocks = g_list_remove (child_clocks, child);
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
	if (duration->HasTimeSpan ())
		natural_duration = *duration;
	else
		natural_duration = timeline->GetNaturalDuration (this);
	current_progress = 0.0;
	current_time = 0;
	reversed = false;
	iter_start = 0;
	parent_offset = 0;
}

void
Clock::TimeUpdated (TimeSpan parent_clock_time)
{
	if ((current_state & (STOPPED | PAUSED)) != 0) {
		current_time_while_paused = parent_clock_time - iter_start - parent_offset;
		return;
	}

	current_time = parent_clock_time - iter_start - parent_offset;

	if (natural_duration.HasTimeSpan ()) {
		TimeSpan duration_timespan = natural_duration.GetTimeSpan();

		if (reversed)
			current_time = duration_timespan - (current_time - duration_timespan);

		if (current_time >= duration_timespan) {
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
				if (remaining_iterations > 0 && *duration != Duration::Automatic)
					remaining_iterations --;

				if (remaining_iterations == 0) {
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
	  events->Emit (/*this,*/ "CurrentTimeInvalidated");
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
	printf ("Starting %lld\n", start_time);
	this->start_time = start_time;
	this->iter_start = start_time;
	current_state = RUNNING; /* should we invalidate the state here? */
	QueueEvent (CURRENT_STATE_INVALIDATED);
}

void
Clock::Pause ()
{
	if ((current_state & PAUSED) == PAUSED)
		return;

	current_state = (ClockState)(current_state | PAUSED);
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
	if ((current_state & PAUSED) == 0)
		return;

	current_state = (ClockState)(current_state & ~PAUSED);
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
	current_state = STOPPED;
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
}

void
ClockGroup::RemoveChild (Clock *clock)
{
	child_clocks = g_list_remove (child_clocks, clock);
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
	/* recompute our current_time */
	this->Clock::TimeUpdated (parent_clock_time);

	for (GList *l = child_clocks; l; l = l->next) {
		((Clock*)l->data)->TimeUpdated (current_time);
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
		  printf ("STOP STOP STOP\n");
			Stop ();
		}
	}
}

void
ClockGroup::RaiseAccumulatedEvents ()
{
	/* raise our events */
	this->Clock::RaiseAccumulatedEvents ();

	/* now cause our children to raise theirs*/
	for (GList *l = child_clocks; l; l = l->next) {
		((Clock*)l->data)->RaiseAccumulatedEvents ();
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
	child_timelines = NULL;
	TimelineCollection *c = new TimelineCollection ();

	this->SetValue (TimelineGroup::ChildrenProperty, Value (c));

	// Ensure that the callback OnPropertyChanged was called.
	g_assert (c == child_timelines);
}

void
TimelineGroup::OnPropertyChanged (DependencyProperty *prop)
{
	Timeline::OnPropertyChanged (prop);

	if (prop == ChildrenProperty){
		// The new value has already been set, so unref the old collection

		TimelineCollection *newcol = GetValue (prop)->AsTimelineCollection();

		if (newcol != child_timelines){
			if (child_timelines){
				for (GSList *l = child_timelines->list; l != NULL; l = l->next){
					DependencyObject *dob = (DependencyObject *) l->data;
					
					base_unref (dob);
				}
				base_unref (child_timelines);
				g_slist_free (child_timelines->list);
			}

			child_timelines = newcol;
			if (child_timelines->closure)
				printf ("Warning we attached a property that was already attached\n");
			child_timelines->closure = this;
			
			base_ref (child_timelines);
		}
	}
}

ClockGroup*
TimelineGroup::CreateClock ()
{
	ClockGroup* group = new ClockGroup (this);
	for (GSList *l = child_timelines->list; l ; l = l->next) {
		group->AddChild (((Timeline*)l->data)->AllocateClock ());
	}

	return group;
}

void
TimelineGroup::AddChild (Timeline *child)
{
	Value fv = Value(child);
	child_timelines->Add (&fv);
}

void
TimelineGroup::RemoveChild (Timeline *child)
{
}

Duration
ParallelTimeline::GetNaturalDurationCore (Clock *clock)
{
	if (!child_timelines->list)
		return Duration (0);

	Duration d = Duration::Automatic;
	TimeSpan duration_span = 0;

	for (GSList *l = child_timelines->list; l ; l = l->next) {
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

void
clock_init ()
{
	/* Timeline properties */
	Timeline::AutoReverseProperty = DependencyObject::Register (Value::TIMELINE, "AutoReverse", new Value (false));
	Timeline::BeginTimeProperty = DependencyObject::Register (Value::TIMELINE, "BeginTime", Value::INT64);
	Timeline::DurationProperty = DependencyObject::Register (Value::TIMELINE, "Duration", new Value (Duration::Automatic));
	Timeline::RepeatBehaviorProperty = DependencyObject::Register (Value::TIMELINE, "RepeatBehavior", new Value (RepeatBehavior ((double)1)));
	Timeline::FillBehaviorProperty = DependencyObject::Register (Value::TIMELINE, "FillBehavior", new Value ((int)FillBehaviorHoldEnd));
	Timeline::SpeedRatioProperty = DependencyObject::Register (Value::TIMELINE, "SpeedRatio", new Value (1.0));

	/* TimelineGroup properties */
	TimelineGroup::ChildrenProperty = DependencyObject::Register (Value::TIMELINEGROUP, "Children", Value::TIMELINE_COLLECTION);
}
