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
#include <string.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
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


AnimationStorage::AnimationStorage (AnimationClock *clock, Animation *timeline,
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
AnimationStorage::UpdatePropertyValue ()
{
	if (targetobj == NULL)
		return;

	Value *current_value = clock->GetCurrentValue (baseValue, NULL/*XXX*/);
	if (current_value != NULL && timeline->GetTimelineStatus () == Timeline::TIMELINE_STATUS_OK) {
		Applier *applier = clock->GetTimeManager ()->GetApplier ();
		applier->AddPropertyChange (targetobj, targetprop, new Value (*current_value), APPLIER_PRECEDENCE_ANIMATION);
	}
		
	delete current_value;
}

void
AnimationStorage::ResetPropertyValue ()
{
	if (nonResetableFlag)
		return;

	if (targetobj == NULL)
		return;
	
	if (timeline->GetTimelineStatus () != Timeline::TIMELINE_STATUS_OK)
		return;

	Applier *applier = clock->GetTimeManager ()->GetApplier ();

	if (stopValue)
		applier->AddPropertyChange (targetobj, targetprop, new Value (*stopValue), APPLIER_PRECEDENCE_ANIMATION_RESET);
	else
		applier->AddPropertyChange (targetobj, targetprop, new Value (*baseValue), APPLIER_PRECEDENCE_ANIMATION_RESET);
}

void 
AnimationStorage::DetachFromPrevStorage (void)
{
	if (targetobj != NULL && targetprop != NULL) {
		targetprop->DetachAnimationStorage (targetobj, this);
	}
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
		clock->RemoveHandler (Clock::CurrentTimeInvalidatedEvent, update_property_value, this);
	}
}

void
AnimationStorage::ReAttachUpdateHandler ()
{
	if (clock != NULL) {
		clock->AddHandler (Clock::CurrentTimeInvalidatedEvent, update_property_value, this);
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
	if (baseValue) {
		delete baseValue;
		baseValue = NULL;
	}

	if (stopValue) {
		delete stopValue;
		stopValue = NULL;
	}

	DetachUpdateHandler ();
	
	if (targetobj != NULL) {
		targetobj->RemoveHandler (EventObject::DestroyedEvent, target_object_destroyed, this);
		targetprop->DetachAnimationStorage (targetobj, this);
	}
}

void
AnimationClock::OnSurfaceDetach ()
{
	Clock::OnSurfaceDetach ();

	if (storage)
		storage->DetachUpdateHandler ();
}

void
AnimationClock::OnSurfaceReAttach ()
{
	Clock::OnSurfaceReAttach ();

	if (storage)
		storage->ReAttachUpdateHandler ();
}

AnimationClock::AnimationClock (Animation *timeline)
  : Clock (timeline)
{
	SetObjectType (Type::ANIMATIONCLOCK);

	this->timeline = timeline;
	storage = NULL;
}

bool
AnimationClock::HookupStorage (DependencyObject *targetobj, DependencyProperty *targetprop)
{
	/* Before hooking up make sure that the values our animation generates
	   (doubles, colors, points...) match the values that the property is
	   ready to receive. If not, print an informative message. */
	if (timeline->GetValueKind () != Type::INVALID && timeline->GetValueKind () != targetprop->GetPropertyType()) {
		Type *timeline_type = Type::Find (timeline->GetValueKind ());
		Type *property_type = Type::Find (targetprop->GetPropertyType());

		const char *timeline_type_name = (timeline_type != NULL) ? timeline_type->GetName () : "Invalid";
		const char *property_type_name = (property_type != NULL) ? property_type->GetName () : "Invalid";
		g_warning ("%s.%s property value type is '%s' but animation type is '%s'.",
			   targetobj->GetTypeName (), targetprop->GetName(),
			   property_type_name, timeline_type_name);

		return false;
	}

	storage = new AnimationStorage (this, timeline, targetobj, targetprop);
	return true;
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
		if (storage->IsCurrentStorage ())
			storage->DetachFromPrevStorage ();
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
		
		if (storage->IsLonely ())
			delete storage;
		else {
			if (state == Clock::Stopped)
				delete storage;
			else {
				if (storage->IsCurrentStorage ()) {
					// FIXME: Why don't we delete storage here? Sadly, we are leaking it on channel9
					// ANSWER: The pointer to the storage is still held by the hash table in the DependencyProperty
					// It gets destroyed next time sombody attaches an animation to the property. 
					storage->Float ();
				} else
					delete storage;
			}
		}
	}
}

Clock*
Animation::AllocateClock()
{
	clock = new AnimationClock (this);
	char *name = g_strdup_printf ("AnimationClock for %s, targetobj = %p/%s, targetprop = %s", GetTypeName(),
				      Storyboard::GetTargetName(this) == NULL ? NULL : FindName (Storyboard::GetTargetName(this)),
				      Storyboard::GetTargetName(this),
				      Storyboard::GetTargetProperty (this) == NULL ? NULL : Storyboard::GetTargetProperty (this)->path);
	clock->SetValue (DependencyObject::NameProperty, name);

	AttachCompletedHandler ();

	g_free (name);
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
		//printf ("Clock %p (ref=%d)\n", root_clock, root_clock->refcount);
		StopWithError (/* ignore any error */ NULL);
		TeardownClockGroup ();
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
Storyboard::HookupAnimationsRecurse (Clock *clock, DependencyObject *targetObject, PropertyPath *targetPropertyPath, MoonError *error)
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

		prop = resolve_property_path (&realTargetObject, targetPropertyPath);

		if (!prop || !realTargetObject) {
			MoonError::FillIn (error, MoonError::INVALID_OPERATION, "TargetProperty could not be resolved");
			g_warning ("No property path %s on object of type type %s!",
				   targetPropertyPath->path, targetObject->GetTypeName());
			return false;
		}

		if (clock->Is(Type::ANIMATIONCLOCK)) {
			Animation *animation = (Animation*)timeline;

			animation->Resolve ();

			if (!((AnimationClock*)clock)->HookupStorage (realTargetObject, prop))
				return false;
		}
	}
	
	return true;
}

