/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * animation.cpp: Animation engine
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
#include <math.h>

#include "application.h"
#include "timemanager.h"
#include "animation.h"

#include "color.h"
#include "runtime.h"
#include "utils.h"

#define LERP(f,t,p) ((f) + ((t) - (f)) * (p))

#define KEYSPLINE_PRECISION_LEVEL 4

// This is 2 to power of KEYSPLINE_PRECISION_LEVEL
#define KEYSPLINE_TOTAL_COUNT 16

KeyTime KeyTime::Paced (KeyTime::PACED);
KeyTime KeyTime::Uniform (KeyTime::UNIFORM);

/*
AnimationStorage sits between an DependencyObject and a DependencyProperty.
The AnimationClock creates a storage for DO, and the storage attaches itself to
a DP. The DP keeps a list (hashtable) keyed by the DO pointer.

The storage can be deleted when
- The DP when it's clearing its list of storage objects. This happens when the DP
is destroyed.
- The AnimationClock is destroyed.
- A new storage is created for the same dependencyobject. In this case, the previous
storage that was on the list for the DO is replaced.

This means the storage can disappear but both the DependencyProperty and the
DependencyObject (and AnimationClock) that it referenced might still be
alive. It is very important that when the storage is removed, the
corresponding references to it be cleared, too - this means removing it
from the DP list when it is deleted by the clock, or removing it from the clock
when it's deleted by the DP, as well as clearing any event handlers that could
still fire up after it's dead.
*/


AnimationStorage::AnimationStorage (AnimationClock *clock, Animation *timeline,
				    DependencyObject *targetobj, DependencyProperty *targetprop)
: baseValue(NULL), stopValue(NULL), disabled(false)
{
	this->clock = clock;
	this->timeline = timeline;
	this->targetobj = targetobj;
	this->targetprop = targetprop;

	AttachUpdateHandler ();
	AttachTargetHandler ();

	AnimationStorage *prev_storage = targetobj->AttachAnimationStorage (targetprop, this);

	baseValue = targetobj->GetValue (targetprop);
	if (baseValue)
		baseValue = new Value(*baseValue);
	else
		baseValue = new Value (targetprop->GetPropertyType ());

	if (prev_storage) {
		Value *v = prev_storage->GetResetValue ();
		stopValue = new Value (*v);
	}
}

bool
AnimationStorage::IsCurrentStorage ()
{
	if (targetobj == NULL || targetprop == NULL)
		return false;

	if (targetobj->GetAnimationStorageFor (targetprop) == this)
		return true;

	return false;
}


void
AnimationStorage::SwitchTarget (DependencyObject *target)
{
	bool wasDisabled = disabled;
	if (!disabled)
		Disable ();
	targetobj = target;
	if (!wasDisabled) {
		AttachTargetHandler ();
		AttachUpdateHandler ();
	}
	disabled = wasDisabled;
}

void
AnimationStorage::Enable ()
{
	if (!disabled)
		return;

	AttachTargetHandler ();
	AttachUpdateHandler ();
	disabled = false;
	UpdatePropertyValue ();
}

void
AnimationStorage::Disable ()
{
	DetachUpdateHandler ();
	DetachTargetHandler ();
	disabled = true;
}

void
AnimationStorage::Stop ()
{
	ResetPropertyValue ();
}

Value*
AnimationStorage::GetResetValue ()
{
	if (stopValue)
		return stopValue;
	else
		return baseValue;
}

void
AnimationStorage::SetStopValue (Value *value)
{
	if (stopValue)
		delete stopValue;

	if (value)
		stopValue = new Value (*value);
	else
		stopValue = NULL;
}
// End of public methods

// Private methods
void
AnimationStorage::target_object_destroyed (EventObject *, EventArgs *, gpointer closure)
{
	((AnimationStorage*)closure)->TargetObjectDestroyed ();
}

void
AnimationStorage::TargetObjectDestroyed ()
{
	DetachUpdateHandler ();
	targetobj = NULL;
}


void
AnimationStorage::update_property_value (EventObject *, EventArgs *, gpointer closure)
{
	((AnimationStorage*)closure)->UpdatePropertyValue ();
}

void
AnimationStorage::UpdatePropertyValue ()
{
	if (!targetobj) return;

	Value *current_value = clock->GetCurrentValue (baseValue, stopValue ? stopValue : baseValue);
	if (current_value != NULL && timeline->GetTimelineStatus () == Timeline::TIMELINE_STATUS_OK) {
		Applier *applier = clock->GetTimeManager ()->GetApplier ();
		applier->AddPropertyChange (targetobj, targetprop, new Value (*current_value), APPLIER_PRECEDENCE_ANIMATION);
	}

	delete current_value;
}

void
AnimationStorage::ResetPropertyValue ()
{
	if (disabled) return;

	if (targetobj == NULL || targetprop == NULL)
		return;

	if (timeline->GetTimelineStatus () != Timeline::TIMELINE_STATUS_OK)
		return;

	Applier *applier = clock->GetTimeManager ()->GetApplier ();

	if (applier)
		applier->AddPropertyChange (targetobj, targetprop,
			    new Value (*GetResetValue ()),
			    APPLIER_PRECEDENCE_ANIMATION_RESET);
}

void
AnimationStorage::DetachFromProperty ()
{
	if (targetobj == NULL || targetprop == NULL)
		return;
	targetobj->DetachAnimationStorage (targetprop, this);
}

void
AnimationStorage::AttachUpdateHandler ()
{
	if (!clock) return;
	clock->AddHandler (Clock::CurrentTimeInvalidatedEvent, update_property_value, this);
}


