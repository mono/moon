/*
 * animation.cpp: Animation engine
 *
 * Author:
 *   Chris Toshok (toshok@novell.com)
 *   Michael Dominic K. <mdk@mdk.am>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <math.h>

#include "animation.h"
#include "color.h"
#include "runtime.h"
#include "utils.h"

#define LERP(f,t,p) ((f) + ((t) - (f)) * (p))




AnimationStorage::AnimationStorage (AnimationClock *clock, Animation/*Timeline*/ *timeline,
				    DependencyObject *targetobj, DependencyProperty *targetprop)
{
	this->nonResetableFlag = false;
	this->floating = false;
	this->clock = clock;
	this->timeline = timeline;
	this->targetobj = targetobj;
	this->targetprop = targetprop;

	clock->AddHandler (clock->CurrentTimeInvalidatedEvent, update_property_value, this);
	targetobj->AddHandler (EventObject::DestroyedEvent, target_object_destroyed, this);

	AnimationStorage *prev_storage = targetprop->AttachAnimationStorage (targetobj, this);

	baseValue = new Value(*targetobj->GetValue (targetprop));

	if (prev_storage) {
		Value *v = prev_storage->GetResetValue ();
		stopValue = new Value (*v);
		prev_storage->FlagAsNonResetable ();
		if (prev_storage->IsFloating ())
			delete prev_storage;
	} else {
		stopValue = NULL;
	}
}

void
AnimationStorage::target_object_destroyed (EventObject *, EventArgs *, gpointer closure)
{
	((AnimationStorage*)closure)->TargetObjectDestroyed ();
}

void
AnimationStorage::TargetObjectDestroyed ()
{
	if (floating)
		return;

	targetprop->DetachAnimationStorage (targetobj, this);
	targetobj = NULL;
	DetachUpdateHandler ();
}

void
AnimationStorage::FlagAsNonResetable ()
{
	nonResetableFlag = true;
}

bool
AnimationStorage::IsCurrentStorage ()
{
	if (targetobj == NULL || targetprop == NULL)
		return false;

	if (targetprop->GetAnimationStorageFor (targetobj) == this)
		return true;

	return false;
}

Value*
AnimationStorage::GetStopValue ()
{
	if (stopValue)
		return stopValue;

	return baseValue;
}

void
AnimationStorage::update_property_value (EventObject *, EventArgs *, gpointer closure)
{
	((AnimationStorage*)closure)->UpdatePropertyValue ();
}

void
AnimationStorage::UpdatePropertyValueWith (Value *v)
{
	if (targetobj == NULL)
		return;

	if (v != NULL)
		targetobj->SetValue (targetprop, *v);
}

void
AnimationStorage::UpdatePropertyValue ()
{
	if (targetobj == NULL)
		return;

	Value *current_value = clock->GetCurrentValue (baseValue, NULL/*XXX*/);
	if (current_value != NULL)
		targetobj->SetValue (targetprop, *current_value);
	//else
	//	targetobj->SetValue (targetprop, NULL);
		
	delete current_value;
}

void
AnimationStorage::ResetPropertyValue ()
{
	if (nonResetableFlag)
		return;

	if (targetobj == NULL)
		return;

	if (stopValue)
		targetobj->SetValue (targetprop, *stopValue);
	else
		targetobj->SetValue (targetprop, *baseValue);
}

void
AnimationStorage::DetachTarget ()
{
	DetachUpdateHandler ();
	if (targetobj) {
		targetobj->RemoveHandler (EventObject::DestroyedEvent, target_object_destroyed, this);
		targetobj = NULL;
	}
}

void
AnimationStorage::DetachUpdateHandler ()
{
	if (clock != NULL) {
		clock->RemoveHandler (clock->CurrentTimeInvalidatedEvent, update_property_value, this);
	}
}

void
AnimationStorage::Float ()
{
	DetachUpdateHandler ();
	DetachTarget ();

	clock = NULL;
	timeline = NULL;
	targetprop = NULL;
}

Value*
AnimationStorage::GetResetValue ()
{
	if (stopValue)
		return stopValue;
	else
		return baseValue;
}

AnimationStorage::~AnimationStorage ()
{
	if (baseValue)
		delete baseValue;

	if (stopValue)
		delete stopValue;

	DetachUpdateHandler ();
	
	if (targetobj != NULL) {
		targetobj->RemoveHandler (EventObject::DestroyedEvent, target_object_destroyed, this);
		targetprop->DetachAnimationStorage (targetobj, this);
	}
}

AnimationClock::AnimationClock (Animation/*Timeline*/ *timeline)
  : Clock (timeline)
{
	this->timeline = timeline;
	storage = NULL;
}

bool
AnimationClock::HookupStorage (DependencyObject *targetobj, DependencyProperty *targetprop)
{
	/* Before hooking up make sure that the values our animation generates
	   (doubles, colors, points...) match the values that the property is
	   ready to receive. If not, print an informative message. */
	if (timeline->GetValueKind () != targetprop->value_type) {
		Type *timeline_type = Type::Find (timeline->GetValueKind ());
		Type *property_type = Type::Find (targetprop->value_type);

		const char *timeline_type_name = (timeline_type != NULL) ? timeline_type->GetName () : "Invalid";
		const char *property_type_name = (property_type != NULL) ? property_type->GetName () : "Invalid";
		g_warning ("%s.%s property value type is '%s' but animation type is '%s'.",
			   targetobj->GetTypeName (), targetprop->name,
			   property_type_name, timeline_type_name);

		return false;
	}

	storage = new AnimationStorage (this, timeline, targetobj, targetprop);
	return true;
}

void
AnimationClock::ExtraRepeatAction ()
{
	if (storage) {
		Value *v = timeline->GetTargetValue (storage->GetStopValue ());
		if (v) {
			storage->UpdatePropertyValueWith (v);
			delete v;
		}
	}
}

Value*
AnimationClock::GetCurrentValue (Value* defaultOriginValue, Value* defaultDestinationValue)
{
	return timeline->GetCurrentValue (defaultOriginValue, defaultDestinationValue, this);
}

void
AnimationClock::Stop ()
{
	if (storage) {
		storage->ResetPropertyValue ();
		storage->DetachUpdateHandler ();
	}

	Clock::Stop ();
}

AnimationClock::~AnimationClock ()
{
	if (storage) {
		if (state == Clock::Stopped)
			delete storage;
		else {
			if (storage->IsCurrentStorage ())
				storage->Float ();
			else
				delete storage;
		}
	}
}

Clock*
Animation/*Timeline*/::AllocateClock()
{
	Clock *clock = new AnimationClock (this);
	char *name = g_strdup_printf ("AnimationClock for %s, targetobj = %p/%s, targetprop = %s", GetTypeName(),
				      Storyboard::GetTargetName(this) == NULL ? NULL : FindName (Storyboard::GetTargetName(this)),
				      Storyboard::GetTargetName(this),
				      Storyboard::GetTargetProperty (this));
	clock->SetValue (DependencyObject::NameProperty, name);
	g_free (name);
	return clock;
}

