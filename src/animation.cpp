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

/*
 * Magic number to convert a time which is relative to
 * Jan 1, 1970 into a value which is relative to Jan 1, 0001.
 */
#define EPOCH_ADJUST    ((guint64)62135596800LL)

static gint64
get_now ()
{
        struct timeval tv;
        gint64 res;

        if (gettimeofday (&tv, NULL) == 0) {
                res = (((gint64)tv.tv_sec + EPOCH_ADJUST)* 1000000 + tv.tv_usec)*10;
                return res;
        }

	// XXX error
	return 0;
}




Clock::Clock (Timeline *tl)
	: timeline (tl),
	  parent_clock (NULL),
	  child_clocks (NULL)
  //current_state(ClockStopped),
  //is_paused (FALSE)
{
}

Clock::Clock (Timeline *tl, Clock *clock)
	: timeline (tl),
	  parent_clock (clock),
	  child_clocks (NULL)
  //current_state (ClockStopped),
  //is_paused (FALSE)
{
	parent_clock->child_clocks = g_list_prepend (parent_clock->child_clocks, clock);
}

void
Clock::Begin ()
{
	if (parent_clock != NULL) {
		/* XXX error */
	  return;
	}
    
	start_time = get_now();
	EmitEvent ("CurrentTimeInvalidated");

	tick_id = gtk_timeout_add (50 /* something suitably small */, Clock::tick_timeout, this);

	for (GList *l = child_clocks; l; l = l->next) {
		((Clock*)l->data)->Begin();
	}
}

gboolean
Clock::tick_timeout (gpointer data)
{
	((Clock*)data)->Tick ();
	return true;
}

void
Clock::Tick ()
{
	for (GList *l = child_clocks; l; l = l->next) {
		((Clock*)l->data)->Tick();
	}

	EmitEvent ("CurrentTimeInvalidated");
}

void
Clock::Pause ()
{
	if (parent_clock != NULL) {
		/* XXX error */
		return;
	}

	g_source_remove (tick_id);
}

void
Clock::Remove ()
{
}

void
Clock::Resume ()
{
	if (parent_clock != NULL) {
		/* XXX error */
		return;
	}

	tick_id = gtk_timeout_add (50 /* something suitably small */, Clock::tick_timeout, this);
}

void
Clock::Seek (/*Timespan*/gint64 timespan)
{
	if (parent_clock != NULL) {
		/* XXX error */
		return;
	}

	EmitEvent ("CurrentTimeInvalidated");
}

void
Clock::SeekAlignedToLastTick ()
{
	if (parent_clock != NULL) {
		/* XXX error */
		return;
	}

	EmitEvent ("CurrentTimeInvalidated");
}

void
Clock::SkipToFill ()
{
	if (parent_clock != NULL) {
		/* XXX error */
		return;
	}

	EmitEvent ("CurrentTimeInvalidated");
}

void
Clock::Stop ()
{
	if (parent_clock != NULL) {
		/* XXX error */
		return;
	}

	g_source_remove (tick_id);
	EmitEvent ("CurrentTimeInvalidated");
}

double
Clock::GetCurrentProgress ()
{
//   if (current_state == ClockStopped)
//     return 0.0;

	gint64/*TimeSpan*/ current_time = GetCurrentTime ();
	gint64 duration = timeline->GetValue (Timeline::DurationProperty)->u.i64;

	return (double)current_time / duration;
}

gint64
Clock::GetCurrentTime ()
{
	return get_now () - start_time;
}



/* timeline */

DependencyProperty* Timeline::AutoReverseProperty;
DependencyProperty* Timeline::BeginTimeProperty;
DependencyProperty* Timeline::DurationProperty;
DependencyProperty* Timeline::FillBehaviorProperty;
DependencyProperty* Timeline::RepeatBehaviorProperty;
DependencyProperty* Timeline::SpeedRatioProperty;

Timeline::Timeline ()
	: clock (NULL)
{
}