void
AnimationStorage::DetachUpdateHandler ()
{
	if (disabled) return;
	if (!clock) return;
	clock->RemoveHandler (Clock::CurrentTimeInvalidatedEvent, update_property_value, this);
}

void
AnimationStorage::AttachTargetHandler ()
{
	if (!targetobj) return;
	targetobj->AddHandler (EventObject::DestroyedEvent, target_object_destroyed, this);
}

void
AnimationStorage::DetachTargetHandler ()
{
	if (disabled) return;
	if (!targetobj) return;
	targetobj->RemoveHandler (EventObject::DestroyedEvent, target_object_destroyed, this);
}

AnimationStorage::~AnimationStorage ()
{
	DetachTargetHandler ();
	DetachUpdateHandler ();
	DetachFromProperty ();

	if (clock != NULL)
		clock->DetachStorage ();

	if (baseValue) {
		delete baseValue;
		baseValue = NULL;
	}

	if (stopValue) {
		delete stopValue;
		stopValue = NULL;
	}
}

AnimationClock::AnimationClock (Animation *timeline)
  : Clock (timeline)
{
	SetObjectType (Type::ANIMATIONCLOCK);

	this->timeline = timeline;
	storage = NULL;
}

AnimationStorage *
AnimationClock::HookupStorage (DependencyObject *targetobj, DependencyProperty *targetprop)
{
	/* Before hooking up make sure that the values our animation generates
	   (doubles, colors, points...) match the values that the property is
	   ready to receive. If not, print an informative message. */
	Type *property_type = Type::Find (GetDeployment (), targetprop->GetPropertyType());
	if (timeline->GetValueKind () != Type::INVALID && !property_type->IsAssignableFrom (timeline->GetValueKind ())) {
		Type *timeline_type = Type::Find (GetDeployment (), timeline->GetValueKind ());

		const char *timeline_type_name = (timeline_type != NULL) ? timeline_type->GetName () : "Invalid";
		const char *property_type_name = (property_type != NULL) ? property_type->GetName () : "Invalid";
		g_warning ("%s.%s property value type is '%s' but animation type is '%s'.",
			   targetobj->GetTypeName (), targetprop->GetName(),
			   property_type_name, timeline_type_name);

		return false;
	}

	char *name = g_strdup_printf ("AnimationClock for %s, targetobj = %p/%s, targetprop = %s", GetTypeName(),
				      targetobj,
				      targetobj->GetName(),
				      targetprop->GetName());
	SetName (name);

	g_free (name);

	if (storage)
		delete storage;
	storage = new AnimationStorage (this, timeline, targetobj, targetprop);
	return storage;
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
		storage->Stop ();
		delete storage;
		storage = NULL;
	}

	Clock::Stop ();
}

void
AnimationClock::Begin (TimeSpan parentTime)
{
	Clock::Begin (parentTime);
}

AnimationClock::~AnimationClock ()
{
	if (storage) {
		delete storage;
		storage = NULL;
	}
}

void
AnimationClock::DetachStorage ()
{
	storage = NULL;
}

Clock*
Animation::AllocateClock()
{
	clock = new AnimationClock (this);

	AttachCompletedHandler ();

	return clock;
}

Value*
Animation::GetTargetValue (Value* defaultOriginValue)
{
	return NULL;
}

Value*
Animation::GetCurrentValue (Value* defaultOriginValue, Value* defaultDestinationValue,
					AnimationClock* animationClock)
{
	return NULL;
}


Duration
Animation::GetNaturalDurationCore (Clock* clock)
{
	return Duration::FromSeconds (1);
}




/* storyboard */

Storyboard::Storyboard ()
{
	SetObjectType (Type::STORYBOARD);
}

Storyboard::~Storyboard ()
{
	if (clock) {
		StopWithError (/* ignore any error */ NULL);
	}
}

TimeSpan
Storyboard::GetCurrentTime ()
{
	return GetClock() ? GetClock()->GetCurrentTime () : 0;
}

int
Storyboard::GetCurrentState ()
{
	return GetClock() ? GetClock()->GetClockState () : Clock::Stopped;
}

DependencyProperty *
Storyboard::GetTargetDependencyProperty ()
{
	PropertyPath *path = GetTargetProperty (this);
	return path ? path->property : NULL;
}

bool
Storyboard::HookupAnimationsRecurse (Clock *clock, DependencyObject *targetObject, PropertyPath *targetPropertyPath, GHashTable *promoted_values, MoonError *error)
{
	DependencyObject *localTargetObject = NULL;
	PropertyPath *localTargetPropertyPath = NULL;

	Timeline *timeline = clock->GetTimeline ();

	/* get the target object at this level */
	if (timeline->HasManualTarget ()) 
		localTargetObject = timeline->GetManualTarget ();
	else {
		const char *targetName = Storyboard::GetTargetName (timeline);
		if (targetName)
			localTargetObject = FindName (targetName);
	}

	/* get the target property path at this level */
	localTargetPropertyPath = Storyboard::GetTargetProperty (timeline);


	/* override the object and property passed from our parent here */
	if (localTargetObject != NULL)
		targetObject = localTargetObject;

	if (localTargetPropertyPath != NULL)
		targetPropertyPath = localTargetPropertyPath;


	if (clock->Is (Type::CLOCKGROUP)) {
		for (GList *l = ((ClockGroup*)clock)->child_clocks; l; l = l->next) {
			if (!HookupAnimationsRecurse ((Clock*)l->data,
						      targetObject,
						      targetPropertyPath,
						      promoted_values,
						      error))
				return false;
		}
	}
	else {
		DependencyProperty *prop = NULL;
		DependencyObject *realTargetObject;

		if (!targetPropertyPath) {
			MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Target Property has not been specified.");
			g_warning ("No target property!");
			return false;
		}

		if (!targetObject) {
			MoonError::FillIn (error, MoonError::INVALID_OPERATION, "No Target or TargetName has been specified");
			return false;
		}

		realTargetObject = targetObject;

		prop = resolve_property_path (&realTargetObject, targetPropertyPath, promoted_values);

		if (!prop || !realTargetObject) {
			MoonError::FillIn (error, MoonError::INVALID_OPERATION, "TargetProperty could not be resolved");
			g_warning ("No property path %s on object of type type %s!",
				   targetPropertyPath->path, targetObject->GetTypeName());
			return false;
		}

		if (clock->Is(Type::ANIMATIONCLOCK)) {
			Animation *animation = (Animation*)timeline;

			if (!animation->Resolve (realTargetObject, prop)) {
				MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Storyboard value could not be converted to the correct type");
				return false;
			}
			
			if (!((AnimationClock*)clock)->HookupStorage (realTargetObject, prop))
				return false;
		}
	}
	
	return true;
}

