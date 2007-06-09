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
get_now ()
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
    queued_events (0)
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
	duration = timeline->GetDuration();
	current_progress = 0.0;
	current_time = 0;
	reversed = false;
	start_time = 0;
	iter_start = 0;
}

void
Clock::TimeUpdated (TimeSpan parent_clock_time)
{
	if ((current_state & (STOPPED | PAUSED)) != 0)
		return;

	if (*duration == Duration::Automatic  /* useful only on clock groups, means the clock stops when all child clocks have stopped. */
	    || *duration == Duration::Forever /* for Forever duration timelines, progress is always 0.0 */) {
		current_progress = 0.0;
	}
	else {
		// we've got a timespan, so our progress will have changed
		TimeSpan duration_timespan = duration->GetTimeSpan();

		current_time = parent_clock_time - iter_start;
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
				if (remaining_iterations > 0)
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

			if (remaining_iterations > 0)
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
		QueueEvent (CURRENT_TIME_INVALIDATED);
	}
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
	current_state = (ClockState)(current_state | PAUSED);
}

void
Clock::Remove ()
{
}

void
Clock::Resume ()
{
	current_state = (ClockState)(current_state & ~PAUSED);
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



/* timeline group */

TimelineGroup::TimelineGroup ()
	: child_timelines (NULL)
{
}

ClockGroup*
TimelineGroup::CreateClock ()
{
	ClockGroup* group = new ClockGroup (this);
	for (GList *l = child_timelines; l ; l = l->next) {
		group->AddChild (((Timeline*)l->data)->AllocateClock ());
	}

	return group;
}

void
TimelineGroup::AddChild (Timeline *child)
{
	child_timelines = g_list_prepend (child_timelines, child);
}

void
TimelineGroup::RemoveChild (Timeline *child)
{
	child_timelines = g_list_remove (child_timelines, child);
}

