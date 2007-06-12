#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>

#include "animation.h"

#define LERP(f,t,p) ((f) + ((t) - (f)) * (p))




AnimationStorage::AnimationStorage (AnimationClock *clock, Animation/*Timeline*/ *timeline,
				    DependencyObject *targetobj, DependencyProperty *targetprop)
  : clock (clock),
    timeline (timeline),
    targetobj (targetobj),
    targetprop (targetprop)
  
{
	clock->events->AddHandler ("CurrentTimeInvalidated", update_property_value, this);

	baseValue = new Value(*targetobj->GetValue (targetprop));
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
	delete current_value;
}


AnimationClock::AnimationClock (Animation/*Timeline*/ *timeline)
  : timeline(timeline),
    Clock (timeline)
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

		//printf ("Got %s %s\n", targetProperty, targetName);
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
Storyboard::Seek (TimeSpan timespan)
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
double_animation_new ()
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
point_animation_new ()
{
	return new PointAnimation ();
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



void
KeyFrameCollection::Add (void *data)
{
	Value *value = (Value*)data;
	Collection::Add (value->AsKeyFrame());
}

void
KeyFrameCollection::Remove (void *data)
{
	Value *value = (Value*)data;
	Collection::Remove (value->AsKeyFrame());
}

KeyFrameCollection *
key_frame_collection_new ()
{
	return new KeyFrameCollection ();
}

DependencyProperty* DoubleKeyFrame::ValueProperty;

DoubleKeyFrame::DoubleKeyFrame ()
{
}


DependencyProperty* ColorKeyFrame::ValueProperty;

ColorKeyFrame::ColorKeyFrame ()
{
}


DependencyProperty* PointKeyFrame::ValueProperty;

PointKeyFrame::PointKeyFrame ()
{
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

DiscreteDoubleKeyFrame*
discrete_double_key_frame_new ()
{
	return new DiscreteDoubleKeyFrame ();
}



Value*
DiscreteColorKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	Color *to = GetValue();
	/* XXX GetValue can return NULL */

	if (keyFrameProgress == 1.0)
		return new Value(*to);
	else
		return new Value (*baseValue->AsColor());
}

DiscreteColorKeyFrame*
discrete_color_key_frame_new ()
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

DiscretePointKeyFrame*
discrete_point_key_frame_new ()
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

LinearDoubleKeyFrame*
linear_double_key_frame_new ()
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

LinearColorKeyFrame*
linear_color_key_frame_new ()
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

LinearPointKeyFrame*
linear_point_key_frame_new ()
{
	return new LinearPointKeyFrame ();
}




DependencyProperty* DoubleAnimationUsingKeyFrames::KeyFramesProperty;

DoubleAnimationUsingKeyFrames::DoubleAnimationUsingKeyFrames()
{
	key_frames = NULL;
	KeyFrameCollection *c = new KeyFrameCollection ();

	this->SetValue (DoubleAnimationUsingKeyFrames::KeyFramesProperty, Value (c));

	// Ensure that the callback OnPropertyChanged was called.
	g_assert (c == key_frames);
}

void
DoubleAnimationUsingKeyFrames::OnPropertyChanged (DependencyProperty *prop)
{
	DoubleAnimation::OnPropertyChanged (prop);

	if (prop == KeyFramesProperty){
		// The new value has already been set, so unref the old collection

		KeyFrameCollection *newcol = GetValue (prop)->AsKeyFrameCollection();

		if (newcol != key_frames){
			if (key_frames){
				for (GSList *l = key_frames->list; l != NULL; l = l->next){
					DependencyObject *dob = (DependencyObject *) l->data;
					
					base_unref (dob);
				}
				base_unref (key_frames);
				g_slist_free (key_frames->list);
			}

			key_frames = newcol;
			if (key_frames->closure)
				printf ("Warning we attached a property that was already attached\n");
			key_frames->closure = this;
			
			base_ref (key_frames);
		}
	}
}

void
DoubleAnimationUsingKeyFrames::AddKeyFrame (DoubleKeyFrame *frame)
{
	KeyFrameCollection *keyframes = GetValue (DoubleAnimationUsingKeyFrames::KeyFramesProperty)->AsKeyFrameCollection ();

	Value fv = Value(frame);
	keyframes->Add (&fv);
}

void
DoubleAnimationUsingKeyFrames::RemoveKeyFrame (DoubleKeyFrame *frame)
{
}

Value*
DoubleAnimationUsingKeyFrames::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					       AnimationClock* animationClock)
{
	/* current segment info */
	TimeSpan current_time = animationClock->GetCurrentTime();
	DoubleKeyFrame *current_keyframe;
	DoubleKeyFrame *previous_keyframe = NULL;
	Value *baseValue;

	TimeSpan key_end_time;

	/* figure out what segment to use (this list needs to be sorted) */
	GSList *prev = NULL;
	for (GSList *l = key_frames->list; l; prev = l, l = l->next) {
		DoubleKeyFrame *keyframe = (DoubleKeyFrame*)l->data;

		key_end_time = keyframe->GetKeyTime()->GetTimeSpan();

		if (key_end_time >= current_time) {
			current_keyframe = keyframe;

			if (prev)
				previous_keyframe = (DoubleKeyFrame*)prev->data;

			break;
		}

	}

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

	TimeSpan key_duration = key_end_time - key_start_time;
	double progress = (double)(current_time - key_start_time) / key_duration;

	/* get the current value out of that segment */
	
	return current_keyframe->InterpolateValue (baseValue, progress);
}

