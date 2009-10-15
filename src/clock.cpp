/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * clock.cpp: Clock management
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <glib.h>

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <time.h>

#include "clock.h"
#include "timeline.h"
#include "timemanager.h"

#include "runtime.h"
#include "deployment.h"

#define CLOCK_DEBUG 0

RepeatBehavior RepeatBehavior::Forever (RepeatBehavior::FOREVER);
Duration Duration::Automatic (Duration::AUTOMATIC);
Duration Duration::Forever (Duration::FOREVER);


struct ClockNode {
	ClockNode *next;
	Clock *clock;
};

typedef void (*ClockFunc)(Clock*);

void
clock_list_foreach (GList *clock_list, ClockFunc func)
{
	GList *list = NULL, *tail = NULL;
	for (GList *l = clock_list; l; l = l->next) {
		list = g_list_prepend (list, l->data);
		if (!tail) tail = list;
		((Clock*)l->data)->ref();
	}
	for (GList *node = tail;node;node = node->prev) {
		func ((Clock*)node->data);
		((Clock*)node->data)->unref ();
	}
	g_list_free (list);
}

static void
CallRaiseAccumulatedEvents (Clock *clock)
{
	clock->RaiseAccumulatedEvents ();
}

static void
CallRaiseAccumulatedCompleted (Clock *clock)
{
	clock->RaiseAccumulatedCompleted ();
}


Clock::Clock (Timeline *tl)
  : natural_duration (Duration::Automatic)
{
	SetObjectType (Type::CLOCK);

	calculated_natural_duration = false;
	state = Clock::Stopped;
	progress = 0.0;
	current_time = 0;
	seek_time = 0;
	time_manager = NULL;
	parent_clock = NULL;
	is_paused = false;
	is_seeking = false;
	begin_pause_time = 0;
	accumulated_pause_time = 0;
	has_started = false;
	timeline = tl;
	timeline->ref();
	queued_events = 0;
	root_parent_time = 0;

	was_stopped = false;
	begin_time = -1;
	begin_on_tick = false;
	emit_completed = false;
	has_completed = false;
}

Clock::~Clock ()
{
	timeline->unref();
}