Value*
Animation/*Timeline*/::GetTargetValue (Value* defaultOriginValue)
{
	return NULL;
}

Value*
Animation/*Timeline*/::GetCurrentValue (Value* defaultOriginValue, Value* defaultDestinationValue,
					AnimationClock* animationClock)
{
	return NULL;
}


Duration
Animation/*Timeline*/::GetNaturalDurationCore (Clock* clock)
{
	return Duration::FromSeconds (1);
}




/* storyboard */

DependencyProperty* Storyboard::TargetNameProperty;
DependencyProperty* Storyboard::TargetPropertyProperty;

Storyboard::Storyboard ()
{
	root_clock = NULL;
}

void
Storyboard::HookupAnimationsRecurse (Clock *clock)
{
	switch (clock->GetObjectType ()) {
	case Type::ANIMATIONCLOCK: {
		AnimationClock *ac = (AnimationClock*)clock;

		char *targetProperty = NULL;
		for (Clock *c = ac; c; c = c->GetParent()) {
			targetProperty = Storyboard::GetTargetProperty (c->GetTimeline());
			if (targetProperty)
				break;
		}
		if (!targetProperty) {
			g_warning ("No target property!");
			return;
		}

		char *targetName = NULL;
		for (Clock *c = ac; c; c = c->GetParent()) {
			targetName = Storyboard::GetTargetName (c->GetTimeline());
			if (targetName)
				break;
		}
		if (!targetName) {
			g_warning ("No target name!");
			return;
		}

		//printf ("Got %s %s\n", targetProperty, targetName);
		DependencyObject *o = FindName (targetName);
		if (!o) {
			g_warning ("No object named %s!", targetName);
			return;
		}

		DependencyObject *real_target_o = o;
		DependencyProperty *prop = resolve_property_path (&real_target_o, targetProperty);

		if (!prop || !real_target_o) {
			g_warning ("No property named %s on object %s, which has type %s!", targetProperty, targetName, o->GetTypeName());
			return;
		}

		((Animation*)ac->GetTimeline())->Resolve ();

		if (! ac->HookupStorage (real_target_o, prop))
			return;

		break;
	}
	case Type::CLOCKGROUP: {
		ClockGroup *cg = (ClockGroup*)clock;
		for (GList *l = cg->child_clocks; l; l = l->next)
			HookupAnimationsRecurse ((Clock*)l->data);
		break;
	}
	default:
		g_warning ("Invalid object type (%d) for the specified clock", clock->GetObjectType ());
		break;
	}
}

void
Storyboard::TeardownClockGroup ()
{
	if (root_clock) {
		ClockGroup *group = root_clock->GetParent();
		if (group)
			group->RemoveChild (root_clock);
		root_clock->unref ();
		root_clock = NULL;
	}
}

void
Storyboard::storyboard_completed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	Storyboard *sb = (Storyboard *) closure;
	
	// Only teardown the clocks if the whole storyboard is stopped.
	// Otherwise just remove from parent not be affected by it's 
	// state changes
	if (sb->root_clock->GetClockState () == Clock::Stopped)
		sb->TeardownClockGroup ();
	
	sb->Emit (sb->CompletedEvent);
}

bool
Storyboard::Begin ()
{
	ClockGroup *group = NULL;
	
#if false
	if (root_clock && root_clock->GetClockState() == Clock::Stopped) {
		group->ComputeBeginTime ();
		root_clock->Begin();
		if (root_clock->GetParent()->GetClockState() != Clock::Active) {
			root_clock->GetParent()->Begin();
		}
		return false;
	}
#else
	/* destroy the clock hierarchy and recreate it to restart.
	   easier than making Begin work again with the existing clock
	   hierarchy */
	if (root_clock)
		TeardownClockGroup ();
#endif

	if (Validate () == false)
		return false;

	if (!group) {
		if (GetSurface() == NULL) {
			g_warning ("unable to find surface to add storyboard clock to.");
			return false;
		}
		group = GetSurface()->GetTimeManager()->GetRootClock();
	}

	// This creates the clock tree for the hierarchy.  if a
	// Timeline A is a child of TimelineGroup B, then Clock cA
	// will be a child of ClockGroup cB.
	root_clock = AllocateClock ();
	char *name = g_strdup_printf ("Storyboard, named '%s'", GetName());
	root_clock->SetValue (DependencyObject::NameProperty, name);
	g_free (name);
	root_clock->AddHandler (root_clock->CompletedEvent, storyboard_completed, this);

	// walk the clock tree hooking up the correct properties and
	// creating AnimationStorage's for AnimationClocks.
	HookupAnimationsRecurse (root_clock);

	group->ComputeBeginTime ();

	group->AddChild (root_clock);

	if (HasBeginTime ())
		root_clock->ComputeBeginTime ();
	else
		root_clock->BeginOnTick ();

	// we delay starting the surface's ClockGroup until the first
	// child has been added.  otherwise we run into timing issues
	// between timelines that explicitly set a BeginTime and those
	// that don't (and so default to 00:00:00).
	if (group->GetClockState() != Clock::Active) {
		group->Begin ();
	}

	return true;
}

void
Storyboard::Pause ()
{
	if (root_clock)
		root_clock->Pause ();
}

void
Storyboard::Resume ()
{
	if (root_clock)
		root_clock->Resume ();
}

void
Storyboard::Seek (TimeSpan timespan)
{
	if (root_clock)
		root_clock->Seek (timespan);
}

void
Storyboard::Stop ()
{
	if (root_clock) {
		root_clock->RemoveHandler (root_clock->CompletedEvent, storyboard_completed, this);
		root_clock->Stop ();
		TeardownClockGroup ();
	}
}

Storyboard *
storyboard_new (void)
{
	return new Storyboard ();
}

void
storyboard_begin  (Storyboard *sb)
{
	sb->Begin ();
}

void
storyboard_pause  (Storyboard *sb)
{
	sb->Pause ();
}

void
storyboard_resume (Storyboard *sb)
{
	sb->Resume ();
}

void
storyboard_seek   (Storyboard *sb, TimeSpan ts)
{
	sb->Seek (ts);
}

void
storyboard_stop   (Storyboard *sb)
{
	sb->Stop ();
}


void
Storyboard::SetTargetProperty (DependencyObject *o,
			       const char *targetProperty)
{
	o->SetValue (Storyboard::TargetPropertyProperty, Value (targetProperty));
}

char*
Storyboard::GetTargetProperty (DependencyObject *o)
{
	Value *v = o->GetValue (Storyboard::TargetPropertyProperty);
	return v == NULL ? NULL : v->AsString();
}

void
Storyboard::SetTargetName (DependencyObject *o,
			   const char *targetName)
{
	o->SetValue (Storyboard::TargetNameProperty, Value (targetName));
}

