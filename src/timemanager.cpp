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

#include <config.h>

#include "clock.h"
#include "timeline.h"
#include "timemanager.h"
#include "timesource.h"
#include "runtime.h"

#define PUT_TIME_MANAGER_TO_SLEEP 0

#if TIMERS
#define STARTTICKTIMER(id,str) STARTTIMER(id,str)
#define ENDTICKTIMER(id,str) ENDTIMER(id,str)
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


class RootClockGroup : public ClockGroup
{
public:
	RootClockGroup (TimelineGroup *timeline) : ClockGroup (timeline, true) { }

	virtual bool UpdateFromParentTime (TimeSpan parentTime)
	{
		bool rv = Clock::UpdateFromParentTime (parentTime);

		bool children_rv = false;
		for (GList *l = child_clocks; l; l = l->next) {
			Clock *clock = (Clock*)l->data;
			children_rv = clock->UpdateFromParentTime (current_time) || children_rv;
		}

		return rv && children_rv;
	}

protected:
	virtual ~RootClockGroup () { }
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
	emitting = false;

	applier = new Applier ();

	timeline = new ParallelTimeline();
	timeline->SetDuration (Duration::Forever);
	root_clock = new RootClockGroup (timeline);
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

	root_clock->Dispose ();

	timeline->unref ();
	timeline = NULL;

	root_clock->unref ();
	root_clock = NULL;

	delete applier;
	applier = NULL;

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

void
TimeManager::InvokeTickCalls ()
{
	emitting = true;
	TickCall *call;
	while ((call = (TickCall *) tick_calls.Pop ())) {
		call->func (call->data);
		delete call;
	}
	dispatcher_calls.Lock ();
	emitting = false;
	dispatcher_calls.MoveTo (tick_calls);
	dispatcher_calls.Unlock ();
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
		source->SetTimerFrequency (current_timeout);
		source->Start();
	}
#endif
}

struct TickCallFindData {
	TickCallHandler func;
	EventObject *data;
};

void
TimeManager::RemoveTickCall (TickCallHandler func, EventObject *tick_data)
{
	TickCallFindData fd;

	fd.func = func;
	fd.data = tick_data;

	tick_calls.Lock ();

	List::Node * call = tick_calls.LinkedList ()->Find (find_tick_call, &fd);
	if (call)
		tick_calls.LinkedList ()->Remove (call);
#if PUT_TIME_MANAGER_TO_SLEEP
	if (tick_calls.IsEmpty()) {
		flags = (TimeManagerOp)(flags & ~TIME_MANAGER_TICK_CALL);
		if (flags == 0 && source_tick_pending)
			source->Stop();
	}
#endif
	tick_calls.Unlock ();
}

void
TimeManager::AddDispatcherCall (TickCallHandler func, EventObject *tick_data)
{
	dispatcher_calls.Lock ();
	if (emitting)
		dispatcher_calls.LinkedList ()->Append (new TickCall (func, tick_data));
	else
		tick_calls.Push (new TickCall (func, tick_data));
	dispatcher_calls.Unlock ();

#if PUT_TIME_MANAGER_TO_SLEEP
	flags = (TimeManagerOp)(flags | TIME_MANAGER_TICK_CALL);
	if (!source_tick_pending) {
		source_tick_pending = true;
		source->SetTimerFrequency (current_timeout);
		source->Start();
	}
#endif
}


bool
find_tick_call (List::Node *node, void *data)
{
	TickCall *tc = (TickCall*)node;
	TickCallFindData *fd = (TickCallFindData*)data;

	return (tc->func == fd->func &&
		tc->data == fd->data);
}

void
TimeManager::NeedRedraw ()
{
#if PUT_TIME_MANAGER_TO_SLEEP
	flags = (TimeManagerOp)(flags | TIME_MANAGER_RENDER);
	if (!source_tick_pending) {
		source_tick_pending = true;
		source->SetTimerFrequency (current_timeout);
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
		source->SetTimerFrequency (current_timeout);
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
	// printf ("%" G_GINT64_FORMAT " / %" G_GINT64_FORMAT " (%.2f) ", clock->GetCurrentTime(), clock->GetNaturalDuration().GetTimeSpan(), clock->GetCurrentProgress());
	printf ("%" G_GINT64_FORMAT " (%.2f) ", clock->GetCurrentTime(), clock->GetCurrentProgress());

	printf ("%" G_GINT64_FORMAT " ", clock->GetTimeline()->GetBeginTime());

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

void
TimeManager::AddClock (Clock *clock)
{
	root_clock->AddChild (clock);

	// we delay starting the surface's ClockGroup until the first
	// child has been added.  otherwise we run into timing issues
	// between timelines that explicitly set a BeginTime and those
	// that don't (and so default to 00:00:00).
	if (root_clock->GetClockState() != Clock::Active)
		root_clock->Begin (GetCurrentTime());

	NeedClockTick ();
}

void
TimeManager::SourceTick ()
{
	TimeManagerOp current_flags = flags;

#if PUT_TIME_MANAGER_TO_SLEEP
	flags = (TimeManagerOp)0;
#endif

	if (current_flags & TIME_MANAGER_TICK_CALL) {
		STARTTICKTIMER (tm_tick_call, "TimeManager::Tick - Call");
		InvokeTickCalls ();
		ENDTICKTIMER (tm_tick_call, "TimeManager::Tick - Call");
	}

	if (current_flags & TIME_MANAGER_UPDATE_CLOCKS) {
		STARTTICKTIMER (tick_update_clocks, "TimeManager::Tick - UpdateClocks");
		current_global_time = source->GetNow();
		current_global_time_usec = current_global_time / 10;

		bool need_another_tick = root_clock->UpdateFromParentTime (GetCurrentTime());

		if (need_another_tick)
			NeedClockTick ();
	
		// ... then cause all clocks to raise the events they've queued up
		root_clock->RaiseAccumulatedEvents ();
		
		applier->Apply ();
		applier->Flush ();
	
		root_clock->RaiseAccumulatedCompleted ();

#if CLOCK_DEBUG
		ListClocks ();
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
		Emit (RenderEvent, new RenderingEventArgs (get_now()));
		ENDTICKTIMER (tick_render, "TimeManager::Tick - Render");
	}

	last_global_time = current_global_time;
}
