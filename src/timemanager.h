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

#ifndef MOON_TIMEMANAGER_H
#define MOON_TIMEMANAGER_H

#include <glib.h>

#include "applier.h"
#include "timesource.h"
#include "dependencyobject.h"

namespace Moonlight {

// our root level time manager (basically the object that registers
// the gtk_timeout and drives all Clock objects
/* @Namespace=None,ManagedEvents=Manual */
class TimeManager : public EventObject {
public:
	TimeManager ();

	void Start ();
	void Stop ();
	
	void Shutdown ();

	TimeSource *GetSource() { return source; }

	void AddClock (Clock *clock);

	virtual TimeSpan GetCurrentTime ()     { return current_global_time - start_time; }
	virtual TimeSpan GetLastTime ()        { return last_global_time - start_time; }
	TimeSpan GetCurrentTimeUsec () { return current_global_time_usec - start_time_usec; }

	/* @GeneratePInvoke */
	void AddTickCall (TickCallHandler handler, EventObject *tick_data);
	/* @GeneratePInvoke */
	void RemoveTickCall (TickCallHandler handler, EventObject *tick_data);
	/* @GeneratePInvoke */
	void AddDispatcherCall (TickCallHandler handler, EventObject *tick_data);

	void NeedRedraw ();
	void NeedClockTick ();

	void InvokeTickCalls ();

	/* @GeneratePInvoke */
	guint AddTimeout (gint priority, guint ms_interval, MoonSourceFunc func, gpointer timeout_data);
	/* @GeneratePInvoke */
	void RemoveTimeout (guint timeout_id);

	/* @GeneratePInvoke,ManagedAccess=Internal */
	void SetMaximumRefreshRate (int hz);
	/* @GeneratePInvoke,ManagedAccess=Internal */
	int GetMaximumRefreshRate () { return max_fps; }

	// Events you can AddHandler to
	const static int UpdateInputEvent;
	const static int RenderEvent;

	void ListClocks ();
	Applier* GetApplier () { return applier; }
	
protected:
	virtual ~TimeManager ();

private:

	RenderingEventArgs *rendering_args;

	TimelineGroup *timeline;
	ClockGroup *root_clock;
	Applier *applier;

	bool was_stopped;
	TimeSpan stop_time;

	void SourceTick ();

	void RemoveAllRegisteredTimeouts ();

	TimeSpan current_global_time;
	TimeSpan last_global_time;
	TimeSpan start_time;

	TimeSpan current_global_time_usec;
	TimeSpan start_time_usec;

	static void source_tick_callback (EventObject *sender, EventArgs *calldata, gpointer closure);
	bool source_tick_pending;
	int current_timeout;
	int max_fps;
	bool first_tick;
	bool emitting;

	TimeSpan previous_smoothed;

	enum TimeManagerOp {
		TIME_MANAGER_UPDATE_CLOCKS = 0x01,
		TIME_MANAGER_RENDER = 0x02,
		TIME_MANAGER_TICK_CALL = 0x04,
		TIME_MANAGER_UPDATE_INPUT = 0x08
	};

	TimeManagerOp flags;

	TimeSource *source;

	Queue tick_calls;
	Queue dispatcher_calls;

	GList *registered_timeouts;
};

G_BEGIN_DECLS

bool find_tick_call (List::Node *node, void *data);

G_END_DECLS

};
#endif /* MOON_TIMEMANAGER_H */