Duration
DoubleAnimationUsingKeyFrames::GetNaturalDurationCore (Clock* clock)
{
	TimeSpan ts = 0;
	Duration d = Duration::Automatic;

	for (GSList *l = key_frames->list; l; l = l->next) {
		DoubleKeyFrame *dkf = (DoubleKeyFrame*)l->data;
		TimeSpan dk_ts = dkf->GetKeyTime()->GetTimeSpan ();

		if (dk_ts > ts) {
			ts = dk_ts;
			d = Duration (ts);
		}
	}

	return d;
}

DoubleAnimationUsingKeyFrames*
double_animation_using_key_frames_new ()
{
	return new DoubleAnimationUsingKeyFrames ();
}



DependencyProperty* ColorAnimationUsingKeyFrames::KeyFramesProperty;

ColorAnimationUsingKeyFrames::ColorAnimationUsingKeyFrames()
{
	key_frames = NULL;
	KeyFrameCollection *c = new KeyFrameCollection ();

	this->SetValue (ColorAnimationUsingKeyFrames::KeyFramesProperty, Value (c));

	// Ensure that the callback OnPropertyChanged was called.
	g_assert (c == key_frames);
}

void
ColorAnimationUsingKeyFrames::OnPropertyChanged (DependencyProperty *prop)
{
	ColorAnimation::OnPropertyChanged (prop);

	if (prop == KeyFramesProperty){
		// The new value has already been set, so unref the old collection

		KeyFrameCollection *newcol = GetValue (prop)->AsKeyFrameCollection();

		if (newcol != key_frames){
			if (key_frames){
				for (GSList *l = key_frames->list; l != NULL; l = l->next){
					DependencyObject *dob = (DependencyObject *) l->data;
					
					base_unref (dob);
				}
				base_unref (key_frames);
				g_slist_free (key_frames->list);
			}

			key_frames = newcol;
			if (key_frames->closure)
				printf ("Warning we attached a property that was already attached\n");
			key_frames->closure = this;
			
			base_ref (key_frames);
		}
	}
}

void
ColorAnimationUsingKeyFrames::AddKeyFrame (ColorKeyFrame *frame)
{
	Value fv = Value(frame);
	key_frames->Add (&fv);
}

void
ColorAnimationUsingKeyFrames::RemoveKeyFrame (ColorKeyFrame *frame)
{
}

Value*
ColorAnimationUsingKeyFrames::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					       AnimationClock* animationClock)
{
	/* current segment info */
	TimeSpan current_time = animationClock->GetCurrentTime();
	ColorKeyFrame *current_keyframe;
	ColorKeyFrame *previous_keyframe = NULL;
	Value *baseValue;

	TimeSpan key_end_time;

	/* figure out what segment to use (this list needs to be sorted) */
	GSList *prev = NULL;
	for (GSList *l = key_frames->list; l; prev = l, l = l->next) {
		ColorKeyFrame *keyframe = (ColorKeyFrame*)l->data;

		key_end_time = keyframe->GetKeyTime()->GetTimeSpan();

		if (key_end_time >= current_time) {
			current_keyframe = keyframe;

			if (prev)
				previous_keyframe = (ColorKeyFrame*)prev->data;

			break;
		}

	}

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

	TimeSpan key_duration = key_end_time - key_start_time;
	double progress = (double)(current_time - key_start_time) / key_duration;

	/* get the current value out of that segment */
	
	return current_keyframe->InterpolateValue (baseValue, progress);
}

Duration
ColorAnimationUsingKeyFrames::GetNaturalDurationCore (Clock* clock)
{
	TimeSpan ts = 0;
	Duration d = Duration::Automatic;

	for (GSList *l = key_frames->list; l; l = l->next) {
		ColorKeyFrame *dkf = (ColorKeyFrame*)l->data;
		TimeSpan dk_ts = dkf->GetKeyTime()->GetTimeSpan ();

		if (dk_ts > ts) {
			ts = dk_ts;
			d = Duration (ts);
		}
	}

	return d;
}


ColorAnimationUsingKeyFrames*
color_animation_using_key_frames_new ()
{
	return new ColorAnimationUsingKeyFrames ();
}




DependencyProperty* PointAnimationUsingKeyFrames::KeyFramesProperty;

PointAnimationUsingKeyFrames::PointAnimationUsingKeyFrames()
{
	key_frames = NULL;
	KeyFrameCollection *c = new KeyFrameCollection ();

	this->SetValue (PointAnimationUsingKeyFrames::KeyFramesProperty, Value (c));

	// Ensure that the callback OnPropertyChanged was called.
	g_assert (c == key_frames);
}

