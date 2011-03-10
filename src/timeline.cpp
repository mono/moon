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

namespace Moonlight {

/* timeline */

Timeline::Timeline ()
	: DependencyObject (Type::TIMELINE), manual_target (this)
{
	had_parent = false;
	timeline_status = TIMELINE_STATUS_OK;
	clock = NULL;
}

Timeline::~Timeline ()
{
}

Clock*
Timeline::AllocateClock ()
{
	Clock *clock = new Clock (this);
	SetClock (clock);
	clock->unref ();

	AttachCompletedHandler ();

	return GetClock ();
}

Clock*
Timeline::GetClock ()
{
	return clock;
}

bool
Timeline::Validate (MoonError *error)
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
Timeline::SetManualTargetWithError (DependencyObject *o, MoonError *error)
{
	if (ThrowIfRunning (error))
		return;

	manual_target = o;
}


bool
Timeline::ThrowIfRunning (MoonError *error)
{
	// If a Storyboard has a clock allocated against it, then it's definitely active
	if (GetClock()) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Operation is not valid on an active Animation or Storyboard.  Root Storyboard must be stopped first.");
		return true;
	}
	return false;
}

void
Timeline::AttachCompletedHandler ()
{
	GetClock ()->AddHandler (Clock::CompletedEvent, clock_completed, this);
}

void
Timeline::DetachCompletedHandler ()
{
	GetClock ()->RemoveHandler (Clock::CompletedEvent, clock_completed, this);
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
Timeline::ClearClock ()
{
	if (clock) {
		DetachCompletedHandler ();
		clock->unref (); /* We may recurse a lot, don't recurse more than necessary */
		//clock->unref_delayed (); /* We may recurse a lot, don't recurse more than necessary */
		clock = NULL;
	}
}

void
Timeline::SetClock (Clock *value)
{
#if SANITY
	g_assert (value != NULL); /* #if SANITY */
	g_assert (clock == NULL); /* #if SANITY */
#endif
	clock = value;
	clock->ref ();
}

/* timeline group */

TimelineGroup::TimelineGroup ()
{
	SetObjectType (Type::TIMELINEGROUP);
}

TimelineGroup::~TimelineGroup ()
{
}

void
TimelineGroup::ClearClock ()
{
	TimelineCollection *collection = GetChildren ();
	int count = collection->GetCount ();

	for (int i = 0; i < count; i++)
		collection->GetValueAt (i)->AsTimeline ()->ClearClock ();

	Timeline::ClearClock ();
}

Clock *
TimelineGroup::AllocateClock ()
{
	ClockGroup *clock_group = new ClockGroup (this);
	TimelineCollection *collection = GetChildren ();

	SetClock (clock_group);

	int count = collection->GetCount ();
	for (int i = 0; i < count; i++)
		clock_group->AddChild (collection->GetValueAt (i)->AsTimeline ()->AllocateClock ());
	clock_group->unref ();

	AttachCompletedHandler ();
	return GetClock ();
}

// Validate this TimelineGroup by validating all of it's children
bool
TimelineGroup::Validate (MoonError *error)
{
	TimelineCollection *collection = GetChildren ();
	Timeline *timeline;
	
	int count = collection->GetCount ();
	for (int i = 0; i < count; i++) {
		timeline = collection->GetValueAt (i)->AsTimeline ();
		if (!timeline->Validate (error))
			return false;
	}
	
	return Timeline::Validate (error);
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
	if (PropertyHasValueNoAutoCreate (TimelineGroup::ChildrenProperty, col)) {
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

bool
TimelineCollection::InsertWithError (int index, Value *value, MoonError *error)
{
	if (ThrowIfParentIsRunning (error))
		return false;
	return DependencyObjectCollection::InsertWithError (index, value, error);
}

bool
TimelineCollection::SetValueAtWithError (int index, Value *value, MoonError *error)
{
	if (ThrowIfParentIsRunning (error))
		return false;
	return DependencyObjectCollection::SetValueAtWithError (index, value, error);
}

bool
TimelineCollection::RemoveAtWithError (int index, MoonError *error)
{
	if (ThrowIfParentIsRunning (error))
		return false;
	return DependencyObjectCollection::RemoveAtWithError (index, error);
}

bool
TimelineCollection::ThrowIfParentIsRunning (MoonError *error)
{
	if (GetParent () && GetParent ()->Is (Type::TIMELINE)) {
		Timeline *timeline = (Timeline *) GetParent ();
		if (timeline->ThrowIfRunning (error))
			return true;
	}
	return false;
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
	
	int count = collection->GetCount ();

	if (count == 0)
		return Duration::FromSeconds (0);
	
	for (int i = 0; i < count; i++) {
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
	: DependencyObject (Type::TIMELINEMARKER)
{
}

TimelineMarker::~TimelineMarker ()
{
}

void
Timeline::Dispose ()
{
	DependencyObject::Dispose ();

	if (clock)
		clock->unref ();
	clock = NULL;
}

DispatcherTimer::DispatcherTimer ()
{
	SetObjectType (Type::DISPATCHERTIMER);

	stopped = false;
	started = false;
	ontick = false;
}

void
DispatcherTimer::Start ()
{
	Clock *clock;
	started = true;
	stopped = false;

	Surface *surface = GetDeployment ()->GetSurface ();

	clock = GetClock ();
	if (clock) {
		clock->Reset ();
		clock->BeginOnTick ();
		clock->SetRootParentTime (surface->GetTimeManager()->GetCurrentTime());
	} else {
		clock = AllocateClock ();

		char *name = g_strdup_printf ("DispatcherTimer (%p)", this);
		clock->SetName (name);

		surface->GetTimeManager()->AddClock (clock);

		clock->BeginOnTick ();

		// we unref ourselves when we're stopped, but we have
		// to force ourselves to remain alive while the timer
		// is active
		GetDeployment ()->SetKeepAlive (this, true);
	}
}

void
DispatcherTimer::Stop ()
{
	Clock *clock = GetClock ();
	if (clock)
		clock->Stop ();
	stopped = true;
	started = false;
	if (!ontick) {
		ClearClock ();
		GetDeployment ()->SetKeepAlive (this, false);
	}
}

void
DispatcherTimer::Restart ()
{
	Clock *clock = GetClock ();
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
	else if (stopped && !started) {
		ClearClock ();
		GetDeployment ()->SetKeepAlive (this, false);
	}
}

Duration
DispatcherTimer::GetNaturalDurationCore (Clock *clock)
{
	return Duration::FromSeconds (0);
}

};
