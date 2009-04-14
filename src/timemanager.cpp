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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "clock.h"
#include "timeline.h"
#include "timemanager.h"
#include "timesource.h"
#include "runtime.h"

#define TIME_TICK 0
#define USE_SMOOTHING 1
//#define PUT_TIME_MANAGER_TO_SLEEP 1

#if TIME_TICK
#define STARTTICKTIMER(id,str) STARTTIMER(id,str)
#define ENDTICKTIMER(id,str) ENDTIMER(it,str)
#else
#define STARTTICKTIMER(id,str)
#define ENDTICKTIMER(id,str)
#endif


#define MINIMUM_FPS 5
#define DEFAULT_FPS 50
#define MAXIMUM_FPS 50

#define FPS_TO_DELAY(fps) (int)(((double)1/(fps)) * 1000)
#define DELAY_TO_FPS(delay) (1000.0 / delay)

class TickCall : public List::Node {
 public:
 	TickCallHandler func;
 	EventObject *data;
 	TickCall (TickCallHandler func, EventObject *data)
 	{
	 	this->func = func;
	 	this->data = data;
	 	if (this->data)
	 		this->data->ref ();
 	}
 	virtual ~TickCall ()
 	{
	 	if (data)
	 		data->unref ();
 	}
};


TimeManager::TimeManager ()
{
	SetObjectType (Type::TIMEMANAGER);

	if (moonlight_flags & RUNTIME_INIT_MANUAL_TIMESOURCE)
		source = new ManualTimeSource();
	else
		source = new SystemTimeSource(Deployment::GetCurrent ());

	current_timeout = FPS_TO_DELAY (DEFAULT_FPS);  /* something suitably small */
	max_fps = MAXIMUM_FPS;
	flags = (TimeManagerOp) (TIME_MANAGER_UPDATE_CLOCKS | TIME_MANAGER_RENDER | TIME_MANAGER_TICK_CALL /*| TIME_MANAGER_UPDATE_INPUT*/);

	start_time = source->GetNow ();
	start_time_usec = start_time / 10;
	source->AddHandler (TimeSource::TickEvent, source_tick_callback, this);

	registered_timeouts = NULL;
	source_tick_pending = false;
	first_tick = true;

	applier = new Applier ();

	timeline = new ParallelTimeline();
	timeline->SetDuration (Duration::Automatic);
	root_clock = new ClockGroup (timeline, true);
	char *name = g_strdup_printf ("Surface clock group for time manager (%p)", this);
	root_clock->SetValue(DependencyObject::NameProperty, name);
	g_free (name);
	root_clock->SetTimeManager (this);
}

TimeManager::~TimeManager ()
{
	source->RemoveHandler (TimeSource::TickEvent, source_tick_callback, this);
	source->unref ();
	source = NULL;

	timeline->unref ();
	timeline = NULL;

	root_clock->unref ();
	root_clock = NULL;

	delete applier;

	RemoveAllRegisteredTimeouts ();
}

void
TimeManager::SetMaximumRefreshRate (int hz)
{
	if (hz == 0)
		hz = 1;
	
	max_fps = hz;
	current_timeout = FPS_TO_DELAY (hz);
	source->SetTimerFrequency (current_timeout);
	first_tick = true;
}

void
TimeManager::Start()
{
	last_global_time = current_global_time = source->GetNow();
	current_global_time_usec = current_global_time / 10;
	source->SetTimerFrequency (current_timeout);
	source->Start ();
	source_tick_pending = true;
}

void
TimeManager::Stop ()
{
	source->Stop ();
}

void
TimeManager::Shutdown ()
{
	RemoveAllRegisteredTimeouts ();
	source->Stop ();
}

void
TimeManager::source_tick_callback (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((TimeManager *) closure)->SourceTick ();
}

bool
TimeManager::InvokeTickCall ()
{
	TickCall *call = (TickCall *) tick_calls.Pop ();
	
	if (call == NULL)
		return false;
	
	call->func (call->data);
	delete call;
	
	return true;
}