void
PointAnimationUsingKeyFrames::OnPropertyChanged (DependencyProperty *prop)
{
	PointAnimation::OnPropertyChanged (prop);

	if (prop == KeyFramesProperty){
		// The new value has already been set, so unref the old collection

		KeyFrameCollection *newcol = GetValue (prop)->AsKeyFrameCollection();

		if (newcol != key_frames){
			if (key_frames){
				for (GSList *l = key_frames->list; l != NULL; l = l->next){
					DependencyObject *dob = (DependencyObject *) l->data;
					
					base_unref (dob);
				}
				base_unref (key_frames);
				g_slist_free (key_frames->list);
			}

			key_frames = newcol;
			if (key_frames->closure)
				printf ("Warning we attached a property that was already attached\n");
			key_frames->closure = this;
			
			base_ref (key_frames);
		}
	}
}

void
PointAnimationUsingKeyFrames::AddKeyFrame (PointKeyFrame *frame)
{
	KeyFrameCollection *keyframes = GetValue (PointAnimationUsingKeyFrames::KeyFramesProperty)->AsKeyFrameCollection ();

	Value fv = Value(frame);
	keyframes->Add (&fv);
}

void
PointAnimationUsingKeyFrames::RemoveKeyFrame (PointKeyFrame *frame)
{
}

Value*
PointAnimationUsingKeyFrames::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					       AnimationClock* animationClock)
{
	/* current segment info */
	TimeSpan current_time = animationClock->GetCurrentTime();
	PointKeyFrame *current_keyframe;
	PointKeyFrame *previous_keyframe = NULL;
	Value *baseValue;

	TimeSpan key_end_time;

	/* figure out what segment to use (this list needs to be sorted) */
	GSList *prev = NULL;
	for (GSList *l = key_frames->list; l; prev = l, l = l->next) {
		PointKeyFrame *keyframe = (PointKeyFrame*)l->data;

		key_end_time = keyframe->GetKeyTime()->GetTimeSpan();

		if (key_end_time >= current_time) {
			current_keyframe = keyframe;

			if (prev)
				previous_keyframe = (PointKeyFrame*)prev->data;

			break;
		}

	}

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

	TimeSpan key_duration = key_end_time - key_start_time;
	double progress = (double)(current_time - key_start_time) / key_duration;

	/* get the current value out of that segment */
	
	return current_keyframe->InterpolateValue (baseValue, progress);
}

Duration
PointAnimationUsingKeyFrames::GetNaturalDurationCore (Clock* clock)
{
	TimeSpan ts = 0;
	Duration d = Duration::Automatic;

	for (GSList *l = key_frames->list; l; l = l->next) {
		PointKeyFrame *dkf = (PointKeyFrame*)l->data;
		TimeSpan dk_ts = dkf->GetKeyTime()->GetTimeSpan ();

		if (dk_ts > ts) {
			ts = dk_ts;
			d = Duration (ts);
		}
	}

	return d;
}


PointAnimationUsingKeyFrames*
point_animation_using_key_frames_new ()
{
	return new PointAnimationUsingKeyFrames ();
}





RepeatBehavior RepeatBehavior::Forever (RepeatBehavior::FOREVER);
Duration Duration::Automatic (Duration::AUTOMATIC);
Duration Duration::Forever (Duration::FOREVER);
KeyTime KeyTime::Paced (KeyTime::PACED);
KeyTime KeyTime::Uniform (KeyTime::UNIFORM);

void 
animation_init ()
{
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

	/* KeyFrame properties */
 	KeyFrame::KeyTimeProperty = DependencyObject::Register (Value::KEYFRAME, "KeyTime", new Value(KeyTime::Uniform));
 	DoubleKeyFrame::ValueProperty = DependencyObject::Register (Value::DOUBLEKEYFRAME, "Value", Value::DOUBLE);
 	PointKeyFrame::ValueProperty = DependencyObject::Register (Value::POINTKEYFRAME, "Value", Value::POINT);
 	ColorKeyFrame::ValueProperty = DependencyObject::Register (Value::COLORKEYFRAME, "Value", Value::COLOR);

	/* KeyFrame animation properties */
	ColorAnimationUsingKeyFrames::KeyFramesProperty = DependencyObject::Register (Value::COLORANIMATIONUSINGKEYFRAMES, "KeyFrames", Value::KEYFRAME_COLLECTION);
	DoubleAnimationUsingKeyFrames::KeyFramesProperty = DependencyObject::Register (Value::DOUBLEANIMATIONUSINGKEYFRAMES, "KeyFrames", Value::KEYFRAME_COLLECTION);
	PointAnimationUsingKeyFrames::KeyFramesProperty = DependencyObject::Register (Value::POINTANIMATIONUSINGKEYFRAMES, "KeyFrames", Value::KEYFRAME_COLLECTION);
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
