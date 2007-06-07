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

	RepeatBehavior *repeat = timeline_get_repeat_behavior (timeline);
	if (repeat->HasCount ()) {
		// remaining_iterations is an int.. GetCount returns a double.  badness?
		remaining_iterations = (int)repeat->GetCount ();
	}
	else {
		// XXX treat both Forever and Duration as Forever.
		remaining_iterations = -1;
	}

	autoreverse = timeline_get_autoreverse (timeline);
	duration = timeline_get_duration (timeline);
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
				printf ("current_time=%llu start_time=%llu diff=%llu\n", current_time, start_time, diff);
				printf ("completed_iter=%g autoreverse=%d duration=%llu\n", completed_iterations, autoreverse, duration_timespan);
				printf ("Assigning=%g\n", current_progress);

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
}

void
AnimationStorage::update_property_value (gpointer data)
{
  ((AnimationStorage*)data)->UpdatePropertyValue ();
}

void
AnimationStorage::UpdatePropertyValue ()
{
	Value *current_value = clock->GetCurrentValue (NULL/*XXX*/, NULL/*XXX*/);
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
timeline_set_autoreverse (Timeline *timeline, bool autoreverse)
{
	timeline->SetValue (Timeline::AutoReverseProperty, Value(autoreverse));
}

bool
timeline_get_autoreverse (Timeline *timeline)
{
	return timeline->GetValue (Timeline::AutoReverseProperty)->u.z;
}

void
timeline_set_duration (Timeline *timeline, Duration duration)
{
	timeline->SetValue (Timeline::DurationProperty, Value(duration));
}

Duration*
timeline_get_duration (Timeline *timeline)
{
	return timeline->GetValue (Timeline::DurationProperty)->u.duration;
}

void
timeline_set_repeat_behavior (Timeline *timeline, RepeatBehavior behavior)
{
	timeline->SetValue (Timeline::RepeatBehaviorProperty, Value(behavior));
}

RepeatBehavior *
timeline_get_repeat_behavior (Timeline *timeline)
{
	return timeline->GetValue (Timeline::RepeatBehaviorProperty)->u.repeat;
}



/* timeline group */

TimelineGroup::TimelineGroup ()
	: child_timelines (NULL)
{
}

ClockGroup*
TimelineGroup::CreateClock ()
{
	ClockGroup* group = (ClockGroup*)AllocateClock ();
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

void
timeline_group_add_child (TimelineGroup *group, Timeline *child)
{
	group->AddChild (child);
}

void
timeline_group_remove_child (TimelineGroup *group, Timeline *child)
{
	group->RemoveChild (child);
}



/* storyboard */

DependencyProperty* Storyboard::TargetNameProperty;
DependencyProperty* Storyboard::TargetPropertyProperty;

Storyboard::Storyboard ()
{
	SetObjectType (Value::STORYBOARD);
}

void
Storyboard::HookupAnimationsRecurse (Clock *clock)
{
	switch (clock->GetObjectType ()) {
	case Value::ANIMATIONCLOCK: {
		AnimationClock *ac = (AnimationClock*)clock;

		char *targetProperty = storyboard_child_get_target_property (this, ac->GetTimeline());
		if (!targetProperty)
			return;

		char *targetName = storyboard_child_get_target_name (this, ac->GetTimeline());
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
storyboard_begin (Storyboard *sb)
{
	sb->Begin ();
}

void
storyboard_pause (Storyboard *sb)
{
	sb->Pause ();
}

void
storyboard_resume (Storyboard *sb)
{
	sb->Resume ();
}

void
storyboard_seek (Storyboard *sb, /*Timespan*/guint64 timespan)
{
	sb->Seek (timespan);
}

void
storyboard_stop (Storyboard *sb)
{
	sb->Stop ();
}

void
storyboard_child_set_target_property (Storyboard *sb,
				      DependencyObject *o,
				      char *targetProperty)
{
	o->SetValue (Storyboard::TargetPropertyProperty, Value (targetProperty));
}

char*
storyboard_child_get_target_property (Storyboard *sb,
				      DependencyObject *o)
{
	Value *v = o->GetValue (Storyboard::TargetPropertyProperty);
	return v == NULL ? NULL : v->u.s;
}

void
storyboard_child_set_target_name (Storyboard *sb,
				  DependencyObject *o,
				  char *targetName)
{
	o->SetValue (Storyboard::TargetNameProperty, Value (targetName));
}

char*
storyboard_child_get_target_name (Storyboard *sb,
				  DependencyObject *o)
{
	Value *v = o->GetValue (Storyboard::TargetNameProperty);
	return v == NULL ? NULL : v->u.s;
}


DependencyProperty* BeginStoryboard::StoryboardProperty;

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
	double by = double_animation_get_by (this);
	double from = double_animation_get_from (this);
	double to = double_animation_get_to (this);

	double progress = animationClock->GetCurrentProgress ();

	Value *v = new Value (from + (to-from) * progress);
	// printf ("Sending %g from=%g to=%g progresss=%g\n", v->u.d, from, to, progress);
	return v;
}

DoubleAnimation *
double_animation_new ()
{
	return new DoubleAnimation ();
}

void
double_animation_set_by (DoubleAnimation *da, double by)
{
	da->SetValue (DoubleAnimation::ByProperty, Value(by));
}

double
double_animation_get_by (DoubleAnimation *da)
{
	Value *v = da->GetValue (DoubleAnimation::ByProperty);
	return v == NULL ? 0.0 : v->u.d;
}

void
double_animation_set_from (DoubleAnimation *da, double by)
{
	da->SetValue (DoubleAnimation::FromProperty, Value(by));
}

double
double_animation_get_from (DoubleAnimation *da)
{
	Value *v = da->GetValue (DoubleAnimation::FromProperty);
	return v == NULL ? 0.0 : v->u.d;
}

void
double_animation_set_to (DoubleAnimation *da, double by)
{
	da->SetValue (DoubleAnimation::ToProperty, Value(by));
}

double
double_animation_get_to (DoubleAnimation *da)
{
	Value *v = da->GetValue (DoubleAnimation::ToProperty);
	return v == NULL ? 0.0 : v->u.d;
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
	Color *by = color_animation_get_by (this);
	Color *from = color_animation_get_from (this);
	Color *to = color_animation_get_to (this);

	double progress = animationClock->GetCurrentProgress ();

	Value *v = new Value (Color (from->r + (to->r - from->r) * progress,
				     from->g + (to->g - from->g) * progress,
				     from->b + (to->b - from->b) * progress,
				     from->a + (to->a - from->a) * progress));

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
	DoubleAnimation::ByProperty   = DependencyObject::Register (Value::DOUBLEANIMATION, "By",   new Value (0.0));
	DoubleAnimation::FromProperty = DependencyObject::Register (Value::DOUBLEANIMATION, "From", new Value (0.0));
	DoubleAnimation::ToProperty   = DependencyObject::Register (Value::DOUBLEANIMATION, "To",   new Value (0.0));


	/* ColorAnimation properties */
	ColorAnimation::ByProperty   = DependencyObject::Register (Value::COLORANIMATION, "By",   new Value (Value::COLOR)); // null defaults
	ColorAnimation::FromProperty = DependencyObject::Register (Value::COLORANIMATION, "From", new Value (Value::COLOR));
	ColorAnimation::ToProperty   = DependencyObject::Register (Value::COLORANIMATION, "To",   new Value (Value::COLOR));

	/* Storyboard properties */
	Storyboard::TargetPropertyProperty = DependencyObject::Register (Value::STORYBOARD, "TargetProperty", 
									 Value::STRING);
	Storyboard::TargetNameProperty     = DependencyObject::Register (Value::STORYBOARD, "TargetName", 
									 Value::STRING);

	/* BeginStoryboard properties */
	BeginStoryboard::StoryboardProperty = DependencyObject::Register (Value::BEGINSTORYBOARD, "Storyboard",	Value::STORYBOARD);
}