void
Clock::Dispose ()
{
	if (!IsDisposed ()) {
		DependencyObject::Dispose ();
		GetTimeline()->TeardownClock ();
	}
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
Clock::UpdateFromParentTime (TimeSpan parentTime)
{
#define CLAMP_NORMALIZED_TIME do {			\
	if (normalizedTime < 0.0) normalizedTime = 0.0; \
	if (normalizedTime > 1.0) normalizedTime = 1.0; \
} while (0)

	//
	// The idea behind this method is that it is possible (and
	// easier, and clearer) to apply a simple function to our
	// parent clock's time to calculate our own time.
	//
	// We also calculate our progress (MS uses the term
	// "normalized time"), a value in the range [0-1] at the same
	// time.
	//
	// This clock's localTime runs from the range
	// [0-natural_duration] for natural_durations with timespans,
	// and [0-$forever] for Forever durations.  Automatic
	// durations are translated into timespans.

	if (!GetHasStarted() && !GetWasStopped() && (GetBeginOnTick() || timeline->GetBeginTime () <= parentTime)) {
		if (GetBeginOnTick())
			BeginOnTick (false);
		Begin (parentTime);
	}

	// root_parent_time is the time we were added to our parent clock.
	// timeline->GetBeginTime() is expressed in the time-space of the parent clock.
	//
	// subtracting those two translates our start time to 0
	//
	// we then have to account for our accumulated pause time, and
	// scale the whole thing by our speed ratio.
	//
	// the result is a timespan unaffected by repeatbehavior or
	// autoreverse.  it is simple the timespan our clock has been
	// running.
	TimeSpan localTime = (parentTime - root_parent_time - timeline->GetBeginTime() - accumulated_pause_time) * timeline->GetSpeedRatio();

	bool seek_completed = false;

	if (is_seeking) {
		// if we're seeking, we need to arrange for the above
		// localTime formula to keep time correctly.  we clear
		// accumulated_pause_time, and adjust root_parent_time
		// such that we can re-evaluate localTime and have
		// localTime = seek_time.

		begin_pause_time = 0;
		accumulated_pause_time = 0;

		/* seek_time = localTime

		   seek_time = (parentTime - root_parent_time - timeline->BeginTime() - 0) * timeline->GetSpeedRatio ()

		          seek_time
  		   ------------------------- = parentTime - root_parent_time - timeline->BeginTime();
		   timeline->GetSpeedRatio()
                                                                                  seek_time         
		   root_parent_time = parentTime - timeline->BeginTime() - -------------------------
									   timeline->GetSpeedRatio()
		*/
		root_parent_time = parentTime - (timeline->GetBeginTime () - seek_time) / timeline->GetSpeedRatio ();
		localTime = (seek_time - timeline->GetBeginTime()) * timeline->GetSpeedRatio();
		is_seeking = false;
		seek_completed = true;

		if (!GetHasStarted())
			CalculateFillTime ();
	}
	else if (is_paused) {
		// if we're paused and not seeking, we don't update
		// anything.
		return true;
	}

	// the clock doesn't update and we don't progress if the
	// translated local time is before our begin time.  Keep in
	// mind that this can happen *after* a clock has started,
	// since parentTime isn't strictly increasing.  It can
	// decrease and represent a time before our start time.
	if (localTime < 0)
		return true;

	if (GetClockState () == Clock::Stopped) {
		if (!seek_completed)
			return false;

		// even for stopped clocks we update their position if they're seeked.
	}

	double normalizedTime = 0.0;


	// we only do the bulk of the work if the duration has a
	// timespan.  if we're automatic/forever, our normalizedTime
	// stays pegged at 0.0, and our localTime progresses
	// undisturbed.  i.e. a RepeatBehavior="2x" means nothing if
	// the Duration of the animation is forever.
	if (GetNaturalDuration().HasTimeSpan()) {
		TimeSpan natural_duration_timespan = GetNaturalDuration().GetTimeSpan();
		
		if (natural_duration_timespan <= 0) {
			// for clocks with instantaneous begin times/durations, expressable like so:
			//     <DoubleAnimation Storyboard.TargetProperty="Opacity" To="1" BeginTime="00:00:00" Duration="00:00:00" />
			// we keep our localtime pegged at 0 (FIXME:
			// without filling?) and our normalizedTime at
			// 1.  The latter makes sure that value is applied in full.
 			localTime = 0;
 			normalizedTime = 1.0;
			if (GetClockState () == Clock::Active) {
				FillOnNextTick ();
				Completed ();
			}
		}
		else if (natural_duration_timespan > 0) {
			RepeatBehavior *repeat = timeline->GetRepeatBehavior ();

			if (!repeat->IsForever() && localTime >= fillTime) {
				// fillTime represents the local time
				// at which the number of repeats
				// (expressed either as a timespan or
				// e.g. "2x") and autoreverses have
				// completed.  i.e. it's the
				// $natural_duration * $repeat_count
				// for repeat behaviors with counts,
				// and $repeat_duration for repeat
				// behaviors with timespans.

				// if the timeline is auto-reversible,
				// we always end at localTime = 0.
				// Otherwise we know it's fillTime.
				localTime = timeline->GetAutoReverse () ? 0 : fillTime;
				normalizedTime = (double)localTime / natural_duration_timespan;
				CLAMP_NORMALIZED_TIME;
				if (GetClockState () == Clock::Active) {
					FillOnNextTick ();
					Completed ();
				}
			}
			else {
				if (GetClockState () != Clock::Active)
					SetClockState (Clock::Active);

				if (localTime > 0) {
					double t = (double)localTime / natural_duration_timespan;
					int ti = (int)t;
					double fract = t - ti;

					// This block of code is the first time where localTime is translated
					// into per-repeat/per-autoreverse segments.  We do it here because it
					// allows us to use a cute hack for determining if we're ascending or
					// descending.
					//
					// for instance:

					// <storyboard duration="00:00:12">
					//   <doubleanimation begintime="00:00:00" repeatbehavior="2x" autoreverse="<below>" duration="00:00:03" />
					// </storyboard>
					//
					//  autoreverse = true                       autoreverse = false
					// 0  / 3 = 0        = 0                  0 / 3 = 0           = 0
					// 1  / 3 = .333     > 0.333              1 / 3 = .333        > 0.333
					// 2  / 3 = .666     > 0.666              2 / 3 = .666        > 0.666
					// 3  / 3 = 1        = 1                  3 / 3 = 1           = 1
					// 4  / 3 = 1.33     < 0.666              4 / 3 = 1.33        > 0.333
					// 5  / 3 = 1.66     < 0.333              5 / 3 = 1.66        > 0.666
					// 6  / 3 = 2        = 0                  6 / 3 = 2           = 1
					// 7  / 3 = 2.33     > 0.333
					// 8  / 3 = 2.66     > 0.666
					// 9  / 3 = 3        = 1
					// 10 / 3 = 3.33     < 0.666
					// 11 / 3 = 3.66     < 0.333
					// 12 / 3 = 4        = 0


					// a little explanation:  the $localtime / $natural_duration = $foo is done
					// to factor out the repeat count.  we know that the time within a given repeated
					// run is just the fractional part of that (if the result has a fraction), or 0 or 1.

					// the >,<,= column above represents whether we're increasing, decreasing, or at an
					// end-point, respectively.

					if (timeline->GetAutoReverse()) {
						// left column above
						if (ti & 1) {
							// e.g:
							// 3  / 3 = 1        = 1    
							// 4  / 3 = 1.33     < 0.666
							// 5  / 3 = 1.66     < 0.333

							// we know we're either at normalized time 1 (at our duration), or we're descending,
							// based on if there's a fractional component.
							if (ti == t) {
								normalizedTime = 1.0;
								localTime = natural_duration_timespan;
							}
							else {
								/* we're descending */
								normalizedTime = 1.0 - fract;
								CLAMP_NORMALIZED_TIME;
								localTime = normalizedTime * natural_duration_timespan;
							}
						}
						else {
							// e.g:
							// 6  / 3 = 2        = 0    
							// 7  / 3 = 2.33     > 0.333
							// 8  / 3 = 2.66     > 0.666

							// we know we're either at normalizd time 0 (at our start time), or we're ascending,
							// based on if there's a fractional component.
							if (ti == t) {
								normalizedTime = 0.0;
								localTime = 0;
							}
							else {
								/* we're ascending */
								normalizedTime = fract;
								CLAMP_NORMALIZED_TIME;
								localTime = normalizedTime * natural_duration_timespan;
							}
						}
					}
					else {
						// e.g.:
						// 0 / 3 = 0           = 0
						// 1 / 3 = .333        > 0.333
						// 2 / 3 = .666        > 0.666
						// 3 / 3 = 1           = 1
						// 4 / 3 = 1.33        > 0.333
						// 5 / 3 = 1.66        > 0.666
						// 6 / 3 = 2           = 1

						// we're always ascending here (since autoreverse is off), and we know we're > 0,
						// so we don't need to concern ourselves with that case.  At the integer points we're
						// at our duration, and otherwise we're at the fractional value.
						if (ti == t) {
							normalizedTime = 1.0;
							localTime = natural_duration_timespan;
						}
						else {
							/* we're ascending */
							normalizedTime = fract;
							CLAMP_NORMALIZED_TIME;
							localTime = normalizedTime * natural_duration_timespan;
						}
					}
				}
			}
		}
	}

	SetCurrentTime (localTime);
	progress = normalizedTime;
	return true;
}