char*
Storyboard::GetTargetName (DependencyObject *o)
{
	Value *v = o->GetValue (Storyboard::TargetNameProperty);
	return v == NULL ? NULL : v->AsString();
}

void
Storyboard::SetSurface (Surface *surface)
{
	if (GetSurface() && surface == NULL && root_clock && root_clock->GetClockState() == Clock::Active) {
		/* we're being detached from a surface, so pause clock */
		Pause ();
	}
 	else if (!GetSurface() && surface) {
		/* we're being (re-)attached to a surface, so resume clock */
		if (root_clock && root_clock->GetIsPaused() && GetLogicalParent()) {
			Resume ();
		}
 	}
	DependencyObject::SetSurface (surface);
}

Storyboard::~Storyboard ()
{
	if (root_clock) {
		//printf ("Clock %p (ref=%d)\n", root_clock, root_clock->refcount);
		Stop ();
		TeardownClockGroup ();
	}
}

DependencyProperty* BeginStoryboard::StoryboardProperty;

void
BeginStoryboard::Fire ()
{
	Storyboard *sb = GetStoryboard ();
	if (sb)
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
	Value *v = GetValue (BeginStoryboard::StoryboardProperty);
	return v ? v->AsStoryboard () : NULL;
}

BeginStoryboard::~BeginStoryboard ()
{
}

BeginStoryboard *
begin_storyboard_new (void)
{
	return new BeginStoryboard ();
}


DependencyProperty* DoubleAnimation::ByProperty;
DependencyProperty* DoubleAnimation::FromProperty;
DependencyProperty* DoubleAnimation::ToProperty;

DoubleAnimation::DoubleAnimation ()
{
}

Value*
DoubleAnimation::GetTargetValue (Value *defaultOriginValue)
{
	double *by = GetBy ();
	double *from = GetFrom ();
	double *to = GetTo ();
	double start = from ? *from : defaultOriginValue->AsDouble();

	if (to)
		return new Value (*to);
	else if (by) 
		return new Value (start + *by);
	else
		return new Value (defaultOriginValue->AsDouble ());
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

	double start = from ? *from : defaultOriginValue->AsDouble();
	double end;

	if (to) {
		end = *to;
	}
	else if (by) {
		end = start + *by;
	}
	else {
		end = defaultOriginValue->AsDouble();
	}

	double progress = animationClock->GetCurrentProgress ();

	return new Value (LERP (start, end, progress));
}

DoubleAnimation *
double_animation_new (void)
{
	return new DoubleAnimation ();
}




DependencyProperty* ColorAnimation::ByProperty;
DependencyProperty* ColorAnimation::FromProperty;
DependencyProperty* ColorAnimation::ToProperty;

ColorAnimation::ColorAnimation ()
{
}

Value*
ColorAnimation::GetTargetValue (Value *defaultOriginValue)
{
	Color *by = GetBy ();
	Color *from = GetFrom ();
	Color *to = GetTo ();
	Color start = from ? *from : *defaultOriginValue->AsColor();

	if (to)
		return new Value (*to);
	else if (by) 
		return new Value (start + *by);
	else
		return new Value (*defaultOriginValue->AsColor ());
}

Value*
ColorAnimation::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
				 AnimationClock* animationClock)
{
	Color *by = GetBy ();
	Color *from = GetFrom ();
	Color *to = GetTo ();

	Color start = from ? *from : *defaultOriginValue->AsColor();
	Color end;

	if (to) {
		end = *to;
	}
	else if (by) {
		end = start + *by;
	}
	else {
		end = *defaultOriginValue->AsColor ();
	}

	double progress = animationClock->GetCurrentProgress ();

	return new Value (LERP (start, end, progress));
}

ColorAnimation *
color_animation_new (void)
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
	return v == NULL ? NULL : v->AsColor();
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
	return v == NULL ? NULL : v->AsColor();
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
	return v == NULL ? NULL : v->AsColor();
}





DependencyProperty* PointAnimation::ByProperty;
DependencyProperty* PointAnimation::FromProperty;
DependencyProperty* PointAnimation::ToProperty;

Value*
PointAnimation::GetTargetValue (Value *defaultOriginValue)
{
	Point *by = GetBy ();
	Point *from = GetFrom ();
	Point *to = GetTo ();
	Point start = from ? *from : *defaultOriginValue->AsPoint();

	if (to)
		return new Value (*to);
	else if (by) 
		return new Value (start + *by);
	else
		return new Value (*defaultOriginValue->AsPoint ());
}

Value*
PointAnimation::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
				 AnimationClock* animationClock)
{
	Point *by = GetBy ();
	Point *from = GetFrom ();
	Point *to = GetTo ();

	Point start = from ? *from : *defaultOriginValue->AsPoint();
	Point end;

	if (to) {
		end = *to;
	}
	else if (by) {
		end = start + *by;
	}
	else {
		end = *defaultOriginValue->AsPoint ();
	}

	double progress = animationClock->GetCurrentProgress ();

	return new Value (LERP (start, end, progress));
}

PointAnimation *
point_animation_new (void)
{
	return new PointAnimation ();
}

static void
regenerate_quadratics_array (Point c1, Point c2, moon_quadratic *qarr)
{
	moon_cubic src;
	src.c0.x = 0; src.c0.y = 0;
	src.c1.x = c1.x; src.c1.y = c1.y;
	src.c2.x = c2.x; src.c2.y = c2.y;
	src.c3.x  = 1.0; src.c3.y = 1.0;

	moon_cubic carr [16];
	
	moon_subdivide_cubic_at_level (carr, 4, &src);
	moon_convert_cubics_to_quadratics (qarr, carr, 16);
}

KeySpline::KeySpline ()
{
	controlPoint1 = Point (0.0, 0.0);
	controlPoint2 = Point (1.0, 1.0);
	regenerate_quadratics_array (controlPoint1, controlPoint2, quadraticsArray);
}

KeySpline::KeySpline (Point controlPoint1, Point controlPoint2)
{
	this->controlPoint1 = controlPoint1;
	this->controlPoint2 = controlPoint2;
	regenerate_quadratics_array (controlPoint1, controlPoint2, quadraticsArray);
}

KeySpline::KeySpline (double x1, double y1,
		      double x2, double y2)
{
	this->controlPoint1 = Point (x1, y1);
	this->controlPoint2 = Point (x2, y2);
	regenerate_quadratics_array (controlPoint1, controlPoint2, quadraticsArray);
}

Point
KeySpline::GetControlPoint1 ()
{
	return controlPoint1;
}

void
KeySpline::SetControlPoint1 (Point controlPoint1)
{
	this->controlPoint1 = controlPoint1;
	regenerate_quadratics_array (controlPoint1, controlPoint2, quadraticsArray);
}

Point
KeySpline::GetControlPoint2 ()
{
	return controlPoint2;
}