guint
TimeManager::AddTimeout (gint priority, guint ms_interval, GSourceFunc func, gpointer tick_data)
{
	guint rv = g_timeout_add_full (priority, ms_interval, func, tick_data, NULL);
	registered_timeouts = g_list_prepend (registered_timeouts, GUINT_TO_POINTER (rv));
	return rv;
}

void
TimeManager::RemoveTimeout (guint timeout_id)
{
	g_source_remove (timeout_id);
	registered_timeouts = g_list_remove_all (registered_timeouts, GUINT_TO_POINTER (timeout_id));
}

void
TimeManager::RemoveAllRegisteredTimeouts ()
{
	GList *t;
	for (t = registered_timeouts; t; t = t->next)
		g_source_remove (GPOINTER_TO_UINT (t->data));

	g_list_free (registered_timeouts);
	registered_timeouts = NULL;
}

void
TimeManager::AddTickCall (TickCallHandler func, EventObject *tick_data)
{
	tick_calls.Push (new TickCall (func, tick_data));

#if PUT_TIME_MANAGER_TO_SLEEP
	flags = (TimeManagerOp)(flags | TIME_MANAGER_TICK_CALL);
	if (!source_tick_pending) {
		source_tick_pending = true;
		source->SetTimerFrequency (0);
		source->Start();
	}
#endif
}

void
TimeManager::RemoveTickCall (TickCallHandler func)
{
	tick_calls.Lock ();
	List::Node * call = tick_calls.LinkedList ()->Find (find_tick_call, (void*)func);
	if (call)
		tick_calls.LinkedList ()->Remove (call);
	tick_calls.Unlock ();
}

bool
find_tick_call (List::Node *node, void *data)
{
	if (((TickCall*)node)->func == data)
		return true;
	return false;
}

void
TimeManager::NeedRedraw ()
{
#if PUT_TIME_MANAGER_TO_SLEEP
	flags = (TimeManagerOp)(flags | TIME_MANAGER_RENDER);
	if (!source_tick_pending) {
		source_tick_pending = true;
		source->SetTimerFrequency (0);
		source->Start();
	}
#endif
}

void
TimeManager::NeedClockTick ()
{
#if PUT_TIME_MANAGER_TO_SLEEP
	flags = (TimeManagerOp)(flags | TIME_MANAGER_UPDATE_CLOCKS);
	if (!source_tick_pending) {
		source_tick_pending = true;
		source->SetTimerFrequency (0);
		source->Start();
	}
#endif
}


static void
spaces (int n)
{
	while (n--) putchar (' ');

}

static void
output_clock (Clock *clock, int level)
{
	spaces (level);
	printf (clock->Is(Type::CLOCKGROUP) ? "ClockGroup " : "Clock ");
	printf ("(%p) ", clock);
	if (clock->GetName ()) {
		printf ("'%s' ", clock->GetName());
	}

	// getting the natural duration here upsets the clock, so let's not
	// printf ("%lld / %lld (%.2f) ", clock->GetCurrentTime(), clock->GetNaturalDuration().GetTimeSpan(), clock->GetCurrentProgress());
	printf ("%lld (%.2f) ", clock->GetCurrentTime(), clock->GetCurrentProgress());

	printf ("%lld ", clock->GetBeginTime());

	switch (clock->GetClockState()) {
	case Clock::Active:
		printf ("A");
		break;
	case Clock::Filling:
		printf ("F");
		break;
	case Clock::Stopped:
		printf ("S");
		break;
	}

	if (clock->GetIsPaused())
		printf (" (paused)");

	if (clock->GetIsReversed())
		printf (" (rev)");

	printf ("\n");

	if (clock->Is(Type::CLOCKGROUP)) {
		ClockGroup *cg = (ClockGroup*)clock;
		level += 2;
		for (GList *l = cg->child_clocks; l; l = l->next) {
// 			if (((Clock*)l->data)->GetClockState () != Clock::Stopped)
				output_clock ((Clock*)l->data, level);
		}
	}
}

void
TimeManager::ListClocks()
{
	printf ("Currently registered clocks:\n");
	printf ("============================\n");

	output_clock (root_clock, 2);

	printf ("============================\n");
}