bool
Storyboard::BeginWithError (MoonError *error)
{
	if (GetHadParent ()) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Cannot Begin a Storyboard which is not the root Storyboard.");
		return false;
	}

	/* destroy the clock hierarchy and recreate it to restart.
	   easier than making Begin work again with the existing clock
	   hierarchy */
	if (clock) {
		DetachCompletedHandler ();
		clock->Dispose ();
	}

	if (Validate () == false)
		return false;

	// This creates the clock tree for the hierarchy.  if a
	// Timeline A is a child of TimelineGroup B, then Clock cA
	// will be a child of ClockGroup cB.
	AllocateClock ();
	char *name = g_strdup_printf ("Storyboard, named '%s'", GetName());
	clock->SetValue (DependencyObject::NameProperty, name);
	g_free (name);


	// walk the clock tree hooking up the correct properties and
	// creating AnimationStorage's for AnimationClocks.
	GHashTable *promoted_values = g_hash_table_new (g_direct_hash, g_direct_equal);
	if (!HookupAnimationsRecurse (clock, NULL, NULL, promoted_values, error)) {
		g_hash_table_destroy (promoted_values);
		return false;
	}
	g_hash_table_destroy (promoted_values);

	Deployment::GetCurrent()->GetSurface()->GetTimeManager()->AddClock (clock);

	if (GetBeginTime() == 0)
		clock->BeginOnTick ();

	return true;
}

void
Storyboard::PauseWithError (MoonError *error)
{
	if (GetHadParent ()) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Cannot Pause a Storyboard which is not the root Storyboard.");
		return;
	}
	if (clock)
		clock->Pause ();
}

void
Storyboard::ResumeWithError (MoonError *error)
{
	if (GetHadParent ()) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Cannot Resume a Storyboard which is not the root Storyboard.");
		return;
	}
	if (clock)
		clock->Resume ();
}

void
Storyboard::SeekWithError (TimeSpan timespan, MoonError *error)
{
	if (GetHadParent ()) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Cannot Seek a Storyboard which is not the root Storyboard.");
		return;
	}
	if (clock)
		clock->Seek (timespan);
}

void
Storyboard::SeekAlignedToLastTickWithError (TimeSpan timespan, MoonError *error)
{
	if (GetHadParent ()) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Cannot Seek a Storyboard which is not the root Storyboard.");
		return;
	}
	if (clock)
		clock->SeekAlignedToLastTick (timespan);
}

void
Storyboard::SkipToFillWithError (MoonError *error)
{
	if (GetHadParent ()) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Cannot SkipToFill a Storyboard which is not the root Storyboard.");
		return;
	}
	if (clock) {
		clock->SkipToFill ();
	}
}

void
Storyboard::StopWithError (MoonError *error)
{
	if (GetHadParent ()) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Cannot Stop a Storyboard which is not the root Storyboard.");
		return;
	}
	if (clock) {
		DetachCompletedHandler ();
		clock->Stop ();
		clock->Dispose ();
	}
}

BeginStoryboard::BeginStoryboard ()
{
	SetObjectType (Type::BEGINSTORYBOARD);
}

BeginStoryboard::~BeginStoryboard ()
{
}

void
BeginStoryboard::Fire ()
{
	Storyboard *sb = GetStoryboard ();
	if (sb) {
		// FIXME I'd imagine we should be bubbling this error/exception upward, no?
		sb->BeginWithError (NULL);
	}
}


DoubleAnimation::DoubleAnimation ()
{
	SetObjectType (Type::DOUBLEANIMATION);

	doubleToCached = NULL;
	doubleFromCached = NULL;
	doubleByCached = NULL;
	hasCached = FALSE;
}
	
void DoubleAnimation::EnsureCache (void)
{
	doubleFromCached = GetFrom ();
	doubleToCached = GetTo ();
	doubleByCached = GetBy ();
	hasCached = TRUE;
}

Value*
DoubleAnimation::GetTargetValue (Value *defaultOriginValue)
{
	if (! hasCached)
		this->EnsureCache ();

	double start;

	if (doubleFromCached)
		start = *doubleFromCached;
	else if (defaultOriginValue->Is(GetDeployment (), Type::DOUBLE))
		start = defaultOriginValue->AsDouble();
	else
		start = 0.0;

	if (doubleToCached)
		return new Value (*doubleToCached);
	else if (doubleByCached) 
		return new Value (start + *doubleByCached);
	else
		return new Value (start);
}