void
KeySpline::SetControlPoint2 (Point controlPoint2)
{
	this->controlPoint2 = controlPoint2;
	regenerate_quadratics_array (controlPoint1, controlPoint2, quadraticsArray);
}

KeySpline *
key_spline_new (void)
{
	return new KeySpline ();
}

double
KeySpline::GetSplineProgress (double linearProgress)
{
	if (linearProgress >= 1.0)
		return 1.0;

	if (linearProgress <= 0.0)
		return 0.0;

	double v =  moon_quadratic_array_y_for_x (quadraticsArray, linearProgress);
	return v;
}

DependencyProperty* KeyFrame::KeyTimeProperty;

KeyFrame::KeyFrame ()
{
}

KeyTime*
KeyFrame::GetKeyTime()
{
	Value *v = GetValue (KeyFrame::KeyTimeProperty);
	if (v == NULL)
		return NULL;
	else
		return v->AsKeyTime();
}

void
KeyFrame::SetKeyTime (KeyTime keytime)
{
	SetValue (KeyFrame::KeyTimeProperty, Value(keytime));
}

Value *
KeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	g_warning ("KeyFrame::InterpolateValue has been called. The derived class %s should have overridden it.",
		   GetName ());
	return NULL;
}

KeyFrame*
key_frame_new (void)
{
	return new KeyFrame ();
}


static int
KeyFrameComparer (gconstpointer kf1, gconstpointer kf2)
{
	// Assumes timespan keytimes only
	TimeSpan ts1 = (*(KeyFrame **) kf1)->resolved_keytime;
	TimeSpan ts2 = (*(KeyFrame **) kf2)->resolved_keytime;
	TimeSpan tsdiff = ts1 - ts2;
	
	if (tsdiff == 0)
		return 0;
	else if (tsdiff < 0)
		return -1;
	else
		return 1;
}

KeyFrameCollection::KeyFrameCollection ()
{
	sorted_list = g_ptr_array_new ();
	resolved = false;
}

KeyFrameCollection::~KeyFrameCollection ()
{
	g_ptr_array_free (sorted_list, true);
}

int
KeyFrameCollection::Add (DependencyObject *data)
{
	int n;
	
	if ((n = Collection::Add (data)) == -1)
		return -1;
	
	resolved = false;
	
	return n;
}

bool
KeyFrameCollection::Insert (int index, DependencyObject *data)
{
	if (!Collection::Insert (index, data))
		return false;
	
	resolved = false;
	
	return true;
}

bool
KeyFrameCollection::Remove (DependencyObject *data)
{
	if (!Collection::Remove (data))
		return false;
	
	resolved = false;
	
	return true;
}

void
KeyFrameCollection::Clear ()
{
	resolved = false;
	g_ptr_array_set_size (sorted_list, 0);
	Collection::Clear ();
}

KeyFrame *
KeyFrameCollection::GetKeyFrameForTime (TimeSpan t, KeyFrame **prev_frame)
{
	KeyFrame *current_keyframe = NULL;
	KeyFrame *previous_keyframe = NULL;
	int i;
	
	if (sorted_list->len == 0) {
		if (prev_frame)
			*prev_frame = NULL;
		
		return NULL;
	}
	
	/* Crawl forward to figure out what segment to use (this assumes the list is sorted) */
	for (i = 0; i < (int) sorted_list->len; i++) {
		KeyFrame *keyframe = (KeyFrame *) sorted_list->pdata[i];
		TimeSpan key_end_time = keyframe->resolved_keytime;
		
		if (key_end_time >= t || (i + 1) >= (int) sorted_list->len) 
			break;
	}

	/* Crawl backward to find first non-null frame */
	for (; i >= 0; i--) {
		KeyFrame *keyframe = (KeyFrame *) sorted_list->pdata[i];
		if (keyframe->GetValue ("Value") != NULL) {
			current_keyframe = keyframe;
			break;
		}
	}

	/* Crawl backward some more to find first non-null prev frame */
	for (i--; i >= 0; i--) {
		KeyFrame *keyframe = (KeyFrame *) sorted_list->pdata[i];
		if (keyframe->GetValue ("Value") != NULL) {
			previous_keyframe = keyframe;
			break;
		}
	}
	
	/* Assign prev frame */
	if (prev_frame != NULL)
		*prev_frame = previous_keyframe;
	
	return current_keyframe;
}

void
KeyFrameCollection::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (subobj_args->property == KeyFrame::KeyTimeProperty) {
		resolved = false;
	}

	Collection::OnSubPropertyChanged (prop, obj, subobj_args);
}

ColorKeyFrameCollection *
color_key_frame_collection_new (void)
{
	return new ColorKeyFrameCollection ();
}

DoubleKeyFrameCollection *
double_key_frame_collection_new (void)
{
	return new DoubleKeyFrameCollection ();
}

PointKeyFrameCollection *
point_key_frame_collection_new (void)
{
	return new PointKeyFrameCollection ();
}

DependencyProperty* DoubleKeyFrame::ValueProperty;

DoubleKeyFrame::DoubleKeyFrame ()
{
	SetValue (0.0);
}

DoubleKeyFrame *
double_key_frame_new (void)
{
	return new DoubleKeyFrame ();
}

DependencyProperty* ColorKeyFrame::ValueProperty;

ColorKeyFrame::ColorKeyFrame ()
{
	static Color c = Color (0, 0, 0, 1);
	SetValue (c);
}

ColorKeyFrame *
color_key_frame_new (void)
{
	return new ColorKeyFrame ();
}

DependencyProperty* PointKeyFrame::ValueProperty;

PointKeyFrame::PointKeyFrame ()
{
	static Point p = Point (0, 0);
	SetValue (p);
}

PointKeyFrame *
point_key_frame_new (void)
{
	return new PointKeyFrame ();
}

Value*
DiscreteDoubleKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	double *to = GetValue();

	if (to && keyFrameProgress == 1.0)
		return new Value(*to);
	else
		return new Value (baseValue->AsDouble());
}

DiscreteDoubleKeyFrame *
discrete_double_key_frame_new (void)
{
	return new DiscreteDoubleKeyFrame ();
}



Value*
DiscreteColorKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	Color *to = GetValue();

	if (to && keyFrameProgress == 1.0)
		return new Value(*to);
	else
		return new Value (*baseValue->AsColor());
}

DiscreteColorKeyFrame *
discrete_color_key_frame_new (void)
{
	return new DiscreteColorKeyFrame ();
}



Value*
DiscretePointKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	Point *to = GetValue();

	if (to && keyFrameProgress == 1.0)
		return new Value(*to);
	else
		return new Value (*baseValue->AsPoint());
}

DiscretePointKeyFrame *
discrete_point_key_frame_new (void)
{
	return new DiscretePointKeyFrame ();
}




Value*
LinearDoubleKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	double *to = GetValue();

	if (!to)
		return new Value (baseValue->AsDouble());

	double start, end;

	start = baseValue->AsDouble();
	end = *to;

	return new Value (LERP (start, end, keyFrameProgress));
}

