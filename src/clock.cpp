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

	start_time = 0;

	if (timeline->HasBeginTime ())
		begintime = timeline->GetBeginTime ();
	/* otherwise it's filled in when we Begin the clock */
}

Clock::~Clock ()
{
}

TimeSpan
Clock::GetParentTime ()
{
	if (parent_clock && time_manager && parent_clock == time_manager->GetRootClock ())
		return parent_clock->GetCurrentTime () - start_time;

	return parent_clock ? parent_clock->GetCurrentTime() : time_manager ? time_manager->GetCurrentTime () : 0LL;
}

TimeSpan
Clock::GetLastParentTime ()
{
	if (parent_clock && time_manager && parent_clock == time_manager->GetRootClock ())
		return parent_clock->GetLastTime () - start_time;

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
	TimeSpan our_delta = ceil ((double)(TimeSpan)((GetParentTime() - GetLastParentTime()) * speed));
	TimeSpan ret_time = current_time;

	our_delta = (TimeSpan) ceil ((our_delta * timeline->GetSpeedRatio()));

	if (! forward)
		our_delta = - our_delta;

	if (seeking) {
		/* MS starts up animations if you seek them */
		if (state != Clock::Active)
			SetClockState (Clock::Active);

		ret_time = seek_time;
	}
	else {
		if (state == Clock::Stopped)
			return ret_time;

		ret_time = current_time + our_delta;
	}

	/* if our duration is automatic or forever, we're done. */
	if (!natural_duration.HasTimeSpan()) {
		seeking = false;
		return ret_time;
	}

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
				int repeated_count = (duration_timespan != 0) ? ret_time / duration_timespan : 1;
				if (repeated_count % 2 == 1) {
					forward = false;
					ret_time = (duration_timespan * repeated_count) - (ret_time - duration_timespan);
				} else {
					forward = true;
					ret_time = ret_time % duration_timespan;
				}
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
	if (repeat_time >= 0 && repeat_time <= ret_time) {
		ret_time = repeat_time;
		SkipToFill ();
	}

	seeking = false;
	return ret_time;

#if 0
	// XXX I think we only need to check repeat_count in
	// the forward direction.
	if (forward) {
		printf ("HERE!\n");
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
Clock::BeginOnTick (bool begin)
{
	begin_on_tick = begin;
	start_time = GetParentTime ();
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
Clock::RaiseAccumulatedCompleted ()
{
}

void
Clock::Begin ()
{
	seeking = false;
	has_started = false;
	was_stopped = false;
	is_paused = false;
	forward = true;

	if (start_time == 0)
		start_time = GetParentTime ();

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
		repeat_time = (repeat->GetDuration() * timeline->GetSpeedRatio ());
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
	// Start the clock if seeking into it's timespan
	if (!GetHasStarted() && !GetWasStopped() && (GetBeginOnTick() || GetBeginTime () <= timespan)) {
		if (GetBeginOnTick()) {
			BeginOnTick (false);
			ComputeBeginTime ();
		}
		Begin ();
		seek_time = (timespan - GetBeginTime ()) * timeline->GetSpeedRatio ();
	} else
		seek_time = timespan * timeline->GetSpeedRatio ();

	seeking = true;
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

void
Clock::Reset ()
{
	calculated_natural_duration = false;
	state = Clock::Stopped;
	progress = 0.0;
	current_time = 0;
	last_time = 0;
	seeking = false;
	seek_time = 0;
	begin_time = -1;
	begin_on_tick = false;
	forward = true;
	start_time = 0;
	is_paused = false;
	has_started = false;
	was_stopped = false;
	GetNaturalDuration (); // ugh
}

void
Clock::Completed ()
{
}

ClockGroup::ClockGroup (TimelineGroup *timeline, bool never_f)
  : Clock (timeline)
{
	SetObjectType (Type::CLOCKGROUP);

	child_clocks = NULL;
	this->timeline = timeline;
	emit_completed = false;
	idle_hint = false;
	never_fill = never_f;
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
	clock->GetNaturalDuration(); // ugh

	child_clocks = g_list_append (child_clocks, clock);
	clock->ref ();
	clock->SetParentClock (this);
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
	clock->SetParentClock (NULL);
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
	if (GetParentClock () && GetParentClock () != GetTimeManager ()->GetRootClock ())
		begin_time = (timeline->HasBeginTime() ? timeline->GetBeginTime() : 0);
	else
		begin_time = GetParentTime () + (timeline->HasBeginTime() ? timeline->GetBeginTime() : 0);
 
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
			idle_hint = true & RUNTIME_INIT_USE_IDLE_HINT;
			if (! never_fill)
				SkipToFill ();
		} else
			DoRepeat (current_time);
	}

	if (state == Clock::Stopped || (idle_hint == true && (moonlight_flags & RUNTIME_INIT_USE_IDLE_HINT)))
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
		if (! seeking)
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
}

void
ClockGroup::RaiseAccumulatedCompleted ()
{
	Clock::RaiseAccumulatedCompleted ();
	
	clock_list_foreach (child_clocks, CallRaiseAccumulatedCompleted);
	
	if (emit_completed && (state == Clock::Stopped || state == Clock::Filling)) {
		emit_completed = false;
		Emit (CompletedEvent);
	}
}

void
ClockGroup::Reset ()
{
	Clock::Reset ();
	emit_completed = false;
}

void
ClockGroup::Completed ()
{
	emit_completed = true;
	start_time = 0;
}

ClockGroup::~ClockGroup ()
{
	GList *node = child_clocks;
	GList *next;
	
	while (node != NULL) {
		Clock *clock = (Clock *) node->data;
		clock->SetParentClock (NULL);
		clock->unref ();

		next = node->next;
		g_list_free_1 (node);
		node = next;
	}
}