Value*
DoubleAnimation::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
				  AnimationClock* animationClock)
{
	if (! hasCached)
		this->EnsureCache ();

	double start;

	if (doubleFromCached)
		start = *doubleFromCached;
	else if (defaultOriginValue->Is(GetDeployment (), Type::DOUBLE))
		start = defaultOriginValue->AsDouble();
	else
		start = 0.0;

	double end;

	if (doubleToCached) {
		end = *doubleToCached;
	}
	else if (doubleByCached) {
		end = start + *doubleByCached;
	}
	else if (defaultDestinationValue->Is(GetDeployment (), Type::DOUBLE)) {
		end = defaultDestinationValue->AsDouble();
	}
	else
		end = start;

	double progress = animationClock->GetCurrentProgress ();

	if (GetEasingFunction ())
		progress = GetEasingFunction()->Ease (progress);

	return new Value (LERP (start, end, progress));
}

void
DoubleAnimation::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::DOUBLEANIMATION) {
		Animation::OnPropertyChanged (args, error);
		return;
	}

	// Get rid of the cache
	hasCached = FALSE;
	doubleToCached = NULL;
	doubleFromCached = NULL;
	doubleByCached = NULL;

	NotifyListenersOfPropertyChange (args, error);
}

ColorAnimation::ColorAnimation ()
{
	SetObjectType (Type::COLORANIMATION);

	colorToCached = NULL;
	colorFromCached = NULL;
	colorByCached = NULL;
	hasCached = FALSE;
}

void ColorAnimation::EnsureCache (void)
{
	colorFromCached = GetFrom ();
	colorToCached = GetTo ();
	colorByCached = GetBy ();
	hasCached = TRUE;
}

Value*
ColorAnimation::GetTargetValue (Value *defaultOriginValue)
{
	if (! hasCached)
		this->EnsureCache ();

	Color start;

	if (colorFromCached)
		start = *colorFromCached;
	else if (defaultOriginValue->Is(GetDeployment (), Type::COLOR))
		start = *defaultOriginValue->AsColor();

	if (colorToCached)
		return new Value (*colorToCached);
	else if (colorByCached) 
		return new Value (start + *colorByCached);
	else
		return new Value (start);
}

Value*
ColorAnimation::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
				 AnimationClock* animationClock)
{
	if (! hasCached)
		this->EnsureCache ();

	Color start;

	if (colorFromCached)
		start = *colorFromCached;
	else if (defaultOriginValue->Is(GetDeployment (), Type::COLOR))
		start = *defaultOriginValue->AsColor();

	Color end;

	if (colorToCached) {
		end = *colorToCached;
	}
	else if (colorByCached) {
		end = start + *colorByCached;
	}
	else if (defaultDestinationValue->Is(GetDeployment (), Type::COLOR)) {
		end = *defaultDestinationValue->AsColor();
	}
	else {
		end = start;
	}

	double progress = animationClock->GetCurrentProgress ();

	if (GetEasingFunction ())
		progress = GetEasingFunction()->Ease (progress);

	return new Value (LERP (start, end, progress));
}

void
ColorAnimation::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::COLORANIMATION) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}

	// Get rid of the cache
	colorToCached = NULL;
	colorFromCached = NULL;
	colorByCached = NULL;
	hasCached = FALSE;

	NotifyListenersOfPropertyChange (args, error);
}

PointAnimation::PointAnimation ()
{
	SetObjectType (Type::POINTANIMATION);

	pointToCached = NULL;
	pointFromCached = NULL;
	pointByCached = NULL;
	hasCached = FALSE;
}

PointAnimation::~PointAnimation ()
{
}

void PointAnimation::EnsureCache (void)
{
	pointFromCached = GetFrom ();
	pointToCached = GetTo ();
	pointByCached = GetBy ();
	hasCached = TRUE;
}

Value*
PointAnimation::GetTargetValue (Value *defaultOriginValue)
{
	if (! hasCached)
		this->EnsureCache ();
	
	Point start;

	if (pointFromCached)
		start = *pointFromCached;
	else if (defaultOriginValue->Is(GetDeployment (), Type::POINT))
		start = *defaultOriginValue->AsPoint();

	if (pointToCached)
		return new Value (*pointToCached);
	else if (pointByCached) 
		return new Value (start + *pointByCached);
	else
		return new Value (start);
}

Value*
PointAnimation::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
				 AnimationClock* animationClock)
{
	if (! hasCached)
		this->EnsureCache ();

	Point start;

	if (pointFromCached)
		start = *pointFromCached;
	else if (defaultOriginValue->Is(GetDeployment (), Type::POINT))
		start = *defaultOriginValue->AsPoint();

	Point end;

	if (pointToCached) {
		end = *pointToCached;
	}
	else if (pointByCached) {
		end = start + *pointByCached;
	}
	else if (defaultDestinationValue->Is(GetDeployment (), Type::POINT)) {
		end = *defaultDestinationValue->AsPoint();
	}
	else {
		end = start;
	}

	double progress = animationClock->GetCurrentProgress ();

	if (GetEasingFunction ())
		progress = GetEasingFunction()->Ease (progress);

	return new Value (LERP (start, end, progress));
}

void
PointAnimation::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::POINTANIMATION) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}

	// Get rid of the cache
	pointToCached = NULL;
	pointFromCached = NULL;
	pointByCached = NULL;
	hasCached = FALSE;

	NotifyListenersOfPropertyChange (args, error);
}

KeySpline::KeySpline ()
{
	SetObjectType (Type::KEYSPLINE);

	quadraticsArray = NULL;
}