void
Clock::BeginOnTick (bool begin)
{
	begin_on_tick = begin;

	// tell the time manager that we need another tick
	time_manager->NeedClockTick ();
}

void
Clock::SetClockState (ClockState state)
{
#if CLOCK_DEBUG
	const char *states[] = { "Active", "Filling", "Stopped" };
	printf ("Setting clock %p state to %s\n", this, states[state]);
#endif
	this->state = state;
	QueueEvent (CURRENT_STATE_INVALIDATED);
}

void
Clock::SetCurrentTime (TimeSpan ts)
{
	current_time = ts;
	QueueEvent (CURRENT_TIME_INVALIDATED);
}

void
Clock::QueueEvent (int event)
{
	queued_events |= event;
}

void
Clock::RaiseAccumulatedEvents ()
{
	if ((queued_events & CURRENT_TIME_INVALIDATED) != 0) {
		Emit (CurrentTimeInvalidatedEvent);
	}

	if ((queued_events & CURRENT_STATE_INVALIDATED) != 0) {
		if (state != Clock::Stopped)
			has_started = true;
		Emit (CurrentStateInvalidatedEvent);
	}

	queued_events = 0;
}

void
Clock::RaiseAccumulatedCompleted ()
{
	if (emit_completed) {
		emit_completed = false;
//  		printf ("clock %p (%s) completed\n", this, GetName ());
		Emit (CompletedEvent);
		has_completed = true;
	}
}