LinearDoubleKeyFrame *
linear_double_key_frame_new (void)
{
	return new LinearDoubleKeyFrame ();
}



Value*
LinearColorKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	Color *to = GetValue();

	if (!to)
		return new Value (*baseValue->AsColor());

	Color start, end;

	start = *baseValue->AsColor();
	end = *to;

	return new Value (LERP (start, end, keyFrameProgress));
}

LinearColorKeyFrame *
linear_color_key_frame_new (void)
{
	return new LinearColorKeyFrame ();
}



Value*
LinearPointKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	Point *to = GetValue();

	if (!to)
		return new Value (*baseValue->AsPoint());

	Point start, end;

	start = *baseValue->AsPoint();
	end = *to;

	return new Value (LERP (start, end, keyFrameProgress));
}

LinearPointKeyFrame *
linear_point_key_frame_new (void)
{
	return new LinearPointKeyFrame ();
}


DependencyProperty* SplineDoubleKeyFrame::KeySplineProperty;

KeySpline*
SplineDoubleKeyFrame::GetKeySpline ()
{
	return this->DependencyObject::GetValue (SplineDoubleKeyFrame::KeySplineProperty)->AsKeySpline();
}

Value*
SplineDoubleKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	double splineProgress = GetKeySpline ()->GetSplineProgress (keyFrameProgress);

	double *to = GetValue();

	if (!to)
		return new Value (baseValue->AsDouble());
	else if (keyFrameProgress >= 1.0)
		return new Value (*to);

	double start, end;

	start = baseValue->AsDouble();
	end = *to;

	return new Value (LERP (start, end, splineProgress));
}

SplineDoubleKeyFrame *
spline_double_key_frame_new (void)
{
	return new SplineDoubleKeyFrame ();
}


DependencyProperty* SplineColorKeyFrame::KeySplineProperty;

KeySpline*
SplineColorKeyFrame::GetKeySpline ()
{
	return this->DependencyObject::GetValue (SplineColorKeyFrame::KeySplineProperty)->AsKeySpline();
}

Value*
SplineColorKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	double splineProgress = GetKeySpline ()->GetSplineProgress (keyFrameProgress);

	Color *to = GetValue();

	if (!to)
		return new Value (*baseValue->AsColor());
	else if (keyFrameProgress >= 1.0)
		return new Value (*to);

	Color start, end;

	start = *baseValue->AsColor();
	end = *to;

	return new Value (LERP (start, end, splineProgress));
}

SplineColorKeyFrame *
spline_color_key_frame_new (void)
{
	return new SplineColorKeyFrame ();
}


DependencyProperty* SplinePointKeyFrame::KeySplineProperty;

KeySpline*
SplinePointKeyFrame::GetKeySpline ()
{
	return this->DependencyObject::GetValue (SplinePointKeyFrame::KeySplineProperty)->AsKeySpline();
}

Value*
SplinePointKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	double splineProgress = GetKeySpline ()->GetSplineProgress (keyFrameProgress);

	Point *to = GetValue();

	if (!to)
		return new Value (*baseValue->AsPoint());
	else if (keyFrameProgress >= 1.0)
		return new Value (*to);

	Point start, end;

	start = *baseValue->AsPoint();
	end = *to;

	return new Value (LERP (start, end, splineProgress));
}

SplinePointKeyFrame *
spline_point_key_frame_new (void)
{
	return new SplinePointKeyFrame ();
}


/* implements the algorithm specified at the bottom of this page:
   http://msdn2.microsoft.com/en-us/library/ms742524.aspx
*/
static void
KeyFrameAnimation_ResolveKeyFrames (Animation/*Timeline*/ *animation, KeyFrameCollection *col)
{
	if (col->resolved)
		return;

	col->resolved = true;

	TimeSpan total_interpolation_time;
	bool has_timespan_keyframe = false;
	TimeSpan highest_keytime_timespan = 0;
	List::Node *cur;

	for (cur = col->list->First (); cur; cur = cur->next) {
		KeyFrame *keyframe = (KeyFrame *) ((Collection::Node *) cur)->obj;
		keyframe->resolved_keytime = 0;
		keyframe->resolved = false;
	}

	/* resolve TimeSpan keyframes (step 1 from url) */
	for (cur = col->list->First (); cur; cur = cur->next) {
		KeyFrame *keyframe = (KeyFrame *) ((Collection::Node *) cur)->obj;
		if (keyframe->GetKeyTime()->HasTimeSpan()) {
			has_timespan_keyframe = true;
			TimeSpan ts = keyframe->GetKeyTime()->GetTimeSpan ();
			if (ts > highest_keytime_timespan)
				highest_keytime_timespan = ts;

			keyframe->resolved_keytime = ts;
			keyframe->resolved = true;
		}
	}
 	
	/* calculate total animation interpolation time (step 2 from url) */
	Duration *d = animation->GetDuration();
	if (d->HasTimeSpan ()) {
		total_interpolation_time = d->GetTimeSpan ();
	}
	else if (has_timespan_keyframe) {
		total_interpolation_time = highest_keytime_timespan;
	}
	else {
		total_interpolation_time = TimeSpan_FromSeconds (1);
	}


	/* use the total interpolation time to resolve percent keytime keyframes (step 3 from url) */
	for (cur = col->list->First (); cur; cur = cur->next) {
		KeyFrame *keyframe = (KeyFrame *) ((Collection::Node *) cur)->obj;
		if (keyframe->GetKeyTime()->HasPercent()) {
			keyframe->resolved_keytime = (TimeSpan)(total_interpolation_time * keyframe->GetKeyTime()->GetPercent ());
			keyframe->resolved = true;
 		}
 	}

	/* step 4 from url */
	KeyFrame *keyframe;
	KeyTime *kt;
	/* if the last frame is KeyTime Uniform or Paced, resolve it
	   to be equal to the total interpolation time */
	cur = col->list->Last ();
	if (cur) {
		keyframe = (KeyFrame *) ((Collection::Node *) cur)->obj;
		kt = keyframe->GetKeyTime ();
		if (*kt == KeyTime::Paced || *kt == KeyTime::Uniform) {
			keyframe->resolved_keytime = total_interpolation_time;
			keyframe->resolved = true;
		}
	}

	/* if the first frame is KeyTime::Paced:
	**   1. if there is only 1 frame, its KeyTime is the total interpolation time.
	**   2. if there is more than 1 frame, its KeyTime is 0.
	**
	** note 1 is handled in the above block so we only have to
	** handle 2 here.
	*/
	cur = (Collection::Node*) col->list->First ();
	if (cur) {
		keyframe = (KeyFrame *) ((Collection::Node*) cur)->obj;
		kt = keyframe->GetKeyTime ();

		if (!keyframe->resolved && *kt == KeyTime::Paced) {
			keyframe->resolved_keytime = 0;
			keyframe->resolved = true;
		}
	}
	
	/* XXX resolve remaining KeyTime::Uniform frames (step 5 from url) */

	/* XXX resolve frames with unspecified keytimes (step 6 from url)

	   -- is this possible?  is the default keytime NULL?  it
              seems to be Uniform? */

	/* XXX resolve remaining KeyTime::Paced frames (step 7 from url) */

	/* insert the nodes into the sorted list using a stable sort
	   with resolved keytime as primary key, declaration order as
	   secondary key (step 8 from url) */
	g_ptr_array_set_size (col->sorted_list, 0);
	
	for (cur = col->list->Last (); cur; cur = cur->prev) {
		KeyFrame *keyframe = (KeyFrame *) ((Collection::Node *) cur)->obj;
		if (!keyframe->resolved) {
			g_warning ("***** unresolved keyframe!");
		}
		
		g_ptr_array_insert_sorted (col->sorted_list, KeyFrameComparer, keyframe);
	}
}

