/*
 * animation.cpp: Animation engine
 *
 * Author:
 *   Chris Toshok (toshok@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <math.h>

#include "garray-ext.h"
#include "animation.h"
#include "color.h"
#include "runtime.h"

#define LERP(f,t,p) ((f) + ((t) - (f)) * (p))




AnimationStorage::AnimationStorage (AnimationClock *clock, Animation/*Timeline*/ *timeline,
				    DependencyObject *targetobj, DependencyProperty *targetprop)
  : clock (clock),
    timeline (timeline),
    targetobj (targetobj),
    targetprop (targetprop)
  
{
	clock->AddHandler (clock->CurrentTimeInvalidatedEvent, update_property_value, this);
	clock->AddHandler (clock->CurrentStateInvalidatedEvent, reset_property_value, this);
	targetobj->AddHandler (EventObject::DestroyedEvent, target_object_destroyed, this);

	baseValue = new Value(*targetobj->GetValue (targetprop));
}

void
AnimationStorage::target_object_destroyed (EventObject *, gpointer, gpointer closure)
{
	((AnimationStorage*)closure)->TargetObjectDestroyed ();
}

void
AnimationStorage::TargetObjectDestroyed ()
{
	targetobj = NULL;
}

void
AnimationStorage::update_property_value (EventObject *, gpointer, gpointer closure)
{
	((AnimationStorage*)closure)->UpdatePropertyValue ();
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
AnimationStorage::reset_property_value (EventObject *, gpointer, gpointer closure)
{
	((AnimationStorage*)closure)->ResetPropertyValue ();
}

void
AnimationStorage::ResetPropertyValue ()
{
	if (targetobj == NULL)
		return;

	if (clock->GetClockState() == Clock::Stopped)
		targetobj->SetValue (targetprop, *baseValue);
}

AnimationStorage::~AnimationStorage ()
{
	if (baseValue)
		delete baseValue;
	
	if (clock != NULL) {
		clock->RemoveHandler (clock->CurrentTimeInvalidatedEvent, update_property_value, this);
		clock->RemoveHandler (clock->CurrentStateInvalidatedEvent, reset_property_value, this);
	}
	
	if (targetobj != NULL) {
		targetobj->RemoveHandler (EventObject::DestroyedEvent, target_object_destroyed, this);
	}
}

AnimationClock::AnimationClock (Animation/*Timeline*/ *timeline)
  : Clock (timeline),
    timeline(timeline),
    storage(NULL)
{
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

AnimationClock::~AnimationClock ()
{
	if (storage)
		delete storage;
}

Clock*
Animation/*Timeline*/::AllocateClock()
{
	Clock *clock = new AnimationClock (this);
	char *name = g_strdup_printf ("AnimationClock for %s, targetobj = %s, targetprop = %s", GetTypeName(),
				      Storyboard::GetTargetName (this), Storyboard::GetTargetProperty (this));
	clock->SetValue (DependencyObject::NameProperty, name);
	g_free (name);
	return clock;
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
int                 Storyboard::CompletedEvent = -1;

Storyboard::Storyboard ()
  : root_clock (NULL)
{
}

void
Storyboard::HookupAnimationsRecurse (Clock *clock)
{
	switch (clock->GetObjectType ()) {
	case Type::ANIMATIONCLOCK: {
		AnimationClock *ac = (AnimationClock*)clock;

		char *targetProperty = Storyboard::GetTargetProperty (ac->GetTimeline());
		if (!targetProperty) {
			printf ("no target property\n");
			return;
		}

		char *targetName = NULL;

		for (Clock *c = ac; c; c = c->GetParent()) {
			targetName = Storyboard::GetTargetName (c->GetTimeline());
			if (targetName)
				break;
		}
		if (!targetName) {
			printf ("no target name\n");
			return;
		}

		//printf ("Got %s %s\n", targetProperty, targetName);
		DependencyObject *o = FindName (targetName);
		if (!o) {
			printf ("no object named %s\n", targetName);
			return;
		}

		DependencyProperty *prop = resolve_property_path (&o, targetProperty);
		if (!prop) {
			printf ("no property named %s on object %s, which has type %s\n", targetProperty, targetName, o->GetTypeName());
			return;
		}


		((Animation*)ac->GetTimeline())->Resolve ();
		ac->HookupStorage (o, prop);
		break;
	}
	case Type::CLOCKGROUP: {
		ClockGroup *cg = (ClockGroup*)clock;
		for (GList *l = cg->child_clocks; l; l = l->next)
			HookupAnimationsRecurse ((Clock*)l->data);
		break;
	}
	default:
		g_assert_not_reached ();
		break;
	}
}

void
Storyboard::invoke_completed (EventObject *, gpointer, gpointer closure)
{
	Storyboard* sb = (Storyboard*)closure;
	sb->Emit (sb->CompletedEvent);
}

void
Storyboard::Begin ()
{
	ClockGroup *group = NULL;

	/* destroy the clock hierarchy and recreate it to restart.
	   easier than making Begin work again with the existing clock
	   hierarchy */
	if (root_clock) {
		group = root_clock->GetParent();
		group->RemoveChild (root_clock);
		root_clock->unref ();
		root_clock = NULL;
	}

	if (!group) {
		Surface *surface = FindSurface ();
		if (surface == NULL) {
			g_warning ("unable to find surface to add storyboard clock to.");
			return;
		}
		group = surface->GetClockGroup();
	}

	// This creates the clock tree for the hierarchy.  if a
	// Timeline A is a child of TimelineGroup B, then Clock cA
	// will be a child of ClockGroup cB.
	root_clock = CreateClock ();
	root_clock->AddHandler (root_clock->CompletedEvent, invoke_completed, this);

	// walk the clock tree hooking up the correct properties and
	// creating AnimationStorage's for AnimationClocks.
	HookupAnimationsRecurse (root_clock);

	group->AddChild (root_clock);

	// we delay starting the surface's ClockGroup until the first
	// child has been added.  otherwise we run into timing issues
	// between timelines that explicitly set a BeginTime and those
	// that don't (and so default to 00:00:00).
	if (group->GetClockState() != Clock::Active) {
		group->Begin ();
	}
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
	if (root_clock)
		root_clock->Stop ();
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

static Surface*
find_surface_recurse (DependencyObject *obj)
{
	if (obj == NULL)
		return NULL;
	else if (obj->Is(Type::UIELEMENT)) {
		return ((UIElement*)obj)->GetSurface();
	}
	else {
		obj = obj->GetParent();
		return find_surface_recurse (obj);
	}
}

Surface*
Storyboard::FindSurface ()
{
	return find_surface_recurse (this);
}

Storyboard::~Storyboard ()
{
	if (root_clock) {
		//printf ("Clock %p (ref=%d)\n", root_clock, root_clock->refcount);
		Stop ();
		if (root_clock->GetParent())
			root_clock->GetParent()->RemoveChild (root_clock);
		//printf ("Unrefing Clock %p (ref=%d)\n", root_clock, root_clock->refcount);
		root_clock->unref ();
		root_clock = NULL;
	}
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
	return GetValue (BeginStoryboard::StoryboardProperty)->AsStoryboard();
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
		end = start;
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
		end = start;
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
		end = start;
	}

	double progress = animationClock->GetCurrentProgress ();

	return new Value (LERP (start, end, progress));
}

PointAnimation *
point_animation_new (void)
{
	return new PointAnimation ();
}


KeySpline::KeySpline () : controlPoint1 (Point (0.0, 0.0)), controlPoint2 (Point (1.0, 1.0))
{
}

KeySpline::KeySpline (Point controlPoint1, Point controlPoint2)
	: controlPoint1 (controlPoint1),
	  controlPoint2 (controlPoint2)
{
}

KeySpline::KeySpline (double x1, double y1,
		      double x2, double y2)
	: controlPoint1 (Point (x1, y1)),
	  controlPoint2 (Point (x2, y2))
{
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
}

KeySpline *
key_spline_new (void)
{
	return new KeySpline ();
}



// the following is from:
// http://steve.hollasch.net/cgindex/curves/cbezarclen.html
//

#define sqr(x) (x * x)

#define _ABS(x) (x < 0 ? -x : x)

const double TOLERANCE = 0.0001;  // Application specific tolerance


//---------------------------------------------------------------------------
static double
balf(double t, double q1, double q2, double q3, double q4, double q5) // Bezier Arc Length Function
{
	double result = q5 + t*(q4 + t*(q3 + t*(q2 + t*q1)));
	result = sqrt(_ABS(result));
	return result;
}

//---------------------------------------------------------------------------
// NOTES:       TOLERANCE is a maximum error ratio
//                      if n_limit isn't a power of 2 it will be act like the next higher
//                      power of two.
static double
Simpson (double (*f)(double, double, double, double, double, double),
	 double a,
	 double b,
	 int n_limit,
	 double q1, double q2, double q3, double q4, double q5)
{
	int n = 1;
	double multiplier = (b - a)/6.0;
	double endsum = f(a, q1, q2, q3, q4, q5) + f(b, q1, q2, q3, q4, q5);
	double interval = (b - a)/2.0;
	double asum = 0;
	double bsum = f(a + interval, q1, q2, q3, q4, q5);
	double est1 = multiplier * (endsum + 2 * asum + 4 * bsum);
	double est0 = 2 * est1;

	while(n < n_limit 
	      && (_ABS(est1) > 0 && _ABS((est1 - est0) / est1) > TOLERANCE)) {
		n *= 2;
		multiplier /= 2;
		interval /= 2;
		asum += bsum;
		bsum = 0;
		est0 = est1;
		double interval_div_2n = interval / (2.0 * n);

		for (int i = 1; i < 2 * n; i += 2) {
			double t = a + i * interval_div_2n;
			bsum += f(t, q1, q2, q3, q4, q5);
		}

		est1 = multiplier*(endsum + 2*asum + 4*bsum);
	}

	return est1;
}

static double
BezierArcLength(Point p1, Point p2, Point p3, Point p4, double t)
{
	Point k1, k2, k3, k4;

	k1 = p1*-1 + (p2 - p3)*3 + p4;
	k2 = (p1 + p3)*3 - p2*6;
	k3 = (p2 - p1)*3;
	k4 = p1;

	double q1 = 9.0*(sqr(k1.x) + sqr(k1.y));
	double q2 = 12.0*(k1.x*k2.x + k1.y*k2.y);
	double q3 = 3.0*(k1.x*k3.x + k1.y*k3.y) + 4.0*(sqr(k2.x) + sqr(k2.y));
	double q4 = 4.0*(k2.x*k3.x + k2.y*k3.y);
	double q5 = sqr(k3.x) + sqr(k3.y);

	return Simpson(balf, 0, t, 1024,
		       q1, q2, q3, q4, q5);
}

double
KeySpline::GetSplineProgress (double linearProgress)
{
	return (BezierArcLength (Point(0,0), controlPoint1, controlPoint2, Point (1,1), linearProgress) /
		BezierArcLength (Point(0,0), controlPoint1, controlPoint2, Point (1,1), 1.0));
}



DependencyProperty* KeyFrame::KeyTimeProperty;

KeyFrame::KeyFrame ()
{
}

KeyTime*
KeyFrame::GetKeyTime()
{
	return GetValue (KeyFrame::KeyTimeProperty)->AsKeyTime();
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
		   dependency_object_get_name (this));
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

bool
KeyFrameCollection::Add (DependencyObject *data)
{
	bool b = Collection::Add (data);
	if (b)
		resolved = false;
	return b;
}

bool
KeyFrameCollection::Insert (int index, DependencyObject *data)
{
	bool b = Collection::Insert (index, data);
	if (b)
		resolved = false;
	return b;
}

bool
KeyFrameCollection::Remove (DependencyObject *data)
{
	bool b = Collection::Remove (data);
	if (b)
		resolved = false;
	return b;
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
	guint i;
	
	*prev_frame = NULL;

	/* figure out what segment to use (this assumes the list is sorted) */
	for (i = 0; i < sorted_list->len; i++) {
		KeyFrame *keyframe = (KeyFrame *) sorted_list->pdata[i];
		TimeSpan key_end_time = keyframe->resolved_keytime;
		
		if (key_end_time >= t || (i + 1) >= sorted_list->len) {
			current_keyframe = keyframe;
			
			if (i > 0)
				previous_keyframe = (KeyFrame *) sorted_list->pdata[i - 1];
			
			break;
		}
	}
	
	if (prev_frame != NULL)
		*prev_frame = previous_keyframe;
	
	return current_keyframe;
}

void
KeyFrameCollection::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, DependencyProperty *subprop)
{
	if (subprop == KeyFrame::KeyTimeProperty) {
		resolved = false;
	}

	Collection::OnSubPropertyChanged (prop, obj, subprop);
}

ColorKeyFrameCollection*
color_key_frame_collection_new ()
{
	return new ColorKeyFrameCollection ();
}

DoubleKeyFrameCollection*
double_key_frame_collection_new ()
{
	return new DoubleKeyFrameCollection ();
}

PointKeyFrameCollection*
point_key_frame_collection_new ()
{
	return new PointKeyFrameCollection ();
}

DependencyProperty* DoubleKeyFrame::ValueProperty;

DoubleKeyFrame::DoubleKeyFrame ()
{
}

DoubleKeyFrame*
double_key_frame_new ()
{
	return new DoubleKeyFrame ();
}

DependencyProperty* ColorKeyFrame::ValueProperty;

ColorKeyFrame::ColorKeyFrame ()
{
}

ColorKeyFrame*
color_key_frame_new ()
{
	return new ColorKeyFrame ();
}

DependencyProperty* PointKeyFrame::ValueProperty;

PointKeyFrame::PointKeyFrame ()
{
}

PointKeyFrame*
point_key_frame_new ()
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



DependencyProperty* DoubleAnimationUsingKeyFrames::KeyFramesProperty;

DoubleAnimationUsingKeyFrames::DoubleAnimationUsingKeyFrames()
{
	this->SetValue (DoubleAnimationUsingKeyFrames::KeyFramesProperty, Value::CreateUnref (new DoubleKeyFrameCollection ()));
}

DoubleAnimationUsingKeyFrames::~DoubleAnimationUsingKeyFrames ()
{
}

void
DoubleAnimationUsingKeyFrames::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::DOUBLEANIMATIONUSINGKEYFRAMES) {
		DoubleAnimation::OnPropertyChanged (prop);
		return;
	}

	if (prop == KeyFramesProperty) {
		DoubleKeyFrameCollection *newcol = GetValue (prop)->AsDoubleKeyFrameCollection();

		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	}

	NotifyAttachersOfPropertyChange (prop);
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
		key_start_time = 0;
	}
	else {
		/* start at the previous keyframe's target value */
		baseValue = new Value(*previous_keyframe->GetValue ());
		/* XXX DoubleKeyFrame::Value is nullable */
		key_start_time = previous_keyframe->resolved_keytime;
	}

	double progress;

	if (current_time == key_end_time) {
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
	return current_keyframe->InterpolateValue (baseValue, progress);
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
ColorAnimationUsingKeyFrames::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::COLORANIMATIONUSINGKEYFRAMES) {
		ColorAnimation::OnPropertyChanged (prop);
		return;
	}

	if (prop == KeyFramesProperty) {
		ColorKeyFrameCollection *newcol = GetValue (prop)->AsColorKeyFrameCollection();

		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	}

	NotifyAttachersOfPropertyChange (prop);
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

	current_keyframe = (ColorKeyFrame*)key_frames->GetKeyFrameForTime (current_time, (KeyFrame**)keyframep);
	if (current_keyframe == NULL)
		return NULL; /* XXX */

	TimeSpan key_end_time = current_keyframe->resolved_keytime;
	TimeSpan key_start_time;

	if (previous_keyframe == NULL) {
		/* the first keyframe, start at the animation's base value */
		baseValue = defaultOriginValue;
		key_start_time = 0;
	}
	else {
		/* start at the previous keyframe's target value */
		baseValue = new Value(*previous_keyframe->GetValue ());
		/* XXX ColorKeyFrame::Value is nullable */
		key_start_time = previous_keyframe->resolved_keytime;
	}

	double progress;

	if (current_time == key_end_time) {
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
	return current_keyframe->InterpolateValue (baseValue, progress);
}