void
Clock::SetRootParentTime (TimeSpan parentTime)
{
	root_parent_time = parentTime;
}

void
Clock::CalculateFillTime ()
{
	if (GetNaturalDuration().HasTimeSpan()) {
		RepeatBehavior *repeat = timeline->GetRepeatBehavior ();
		if (repeat->HasDuration ()) {
			fillTime = (repeat->GetDuration() * timeline->GetSpeedRatio ());
		}
		else if (repeat->HasCount ()) {
			fillTime = repeat->GetCount() * GetNaturalDuration().GetTimeSpan() * (timeline->GetAutoReverse() ? 2 : 1);
		}
		else {
			fillTime = GetNaturalDuration().GetTimeSpan() * (timeline->GetAutoReverse() ? 2 : 1);
		}
	}
}

void
Clock::Begin (TimeSpan parentTime)
{
	//printf ("clock %p (%s) begin\n", this, GetName ());
	emit_completed = false;
	has_completed = false;
	was_stopped = false;

	/* we're starting.  initialize our current_time field */
	SetCurrentTime ((parentTime - root_parent_time - timeline->GetBeginTime ()) * timeline->GetSpeedRatio());

	Duration d = GetNaturalDuration ();
	if (d.HasTimeSpan ()) {
		if (d.GetTimeSpan() == 0) {
			progress = 1.0;
		}
		else {
			progress = (double)current_time / d.GetTimeSpan();
			if (progress > 1.0)
				progress = 1.0;
		}
	}
	else
		progress = 0.0;

	CalculateFillTime ();

	SetClockState (Clock::Active);

	// force the time manager to tick the clock hierarchy to wake it up
	time_manager->NeedClockTick ();
}

void
Clock::Pause ()
{
 	//printf ("clock %p (%s) paused\n", this, GetName ());

	if (is_paused)
		return;

	is_paused = true;
	begin_pause_time = GetCurrentTime();
}

void
Clock::Resume ()
{
	if (!is_paused)
		return;

	is_paused = false;

	accumulated_pause_time += GetCurrentTime() - begin_pause_time;
}

void
Clock::Seek (TimeSpan timespan)
{
 	//printf ("clock %p (%s) seek to timespan %" G_GINT64_FORMAT "\n", this, GetName (), timespan);

	seek_time = timespan;

	is_seeking = true;
}

void
Clock::SeekAlignedToLastTick (TimeSpan timespan)
{
	Seek (timespan);

	if (parent_clock)
		UpdateFromParentTime (parent_clock->GetCurrentTime());
}

