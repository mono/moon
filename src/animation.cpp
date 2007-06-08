#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>

//#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
//#endif

#include "animation.h"

#define LERP(f,t,p) ((f) + (p) * ((t) - (f)))

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
	SetObjectType (Value::CLOCK);

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
	SetObjectType (Value::CLOCKGROUP);
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



AnimationStorage::AnimationStorage (AnimationClock *clock, Animation/*Timeline*/ *timeline,
				    DependencyObject *targetobj, DependencyProperty *targetprop)
  : clock (clock),
    timeline (timeline),
    targetobj (targetobj),
    targetprop (targetprop)
  
{
	clock->events->AddHandler ("CurrentTimeInvalidated", update_property_value, this);

	baseValue = targetobj->GetValue (targetprop);
}

void
AnimationStorage::update_property_value (gpointer data)
{
	((AnimationStorage*)data)->UpdatePropertyValue ();
}

void
AnimationStorage::UpdatePropertyValue ()
{
	Value *current_value = clock->GetCurrentValue (baseValue, NULL/*XXX*/);
	targetobj->SetValue (targetprop, *current_value);
}


AnimationClock::AnimationClock (Animation/*Timeline*/ *timeline)
  : timeline(timeline),
    Clock (timeline)
{
	SetObjectType (Value::ANIMATIONCLOCK);
}

void
AnimationClock::HookupStorage (DependencyObject *targetobj, DependencyProperty *targetprop)
{
	storage = new AnimationStorage (this, timeline, targetobj, targetprop);
}

Value*
AnimationClock::GetCurrentValue (Value* defaultOriginValue, Value* defaultDestinationValue)
{
	return timeline->GetCurrentValue (defaultOriginValue, defaultDestinationValue, this);
}



Value*
Animation/*Timeline*/::GetCurrentValue (Value* defaultOriginValue, Value* defaultDestinationValue,
				    AnimationClock* animationClock)
{
	return NULL;
}

// Duration
// AnimationTimeline::GetNaturalDurationCore (Clock* clock)
// {
// }


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



/* storyboard */

DependencyProperty* Storyboard::TargetNameProperty;
DependencyProperty* Storyboard::TargetPropertyProperty;

Storyboard::Storyboard ()
  : root_clock (NULL)
{
	SetObjectType (Value::STORYBOARD);
}

void
Storyboard::HookupAnimationsRecurse (Clock *clock)
{
	switch (clock->GetObjectType ()) {
	case Value::ANIMATIONCLOCK: {
		AnimationClock *ac = (AnimationClock*)clock;

		char *targetProperty = Storyboard::GetTargetProperty (ac->GetTimeline());
		if (!targetProperty)
			return;

		char *targetName = Storyboard::GetTargetName (ac->GetTimeline());
		if (!targetName)
			return;

		printf ("Got %s %s\n", targetProperty, targetName);
		DependencyObject *o = FindName (targetName);
		if (!o)
			return;

		DependencyProperty *prop = o->GetDependencyProperty (targetProperty);
		if (!prop)
			return;


		ac->HookupStorage (o, prop);
		break;
	}
	case Value::CLOCKGROUP: {
		ClockGroup *cg = (ClockGroup*)clock;
		for (GList *l = cg->child_clocks; l; l = l->next)
			HookupAnimationsRecurse ((Clock*)l->data);
		break;
	}
	}
}

void
Storyboard::Begin ()
{
	// we shouldn't begin again, I'd imagine..
	if (root_clock)
		return;

	// This creates the clock tree for the hierarchy.  if a
	// Timeline A is a child of TimelineGroup B, then Clock cA
	// will be a child of ClockGroup cB.
	root_clock = CreateClock ();

	// walk the clock tree hooking up the correct properties and
	// creating AnimationStorage's for AnimationClocks.
	HookupAnimationsRecurse (root_clock);

	// hack to make storyboards work..  we need to attach them to
	// TimeManager's list of clocks
	TimeManager::Instance()->AddChild (root_clock);

	root_clock->Begin (TimeManager::GetCurrentGlobalTime());
}

void
Storyboard::Pause ()
{
	root_clock->Pause ();
}

void
Storyboard::Resume ()
{
	root_clock->Resume ();
}

void
Storyboard::Seek (/*Timespan*/guint64 timespan)
{
	root_clock->Seek (timespan);
}

void
Storyboard::Stop ()
{
	root_clock->Stop ();
}

Storyboard *
storyboard_new ()
{
	return new Storyboard ();
}

void
Storyboard::SetTargetProperty (DependencyObject *o,
			       char *targetProperty)
{
	o->SetValue (Storyboard::TargetPropertyProperty, Value (targetProperty));
}

char*
Storyboard::GetTargetProperty (DependencyObject *o)
{
	Value *v = o->GetValue (Storyboard::TargetPropertyProperty);
	return v == NULL ? NULL : v->u.s;
}

void
Storyboard::SetTargetName (DependencyObject *o,
			   char *targetName)
{
	o->SetValue (Storyboard::TargetNameProperty, Value (targetName));
}

