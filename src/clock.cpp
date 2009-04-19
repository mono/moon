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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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

void
Clock::UpdateFromParentTime (TimeSpan parentTime)
{
	if (is_paused)
		return;

	double normalizedTime = 0.0;
	TimeSpan localTime = (parentTime - root_parent_time - timeline->GetBeginTime()) * timeline->GetSpeedRatio();

	localTime -= accumulated_pause_time * timeline->GetSpeedRatio();

	if (localTime < 0)
		return;

	if (!GetHasStarted() && !GetWasStopped() && (GetBeginOnTick() || timeline->GetBeginTime () <= parentTime)) {
		if (GetBeginOnTick())
			BeginOnTick (false);
		Begin (parentTime);
	}

	if (GetClockState () == Clock::Stopped)
		return;

	if (GetNaturalDuration().HasTimeSpan()) {
		TimeSpan natural_duration_timespan = GetNaturalDuration().GetTimeSpan();
		
		if (natural_duration_timespan <= 0) {
 			localTime = 0;
 			normalizedTime = 1.0;
		}
		else if (natural_duration_timespan > 0) {
			RepeatBehavior *repeat = timeline->GetRepeatBehavior ();

			if (!repeat->IsForever() && localTime >= fillTime) {
				localTime = timeline->GetAutoReverse () ? 0 : fillTime;
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

					if (timeline->GetAutoReverse()) {
						if (ti & 1) {
							if (ti == t)
								localTime = natural_duration_timespan;
							else 
								/* we're descending */
								localTime = (1 - fract) * natural_duration_timespan;
						}
						else {
							if (ti == t)
								localTime = 0;
							else
								/* we're ascending */
								localTime = fract * natural_duration_timespan;
						}
					}
					else {
						if (ti == t)
							localTime = natural_duration_timespan;
						else
							/* we're ascending */
							localTime = fract * natural_duration_timespan;
					}
				}
			}

			normalizedTime = (double)localTime / natural_duration_timespan;

			if (normalizedTime < 0.0) normalizedTime = 0.0;
			if (normalizedTime > 1.0) normalizedTime = 1.0;
		}
	}

	SetCurrentTime (localTime);
	progress = normalizedTime;
}

void
Clock::BeginOnTick (bool begin)
{
	begin_on_tick = begin;
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
		if (state == Clock::Active)
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
// 		printf ("clock %p (%s) completed\n", this, GetTimeline()->GetTypeName());
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
Clock::Begin (TimeSpan parentTime)
{
	emit_completed = false;
	has_completed = false;
	has_started = false;
	was_stopped = false;
	is_paused = false;

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

	SetClockState (Clock::Active);

	// force the time manager to tick the clock hierarchy to wake it up
	time_manager->NeedClockTick ();
}

void
Clock::Pause ()
{
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
	// Start the clock if seeking into it's timespan
	if (!GetHasStarted() && !GetWasStopped() && (GetBeginOnTick() || timeline->GetBeginTime () <= timespan)) {
		if (GetBeginOnTick())
			BeginOnTick (false);
		Begin (timespan); // FIXME
		seek_time = (timespan - timeline->GetBeginTime ()) * timeline->GetSpeedRatio ();
	} else
		seek_time = timespan * timeline->GetSpeedRatio ();
}

void
Clock::SeekAlignedToLastTick (TimeSpan timespan)
{
	// FIXME: this does a synchronous seek before returning
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
// 	printf ("clock %p (%s) reset\n", this, GetTimeline()->GetTypeName());
	calculated_natural_duration = false;
	state = Clock::Stopped;
	progress = 0.0;
	current_time = 0;
	seek_time = 0;
	begin_time = -1;
	begin_on_tick = false;
	is_paused = false;
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

	this->timeline = timeline;
	this->timemanager_clockgroup = timemanager_clockgroup;

	child_clocks = NULL;
	idle_hint = false;
}

void
ClockGroup::OnSurfaceDetach ()
{
	Clock::OnSurfaceDetach ();

	for (GList *l = child_clocks; l; l = l->next) {
		Clock *c = (Clock*)l->data;
		c->OnSurfaceDetach ();
	}
}

void
ClockGroup::OnSurfaceReAttach ()
{
	Clock::OnSurfaceReAttach ();

	for (GList *l = child_clocks; l; l = l->next) {
		Clock *c = (Clock*)l->data;
		c->OnSurfaceReAttach ();
	}
}

void
ClockGroup::AddChild (Clock *clock)
{
	clock->SetRootParentTime (GetCurrentTime ());

	child_clocks = g_list_append (child_clocks, clock);
	clock->ref ();
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
	clock->unref ();
}

void
ClockGroup::Begin (TimeSpan parentTime)
{
	idle_hint = false;
	
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

void
ClockGroup::UpdateFromParentTime (TimeSpan parentTime)
{
	Clock::UpdateFromParentTime (parentTime);

	if (GetClockState() != Clock::Stopped) {
		for (GList *l = child_clocks; l; l = l->next) {
			Clock *clock = (Clock*)l->data;
			clock->UpdateFromParentTime (current_time);
		}
	}
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
	GList *node = child_clocks;
	GList *next;
	
	while (node != NULL) {
		Clock *clock = (Clock *) node->data;
		clock->unref ();

		next = node->next;
		g_list_free_1 (node);
		node = next;
	}
}