KeySpline::KeySpline (Point controlPoint1, Point controlPoint2)
{
	SetObjectType (Type::KEYSPLINE);

	quadraticsArray = NULL;
	SetControlPoint1 (&controlPoint1);
	SetControlPoint2 (&controlPoint2);
}

KeySpline::KeySpline (double x1, double y1,
		      double x2, double y2)
{
	SetObjectType (Type::KEYSPLINE);

	quadraticsArray = NULL;

	Point p1 = Point (x1, y1);
	Point p2 = Point (x2, y2);

	SetControlPoint1 (&p1);
	SetControlPoint2 (&p2);
}

KeySpline::~KeySpline ()
{
	g_free (quadraticsArray);
	quadraticsArray = NULL;
}


void
KeySpline::RegenerateQuadratics ()
{
	quadraticsArray = (moon_quadratic *) g_malloc (sizeof (moon_quadratic) * KEYSPLINE_TOTAL_COUNT);

	Point c1 = *GetControlPoint1 ();
	Point c2 = *GetControlPoint2 ();

	moon_cubic src;
	src.c0.x = 0; src.c0.y = 0;
	src.c1.x = c1.x; src.c1.y = c1.y;
	src.c2.x = c2.x; src.c2.y = c2.y;
	src.c3.x  = 1.0; src.c3.y = 1.0;

	moon_cubic carr [KEYSPLINE_TOTAL_COUNT];
	
	moon_subdivide_cubic_at_level (carr, KEYSPLINE_PRECISION_LEVEL, &src);
	moon_convert_cubics_to_quadratics (quadraticsArray, carr, KEYSPLINE_TOTAL_COUNT);
}

void
KeySpline::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::KEYSPLINE) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}

	g_free (quadraticsArray);
	quadraticsArray = NULL;

	NotifyListenersOfPropertyChange (args, error);
}

double
KeySpline::GetSplineProgress (double linearProgress)
{
	if (linearProgress >= 1.0)
		return 1.0;

	if (linearProgress <= 0.0)
		return 0.0;

	if (quadraticsArray == NULL)
		RegenerateQuadratics ();

	return moon_quadratic_array_y_for_x (quadraticsArray, linearProgress, KEYSPLINE_TOTAL_COUNT);
}

KeyFrame::KeyFrame ()
{
	SetObjectType (Type::KEYFRAME);
}

KeyFrame::~KeyFrame ()
{
}

Value *
KeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	g_warning ("KeyFrame::InterpolateValue has been called. The derived class %s should have overridden it.",
		   GetName ());
	return NULL;
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
	SetObjectType (Type::KEYFRAME_COLLECTION);

	sorted_list = g_ptr_array_new ();
	resolved = false;
}

KeyFrameCollection::~KeyFrameCollection ()
{
	g_ptr_array_free (sorted_list, true);
}

bool
KeyFrameCollection::AddedToCollection (Value *value, MoonError *error)
{
	if (!DependencyObjectCollection::AddedToCollection (value, error))
		return false;
	
	resolved = false;

	return true;
}

void
KeyFrameCollection::RemovedFromCollection (Value *value)
{
	DependencyObjectCollection::RemovedFromCollection (value);
	
	resolved = false;
}