// Generic validator of KeyFrameCollection's. Collection vallidates 
// if all keyframes have valid time.
static bool
generic_keyframe_validator (KeyFrameCollection *col)
{
	List::Node *cur;

	for (cur = col->list->First (); cur; cur = cur->next) {
		KeyFrame *keyframe = (KeyFrame *) ((Collection::Node *) cur)->obj;
		if (keyframe->GetKeyTime () == NULL)
			return false;
	}

	return true;
}

DependencyProperty* DoubleAnimationUsingKeyFrames::KeyFramesProperty;

DoubleAnimationUsingKeyFrames::DoubleAnimationUsingKeyFrames()
{
	this->SetValue (DoubleAnimationUsingKeyFrames::KeyFramesProperty, Value::CreateUnref (new DoubleKeyFrameCollection ()));
}

DoubleAnimationUsingKeyFrames::~DoubleAnimationUsingKeyFrames ()
{
}

void
DoubleAnimationUsingKeyFrames::AddKeyFrame (DoubleKeyFrame *frame)
{
	DoubleKeyFrameCollection *key_frames = GetValue (DoubleAnimationUsingKeyFrames::KeyFramesProperty)->AsDoubleKeyFrameCollection ();

	key_frames->Add (frame);
}

void
DoubleAnimationUsingKeyFrames::RemoveKeyFrame (DoubleKeyFrame *frame)
{
	DoubleKeyFrameCollection *key_frames = GetValue (DoubleAnimationUsingKeyFrames::KeyFramesProperty)->AsDoubleKeyFrameCollection ();

	key_frames->Remove (frame);
}

Value*
DoubleAnimationUsingKeyFrames::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
						AnimationClock* animationClock)
{
	DoubleKeyFrameCollection *key_frames = GetValue (DoubleAnimationUsingKeyFrames::KeyFramesProperty)->AsDoubleKeyFrameCollection ();

	/* current segment info */
	TimeSpan current_time = animationClock->GetCurrentTime();
	DoubleKeyFrame *current_keyframe;
	DoubleKeyFrame *previous_keyframe;
	DoubleKeyFrame** keyframep = &previous_keyframe;
	Value *baseValue;
	bool deleteBaseValue;

	current_keyframe = (DoubleKeyFrame*)key_frames->GetKeyFrameForTime (current_time, (KeyFrame**)keyframep);
	if (current_keyframe == NULL) {
	  //abort ();
		return NULL; /* XXX */
	}

	TimeSpan key_end_time = current_keyframe->resolved_keytime;
	TimeSpan key_start_time;

	if (previous_keyframe == NULL) {
		/* the first keyframe, start at the animation's base value */
		baseValue = defaultOriginValue;
		deleteBaseValue = false;
		key_start_time = 0;
	}
	else {
		/* start at the previous keyframe's target value */
		baseValue = new Value (*previous_keyframe->GetValue ());
		deleteBaseValue = true;
		key_start_time = previous_keyframe->resolved_keytime;
	}

	double progress;

	if (current_time >= key_end_time) {
		progress = 1.0;
	}
	else {
		TimeSpan key_duration = key_end_time - key_start_time;
		if (key_duration == 0)
			progress = 1.0;
		else
			progress = (double)(current_time - key_start_time) / key_duration;
	}

	/* get the current value out of that segment */
	Value *rv = current_keyframe->InterpolateValue (baseValue, progress);
	if (deleteBaseValue)
		delete baseValue;
	return rv;
}

Duration
DoubleAnimationUsingKeyFrames::GetNaturalDurationCore (Clock *clock)
{
	DoubleKeyFrameCollection *key_frames = GetValue (DoubleAnimationUsingKeyFrames::KeyFramesProperty)->AsDoubleKeyFrameCollection ();
	
	KeyFrameAnimation_ResolveKeyFrames (this, key_frames);

	guint len = key_frames->sorted_list->len;
	if (len > 0)
		return ((KeyFrame *) key_frames->sorted_list->pdata[len - 1])->resolved_keytime;
	else
		return Duration::Automatic;
}

void
DoubleAnimationUsingKeyFrames::Resolve ()
{
	KeyFrameAnimation_ResolveKeyFrames (this,
					    GetValue (DoubleAnimationUsingKeyFrames::KeyFramesProperty)->AsDoubleKeyFrameCollection ());
}

bool
DoubleAnimationUsingKeyFrames::Validate ()
{
	KeyFrameCollection *col = GetValue (DoubleAnimationUsingKeyFrames::KeyFramesProperty)->AsKeyFrameCollection ();
	return generic_keyframe_validator (col);
}

DoubleAnimationUsingKeyFrames *
double_animation_using_key_frames_new (void)
{
	return new DoubleAnimationUsingKeyFrames ();
}



DependencyProperty* ColorAnimationUsingKeyFrames::KeyFramesProperty;

ColorAnimationUsingKeyFrames::ColorAnimationUsingKeyFrames()
{
	this->SetValue (ColorAnimationUsingKeyFrames::KeyFramesProperty, Value::CreateUnref (new ColorKeyFrameCollection ()));
}

ColorAnimationUsingKeyFrames::~ColorAnimationUsingKeyFrames ()
{
}

void
ColorAnimationUsingKeyFrames::AddKeyFrame (ColorKeyFrame *frame)
{
	ColorKeyFrameCollection *key_frames = GetValue (ColorAnimationUsingKeyFrames::KeyFramesProperty)->AsColorKeyFrameCollection ();

	key_frames->Add (frame);
}

void
ColorAnimationUsingKeyFrames::RemoveKeyFrame (ColorKeyFrame *frame)
{
	ColorKeyFrameCollection *key_frames = GetValue (ColorAnimationUsingKeyFrames::KeyFramesProperty)->AsColorKeyFrameCollection ();

	key_frames->Remove (frame);
}