char*
Storyboard::GetTargetName (DependencyObject *o)
{
	Value *v = o->GetValue (Storyboard::TargetNameProperty);
	return v == NULL ? NULL : v->u.s;
}


DependencyProperty* BeginStoryboard::StoryboardProperty;

void
BeginStoryboard::Fire ()
{
	Storyboard *sb = GetStoryboard ();

	g_assert (sb);

	sb->Begin ();
}

void
BeginStoryboard::SetStoryboard (Storyboard *sb)
{
	SetValue (BeginStoryboard::StoryboardProperty, Value (sb));
}

Storyboard *
BeginStoryboard::GetStoryboard ()
{
	return (Storyboard *) GetValue (BeginStoryboard::StoryboardProperty)->u.dependency_object;
}

BeginStoryboard *
begin_storyboard_new ()
{
	return new BeginStoryboard ();
}


DependencyProperty* DoubleAnimation::ByProperty;
DependencyProperty* DoubleAnimation::FromProperty;
DependencyProperty* DoubleAnimation::ToProperty;

DoubleAnimation::DoubleAnimation ()
{
	SetObjectType (Value::DOUBLEANIMATION);
}

Value*
DoubleAnimation::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
				  AnimationClock* animationClock)
{
	// we should cache these in the clock, probably, instead of
	// using the getters at every iteration
	double* from = GetFrom ();
	double* to = GetTo ();
	double* by = GetBy ();

	double start = from ? *from : defaultOriginValue->u.d;
	double end;

	if (to) {
		end = *to;
	}
	else if (by) {
		end = start + *by;
	}
	else {
		end = start;
	}

	double progress = animationClock->GetCurrentProgress ();

	Value *v = new Value (LERP (start, end, progress));
	// printf ("Sending %g from=%g to=%g progresss=%g\n", v->u.d, *from, *to, progress);
	return v;
}

DoubleAnimation *
double_animation_new ()
{
	return new DoubleAnimation ();
}




DependencyProperty* ColorAnimation::ByProperty;
DependencyProperty* ColorAnimation::FromProperty;
DependencyProperty* ColorAnimation::ToProperty;

ColorAnimation::ColorAnimation ()
{
	SetObjectType (Value::COLORANIMATION);
}

Value*
ColorAnimation::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
				 AnimationClock* animationClock)
{
	Color *by = GetBy ();
	Color *from = GetFrom ();
	Color *to = GetTo ();

	Color *start = from ? from : defaultOriginValue->u.color;
	Color *end;

	if (to) {
		end = to;
	}
	else if (by) {
		end = new Color (from->r + by->r,
				 from->g + by->g,
				 from->b + by->b,
				 from->a + by->a);
	}
	else {
		end = start;
	}

	double progress = animationClock->GetCurrentProgress ();

	Value *v = new Value (Color (LERP (from->r, to->r, progress),
				     LERP (from->g, to->g, progress),
				     LERP (from->b, to->b, progress),
				     LERP (from->a, to->a, progress)));

	// printf ("Sending %g from=%g to=%g progresss=%g\n", v->u.d, from, to, progress);
	return v;
}

ColorAnimation *
color_animation_new ()
{
	return new ColorAnimation ();
}

void
color_animation_set_by (ColorAnimation *da, Color by)
{
	da->SetValue (ColorAnimation::ByProperty, Value(by));
}

Color*
color_animation_get_by (ColorAnimation *da)
{
	Value *v = da->GetValue (ColorAnimation::ByProperty);
	return v == NULL ? NULL : v->u.color;
}

void
color_animation_set_from (ColorAnimation *da, Color from)
{
	da->SetValue (ColorAnimation::FromProperty, Value(from));
}

Color*
color_animation_get_from (ColorAnimation *da)
{
	Value *v = da->GetValue (ColorAnimation::FromProperty);
	return v == NULL ? NULL : v->u.color;
}

void
color_animation_set_to (ColorAnimation *da, Color to)
{
	da->SetValue (ColorAnimation::ToProperty, Value(to));
}

Color*
color_animation_get_to (ColorAnimation *da)
{
	Value *v = da->GetValue (ColorAnimation::ToProperty);
	return v == NULL ? NULL : v->u.color;
}





DependencyProperty* PointAnimation::ByProperty;
DependencyProperty* PointAnimation::FromProperty;
DependencyProperty* PointAnimation::ToProperty;

PointAnimation::PointAnimation ()
{
	SetObjectType (Value::POINTANIMATION);
}

Value*
PointAnimation::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
				 AnimationClock* animationClock)
{
	Point *by = GetBy ();
	Point *from = GetFrom ();
	Point *to = GetTo ();

	Point *start = from ? from : defaultOriginValue->u.point;
	Point *end;

	if (to) {
		end = to;
	}
	else if (by) {
		end = new Point (from->x + by->x,
				 from->y + by->y);
	}
	else {
		end = start;
	}

	double progress = animationClock->GetCurrentProgress ();

	Value *v = new Value (Point (LERP (from->x, to->x, progress),
				     LERP (from->y, to->y, progress)));

	return v;
}