void
Clock::FillOnNextTick ()
{
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
Clock::SkipToFill ()
{
	// FIXME: this only works on clocks that have a duration
#if CLOCK_DEBUG
	printf ("filling clock %p after this tick\n", this);
#endif

	Seek (fillTime);
	
	FillOnNextTick ();
}

void
Clock::Stop ()
{
	SetClockState (Clock::Stopped);
	was_stopped = true;
}

void
Clock::Reset ()
{
//  	printf ("clock %p (%s) reset\n", this, GetName());
	calculated_natural_duration = false;
	state = Clock::Stopped;
	progress = 0.0;
	current_time = 0;
	seek_time = 0;
	begin_time = -1;
	begin_on_tick = false;
	is_paused = false;
	is_seeking = false;
	begin_pause_time = 
	accumulated_pause_time = 0;
	has_started = false;
	was_stopped = false;
	emit_completed = false;
	has_completed = false;
}

void
Clock::Completed ()
{
	if (!has_completed)
		emit_completed = true;
}

ClockGroup::ClockGroup (TimelineGroup *timeline, bool timemanager_clockgroup)
  : Clock (timeline)
{
	SetObjectType (Type::CLOCKGROUP);

	this->timemanager_clockgroup = timemanager_clockgroup;

	child_clocks = NULL;
}

void
ClockGroup::AddChild (Clock *clock)
{
	clock->SetRootParentTime (GetCurrentTime ());
	clock->SetParentClock (this);

	child_clocks = g_list_append (child_clocks, clock);
	clock->ref ();
	clock->SetTimeManager (GetTimeManager());
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
	clock->SetTimeManager (NULL);
	clock->SetParentClock (NULL);
	clock->unref ();
}

void
ClockGroup::Begin (TimeSpan parentTime)
{
	Clock::Begin (parentTime);

	for (GList *l = child_clocks; l; l = l->next) {
		Clock *c = (Clock*)l->data;
		c->ClearHasStarted ();

		/* start any clocks that need starting immediately */
		if (c->GetTimeline()->GetBeginTime() <= current_time) {
			c->Begin (current_time);
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
ClockGroup::Stop ()
{
	for (GList *l = child_clocks; l; l = l->next) {
		Clock *clock = (Clock*)l->data;

		if (timemanager_clockgroup || !clock->Is(Type::CLOCKGROUP)) {
			// we don't stop sub-clock groups, since if we
			// nest storyboards under one another they
			// seem to behave independent of each other
			// from this perspective.
			((Clock*)l->data)->Stop ();
		}
	}

	Clock::Stop ();
}

bool
ClockGroup::UpdateFromParentTime (TimeSpan parentTime)
{
	// we need to cache this here because
	// Clock::UpdateFromParentTime will be updating it for the
	// next tick.
	ClockState current_state = GetClockState();

	/* likewise, we need to cache this here since
	   Clock::UpdateFromParentTime will clear it */
	bool seeking = GetIsSeeking();

	bool rv = Clock::UpdateFromParentTime (parentTime);

	// ClockGroups (which correspond to storyboards generally)
	// only cause their children to update (and therefore for
	// animations to hold/progress their value) if they are
	// active, or if they've had Seek called on them.
	//
	// but it also happens when the clockgroup is in the Filling
	// state.  This means that you can attach a handler to
	// Storyboard.Completed and inside the handler modify a
	// property that an animation under that storyboard was
	// targetting.  and the new setting isnt clobbered by the
	// animation like it would be if the storyboard was active.

	bool update_child_clocks = (current_state == Clock::Active || seeking);

	for (GList *l = child_clocks; l; l = l->next) {
		Clock *clock = (Clock*)l->data;
		if (update_child_clocks || clock->Is(Type::CLOCKGROUP))
			rv = clock->UpdateFromParentTime (current_time) || rv;
	}

	return rv;
}

void
ClockGroup::RaiseAccumulatedEvents ()
{
	/* raise our events */
	Clock::RaiseAccumulatedEvents ();
	
	/* now cause our children to raise theirs */
	clock_list_foreach (child_clocks, CallRaiseAccumulatedEvents);
}

void
ClockGroup::RaiseAccumulatedCompleted ()
{
	Clock::RaiseAccumulatedCompleted ();
	clock_list_foreach (child_clocks, CallRaiseAccumulatedCompleted);
}

void
ClockGroup::Reset ()
{
	Clock::Reset ();

	for (GList *l = child_clocks; l; l = l->next)
		((Clock*)l->data)->Reset();
}

ClockGroup::~ClockGroup ()
{
}

void
ClockGroup::Dispose ()
{
	GList *node = child_clocks;
	while (node) {
		Clock *clock = (Clock*)node->data;
		node = node->next;
		clock->Dispose ();
	}
	Clock::Dispose ();
}
