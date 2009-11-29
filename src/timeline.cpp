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

#include "timeline.h"
#include "timemanager.h"

#include "uielement.h"
#include "runtime.h"
#include "deployment.h"
#include "ptr.h"

/* timeline */

Timeline::Timeline ()
{
	SetObjectType (Type::TIMELINE);

	had_parent = false;
	manual_target = NULL;
	timeline_status = TIMELINE_STATUS_OK;
	clock = NULL;
}

Timeline::~Timeline ()
{
}

Clock*
Timeline::AllocateClock ()
{
	clock = new Clock (this);

	AttachCompletedHandler ();

	return clock;
}

Clock*
Timeline::GetClock ()
{
	return clock;
}

bool
Timeline::Validate ()
{
	RepeatBehavior *repeat = GetRepeatBehavior ();
	Duration *duration = GetDuration ();

	if (duration->HasTimeSpan () && duration->GetTimeSpan () == 0 && 
	    (GetFillBehavior () == FillBehaviorStop || (repeat->HasCount () && repeat->GetCount () > 1.0)))
		timeline_status = TIMELINE_STATUS_DETACHED;

	// FIXME This should prolly be changed to a more generic if BeginTime > Duration
	// Need to investigate, though SL checking seems to be very selective
	if (duration->HasTimeSpan () && duration->GetTimeSpan () == 0 && 
	    this->GetBeginTime () > 0)
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
//  			printf (" + duration (%" G_GINT64_FORMAT " timespan)\n", cd.GetTimeSpan ());
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

TimeSpan
Timeline::GetBeginTime ()
{
	Value *v = GetValue (Timeline::BeginTimeProperty);
	return v == NULL ? 0LL : v->AsTimeSpan();
}

void
Timeline::AttachCompletedHandler ()
{
	clock->AddHandler (Clock::CompletedEvent, clock_completed, this);
}

void
Timeline::DetachCompletedHandler ()
{
	clock->RemoveHandler (Clock::CompletedEvent, clock_completed, this);
}

void
Timeline::clock_completed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	Timeline *timeline = (Timeline *) closure;
	timeline->OnClockCompleted ();
}

void
Timeline::OnClockCompleted ()
{
	Emit (Timeline::CompletedEvent);
}

void
Timeline::TeardownClock ()
{
	if (clock) {
		DetachCompletedHandler ();
		Clock *c = clock;
		ClockGroup *group = c->GetParentClock();
		if (group)
			group->RemoveChild (c);
		clock = NULL;
		c->unref ();
	}
}

/* timeline group */

TimelineGroup::TimelineGroup ()
{
	SetObjectType (Type::TIMELINEGROUP);
}

TimelineGroup::~TimelineGroup ()
{
}

Clock *
TimelineGroup::AllocateClock ()
{
	clock = new ClockGroup (this);
	TimelineCollection *collection = GetChildren ();

	for (int i = 0; i < collection->GetCount (); i++)
		((ClockGroup*)clock)->AddChild (collection->GetValueAt (i)->AsTimeline ()->AllocateClock ());

	AttachCompletedHandler ();
	return clock;
}

// Validate this TimelineGroup by validating all of it's children
bool
TimelineGroup::Validate ()
{
	TimelineCollection *collection = GetChildren ();
	Timeline *timeline;
	
	for (int i = 0; i < collection->GetCount (); i++) {
		timeline = collection->GetValueAt (i)->AsTimeline ();
		if (!timeline->Validate ())
			return false;
	}
	
	return Timeline::Validate ();
}

void
TimelineGroup::AddChild (Timeline *child)
{
	TimelineCollection *children = GetChildren ();
	children->Add (child);
}

void
TimelineGroup::RemoveChild (Timeline *child)
{
	TimelineCollection *children = GetChildren ();
	
	children->Remove (child);
}

void
TimelineGroup::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col == GetChildren()) {
		if (args->GetChangedAction() == CollectionChangedActionAdd ||
		    args->GetChangedAction() == CollectionChangedActionReplace) {
			Timeline *timeline = args->GetNewItem()->AsTimeline ();
			if (timeline)
				timeline->SetHadParent (true);
		}
	}
}