bool
KeyFrameCollection::Clear ()
{
	resolved = false;
	g_ptr_array_set_size (sorted_list, 0);
	return DependencyObjectCollection::Clear ();
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
		DependencyProperty *value_prop = keyframe->GetDependencyProperty ("Value");
		if (keyframe->GetValue (value_prop) != NULL) {
			current_keyframe = keyframe;
			break;
		}
	}

	/* Crawl backward some more to find first non-null prev frame */
	for (i--; i >= 0; i--) {
		KeyFrame *keyframe = (KeyFrame *) sorted_list->pdata[i];
		DependencyProperty *value_prop = keyframe->GetDependencyProperty ("Value");
		if (keyframe->GetValue (value_prop) != NULL) {
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
	if (strcmp (subobj_args->GetProperty ()->GetName (), "KeyTime") == 0) {
		resolved = false;
	}

	Collection::OnSubPropertyChanged (prop, obj, subobj_args);
}

ColorKeyFrameCollection::ColorKeyFrameCollection ()
{
	SetObjectType (Type::COLORKEYFRAME_COLLECTION);
}

ColorKeyFrameCollection::~ColorKeyFrameCollection ()
{
}

DoubleKeyFrameCollection::DoubleKeyFrameCollection ()
{
	SetObjectType (Type::DOUBLEKEYFRAME_COLLECTION);
}

DoubleKeyFrameCollection::~DoubleKeyFrameCollection ()
{
}

PointKeyFrameCollection::PointKeyFrameCollection ()
{
	SetObjectType (Type::POINTKEYFRAME_COLLECTION);
}

PointKeyFrameCollection::~PointKeyFrameCollection ()
{
}

DoubleKeyFrame::DoubleKeyFrame ()
{
	SetObjectType (Type::DOUBLEKEYFRAME);
	SetValue (0.0);
}

DoubleKeyFrame::~DoubleKeyFrame ()
{
}

ColorKeyFrame::ColorKeyFrame ()
{
	SetObjectType (Type::COLORKEYFRAME);
	static Color c = Color (0, 0, 0, 1);
	SetValue (c);
}

ColorKeyFrame::~ColorKeyFrame ()
{
}

PointKeyFrame::PointKeyFrame ()
{
	SetObjectType (Type::POINTKEYFRAME);
	SetValue (Point (0,0));
}

PointKeyFrame::~PointKeyFrame ()
{
}

DiscreteDoubleKeyFrame::DiscreteDoubleKeyFrame ()
{
	SetObjectType(Type::DISCRETEDOUBLEKEYFRAME);
}

DiscreteDoubleKeyFrame::~DiscreteDoubleKeyFrame ()
{
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

DiscreteColorKeyFrame::DiscreteColorKeyFrame ()
{
	SetObjectType(Type::DISCRETECOLORKEYFRAME);
}

DiscreteColorKeyFrame::~DiscreteColorKeyFrame ()
{
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

DiscretePointKeyFrame::DiscretePointKeyFrame ()
{
	SetObjectType(Type::DISCRETEPOINTKEYFRAME);
}

DiscretePointKeyFrame::~DiscretePointKeyFrame ()
{
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


LinearDoubleKeyFrame::LinearDoubleKeyFrame ()
{
	SetObjectType(Type::LINEARDOUBLEKEYFRAME);
}

LinearDoubleKeyFrame::~LinearDoubleKeyFrame ()
{
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

LinearColorKeyFrame::LinearColorKeyFrame ()
{
	SetObjectType(Type::LINEARCOLORKEYFRAME);
}

LinearColorKeyFrame::~LinearColorKeyFrame ()
{
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

LinearPointKeyFrame::LinearPointKeyFrame ()
{
	SetObjectType(Type::LINEARPOINTKEYFRAME);
}

LinearPointKeyFrame::~LinearPointKeyFrame ()
{
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

SplineDoubleKeyFrame::SplineDoubleKeyFrame ()
{
	SetObjectType (Type::SPLINEDOUBLEKEYFRAME);
}

SplineDoubleKeyFrame::~SplineDoubleKeyFrame ()
{
}

Value *
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


SplineColorKeyFrame::SplineColorKeyFrame ()
{
	SetObjectType (Type::SPLINECOLORKEYFRAME);
}

SplineColorKeyFrame::~SplineColorKeyFrame ()
{
}

Value *
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


SplinePointKeyFrame::SplinePointKeyFrame ()
{
	SetObjectType (Type::SPLINEPOINTKEYFRAME);
}

SplinePointKeyFrame::~SplinePointKeyFrame ()
{
}

Value *
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

EasingColorKeyFrame::EasingColorKeyFrame ()
{
	SetObjectType (Type::EASINGCOLORKEYFRAME);
}

EasingColorKeyFrame::~EasingColorKeyFrame ()
{
}

Value *
EasingColorKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	Color *to = GetValue();

	if (!to)
		return new Value (*baseValue->AsColor());
	else if (keyFrameProgress >= 1.0)
		return new Value (*to);

	Color start, end;

	start = *baseValue->AsColor();
	end = *to;

	if (GetEasingFunction ())
		keyFrameProgress = GetEasingFunction ()->Ease (keyFrameProgress);

	return new Value (LERP (start, end, keyFrameProgress));
}

EasingDoubleKeyFrame::EasingDoubleKeyFrame ()
{
	SetObjectType (Type::EASINGDOUBLEKEYFRAME);
}

EasingDoubleKeyFrame::~EasingDoubleKeyFrame ()
{
}

Value *
EasingDoubleKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	double *to = GetValue();

	if (!to)
		return new Value (baseValue->AsDouble());
	else if (keyFrameProgress >= 1.0)
		return new Value (*to);

	double start, end;

	start = baseValue->AsDouble();
	end = *to;

	if (GetEasingFunction ())
		keyFrameProgress = GetEasingFunction ()->Ease (keyFrameProgress);

	return new Value (LERP (start, end, keyFrameProgress));
}


EasingPointKeyFrame::EasingPointKeyFrame ()
{
	SetObjectType (Type::EASINGPOINTKEYFRAME);
}

EasingPointKeyFrame::~EasingPointKeyFrame ()
{
}

Value *
EasingPointKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	Point *to = GetValue();

	if (!to)
		return new Value (*baseValue->AsPoint());
	else if (keyFrameProgress >= 1.0)
		return new Value (*to);

	Point start, end;

	start = *baseValue->AsPoint();
	end = *to;

	if (GetEasingFunction ())
		keyFrameProgress = GetEasingFunction ()->Ease (keyFrameProgress);

	return new Value (LERP (start, end, keyFrameProgress));
}

/* implements the algorithm specified at the bottom of this page:
   http://msdn2.microsoft.com/en-us/library/ms742524.aspx
*/
static void
KeyFrameAnimation_ResolveKeyFrames (Animation *animation, KeyFrameCollection *col)
{
	if (col->resolved)
		return;

	col->resolved = true;

	TimeSpan total_interpolation_time;
	bool has_timespan_keyframe = false;
	TimeSpan highest_keytime_timespan = 0;
	KeyFrame *keyframe;
	Value *value;
	int i;
	
	for (i = 0; i < col->GetCount (); i++) {
		value = col->GetValueAt (i);
		keyframe = value->AsKeyFrame ();
		keyframe->resolved_keytime = 0;
		keyframe->resolved = false;
	}

	/* resolve TimeSpan keyframes (step 1 from url) */
	for (i = 0; i < col->GetCount (); i++) {
		value = col->GetValueAt (i);
		keyframe = value->AsKeyFrame ();
		
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
	for (i = 0; i < col->GetCount (); i++) {
		value = col->GetValueAt (i);
		keyframe = value->AsKeyFrame ();
		
		if (keyframe->GetKeyTime()->HasPercent()) {
			keyframe->resolved_keytime = (TimeSpan)(total_interpolation_time * keyframe->GetKeyTime()->GetPercent ());
			keyframe->resolved = true;
 		}
 	}

	/* step 4 from url */
	KeyTime *kt;
	
	/* if the last frame is KeyTime Uniform or Paced, resolve it
	   to be equal to the total interpolation time */
	if (col->GetCount () > 0) {
		value = col->GetValueAt (col->GetCount () - 1);
		keyframe = value->AsKeyFrame ();
		
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
	if (col->GetCount () > 0) {
		value = col->GetValueAt (0);
		keyframe = value->AsKeyFrame ();
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
	
	for (i = col->GetCount (); i > 0; i--) {
		value = col->GetValueAt (i - 1);
		keyframe = value->AsKeyFrame ();
		
		if (!keyframe->resolved)
			g_warning ("***** unresolved keyframe!");
		
		g_ptr_array_insert_sorted (col->sorted_list, KeyFrameComparer, keyframe);
	}
}

// Generic validator of KeyFrameCollection's. Collection vallidates 
// if all keyframes have valid time.
static bool
generic_keyframe_validator (KeyFrameCollection *col)
{
	KeyFrame *keyframe;
	Value *value;
	
	for (int i = 0; i < col->GetCount (); i++) {
		value = col->GetValueAt (i);
		keyframe = value->AsKeyFrame ();
		if (keyframe->GetKeyTime () == NULL)
			return false;
	}

	return true;
}


DoubleAnimationUsingKeyFrames::DoubleAnimationUsingKeyFrames ()
{
	SetObjectType (Type::DOUBLEANIMATIONUSINGKEYFRAMES);
}

DoubleAnimationUsingKeyFrames::~DoubleAnimationUsingKeyFrames ()
{
}

void
DoubleAnimationUsingKeyFrames::AddKeyFrame (DoubleKeyFrame *frame)
{
	DoubleKeyFrameCollection *key_frames = GetKeyFrames ();

	key_frames->Add (frame);
}

void
DoubleAnimationUsingKeyFrames::RemoveKeyFrame (DoubleKeyFrame *frame)
{
	DoubleKeyFrameCollection *key_frames = GetKeyFrames ();

	key_frames->Remove (frame);
}

Value*
DoubleAnimationUsingKeyFrames::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
						AnimationClock* animationClock)
{
	DoubleKeyFrameCollection *key_frames = GetKeyFrames ();

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
	DoubleKeyFrameCollection *key_frames = GetKeyFrames ();
	
	KeyFrameAnimation_ResolveKeyFrames (this, key_frames);

	guint len = key_frames->sorted_list->len;
	if (len > 0)
		return ((KeyFrame *) key_frames->sorted_list->pdata[len - 1])->resolved_keytime;
	else
		return Duration (0);
}

bool
DoubleAnimationUsingKeyFrames::Resolve (DependencyObject *target, DependencyProperty *property)
{
	KeyFrameAnimation_ResolveKeyFrames (this, GetKeyFrames ());
	return true;
}

bool
DoubleAnimationUsingKeyFrames::Validate ()
{
	return generic_keyframe_validator (GetKeyFrames ());
}

ColorAnimationUsingKeyFrames::ColorAnimationUsingKeyFrames()
{
	SetObjectType (Type::COLORANIMATIONUSINGKEYFRAMES);
}

ColorAnimationUsingKeyFrames::~ColorAnimationUsingKeyFrames ()
{
}

void
ColorAnimationUsingKeyFrames::AddKeyFrame (ColorKeyFrame *frame)
{
	ColorKeyFrameCollection *key_frames = GetKeyFrames ();

	key_frames->Add (frame);
}

void
ColorAnimationUsingKeyFrames::RemoveKeyFrame (ColorKeyFrame *frame)
{
	ColorKeyFrameCollection *key_frames = GetKeyFrames ();

	key_frames->Remove (frame);
}

Value*
ColorAnimationUsingKeyFrames::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					       AnimationClock* animationClock)
{
	ColorKeyFrameCollection *key_frames = GetKeyFrames ();
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
	ColorKeyFrameCollection *key_frames = GetKeyFrames ();
	
	KeyFrameAnimation_ResolveKeyFrames (this, key_frames);

	guint len = key_frames->sorted_list->len;
	if (len > 0)
		return ((KeyFrame *) key_frames->sorted_list->pdata[len - 1])->resolved_keytime;
	else
		return Duration (0);
}

bool
ColorAnimationUsingKeyFrames::Resolve (DependencyObject *target, DependencyProperty *property)
{
	KeyFrameAnimation_ResolveKeyFrames (this, GetKeyFrames ());
	return true;
}

bool
ColorAnimationUsingKeyFrames::Validate ()
{
	return generic_keyframe_validator (GetKeyFrames ());
}

PointAnimationUsingKeyFrames::PointAnimationUsingKeyFrames()
{
	SetObjectType (Type::POINTANIMATIONUSINGKEYFRAMES);
}

PointAnimationUsingKeyFrames::~PointAnimationUsingKeyFrames ()
{
}

void
PointAnimationUsingKeyFrames::AddKeyFrame (PointKeyFrame *frame)
{
	PointKeyFrameCollection *key_frames = GetKeyFrames ();

	key_frames->Add (frame);
}

void
PointAnimationUsingKeyFrames::RemoveKeyFrame (PointKeyFrame *frame)
{
	PointKeyFrameCollection *key_frames = GetKeyFrames ();

	key_frames->Remove (frame);
}

Value*
PointAnimationUsingKeyFrames::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					       AnimationClock* animationClock)
{
	PointKeyFrameCollection *key_frames = GetKeyFrames ();
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
	PointKeyFrameCollection *key_frames = GetKeyFrames ();
	
	KeyFrameAnimation_ResolveKeyFrames (this, key_frames);

	guint len = key_frames->sorted_list->len;
	if (len > 0)
		return ((KeyFrame *) key_frames->sorted_list->pdata[len - 1])->resolved_keytime;
	else
		return Duration (0);
}

bool
PointAnimationUsingKeyFrames::Resolve (DependencyObject *target, DependencyProperty *property)
{
	KeyFrameAnimation_ResolveKeyFrames (this, GetKeyFrames ());
	return true;
}

bool
PointAnimationUsingKeyFrames::Validate ()
{
	return generic_keyframe_validator (GetKeyFrames ());
}

ObjectKeyFrameCollection::ObjectKeyFrameCollection ()
{
	SetObjectType (Type::OBJECTKEYFRAME_COLLECTION);
}

ObjectKeyFrameCollection::~ObjectKeyFrameCollection ()
{
}

ObjectKeyFrame::ObjectKeyFrame ()
{
	SetObjectType (Type::OBJECTKEYFRAME);
}

ObjectKeyFrame::~ObjectKeyFrame ()
{
}

Value *
ObjectKeyFrame::GetValue ()
{
	return DependencyObject::GetValue (ValueProperty);
}

DiscreteObjectKeyFrame::DiscreteObjectKeyFrame ()
{
	SetObjectType (Type::DISCRETEOBJECTKEYFRAME);
}

DiscreteObjectKeyFrame::~DiscreteObjectKeyFrame ()
{
}

Value*
DiscreteObjectKeyFrame::InterpolateValue (Value *baseValue, double keyFrameProgress)
{
	Value *to = GetConvertedValue ();

	if (to && keyFrameProgress == 1.0)
		return new Value (*to);
	else
		return new Value (*baseValue);
}

ObjectAnimationUsingKeyFrames::ObjectAnimationUsingKeyFrames ()
{
	SetObjectType (Type::OBJECTANIMATIONUSINGKEYFRAMES);
}

ObjectAnimationUsingKeyFrames::~ObjectAnimationUsingKeyFrames ()
{
}

void
ObjectAnimationUsingKeyFrames::AddKeyFrame (ObjectKeyFrame *frame)
{
	ObjectKeyFrameCollection *key_frames = GetKeyFrames ();

	key_frames->Add (frame);
}

void
ObjectAnimationUsingKeyFrames::RemoveKeyFrame (ObjectKeyFrame *frame)
{
	ObjectKeyFrameCollection *key_frames = GetKeyFrames ();

	key_frames->Remove (frame);
}

bool
ObjectAnimationUsingKeyFrames::Resolve (DependencyObject *target, DependencyProperty *property)
{
	ObjectKeyFrameCollection *frames = (ObjectKeyFrameCollection *) GetKeyFrames ();
	for (int i = 0; i < frames->GetCount (); i++) {
		ObjectKeyFrame *frame = frames->GetValueAt (i)->AsObjectKeyFrame ();
		
		Value *value = frame->GetValue ();
		if (!value || value->GetIsNull ()) {
			// If the value is null, don't convert
			frame->SetValue (ObjectKeyFrame::ConvertedValueProperty, NULL);
		} else if (value->GetKind () == property->GetPropertyType ()) {
			// If the value is of the correct type already, don't convert
			frame->SetValue (ObjectKeyFrame::ConvertedValueProperty, value);
		} else {
			Value converted;
			Application::GetCurrent ()->ConvertKeyframeValue (target->GetType ()->GetKind (), property, value, &converted);
		
			if (converted.GetKind () == Type::INVALID) {
				printf ("Couldn't convert value.\n");
				return false;
			}
			frame->SetValue (ObjectKeyFrame::ConvertedValueProperty, converted);
		}
	}
	KeyFrameAnimation_ResolveKeyFrames (this, frames);
	return true;
}

Value*
ObjectAnimationUsingKeyFrames::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
					        AnimationClock* animationClock)
{
	ObjectKeyFrameCollection *key_frames = GetKeyFrames ();

	/* current segment info */
	TimeSpan current_time = animationClock->GetCurrentTime();
	ObjectKeyFrame *current_keyframe;
	ObjectKeyFrame *previous_keyframe;
	ObjectKeyFrame** keyframep = &previous_keyframe;
	Value *baseValue;
	bool deleteBaseValue;

	current_keyframe = (ObjectKeyFrame*)key_frames->GetKeyFrameForTime (current_time, (KeyFrame**)keyframep);
	if (current_keyframe == NULL)
		return NULL; /* XXX */

	TimeSpan key_end_time = current_keyframe->resolved_keytime;
	TimeSpan key_start_time;

	if (previous_keyframe == NULL) {
		/* the first keyframe, start at the animation's base value */
		baseValue = defaultOriginValue;
		deleteBaseValue = false;
		key_start_time = 0;
	} else {
		/* start at the previous keyframe's target value */
		baseValue = new Value (*previous_keyframe->GetConvertedValue ());
		deleteBaseValue = true;
		key_start_time = previous_keyframe->resolved_keytime;
	}

	double progress;

	if (current_time >= key_end_time) {
		progress = 1.0;
	} else {
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
ObjectAnimationUsingKeyFrames::GetNaturalDurationCore (Clock *clock)
{
	ObjectKeyFrameCollection *key_frames = GetKeyFrames ();
	
	KeyFrameAnimation_ResolveKeyFrames (this, key_frames);

	guint len = key_frames->sorted_list->len;
	if (len > 0)
		return ((KeyFrame *) key_frames->sorted_list->pdata[len - 1])->resolved_keytime;
	else
		return Duration (0);
}

bool
ObjectAnimationUsingKeyFrames::Validate ()
{
	// Interesting question -- should we check for null here?
	return generic_keyframe_validator (GetKeyFrames ());
}