Value*
ColorAnimationUsingKeyFrames::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					       AnimationClock* animationClock)
{
	ColorKeyFrameCollection *key_frames = GetValue (ColorAnimationUsingKeyFrames::KeyFramesProperty)->AsColorKeyFrameCollection ();
	/* current segment info */
	TimeSpan current_time = animationClock->GetCurrentTime();
	ColorKeyFrame *current_keyframe;
	ColorKeyFrame *previous_keyframe;
	ColorKeyFrame** keyframep = &previous_keyframe;
	Value *baseValue;
	bool deleteBaseValue;

	current_keyframe = (ColorKeyFrame*)key_frames->GetKeyFrameForTime (current_time, (KeyFrame**)keyframep);
	if (current_keyframe == NULL)
		return NULL; /* XXX */

	TimeSpan key_end_time = current_keyframe->resolved_keytime;
	TimeSpan key_start_time;

	if (previous_keyframe == NULL) {
		/* the first keyframe, start at the animation's base value */
		baseValue = defaultOriginValue;
		deleteBaseValue = false;
		key_start_time = 0;
	}
	else {
		/* start at the previous keyframe's target value */
		baseValue = new Value(*previous_keyframe->GetValue ());
		deleteBaseValue = true;
		key_start_time = previous_keyframe->resolved_keytime;
	}

	double progress;

	if (current_time >= key_end_time) {
		progress = 1.0;
	}
	else {
		TimeSpan key_duration = key_end_time - key_start_time;
		if (key_duration == 0)
			progress = 1.0;
		else
			progress = (double)(current_time - key_start_time) / key_duration;
	}

	/* get the current value out of that segment */
	Value *rv = current_keyframe->InterpolateValue (baseValue, progress);
	if (deleteBaseValue)
		delete baseValue;
	return rv;;
}

Duration
ColorAnimationUsingKeyFrames::GetNaturalDurationCore (Clock *clock)
{
	ColorKeyFrameCollection *key_frames = GetValue (ColorAnimationUsingKeyFrames::KeyFramesProperty)->AsColorKeyFrameCollection ();
	
	KeyFrameAnimation_ResolveKeyFrames (this, key_frames);

	guint len = key_frames->sorted_list->len;
	if (len > 0)
		return ((KeyFrame *) key_frames->sorted_list->pdata[len - 1])->resolved_keytime;
	else
		return Duration::Automatic;
}

void
ColorAnimationUsingKeyFrames::Resolve ()
{
	KeyFrameAnimation_ResolveKeyFrames (this,
					    GetValue (ColorAnimationUsingKeyFrames::KeyFramesProperty)->AsColorKeyFrameCollection ());
}

bool
ColorAnimationUsingKeyFrames::Validate ()
{
	KeyFrameCollection *col = GetValue (ColorAnimationUsingKeyFrames::KeyFramesProperty)->AsKeyFrameCollection ();
	return generic_keyframe_validator (col);
}

ColorAnimationUsingKeyFrames *
color_animation_using_key_frames_new (void)
{
	return new ColorAnimationUsingKeyFrames ();
}




DependencyProperty* PointAnimationUsingKeyFrames::KeyFramesProperty;

PointAnimationUsingKeyFrames::PointAnimationUsingKeyFrames()
{
	this->SetValue (PointAnimationUsingKeyFrames::KeyFramesProperty, Value::CreateUnref (new PointKeyFrameCollection ()));
}

PointAnimationUsingKeyFrames::~PointAnimationUsingKeyFrames ()
{
}

void
PointAnimationUsingKeyFrames::AddKeyFrame (PointKeyFrame *frame)
{
	PointKeyFrameCollection *key_frames = GetValue (PointAnimationUsingKeyFrames::KeyFramesProperty)->AsPointKeyFrameCollection ();

	key_frames->Add (frame);
}

void
PointAnimationUsingKeyFrames::RemoveKeyFrame (PointKeyFrame *frame)
{
	PointKeyFrameCollection *key_frames = GetValue (PointAnimationUsingKeyFrames::KeyFramesProperty)->AsPointKeyFrameCollection ();

	key_frames->Remove (frame);
}

Value*
PointAnimationUsingKeyFrames::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					       AnimationClock* animationClock)
{
	PointKeyFrameCollection *key_frames = GetValue (PointAnimationUsingKeyFrames::KeyFramesProperty)->AsPointKeyFrameCollection ();
	/* current segment info */
	TimeSpan current_time = animationClock->GetCurrentTime();
	PointKeyFrame *current_keyframe;
	PointKeyFrame *previous_keyframe;
	PointKeyFrame** keyframep = &previous_keyframe;
	Value *baseValue;
	bool deleteBaseValue;

	current_keyframe = (PointKeyFrame*)key_frames->GetKeyFrameForTime (current_time, (KeyFrame**)keyframep);
	if (current_keyframe == NULL)
		return NULL; /* XXX */

	TimeSpan key_end_time = current_keyframe->resolved_keytime;
	TimeSpan key_start_time;

	if (previous_keyframe == NULL) {
		/* the first keyframe, start at the animation's base value */
		baseValue = defaultOriginValue;
		deleteBaseValue = false;
		key_start_time = 0;
	}
	else {
		/* start at the previous keyframe's target value */
		baseValue = new Value(*previous_keyframe->GetValue ());
		deleteBaseValue = true;
		key_start_time = previous_keyframe->resolved_keytime;
	}

	double progress;

	if (current_time >= key_end_time) {
		progress = 1.0;
	}
	else {
		TimeSpan key_duration = key_end_time - key_start_time;
		if (key_duration == 0)
			progress = 1.0;
		else
			progress = (double)(current_time - key_start_time) / key_duration;
	}

	/* get the current value out of that segment */
	Value *rv = current_keyframe->InterpolateValue (baseValue, progress);
	if (deleteBaseValue)
		delete baseValue;
	return rv;
}

Duration
PointAnimationUsingKeyFrames::GetNaturalDurationCore (Clock* clock)
{
	PointKeyFrameCollection *key_frames = GetValue (PointAnimationUsingKeyFrames::KeyFramesProperty)->AsPointKeyFrameCollection ();
	
	KeyFrameAnimation_ResolveKeyFrames (this, key_frames);

	guint len = key_frames->sorted_list->len;
	if (len > 0)
		return ((KeyFrame *) key_frames->sorted_list->pdata[len - 1])->resolved_keytime;
	else
		return Duration::Automatic;
}

void
PointAnimationUsingKeyFrames::Resolve ()
{
	KeyFrameAnimation_ResolveKeyFrames (this,
					    GetValue (PointAnimationUsingKeyFrames::KeyFramesProperty)->AsPointKeyFrameCollection ());
}

bool
PointAnimationUsingKeyFrames::Validate ()
{
	KeyFrameCollection *col = GetValue (PointAnimationUsingKeyFrames::KeyFramesProperty)->AsKeyFrameCollection ();
	return generic_keyframe_validator (col);
}

PointAnimationUsingKeyFrames *
point_animation_using_key_frames_new (void)
{
	return new PointAnimationUsingKeyFrames ();
}