TimelineCollection::TimelineCollection ()
{
	SetObjectType (Type::TIMELINE_COLLECTION);
}

TimelineCollection::~TimelineCollection ()
{
}

ParallelTimeline::ParallelTimeline ()
{
	SetObjectType (Type::PARALLELTIMELINE);
}

ParallelTimeline::~ParallelTimeline ()
{
}

Duration
ParallelTimeline::GetNaturalDurationCore (Clock *clock)
{
	TimelineCollection *collection = GetChildren ();
	Duration d = Duration::Automatic;
	TimeSpan duration_span = 0;
	Timeline *timeline;
	
	if (collection->GetCount () == 0)
		return Duration::FromSeconds (0);
	
	for (int i = 0; i < collection->GetCount (); i++) {
		timeline = collection->GetValueAt (i)->AsTimeline ();
		
		Duration duration = timeline->GetNaturalDuration (clock);
		if (duration.IsAutomatic())
			continue;
		
		if (duration.IsForever())
			return Duration::Forever;
		
		TimeSpan span = duration.GetTimeSpan ();
		
		RepeatBehavior *repeat = timeline->GetRepeatBehavior ();
		if (repeat->IsForever())
			return Duration::Forever;
		
		if (repeat->HasCount ())
			span = (TimeSpan) (span * repeat->GetCount ());
		
		if (timeline->GetAutoReverse ())
			span *= 2;

		// If we have duration-base repeat behavior, 
		// clamp/up our span to that.
		if (repeat->HasDuration ())
			span = repeat->GetDuration ();
		
		if (span != 0)
			span = (TimeSpan)(span / timeline->GetSpeedRatio());

		span += timeline->GetBeginTime ();

		if (duration_span <= span) {
			duration_span = span;
			d = Duration (duration_span);
		}
	}

	return d;
}

TimelineMarker::TimelineMarker ()
{
	SetObjectType (Type::TIMELINEMARKER);
}

TimelineMarker::~TimelineMarker ()
{
}

DispatcherTimer::DispatcherTimer ()
{
	SetObjectType (Type::DISPATCHERTIMER);

	clock = NULL;
	stopped = false;
	started = false;
	ontick = false;
}

void
DispatcherTimer::Start ()
{
	started = true;
	stopped = false;

	Surface *surface = Deployment::GetCurrent ()->GetSurface ();

	if (clock) {
		clock->Reset ();
		clock->BeginOnTick ();
		clock->SetRootParentTime (surface->GetTimeManager()->GetCurrentTime());
	} else {
		AllocateClock ();
		char *name = g_strdup_printf ("DispatcherTimer (%p)", this);
		clock->SetValue (DependencyObject::NameProperty, name);
		g_free (name);

		surface->GetTimeManager()->AddClock (clock);

		clock->BeginOnTick ();
	}
}

void
DispatcherTimer::Stop ()
{
	if (clock)
		clock->Stop ();
	stopped = true;
	started = false;
	if (!ontick)
		Timeline::TeardownClock ();
}

void
DispatcherTimer::Restart ()
{
	started = false;
	stopped = false;
	clock->Reset ();
	TimeSpan time = clock->GetParentClock()->GetCurrentTime();
	clock->SetRootParentTime (time);
	clock->Begin (time);
}

void
DispatcherTimer::OnClockCompleted ()
{
	started = false;
	ontick = true;
	Emit (DispatcherTimer::TickEvent);
	ontick = false;

	/* if the timer wasn't stopped or started on
	   the tick event, restart it. Unlike Start,
	   which makes it go on the next tick, Restart
	   includes the time spent on the tick.

	   if the timer was stopped but not started
	   on the tick event, we don't need to keep
	   the clock around anymore.
	*/
	if (!stopped && !started)
		Restart ();
	else if (stopped && !started)
		Timeline::TeardownClock ();
}

Duration
DispatcherTimer::GetNaturalDurationCore (Clock *clock)
{
	return Duration::FromSeconds (0);
}

void
DispatcherTimer::TeardownClock ()
{
	if (clock) {
		Stop ();
		Timeline::TeardownClock ();
	}
}