void
Storyboard::TeardownClockGroup ()
{
	if (GetClock()) {
		Clock *c = GetClock ();
		ClockGroup *group = c->GetParentClock();
		if (group)
			group->RemoveChild (c);
		clock = NULL;
	}
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
		TeardownClockGroup ();
	}

	if (Validate () == false)
		return false;

	// This creates the clock tree for the hierarchy.  if a
	// Timeline A is a child of TimelineGroup B, then Clock cA
	// will be a child of ClockGroup cB.
	Clock *root_clock = AllocateClock ();
	char *name = g_strdup_printf ("Storyboard, named '%s'", GetName());
	root_clock->SetValue (DependencyObject::NameProperty, name);
	g_free (name);

	// walk the clock tree hooking up the correct properties and
	// creating AnimationStorage's for AnimationClocks.
	if (!HookupAnimationsRecurse (root_clock, NULL, NULL, error))
		return false;

	Deployment::GetCurrent()->GetSurface()->GetTimeManager()->AddClock (root_clock);

	if (GetBeginTime() == 0)
		root_clock->BeginOnTick ();

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
		TeardownClockGroup ();
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

	double start = doubleFromCached ? *doubleFromCached : defaultOriginValue->AsDouble();

	if (doubleToCached)
		return new Value (*doubleToCached);
	else if (doubleByCached) 
		return new Value (start + *doubleByCached);
	else
		return new Value (defaultOriginValue->AsDouble ());
}

Value*
DoubleAnimation::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
				  AnimationClock* animationClock)
{
	if (! hasCached)
		this->EnsureCache ();

	double start = doubleFromCached ? *doubleFromCached : defaultOriginValue->AsDouble();
	double end;

	if (doubleToCached) {
		end = *doubleToCached;
	}
	else if (doubleByCached) {
		end = start + *doubleByCached;
	}
	else {
		end = defaultOriginValue->AsDouble();
	}

	double progress = animationClock->GetCurrentProgress ();

	if (GetEasingFunction ())
		progress = GetEasingFunction()->Ease (progress);

	return new Value (LERP (start, end, progress));
}

void
DoubleAnimation::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::DOUBLEANIMATION) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}

	// Get rid of the cache
	hasCached = FALSE;
	doubleToCached = NULL;
	doubleFromCached = NULL;
	doubleByCached = NULL;

	NotifyListenersOfPropertyChange (args);
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

	Color start = colorFromCached ? *colorFromCached : *defaultOriginValue->AsColor();

	if (colorToCached)
		return new Value (*colorToCached);
	else if (colorByCached) 
		return new Value (start + *colorByCached);
	else
		return new Value (*defaultOriginValue->AsColor ());
}

Value*
ColorAnimation::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
				 AnimationClock* animationClock)
{
	if (! hasCached)
		this->EnsureCache ();

	Color start = colorFromCached ? *colorFromCached : *defaultOriginValue->AsColor();
	Color end;

	if (colorToCached) {
		end = *colorToCached;
	}
	else if (colorByCached) {
		end = start + *colorByCached;
	}
	else {
		end = *defaultOriginValue->AsColor ();
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

	NotifyListenersOfPropertyChange (args);
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
	
	Point start = pointFromCached ? *pointFromCached : *defaultOriginValue->AsPoint();

	if (pointToCached)
		return new Value (*pointToCached);
	else if (pointByCached) 
		return new Value (start + *pointByCached);
	else
		return new Value (*defaultOriginValue->AsPoint ());
}

Value*
PointAnimation::GetCurrentValue (Value *defaultOriginValue, Value *defaultDestinationValue,
				 AnimationClock* animationClock)
{
	if (! hasCached)
		this->EnsureCache ();

	Point start = pointFromCached ? *pointFromCached : *defaultOriginValue->AsPoint();
	Point end;

	if (pointToCached) {
		end = *pointToCached;
	}
	else if (pointByCached) {
		end = start + *pointByCached;
	}
	else {
		end = *defaultOriginValue->AsPoint ();
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

	NotifyListenersOfPropertyChange (args);
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

	NotifyListenersOfPropertyChange (args);
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
		GetEasingFunction ()->Ease (keyFrameProgress);

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
		GetEasingFunction ()->Ease (keyFrameProgress);

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
		GetEasingFunction ()->Ease (keyFrameProgress);

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

void
DoubleAnimationUsingKeyFrames::Resolve ()
{
	KeyFrameAnimation_ResolveKeyFrames (this, GetKeyFrames ());
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

void
ColorAnimationUsingKeyFrames::Resolve ()
{
	KeyFrameAnimation_ResolveKeyFrames (this, GetKeyFrames ());
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

void
PointAnimationUsingKeyFrames::Resolve ()
{
	KeyFrameAnimation_ResolveKeyFrames (this, GetKeyFrames ());
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
	Value *to = GetValue();

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

void
ObjectAnimationUsingKeyFrames::Resolve ()
{
	KeyFrameAnimation_ResolveKeyFrames (this, GetKeyFrames ());
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
		baseValue = new Value (*previous_keyframe->GetValue ());
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