RepeatBehavior RepeatBehavior::Forever (RepeatBehavior::FOREVER);
Duration Duration::Automatic (Duration::AUTOMATIC);
Duration Duration::Forever (Duration::FOREVER);
KeyTime KeyTime::Paced (KeyTime::PACED);
KeyTime KeyTime::Uniform (KeyTime::UNIFORM);

void 
animation_init (void)
{
	/* DoubleAnimation properties */
	DoubleAnimation::ByProperty   = DependencyObject::RegisterNullable (Type::DOUBLEANIMATION, "By",   Type::DOUBLE);
	DoubleAnimation::FromProperty = DependencyObject::RegisterNullable (Type::DOUBLEANIMATION, "From", Type::DOUBLE);
	DoubleAnimation::ToProperty   = DependencyObject::RegisterNullable (Type::DOUBLEANIMATION, "To",   Type::DOUBLE);


	/* ColorAnimation properties */
	ColorAnimation::ByProperty   = DependencyObject::RegisterNullable (Type::COLORANIMATION, "By",   Type::COLOR); // null defaults
	ColorAnimation::FromProperty = DependencyObject::RegisterNullable (Type::COLORANIMATION, "From", Type::COLOR);
	ColorAnimation::ToProperty   = DependencyObject::RegisterNullable (Type::COLORANIMATION, "To",   Type::COLOR);

	/* PointAnimation properties */
	PointAnimation::ByProperty   = DependencyObject::RegisterNullable (Type::POINTANIMATION, "By",   Type::POINT); // null defaults
	PointAnimation::FromProperty = DependencyObject::RegisterNullable (Type::POINTANIMATION, "From", Type::POINT);
	PointAnimation::ToProperty   = DependencyObject::RegisterNullable (Type::POINTANIMATION, "To",   Type::POINT);

	/* Storyboard properties */
	Storyboard::TargetPropertyProperty = DependencyObject::RegisterFull (Type::STORYBOARD, "TargetProperty", 
									     NULL, Type::STRING, true, false);
	Storyboard::TargetNameProperty     = DependencyObject::RegisterFull (Type::STORYBOARD, "TargetName", 
									     NULL, Type::STRING, true, false);

	/* BeginStoryboard properties */
	BeginStoryboard::StoryboardProperty = DependencyObject::Register (Type::BEGINSTORYBOARD, "Storyboard",	Type::STORYBOARD);

	/* KeyFrame properties */
 	KeyFrame::KeyTimeProperty = DependencyObject::RegisterNullable (Type::KEYFRAME, "KeyTime", Type::KEYTIME);
 	DoubleKeyFrame::ValueProperty = DependencyObject::RegisterNullable (Type::DOUBLEKEYFRAME, "Value", Type::DOUBLE);
 	PointKeyFrame::ValueProperty = DependencyObject::RegisterNullable (Type::POINTKEYFRAME, "Value", Type::POINT);
 	ColorKeyFrame::ValueProperty = DependencyObject::RegisterNullable (Type::COLORKEYFRAME, "Value", Type::COLOR);

	/* Spline keyframe properties */
	SplineDoubleKeyFrame::KeySplineProperty = DependencyObject::Register (Type::SPLINEDOUBLEKEYFRAME, "KeySpline", Value::CreateUnrefPtr (new KeySpline (0, 0, 1, 0)));
 	SplineColorKeyFrame::KeySplineProperty = DependencyObject::Register (Type::SPLINECOLORKEYFRAME, "KeySpline", Value::CreateUnrefPtr (new KeySpline (0, 0, 1, 0)));
 	SplinePointKeyFrame::KeySplineProperty = DependencyObject::Register (Type::SPLINEPOINTKEYFRAME, "KeySpline", Value::CreateUnrefPtr (new KeySpline (0, 0, 1, 0)));

	/* KeyFrame animation properties */
	ColorAnimationUsingKeyFrames::KeyFramesProperty = DependencyObject::Register (Type::COLORANIMATIONUSINGKEYFRAMES, "KeyFrames", Type::COLORKEYFRAME_COLLECTION);
	DoubleAnimationUsingKeyFrames::KeyFramesProperty = DependencyObject::Register (Type::DOUBLEANIMATIONUSINGKEYFRAMES, "KeyFrames", Type::DOUBLEKEYFRAME_COLLECTION);
	PointAnimationUsingKeyFrames::KeyFramesProperty = DependencyObject::Register (Type::POINTANIMATIONUSINGKEYFRAMES, "KeyFrames", Type::POINTKEYFRAME_COLLECTION);
}

void
animation_shutdown (void)
{
	// no-op
}

/* The nullable setters/getters for the various animation types */
#define SET_NULLABLE_FUNC(t) \
static void SetNullable##t##Prop (DependencyObject *obj, DependencyProperty *prop, t *pv) \
{ \
	if (!pv) \
		obj->SetValue (prop, NULL); \
	else \
		obj->SetValue (prop, Value(*pv)); \
}

#define NULLABLE_GETSET_IMPL(klass,prop,t,T) \
void klass::Set##prop (t v) { Set##prop (&v); } \
void klass::Set##prop (t *pv) { SetNullable##t##Prop (this, klass::prop##Property, pv); } \
t* klass::Get##prop () { Value* v = this->DependencyObject::GetValue (klass::prop##Property);  return v ? v->As##T () : NULL; }

#define NULLABLE_PRIM_GETSET_IMPL(klass,prop,t,T) \
void klass::Set##prop (t v) { Set##prop (&v); } \
void klass::Set##prop (t *pv) { SetNullable##t##Prop (this, klass::prop##Property, pv); } \
t* klass::Get##prop () { Value* v = this->DependencyObject::GetValue (klass::prop##Property);  return v ? v->AsNullable##T () : NULL; }



SET_NULLABLE_FUNC(double)
SET_NULLABLE_FUNC(Color)
SET_NULLABLE_FUNC(Point)

NULLABLE_PRIM_GETSET_IMPL (DoubleAnimation, By, double, Double)
NULLABLE_PRIM_GETSET_IMPL (DoubleAnimation, To, double, Double)
NULLABLE_PRIM_GETSET_IMPL (DoubleAnimation, From, double, Double)

NULLABLE_GETSET_IMPL (ColorAnimation, By, Color, Color)
NULLABLE_GETSET_IMPL (ColorAnimation, To, Color, Color)
NULLABLE_GETSET_IMPL (ColorAnimation, From, Color, Color)

NULLABLE_GETSET_IMPL (PointAnimation, By, Point, Point)
NULLABLE_GETSET_IMPL (PointAnimation, To, Point, Point)
NULLABLE_GETSET_IMPL (PointAnimation, From, Point, Point)


NULLABLE_GETSET_IMPL (ColorKeyFrame, Value, Color, Color)
NULLABLE_GETSET_IMPL (PointKeyFrame, Value, Point, Point)
NULLABLE_PRIM_GETSET_IMPL (DoubleKeyFrame, Value, double, Double)
