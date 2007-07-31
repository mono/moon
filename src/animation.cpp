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

#include "animation.h"
#include "color.h"

#define LERP(f,t,p) ((f) + ((t) - (f)) * (p))




AnimationStorage::AnimationStorage (AnimationClock *clock, Animation/*Timeline*/ *timeline,
				    DependencyObject *targetobj, DependencyProperty *targetprop)
  : clock (clock),
    timeline (timeline),
    targetobj (targetobj),
    targetprop (targetprop)
  
{
	clock->AddHandler (clock->CurrentTimeInvalidatedEvent, update_property_value, this);

	baseValue = new Value(*targetobj->GetValue (targetprop));

	targetobj->ref();
}

void
AnimationStorage::update_property_value (EventObject *, gpointer, gpointer closure)
{
	((AnimationStorage*)closure)->UpdatePropertyValue ();
}

void
AnimationStorage::UpdatePropertyValue ()
{
	Value *current_value = clock->GetCurrentValue (baseValue, NULL/*XXX*/);
	targetobj->SetValue (targetprop, *current_value);
	delete current_value;
}

AnimationStorage::~AnimationStorage ()
{
	if (baseValue)
		delete baseValue;

	targetobj->unref();
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
  : root_clock (NULL)
{
	CompletedEvent = RegisterEvent ("Completed");
}

void
Storyboard::HookupAnimationsRecurse (Clock *clock)
{
	switch (clock->GetObjectType ()) {
	case Type::ANIMATIONCLOCK: {
		AnimationClock *ac = (AnimationClock*)clock;

		char *targetProperty = Storyboard::GetTargetProperty (ac->GetTimeline());
		if (!targetProperty)
			return;

		char *targetName = Storyboard::GetTargetName (ac->GetTimeline());
		if (!targetName)
			return;

		//printf ("Got %s %s\n", targetProperty, targetName);
		DependencyObject *o = FindName (targetName);
		if (!o)
			return;

		DependencyProperty *prop = resolve_property_path (&o, targetProperty);
		if (!prop)
			return;


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
	/* destroy the clock hierarchy and recreate it to restart.
	   easier than making Begin work again with the existing clock
	   hierarchy */
	if (root_clock) {
		TimeManager::Instance()->RemoveChild (root_clock);
		root_clock->unref ();
	}

	// This creates the clock tree for the hierarchy.  if a
	// Timeline A is a child of TimelineGroup B, then Clock cA
	// will be a child of ClockGroup cB.
	root_clock = CreateClock ();
	root_clock->ref ();
	root_clock->AddHandler (root_clock->CompletedEvent, invoke_completed, this);

	// walk the clock tree hooking up the correct properties and
	// creating AnimationStorage's for AnimationClocks.
	HookupAnimationsRecurse (root_clock);

	// hack to make storyboards work..  we need to attach them to
	// TimeManager's list of clocks
	TimeManager::Instance()->AddChild (root_clock);
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
			       char *targetProperty)
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
			   char *targetName)
{
	o->SetValue (Storyboard::TargetNameProperty, Value (targetName));
}

char*
Storyboard::GetTargetName (DependencyObject *o)
{
	Value *v = o->GetValue (Storyboard::TargetNameProperty);
	return v == NULL ? NULL : v->AsString();
}

Storyboard::~Storyboard ()
{
	printf ("Clock %p (ref=%d)\n", root_clock, root_clock->refcount);
	Stop ();
	TimeManager::Instance()->RemoveChild (root_clock);
	printf ("Unrefing Clock %p (ref=%d)\n", root_clock, root_clock->refcount);
	root_clock->unref ();
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

const double TOLERANCE = 0.0000001;  // Application specific tolerance

static double q1, q2, q3, q4, q5;      // These belong to balf()


//---------------------------------------------------------------------------
static double
balf(double t)                   // Bezier Arc Length Function
{
	double result = q5 + t*(q4 + t*(q3 + t*(q2 + t*q1)));
	result = sqrt(result);
	return result;
}

//---------------------------------------------------------------------------
// NOTES:       TOLERANCE is a maximum error ratio
//                      if n_limit isn't a power of 2 it will be act like the next higher
//                      power of two.
static double
Simpson (double (*f)(double),
	 double a,
	 double b,
	 int n_limit)
{
	int n = 1;
	double multiplier = (b - a)/6.0;
	double endsum = f(a) + f(b);
	double interval = (b - a)/2.0;
	double asum = 0;
	double bsum = f(a + interval);
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
			bsum += f(t);
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

	q1 = 9.0*(sqr(k1.x) + sqr(k1.y));
	q2 = 12.0*(k1.x*k2.x + k1.y*k2.y);
	q3 = 3.0*(k1.x*k3.x + k1.y*k3.y) + 4.0*(sqr(k2.x) + sqr(k2.y));
	q4 = 4.0*(k2.x*k3.x + k2.y*k3.y);
	q5 = sqr(k3.x) + sqr(k3.y);

	return Simpson(balf, 0, t, 1024);
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


class KeyFrameNode : public List::Node {
public:
	KeyFrame *key_frame;
	
	KeyFrameNode (KeyFrame *kf) { key_frame = kf; }
};

static int
KeyFrameNodeComparer (List::Node *kfn1, List::Node *kfn2)
{
	// Assumes timespan keytimes only
	TimeSpan ts1 = ((KeyFrameNode *) kfn1)->key_frame->GetKeyTime()->GetTimeSpan ();
	TimeSpan ts2 = ((KeyFrameNode *) kfn2)->key_frame->GetKeyTime()->GetTimeSpan ();
	TimeSpan tsdiff = ts1 - ts2;
	
	if (tsdiff == 0)
		return 0;
	else if (tsdiff < 0)
		return -1;
	else
		return 1;
}

static bool
KeyFrameNodeFinder (List::Node *kfn, void *data)
{
	return ((KeyFrameNode *) kfn)->key_frame == (KeyFrame *) data;
}

KeyFrameCollection::KeyFrameCollection ()
{
	sorted_list = new List ();
}

KeyFrameCollection::~KeyFrameCollection ()
{
	sorted_list->Clear (true);
	delete sorted_list;
}

void
KeyFrameCollection::Add (DependencyObject *data)
{
	KeyFrameNode *kfn = new KeyFrameNode ((KeyFrame *) data);
	
	Collection::Add (data);
	
	sorted_list->InsertSorted (kfn, KeyFrameNodeComparer);
}

void
KeyFrameCollection::Insert (int index, DependencyObject *data)
{
	KeyFrameNode *kfn = new KeyFrameNode ((KeyFrame *) data);
	
	Collection::Insert (index, data);
	
	sorted_list->InsertSorted (kfn, KeyFrameNodeComparer);
}

bool
KeyFrameCollection::Remove (DependencyObject *data)
{
	KeyFrame *kf = (KeyFrame *) data;
	
	sorted_list->Remove (KeyFrameNodeFinder, kf);
	return Collection::Remove (kf);
}

void
KeyFrameCollection::Clear ()
{
	sorted_list->Clear (true);
	Collection::Clear ();
}

KeyFrame*
KeyFrameCollection::GetKeyFrameForTime (TimeSpan t, KeyFrame **prev_frame)
{
	KeyFrame *current_keyframe = NULL;
	KeyFrame *previous_keyframe = NULL;
	List::Node *cur, *prev = NULL;
	
	/* figure out what segment to use (this assumes the list is sorted) */
	for (cur = sorted_list->First (); cur; prev = cur, cur = cur->Next ()) {
		KeyFrame *keyframe = ((KeyFrameNode *) cur)->key_frame;
		TimeSpan key_end_time = keyframe->GetKeyTime()->GetTimeSpan();
		
		if (key_end_time >= t) {
			current_keyframe = keyframe;
			
			if (prev)
				previous_keyframe = ((KeyFrameNode *) prev)->key_frame;
			
			break;
		}

	}
	
	if (prev_frame != NULL)
		*prev_frame = previous_keyframe;
	
	return current_keyframe;
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
	/* XXX GetValue can return NULL */

	if (keyFrameProgress == 1.0)
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
	/* XXX GetValue can return NULL */

	//printf ("DiscreteColorKeyFrame::InterpolateValue (progress = %f)\n", keyFrameProgress);


	if (keyFrameProgress == 1.0)
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
	/* XXX GetValue can return NULL */

	if (keyFrameProgress == 1.0)
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
	/* XXX GetValue can return NULL */

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
	/* XXX GetValue can return NULL */

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
	/* XXX GetValue can return NULL */

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
	/* XXX GetValue can return NULL */

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
	/* XXX GetValue can return NULL */

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
	/* XXX GetValue can return NULL */

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


DependencyProperty* DoubleAnimationUsingKeyFrames::KeyFramesProperty;

DoubleAnimationUsingKeyFrames::DoubleAnimationUsingKeyFrames()
{
	this->SetValue (DoubleAnimationUsingKeyFrames::KeyFramesProperty, Value (new DoubleKeyFrameCollection ()));
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

	NotifyAttacheesOfPropertyChange (prop);
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
	if (current_keyframe == NULL)
		return NULL; /* XXX */

	TimeSpan key_end_time = current_keyframe->GetKeyTime()->GetTimeSpan(); /* XXX this assumes a timespan keyframe */
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
		key_start_time = previous_keyframe->GetKeyTime()->GetTimeSpan ();
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
DoubleAnimationUsingKeyFrames::GetNaturalDurationCore (Clock* clock)
{
	DoubleKeyFrameCollection *key_frames = GetValue (DoubleAnimationUsingKeyFrames::KeyFramesProperty)->AsDoubleKeyFrameCollection ();
	Duration d = Duration::Automatic;
	Collection::Node *node;
	TimeSpan ts = 0;
	
	node = (Collection::Node *) key_frames->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->Next ()) {
		DoubleKeyFrame *dkf = (DoubleKeyFrame *) node->obj;
		TimeSpan dk_ts = dkf->GetKeyTime()->GetTimeSpan ();
		
		if (dk_ts > ts) {
			ts = dk_ts;
			d = Duration (ts);
		}
	}
	
	return d;
}

DoubleAnimationUsingKeyFrames *
double_animation_using_key_frames_new (void)
{
	return new DoubleAnimationUsingKeyFrames ();
}



DependencyProperty* ColorAnimationUsingKeyFrames::KeyFramesProperty;

ColorAnimationUsingKeyFrames::ColorAnimationUsingKeyFrames()
{
	this->SetValue (ColorAnimationUsingKeyFrames::KeyFramesProperty, Value (new ColorKeyFrameCollection ()));
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

	NotifyAttacheesOfPropertyChange (prop);
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

	TimeSpan key_end_time = current_keyframe->GetKeyTime()->GetTimeSpan(); /* XXX this assumes a timespan keyframe */
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
		key_start_time = previous_keyframe->GetKeyTime()->GetTimeSpan ();
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
	Duration d = Duration::Automatic;
	Collection::Node *node;
	TimeSpan ts = 0;
	
	node = (Collection::Node *) key_frames->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->Next ()) {
		ColorKeyFrame *dkf = (ColorKeyFrame *) node->obj;
		TimeSpan dk_ts = dkf->GetKeyTime()->GetTimeSpan ();
		
		if (dk_ts > ts) {
			ts = dk_ts;
			d = Duration (ts);
		}
	}
	
	return d;
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

	NotifyAttacheesOfPropertyChange (prop);
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

	TimeSpan key_end_time = current_keyframe->GetKeyTime()->GetTimeSpan(); /* XXX this assumes a timespan keyframe */
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
		key_start_time = previous_keyframe->GetKeyTime()->GetTimeSpan ();
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
	Duration d = Duration::Automatic;
	Collection::Node *node;
	TimeSpan ts = 0;
	
	node = (Collection::Node *) key_frames->list->First ();
	for ( ; node != NULL; node = (Collection::Node *) node->Next ()) {
		PointKeyFrame *dkf = (PointKeyFrame *) node->obj;
		TimeSpan dk_ts = dkf->GetKeyTime()->GetTimeSpan ();
		
		if (dk_ts > ts) {
			ts = dk_ts;
			d = Duration (ts);
		}
	}
	
	return d;
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
									     NULL, Type::STRING, true);
	Storyboard::TargetNameProperty     = DependencyObject::RegisterFull (Type::STORYBOARD, "TargetName", 
									     NULL, Type::STRING, true);

	/* BeginStoryboard properties */
	BeginStoryboard::StoryboardProperty = DependencyObject::Register (Type::BEGINSTORYBOARD, "Storyboard",	Type::STORYBOARD);

	/* KeyFrame properties */
 	KeyFrame::KeyTimeProperty = DependencyObject::Register (Type::KEYFRAME, "KeyTime", new Value(KeyTime::Uniform));
 	DoubleKeyFrame::ValueProperty = DependencyObject::RegisterNullable (Type::DOUBLEKEYFRAME, "Value", Type::DOUBLE);
 	PointKeyFrame::ValueProperty = DependencyObject::RegisterNullable (Type::POINTKEYFRAME, "Value", Type::POINT);
 	ColorKeyFrame::ValueProperty = DependencyObject::RegisterNullable (Type::COLORKEYFRAME, "Value", Type::COLOR);

	/* Spline keyframe properties */
	SplineDoubleKeyFrame::KeySplineProperty = DependencyObject::Register (Type::SPLINEDOUBLEKEYFRAME, "KeySpline", new Value (new KeySpline (0, 0, 1, 0)));
 	SplineColorKeyFrame::KeySplineProperty = DependencyObject::Register (Type::SPLINECOLORKEYFRAME, "KeySpline", new Value (new KeySpline (0, 0, 1, 0)));
 	SplinePointKeyFrame::KeySplineProperty = DependencyObject::Register (Type::SPLINEPOINTKEYFRAME, "KeySpline", new Value (new KeySpline (0, 0, 1, 0)));

	/* KeyFrame animation properties */
	ColorAnimationUsingKeyFrames::KeyFramesProperty = DependencyObject::Register (Type::COLORANIMATIONUSINGKEYFRAMES, "KeyFrames", Type::COLORKEYFRAME_COLLECTION);
	DoubleAnimationUsingKeyFrames::KeyFramesProperty = DependencyObject::Register (Type::DOUBLEANIMATIONUSINGKEYFRAMES, "KeyFrames", Type::DOUBLEKEYFRAME_COLLECTION);
	PointAnimationUsingKeyFrames::KeyFramesProperty = DependencyObject::Register (Type::POINTANIMATIONUSINGKEYFRAMES, "KeyFrames", Type::POINTKEYFRAME_COLLECTION);
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
