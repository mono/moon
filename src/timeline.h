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

#include "clock.h"
#include <glib.h>
#include <stdint.h>

/* @Namespace=System.Windows.Media.Animation */
class Timeline : public DependencyObject {
	DependencyObject *manual_target;
	
 protected:
	virtual ~Timeline ();

 public:
	/* @PropertyType=bool,DefaultValue=false,GenerateAccessors */
	const static int AutoReverseProperty;
 	/* @PropertyType=TimeSpan,Nullable */
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
	bool HasBeginTime ();
	
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
	
	virtual Clock *AllocateClock () { return new Clock (this); }
	virtual bool Validate ();

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


 private:
 	bool had_parent;
	TimelineStatus timeline_status;
};


/* @Namespace=System.Windows.Media.Animation */
class TimelineCollection : public DependencyObjectCollection {
 protected:
	virtual ~TimelineCollection ();

 public:
 	/* @GenerateCBinding,GeneratePInvoke */
	TimelineCollection ();
	
	virtual bool AddedToCollection (Value *value, MoonError *error);
	virtual Type::Kind GetElementType() { return Type::TIMELINE; }
};


/* @Namespace=None,ManagedDependencyProperties=None */
class TimelineGroup : public Timeline {
 protected:
	virtual ~TimelineGroup ();
	
 public:
	/* @PropertyType=TimelineCollection,AutoCreateValue,GenerateAccessors */
	const static int ChildrenProperty;
	
 	/* @GenerateCBinding,GeneratePInvoke */
	TimelineGroup ();
	
	virtual Clock *AllocateClock ();
	virtual bool Validate ();
	
	void AddChild (Timeline *child);
	void RemoveChild (Timeline *child);
	
	//
	// Property Accessors
	//
	void SetChildren (TimelineCollection *children);
	TimelineCollection *GetChildren ();
};


/* @Namespace=None */
class ParallelTimeline : public TimelineGroup {
 protected:
	virtual ~ParallelTimeline ();

 public:
 	/* @GenerateCBinding,GeneratePInvoke */
	ParallelTimeline ();

	virtual Duration GetNaturalDurationCore (Clock *clock);
};


/* @Namespace=System.Windows.Media */
class TimelineMarker : public DependencyObject {
 protected:
	virtual ~TimelineMarker ();

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
};


/* @Namespace=Mono,Version=2 */
class DispatcherTimer : public TimelineGroup {
	Clock *root_clock;
	static void OnTick (EventObject *sender, EventArgs *calldata, gpointer data);
	bool stopped;
	bool started;

 protected:
	virtual ~DispatcherTimer ();

 public:
	/* @GenerateCBinding,GeneratePInvoke,MainThread,Version=2 */
	DispatcherTimer ();

	/* @GenerateCBinding,GeneratePInvoke,Version=2 */
	void Start ();

	/* @GenerateCBinding,GeneratePInvoke,Version=2 */
	void Stop ();

	const static int TickEvent;

	bool IsStopped () { return stopped; }
	bool IsStarted () { return started; }
	void SetStarted (bool s) { started = s; }
	void Run ();

	virtual Duration GetNaturalDurationCore (Clock *clock);
};

#endif /* MOON_TIMELINE_H */
