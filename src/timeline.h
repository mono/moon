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
	
 	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Timeline ();
	
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
	virtual bool Validate ();

	Clock* GetClock ();

	enum TimelineStatus {
		TIMELINE_STATUS_OK, 
		TIMELINE_STATUS_DETACHED
	};

	TimelineStatus GetTimelineStatus () { return timeline_status; }

	bool HasManualTarget () { return manual_target != NULL; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObject* GetManualTarget () { return manual_target; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetManualTarget (DependencyObject *o) { manual_target = o; }

	// events
	const static int CompletedEvent;

	virtual void TeardownClock ();

protected:
	virtual ~Timeline ();

	void AttachCompletedHandler ();
	void DetachCompletedHandler ();

	virtual void OnClockCompleted ();

	static void clock_completed (EventObject *sender, EventArgs *calldata, gpointer closure);

	Clock *clock;

private:
 	bool had_parent;
	TimelineStatus timeline_status;
	DependencyObject *manual_target;
};


/* @Namespace=System.Windows.Media.Animation */
class TimelineCollection : public DependencyObjectCollection {
public:
 	/* @GenerateCBinding,GeneratePInvoke */
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
	
 	/* @GenerateCBinding,GeneratePInvoke */
	TimelineGroup ();
	
	virtual Clock *AllocateClock ();
	virtual bool Validate ();
	
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
 	/* @GenerateCBinding,GeneratePInvoke */
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
 	/* @PropertyType=TimeSpan,GenerateAccessors */
	const static int TimeProperty;
 	/* @PropertyType=string,GenerateAccessors */
	const static int TypeProperty;
	
 	/* @GenerateCBinding,GeneratePInvoke */
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


/* @Namespace=Mono,Version=2 */
/* @ManagedEvents=Manual */
class DispatcherTimer : public Timeline {
public:
	/* @GenerateCBinding,GeneratePInvoke,MainThread,Version=2 */
	DispatcherTimer ();

	/* @GenerateCBinding,GeneratePInvoke,Version=2 */
	void Start ();

	/* @GenerateCBinding,GeneratePInvoke,Version=2 */
	void Stop ();

	const static int TickEvent;

	void Restart ();

	virtual Duration GetNaturalDurationCore (Clock *clock);
	virtual void TeardownClock ();

protected:
	virtual void OnClockCompleted ();

private:
	bool stopped;
	bool started;
	bool ontick;
};

#endif /* MOON_TIMELINE_H */