PointAnimation *
point_animation_new ()
{
	return new PointAnimation ();
}

void
point_animation_set_by (PointAnimation *da, Point by)
{
	da->SetValue (PointAnimation::ByProperty, Value(by));
}

Point*
point_animation_get_by (PointAnimation *da)
{
	Value *v = da->GetValue (PointAnimation::ByProperty);
	return v == NULL ? NULL : v->u.point;
}

void
point_animation_set_from (PointAnimation *da, Point from)
{
	da->SetValue (PointAnimation::FromProperty, Value(from));
}

Point*
point_animation_get_from (PointAnimation *da)
{
	Value *v = da->GetValue (PointAnimation::FromProperty);
	return v == NULL ? NULL : v->u.point;
}

void
point_animation_set_to (PointAnimation *da, Point to)
{
	da->SetValue (PointAnimation::ToProperty, Value(to));
}

Point*
point_animation_get_to (PointAnimation *da)
{
	Value *v = da->GetValue (PointAnimation::ToProperty);
	return v == NULL ? NULL : v->u.point;
}





RepeatBehavior RepeatBehavior::Forever (RepeatBehavior::FOREVER);
Duration Duration::Automatic (Duration::AUTOMATIC);
Duration Duration::Forever (Duration::FOREVER);

void 
animation_init ()
{
	/* Timeline properties */
	Timeline::AutoReverseProperty = DependencyObject::Register (Value::TIMELINE, "AutoReverse", new Value (false));
	Timeline::BeginTimeProperty = DependencyObject::Register (Value::TIMELINE, "BeginTime", new Value ((guint64)0));
	Timeline::DurationProperty = DependencyObject::Register (Value::TIMELINE, "Duration", new Value (Duration::Automatic));
	Timeline::RepeatBehaviorProperty = DependencyObject::Register (Value::TIMELINE, "RepeatBehavior", new Value (RepeatBehavior ((double)1)));
	//DependencyObject::Register (DependencyObject::TIMELINE, "FillBehavior", new Value (0));
	//DependencyObject::Register (DependencyObject::TIMELINE, "SpeedRatio", new Value (0));


	/* DoubleAnimation properties */
	DoubleAnimation::ByProperty   = DependencyObject::Register (Value::DOUBLEANIMATION, "By",   Value::DOUBLE);
	DoubleAnimation::FromProperty = DependencyObject::Register (Value::DOUBLEANIMATION, "From", Value::DOUBLE);
	DoubleAnimation::ToProperty   = DependencyObject::Register (Value::DOUBLEANIMATION, "To",   Value::DOUBLE);


	/* ColorAnimation properties */
	ColorAnimation::ByProperty   = DependencyObject::Register (Value::COLORANIMATION, "By",   Value::COLOR); // null defaults
	ColorAnimation::FromProperty = DependencyObject::Register (Value::COLORANIMATION, "From", Value::COLOR);
	ColorAnimation::ToProperty   = DependencyObject::Register (Value::COLORANIMATION, "To",   Value::COLOR);

	/* PointAnimation properties */
	PointAnimation::ByProperty   = DependencyObject::Register (Value::POINTANIMATION, "By",   Value::POINT); // null defaults
	PointAnimation::FromProperty = DependencyObject::Register (Value::POINTANIMATION, "From", Value::POINT);
	PointAnimation::ToProperty   = DependencyObject::Register (Value::POINTANIMATION, "To",   Value::POINT);

	/* Storyboard properties */
	Storyboard::TargetPropertyProperty = DependencyObject::Register (Value::STORYBOARD, "TargetProperty", 
									 Value::STRING);
	Storyboard::TargetNameProperty     = DependencyObject::Register (Value::STORYBOARD, "TargetName", 
									 Value::STRING);

	/* BeginStoryboard properties */
	BeginStoryboard::StoryboardProperty = DependencyObject::Register (Value::BEGINSTORYBOARD, "Storyboard",	Value::STORYBOARD);
}


/* The nullable setters/getters for the various animation types */
SET_NULLABLE_FUNC(double)
SET_NULLABLE_FUNC(Color)
SET_NULLABLE_FUNC(Point)

NULLABLE_GETSET_IMPL (DoubleAnimation, By, double, Double, &v->u.d)
NULLABLE_GETSET_IMPL (DoubleAnimation, To, double, Double, &v->u.d)
NULLABLE_GETSET_IMPL (DoubleAnimation, From, double, Double, &v->u.d)

NULLABLE_GETSET_IMPL (ColorAnimation, By, Color, Color, v->u.color)
NULLABLE_GETSET_IMPL (ColorAnimation, To, Color, Color, v->u.color)
NULLABLE_GETSET_IMPL (ColorAnimation, From, Color, Color, v->u.color)

NULLABLE_GETSET_IMPL (PointAnimation, By, Point, Point, v->u.point)
NULLABLE_GETSET_IMPL (PointAnimation, To, Point, Point, v->u.point)
NULLABLE_GETSET_IMPL (PointAnimation, From, Point, Point, v->u.point)
