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



static guint64
get_now ()
{
        struct timeval tv;
        guint64 res;

        if (gettimeofday (&tv, NULL) == 0) {
                res = (guint64)tv.tv_sec * 1000000 + tv.tv_usec;
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
	completed_iterations = 0.0;
	current_progress = 0.0;
	current_time = 0;
	reversed = false;
	start_time = 0;
}

void
Clock::TimeUpdated (guint64 parent_clock_time)
{
	if ((current_state & (STOPPED | PAUSED)) != 0)
		return;

	current_time = parent_clock_time - start_time;
	QueueEvent (CURRENT_TIME_INVALIDATED);

	if (*duration == Duration::Automatic  /* useful only on clock groups, means the clock stops when all child clocks have stopped. */
	    || *duration == Duration::Forever /* for Forever duration timelines, progress is always 0.0 */) {
		current_progress = 0.0;
	}
	else {
		// we've got a timespan, so our progress will have changed
		guint64 duration_timespan = duration->GetTimeSpan();

		double new_progress;

		new_progress = (double)(current_time - start_time - completed_iterations * (autoreverse ? 2 : 1) * duration_timespan) / duration_timespan;

		if (new_progress >= 1.0) {
			if (reversed) {
				reversed = false;

				/* we were heading from 1.0 to 0.0 and
				   passed it.  decrement our
				   remaining-iteration count (if we have
				   one) */
				completed_iterations += 0.5;

				current_progress = new_progress - 1.0;
				guint64 diff = current_time - start_time;
// 				printf ("current_time=%llu start_time=%llu diff=%llu\n", current_time, start_time, diff);
// 				printf ("completed_iter=%g autoreverse=%d duration=%llu\n", completed_iterations, autoreverse, duration_timespan);
// 				printf ("Assigning=%g\n", current_progress);

				if (remaining_iterations > 0)
					remaining_iterations --;

				if (remaining_iterations == 0) {
					current_progress = 0.0;
					Stop ();
				}
			}
			else {
				if (autoreverse) {
					reversed = true;
					completed_iterations += 0.5;
					current_progress = 1.0 - (new_progress - 1.0);
				}
				else {

					completed_iterations += 1.0;

					current_progress = new_progress - 1.0;

					if (remaining_iterations > 0)
						remaining_iterations --;

					if (remaining_iterations == 0) {
						current_progress = 1.0;
						Stop ();
					}
				}
			}
		}
		else {
			current_progress = reversed ? 1.0 - new_progress : new_progress;
		}
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

guint64
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
Clock::Begin (guint64 start_time)
{
	printf ("Starting %llu\n", start_time);
	this->start_time = start_time;
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
Clock::Seek (/*Timespan*/guint64 timespan)
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
ClockGroup::Begin (guint64 start_time)
{
	this->Clock::Begin (start_time);

	for (GList *l = child_clocks; l; l = l->next) {
		((Clock*)l->data)->Begin (current_time);
	}
}

void
ClockGroup::TimeUpdated (guint64 parent_clock_time)
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
	return GetValue (Timeline::RepeatBehaviorProperty)->u.repeat;
}

void
Timeline::SetAutoReverse (bool autoreverse)
{
	SetValue (Timeline::AutoReverseProperty, Value(autoreverse));
}

bool
Timeline::GetAutoReverse ()
{
	return (bool) GetValue (Timeline::AutoReverseProperty)->u.i32;
}

void
Timeline::SetDuration (Duration duration)
{
	SetValue (Timeline::DurationProperty, Value(duration));
}

Duration*
Timeline::GetDuration ()
{
	return GetValue (Timeline::DurationProperty)->u.duration;
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