void
Timeline::SetClock (Clock *new_clock)
{
	if (clock != NULL) {
		clock->RemoveEventHandler ("CurrentTimeInvalidated", Timeline::clock_time_changed, this);
		delete clock;
	}

	clock = new_clock;

	if (clock != NULL)
		clock->AddEventHandler ("CurrentTimeInvalidated", Timeline::clock_time_changed, this);
}

void
Timeline::clock_time_changed (gpointer data)
{
	((Timeline*)data)->ClockTimeChanged();
}

/* timeline group */

TimelineGroup::TimelineGroup ()
	: child_timelines (NULL)
{
}

void
TimelineGroup::AddChild (Timeline *child)
{
	child_timelines = g_list_prepend (child_timelines, child);

	child->SetClock (new Clock (child, this->clock));
}

void
TimelineGroup::RemoveChild (Timeline *child)
{
	/* nada for now */
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
}

void
Storyboard::Begin ()
{
	clock->Begin ();
}

void
Storyboard::Pause ()
{
	clock->Pause ();
}

void
Storyboard::Resume ()
{
	clock->Resume ();
}

void
Storyboard::Seek (/*Timespan*/gint64 timespan)
{
	clock->Seek (timespan);
}

void
Storyboard::Stop ()
{
	clock->Stop ();
}

void
Storyboard::ClockTimeChanged ()
{
	for (GList *l = child_timelines; l; l = l->next) {

	  	char *targetProperty = storyboard_child_get_target_property (this, ((Timeline*)l->data));
		if (!targetProperty)
			continue;

		char *targetName = storyboard_child_get_target_name (this, ((Timeline*)l->data));
		if (!targetName)
			continue;

#if notyet
		DependencyObject *o = name_scope.LookupObject (targetName);
		if (!o)
			continue;

		DependencyProperty *prop = o->FindProperty (targetProperty);
		if (!prop)
			continue;

		o->SetValue (prop, child_timeline.GetCurrentValue ());
#endif
	}
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
storyboard_seek (Storyboard *sb, /*Timespan*/gint64 timespan)
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
	o->SetValue (Storyboard::TargetPropertyProperty, targetProperty);
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
	o->SetValue (Storyboard::TargetNameProperty, targetName);
}

char*
storyboard_child_get_target_name (Storyboard *sb,
				  DependencyObject *o)
{
	Value *v = o->GetValue (Storyboard::TargetNameProperty);
	return v == NULL ? NULL : v->u.s;
}


DependencyProperty* DoubleAnimation::ByProperty;
DependencyProperty* DoubleAnimation::FromProperty;
DependencyProperty* DoubleAnimation::ToProperty;

DoubleAnimation::DoubleAnimation ()
{
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
animation_init ()
{
	/* Timeline properties */
	DependencyObject::Register (DependencyObject::TIMELINE, "AutoReverse", new Value (false));
	DependencyObject::Register (DependencyObject::TIMELINE, "BeginTime", new Value (0));
	DependencyObject::Register (DependencyObject::TIMELINE, "Duration", new Value (0));
	//DependencyObject::Register (DependencyObject::TIMELINE, "FillBehavior", new Value (0));
	//DependencyObject::Register (DependencyObject::TIMELINE, "RepeatBehavior", new Value (0));
	//DependencyObject::Register (DependencyObject::TIMELINE, "SpeedRatio", new Value (0));


	/* DoubleAnimation properties */
	DependencyObject::Register (DependencyObject::DOUBLEANIMATION, "By", new Value (0.0));
	DependencyObject::Register (DependencyObject::DOUBLEANIMATION, "From", new Value (0.0));
	DependencyObject::Register (DependencyObject::DOUBLEANIMATION, "To", new Value (0.0));

	/* Storyboard properties */
	DependencyObject::Register (DependencyObject::STORYBOARD, "TargetProperty", NULL);
	DependencyObject::Register (DependencyObject::STORYBOARD, "TargetName", NULL);
	
}