#if NOT_ANYMORE
void
TimeManager::SourceTick ()
{
// 	printf ("TimeManager::SourceTick\n");
	STARTTICKTIMER (tick, "TimeManager::Tick");
	TimeSpan pre_tick = source->GetNow();

	TimeManagerOp current_flags = flags;

#if PUT_TIME_MANAGER_TO_SLEEP
	flags = (TimeManagerOp)0;
#endif

	source_tick_pending = false;

	if (current_flags & TIME_MANAGER_UPDATE_CLOCKS) {
		STARTTICKTIMER (tick_update_clocks, "TimeManager::Tick - UpdateClocks");
		current_global_time = source->GetNow();
		current_global_time_usec = current_global_time / 10;

		bool need_another_tick = root_clock->Tick ();
		if (need_another_tick)
			flags = (TimeManagerOp)(flags | TIME_MANAGER_UPDATE_CLOCKS);

	
		// ... then cause all clocks to raise the events they've queued up
		root_clock->RaiseAccumulatedEvents ();
		
		applier->Apply ();
		applier->Flush ();
	
		root_clock->RaiseAccumulatedCompleted ();

#if PUT_TIME_MANAGER_TO_SLEEP
		// kind of a hack to make sure we render animation
		// changes in the same tick as when they happen.
		if (flags & TIME_MANAGER_RENDER) {
			current_flags = (TimeManagerOp)(current_flags | TIME_MANAGER_RENDER);
			flags = (TimeManagerOp)(flags & ~TIME_MANAGER_RENDER);
		}
#endif

		ENDTICKTIMER (tick_update_clocks, "TimeManager::Tick - UpdateClocks");
	}

	if (current_flags & TIME_MANAGER_UPDATE_INPUT) {
		STARTTICKTIMER (tick_input, "TimeManager::Tick - Input");
		Emit (UpdateInputEvent);
		ENDTICKTIMER (tick_input, "TimeManager::Tick - Input");
	}

	if (current_flags & TIME_MANAGER_RENDER) {
		// fprintf (stderr, "rendering\n"); fflush (stderr);
		STARTTICKTIMER (tick_render, "TimeManager::Tick - Render");
		Emit (RenderEvent);
		ENDTICKTIMER (tick_render, "TimeManager::Tick - Render");
	}
	
	TimeSpan post_tick = source->GetNow ();
	TimeSpan xt = post_tick - pre_tick;
	TimeSpan target;
	
	// Flush as many async operations from our queue as we can in
	// the time we have left for rendering this frame.
	//
	// Note: If this is our first frame, we allow 1/10 of a second
	// to process queued operations regardless of how much time we
	// have remaining to render this frame and we also do not
	// include the time taken to flush async operations in the FPS
	// smoothing calculation.
	
	if (first_tick)
		target = post_tick + (TIMESPANTICKS_IN_SECOND / 10);
	else
		target = pre_tick + (TIMESPANTICKS_IN_SECOND / max_fps);
	
	if (current_flags & TIME_MANAGER_TICK_CALL) {
		STARTTICKTIMER (tick_call, "TimeManager::Tick - InvokeTickCall");
		bool remaining_tick_calls;
		TimeSpan now = post_tick;
		//int fired = 0;
		
		// Invoke as many async tick calls as we can in the remaining time alotted for rendering this frame
		do {
			remaining_tick_calls = InvokeTickCall ();
			now = get_now ();
			//fired++;
		} while (remaining_tick_calls && now < target);

		if (remaining_tick_calls) {
			flags = (TimeManagerOp)(flags | TIME_MANAGER_TICK_CALL);
			//printf ("Render Statistics:\n");
			//printf ("\ttime alotted per render pass = %d (%d FPS), time needed for render = %lld, time remaining for tick-calls = %lld\n",
			//	(TIMESPANTICKS_IN_SECOND / max_fps), max_fps, xt, target - post_tick);
			//printf ("\tfired %d TickCalls in %lld usec\n", fired, now - post_tick);
		}
		
		if (!first_tick) {
			// update our post_tick and time-elapsed variables
			xt = now - pre_tick;
			post_tick = now;
		}
		
		ENDTICKTIMER (tick_call, "TimeManager::Tick - InvokeTickCall");
	}
	
#if CLOCK_DEBUG
	ListClocks ();
#endif

	ENDTICKTIMER (tick, "TimeManager::Tick");
	
	/* implement an exponential moving average by way of simple
	   exponential smoothing:

	   s(0) = x(0)
	   s(t) = alpha * x(t) + (1 - alpha) * s(t-1)

	   where 0 < alpha < 1.

	   see http://en.wikipedia.org/wiki/Exponential_smoothing.
	*/
#if USE_SMOOTHING
#define SMOOTHING_ALPHA 0.03 /* we probably want to play with this value some.. - toshok */
#define TIMEOUT_ERROR_DELTA 20 /* how far off of the current_timeout can we be */
	
	/* the s(0) case */
	if (first_tick) {
		first_tick = false;
		previous_smoothed = FPS_TO_DELAY (max_fps);
		return;
	}

	/* the s(t) case */
	TimeSpan current_smoothed = (TimeSpan)(SMOOTHING_ALPHA * xt + (1 - SMOOTHING_ALPHA) * previous_smoothed);

	/* current_smoothed now contains the prediction for what our next delay should be */
	
	
	int suggested_timeout = current_smoothed / 10000;
	if (suggested_timeout < FPS_TO_DELAY (max_fps)) {
		suggested_timeout = FPS_TO_DELAY (max_fps);
	}
	else if (suggested_timeout > FPS_TO_DELAY (MINIMUM_FPS)) {
		suggested_timeout = FPS_TO_DELAY (MINIMUM_FPS);
	}

	current_timeout = suggested_timeout;
	
#if PUT_TIME_MANAGER_TO_SLEEP
	// set up the next timeout here, but only if we need to
	if (flags || registered_timeouts) {
		source->SetTimerFrequency (MAX (0, current_timeout - xt / 10000));
		source->Start();
		source_tick_pending = true;
	}
	else {
		printf ("no work to do, TimeManager going to sleep\n");
		source->Stop ();
	}
#else
		source->SetTimerFrequency (MAX (0, current_timeout - xt / 10000));
#endif
	
	previous_smoothed = current_smoothed;

#if SHOW_SMOOTHING_COST
	TimeSpan post_smooth = source->GetNow();

	printf ("for a clock tick of %lld, we spent %lld computing the smooth delay\n",
		xt, post_smooth - post_tick);
#endif
#endif

	last_global_time = current_global_time;
}
#else
void
TimeManager::SourceTick ()
{
	TimeManagerOp current_flags = flags;

	if (current_flags & TIME_MANAGER_TICK_CALL) {
		bool remaining_tick_calls = false;
		do {
			remaining_tick_calls = InvokeTickCall ();
		} while (remaining_tick_calls);
	}

	if (current_flags & TIME_MANAGER_UPDATE_CLOCKS) {
		STARTTICKTIMER (tick_update_clocks, "TimeManager::Tick - UpdateClocks");
		current_global_time = source->GetNow();
		current_global_time_usec = current_global_time / 10;

		bool need_another_tick = root_clock->Tick ();
		if (need_another_tick)
			flags = (TimeManagerOp)(flags | TIME_MANAGER_UPDATE_CLOCKS);

	
		// ... then cause all clocks to raise the events they've queued up
		root_clock->RaiseAccumulatedEvents ();
		
		applier->Apply ();
		applier->Flush ();
	
		root_clock->RaiseAccumulatedCompleted ();

		ENDTICKTIMER (tick_update_clocks, "TimeManager::Tick - UpdateClocks");
	}

	if (current_flags & TIME_MANAGER_UPDATE_INPUT) {
		STARTTICKTIMER (tick_input, "TimeManager::Tick - Input");
		Emit (UpdateInputEvent);
		ENDTICKTIMER (tick_input, "TimeManager::Tick - Input");
	}

	if (current_flags & TIME_MANAGER_RENDER) {
		// fprintf (stderr, "rendering\n"); fflush (stderr);
		STARTTICKTIMER (tick_render, "TimeManager::Tick - Render");
		Emit (RenderEvent);
		ENDTICKTIMER (tick_render, "TimeManager::Tick - Render");
	}
	
	last_global_time = current_global_time;
}
#endif
