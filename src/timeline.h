/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_TIMELINE_H
#define MOON_TIMELINE_H

#include <glib.h>

#include "clock.h"

namespace Moonlight {

/* @Namespace=System.Windows.Media.Animation */
class Timeline : public DependencyObject {
public:
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int AutoReverseProperty;
 	/* @PropertyType=TimeSpan,Nullable,DefaultValue=(gint64) 0\, Type::TIMESPAN */
	const static int BeginTimeProperty;
 	/* @PropertyType=Duration,DefaultValue=Duration::Automatic */
	const static int DurationProperty;
 	/* @PropertyType=FillBehavior,DefaultValue=FillBehaviorHoldEnd,GenerateAccessors */
	const static int FillBehaviorProperty;
 	/* @PropertyType=RepeatBehavior,DefaultValue=RepeatBehavior ((double) 1) */
	const static int RepeatBehaviorProperty;
 	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int SpeedRatioProperty;
	
 	/* @GeneratePInvoke,ManagedAccess=Protected */
	Timeline ();

	virtual void Dispose ();

	void SetAutoReverse (bool autoreverse);
	bool GetAutoReverse ();
	
	TimeSpan GetBeginTime ();
	
	void SetDuration (Duration duration);
	Duration *GetDuration ();
	
	FillBehavior GetFillBehavior ();
	void SetFillBehavior (FillBehavior behavior);
	
	bool GetHadParent () { return this->had_parent; }
	void SetHadParent (bool had_parent) { this->had_parent = had_parent; }

	void SetRepeatBehavior (RepeatBehavior behavior);
	RepeatBehavior *GetRepeatBehavior ();
	
	void SetSpeedRatio (double ratio);
	double GetSpeedRatio ();
	
	Duration GetNaturalDuration (Clock *clock);
	virtual Duration GetNaturalDurationCore (Clock *clock);
	
	virtual Clock *AllocateClock ();
	virtual void ClearClock (bool dispose);
	virtual bool Validate (MoonError *error);

	Clock* GetClock ();

	enum TimelineStatus {
		TIMELINE_STATUS_OK, 
		TIMELINE_STATUS_DETACHED
	};

	TimelineStatus GetTimelineStatus () { return timeline_status; }

	bool HasManualTarget () { return manual_target != NULL; }
	
	/* @GeneratePInvoke */
	DependencyObject* GetManualTarget () { return manual_target; }
	
	/* @GeneratePInvoke */
	void SetManualTarget (DependencyObject *o);

	// events
	const static int CompletedEvent;

	virtual void TeardownClock ();

protected:
	virtual ~Timeline ();

	void AttachCompletedHandler ();
	void DetachCompletedHandler ();

	virtual void OnClockCompleted ();

	static void clock_completed (EventObject *sender, EventArgs *calldata, gpointer closure);

	void SetClock (Clock *value);

private:
 	bool had_parent;
	Clock *clock;
	TimelineStatus timeline_status;
	WeakRef<DependencyObject> manual_target;
};


/* @Namespace=System.Windows.Media.Animation */
class TimelineCollection : public DependencyObjectCollection {
public:
 	/* @GeneratePInvoke */
	TimelineCollection ();

	virtual Type::Kind GetElementType() { return Type::TIMELINE; }

protected:
	virtual ~TimelineCollection ();
};


/* @Namespace=None,ManagedDependencyProperties=None,ManagedEvents=None */
class TimelineGroup : public Timeline {
public:
	/* @PropertyType=TimelineCollection,AutoCreateValue,GenerateAccessors */
	const static int ChildrenProperty;
	
 	/* @GeneratePInvoke */
	TimelineGroup ();
	
	virtual Clock *AllocateClock ();
	virtual void ClearClock (bool dispose);
	virtual bool Validate (MoonError *error);
	
	void AddChild (Timeline *child);
	void RemoveChild (Timeline *child);

	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	
	//
	// Property Accessors
	//
	void SetChildren (TimelineCollection *children);
	TimelineCollection *GetChildren ();

protected:
	virtual ~TimelineGroup ();
};


/* @Namespace=None */
class ParallelTimeline : public TimelineGroup {
public:
 	/* @GeneratePInvoke */
	ParallelTimeline ();

	virtual Duration GetNaturalDurationCore (Clock *clock);

protected:
	virtual ~ParallelTimeline ();
};


/* @Namespace=System.Windows.Media */
class TimelineMarker : public DependencyObject {
public:
 	/* @PropertyType=string,GenerateAccessors */
	const static int TextProperty;
 	/* @PropertyType=TimeSpan,DefaultValue=(TimeSpan)0\,Type::TIMESPAN,GenerateAccessors */
	const static int TimeProperty;
 	/* @PropertyType=string,GenerateAccessors */
	const static int TypeProperty;
	
 	/* @GeneratePInvoke */
	TimelineMarker ();

	//
	// Property Accessors
	//
	void SetText (const char *text);
	const char *GetText ();
	
	void SetTime (TimeSpan time);
	TimeSpan GetTime ();
	
	void SetType (const char *type);
	const char *GetType ();

protected:
	virtual ~TimelineMarker ();
};


/* @Namespace=Mono */
/* @ManagedEvents=Manual */
class DispatcherTimer : public Timeline {
public:
	/* @GeneratePInvoke */
	void Start ();

	/* @GeneratePInvoke */
	void Stop ();

	const static int TickEvent;

	void Restart ();

	virtual Duration GetNaturalDurationCore (Clock *clock);
	virtual void TeardownClock ();

protected:
	/* @GeneratePInvoke,MainThread */
	DispatcherTimer ();

	virtual ~DispatcherTimer() {}

	virtual void OnClockCompleted ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:
	bool stopped;
	bool started;
	bool ontick;
};

};
#endif /* MOON_TIMELINE_H */