Duration
ColorAnimationUsingKeyFrames::GetNaturalDurationCore (Clock *clock)
{
	ColorKeyFrameCollection *key_frames = GetValue (ColorAnimationUsingKeyFrames::KeyFramesProperty)->AsColorKeyFrameCollection ();
	
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

ColorAnimationUsingKeyFrames *
color_animation_using_key_frames_new (void)
{
	return new ColorAnimationUsingKeyFrames ();
}




DependencyProperty* PointAnimationUsingKeyFrames::KeyFramesProperty;

PointAnimationUsingKeyFrames::PointAnimationUsingKeyFrames()
{
	this->SetValue (PointAnimationUsingKeyFrames::KeyFramesProperty, Value (new PointKeyFrameCollection ()));
}

PointAnimationUsingKeyFrames::~PointAnimationUsingKeyFrames ()
{
}

void
PointAnimationUsingKeyFrames::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop->type != Type::POINTANIMATIONUSINGKEYFRAMES) {
		PointAnimation::OnPropertyChanged (prop);
		return;
	}

	if (prop == KeyFramesProperty) {
		PointKeyFrameCollection *newcol = GetValue (prop)->AsPointKeyFrameCollection();

		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	}

	NotifyAttachersOfPropertyChange (prop);
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

	current_keyframe = (PointKeyFrame*)key_frames->GetKeyFrameForTime (current_time, (KeyFrame**)keyframep);
	if (current_keyframe == NULL)
		return NULL; /* XXX */

	TimeSpan key_end_time = current_keyframe->resolved_keytime;
	TimeSpan key_start_time;

	if (previous_keyframe == NULL) {
		/* the first keyframe, start at the animation's base value */
		baseValue = defaultOriginValue;
		key_start_time = 0;
	}
	else {
		/* start at the previous keyframe's target value */
		baseValue = new Value(*previous_keyframe->GetValue ());
		/* XXX PointKeyFrame::Value is nullable */
		key_start_time = previous_keyframe->resolved_keytime;
	}

	double progress;

	if (current_time == key_end_time) {
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
	return current_keyframe->InterpolateValue (baseValue, progress);
}

Duration
PointAnimationUsingKeyFrames::GetNaturalDurationCore (Clock* clock)
{
	PointKeyFrameCollection *key_frames = GetValue (PointAnimationUsingKeyFrames::KeyFramesProperty)->AsPointKeyFrameCollection ();
	
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
 	KeyFrame::KeyTimeProperty = DependencyObject::Register (Type::KEYFRAME, "KeyTime", new Value(KeyTime::Uniform));
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

	/* lookup events */
	Storyboard::CompletedEvent = Type::Find(Type::STORYBOARD)->LookupEvent ("Completed");
}

void
animation_destroy ()
{
}

/* The nullable setters/getters for the various animation types */
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
