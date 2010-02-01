/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * dependencyobject.cpp: 
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "debug.h"
#include "namescope.h"
#include "list.h"
#include "collection.h"
#include "dependencyobject.h"
#include "textblock.h"
#include "timemanager.h"
#include "runtime.h"
#include "uielement.h"
#include "animation.h"
#include "deployment.h"

// event handlers for c++
class EventClosure : public List::Node {
public:
	EventClosure (EventHandler func, gpointer data, GDestroyNotify data_dtor, int token) {
		this->func = func;
		this->data = data;
		this->data_dtor = data_dtor;
		this->token = token;

		pending_removal = false;
		emit_count = 0;
	}
	
	~EventClosure ()
	{
		if (data_dtor)
			data_dtor (data);
	}

	EventHandler func;
	gpointer data;
	GDestroyNotify data_dtor;
	int token;
	bool pending_removal;
	int emit_count;
};

struct EmitContext {
	int length;
	bool only_unemitted;
	int starting_generation;
	EventClosure **closures;

	EmitContext()
	{
		length = 0;
		closures = NULL;
	}
	~EmitContext()
	{
		g_free (closures);
	}
};

class EmitContextNode : public List::Node {
public:
	EmitContextNode (EmitContext *ctx) : ctx (ctx) { }

	virtual ~EmitContextNode () { delete ctx; }

	EmitContext *GetEmitContext () { return ctx; }

private:
	EmitContext *ctx;
};

struct EventList {
	int current_token;
	int last_foreach_generation;
	List *context_stack;
	EventClosure *onevent;
	List *event_list;
};

class EventLists {
public:
	int size;
	int emitting;
	EventList *lists;
	
	EventLists (int n)
	{
		size = n;
		emitting = 0;
		lists = new EventList [size];
		for (int i = 0; i < size; i++) {
			lists [i].current_token = 1;
			lists [i].last_foreach_generation = -1;
			lists [i].context_stack = new List();
			lists [i].onevent = NULL;
			lists [i].event_list = new List ();
		}
	}

	~EventLists ()
	{
		for (int i = 0; i < size; i++) {
			delete lists [i].event_list;
			delete lists [i].onevent;
			delete lists [i].context_stack;
		}
		delete [] lists;
	}
};

/*
 * EventObject
 */

#if OBJECT_TRACKING
#define OBJECT_TRACK(x,y) Track((x),(y))
#else
#define OBJECT_TRACK(x,y)
#endif

EventObject::EventObject ()
{
	Initialize (NULL, Type::EVENTOBJECT);
}

EventObject::EventObject (Type::Kind type)
{
	Initialize (NULL, type);
}

EventObject::EventObject (Type::Kind type, bool multi_threaded_safe)
{
	Initialize (NULL, type);
	if (multi_threaded_safe)
		flags |= (gint32) MultiThreadedSafe;
}

EventObject::EventObject (Deployment *deployment)
{
	Initialize (deployment, Type::EVENTOBJECT);
}

EventObject::EventObject (Deployment *deployment, Type::Kind type)
{
	Initialize (deployment, type);
}

static gint current_id = 0;

void
EventObject::Initialize (Deployment *depl, Type::Kind type)
{
	if (depl == NULL)
		depl = Deployment::GetCurrent ();
	
	object_type = type;
	deployment = depl;
	if (deployment != NULL && this != deployment)
		deployment->ref ();
	surface = NULL;
	flags = g_atomic_int_exchange_and_add (&current_id, 1);
	refcount = 1;
	events = NULL;
	toggleNotifyListener = NULL;

#if OBJECT_TRACKING
	switch (object_type) {
	case Type::INVALID:
		Track ("Created", "<unknown>");
		break;
	case Type::DEPLOYMENT:
		Track ("Created", "Deployment");
		break;
	default:
		Track ("Created", depl->GetTypes ()->Find (object_type)->GetName ());
		break;
	}
	
	if (object_type != Type::DEPLOYMENT)
		Deployment::GetCurrent ()->TrackObjectCreated (this);
#endif

#if SANITY
	if (object_type == Type::INVALID)
		g_warning ("EventObject::EventObject (): created object with type: INVALID.\n");
	if (deployment == NULL)
		g_warning ("EventObject::EventObject (): created object with a null deployment.\n");
#endif
}

EventObject::~EventObject()
{
#if OBJECT_TRACKING
	if (object_type != Type::DEPLOYMENT)
		Deployment::GetCurrent ()->TrackObjectDestroyed (this);
	Track ("Destroyed", "");
#endif

#if SANITY
	if (refcount != 0) {
		g_warning ("EventObject::~EventObject () #%i was deleted before its refcount reached 0 (current refcount: %i)\n", GetId (), refcount);
	}
#endif

	delete events;
	
	// We can't unref the deployment in Dispose, it breaks
	// object tracking.
	if (deployment && this != deployment) {
		deployment->unref ();
		deployment = NULL;
	}
}

static pthread_rwlock_t surface_lock = PTHREAD_RWLOCK_INITIALIZER;

bool
EventObject::SetSurfaceLock ()
{
	int result;
	
	if ((result = pthread_rwlock_wrlock (&surface_lock)) != 0) {
		printf ("EventObject::SetSurface (%p): Couldn't aquire write lock: %s\n", surface, strerror (result));
		return false;
	}
	
	return true;
}


void
EventObject::SetSurface (Surface *surface)
{
	if (!Surface::InMainThread () && surface != this->surface) {
		g_warning ("EventObject::SetSurface (): This method must not be called on any other than the main thread!\n");
#if DEBUG
		if (debug_flags & RUNTIME_DEBUG_DP)
			print_stack_trace ();
#endif
		return;
	}
	
	this->surface = surface;
}

void
EventObject::SetSurfaceUnlock ()
{
	pthread_rwlock_unlock (&surface_lock);
}

void
EventObject::AddTickCallSafe (TickCallHandler handler, EventObject *data)
{
	int result;
	
	/*
	 * This code assumes that the surface won't be destructed without setting the surface field on to NULL first. 
	 * In other words: if we have a read lock here, the surface won't get destroyed, given that setting
	 * the surface field to NULL will block until we release the read lock.
	 */
	
	if ((result = pthread_rwlock_rdlock (&surface_lock)) != 0) {
		printf ("EventObject::AddTickCallSafe (): Couldn't aquire read lock: %s\n", strerror (result));
		return;
	}

	AddTickCallInternal (handler, data);
 	
	pthread_rwlock_unlock (&surface_lock);
}

void
EventObject::AddTickCall (TickCallHandler handler, EventObject *data)
{
	if (!Surface::InMainThread ()) {
		g_warning ("EventObject::AddTickCall (): This method must not be called on any other than the main thread! Tick call won't be added.\n");
#if DEBUG
		if (debug_flags & RUNTIME_DEBUG_DP)
			print_stack_trace ();
#endif
		return;
	}
	
	AddTickCallInternal (handler, data);
}

void
EventObject::AddTickCallInternal (TickCallHandler handler, EventObject *data)
{
	Surface *surface;
	TimeManager *timemanager;
	
	surface = GetSurface ();
	
	if (surface == NULL)
		surface = GetDeployment ()->GetSurface ();
	
	if (!surface) {
		LOG_DP ("EventObject::AddTickCall (): Could not add tick call, no surface\n");
		return;
	}
	
	timemanager = surface->GetTimeManager ();
	
	if (!timemanager) {
		LOG_DP ("EventObject::AddTickCall (): Could not add tick call, no time manager\n");
		return;
	}

	timemanager->AddTickCall (handler, data ? data : this);
}

#if SANITY
Deployment *
EventObject::GetDeployment ()
{
	if (deployment == NULL)
		g_warning ("EventObject::GetDeployment () should not be reached with a null deployment");
	
	if (deployment != Deployment::GetCurrent () && Deployment::GetCurrent () != NULL) {
		g_warning ("EventObject::GetDeployment () our deployment %p doesn't match Deployment::GetCurrent () %p", deployment, Deployment::GetCurrent ());
		// print_stack_trace ();
	}

	return deployment;
}
#endif

void
EventObject::SetCurrentDeployment (bool domain, bool register_thread)
{	
	if (deployment != NULL) {
		if (register_thread)
			Deployment::RegisterThread (deployment);
		Deployment::SetCurrent (deployment, domain);
	}
}

Surface *
EventObject::GetSurface ()
{
#if 0
	Deployment *current_deployment = Deployment::GetCurrent ();
	Application *current_application;
	current_application = deployment != NULL ? deployment->GetCurrentApplication () : (current_deployment != NULL ? current_deployment->GetCurrentApplication () : NULL);

	if (deployment != NULL && deployment != current_deployment) {
		printf ("EventObject::GetSurface (): WARNING deployment: %p, Deployment::GetCurrent (): %p type: %s, id: %i\n", deployment, current_deployment, GetTypeName (), GET_OBJ_ID (this));
		print_stack_trace ();
	}
	// current_application is null in the Surface ctor
	if (!(current_application == NULL || surface == NULL || current_application->GetSurface () == surface))
		printf ("EventObject::GetSurface (): assert failed: current application: %p, surface: %p, current application's surface: %p\n", current_application, surface, current_application->GetSurface ());
#endif
	
	return surface; // When surface have been removed, return the Surface stored on the Deployment.
}

void
EventObject::Dispose ()
{
	if (!IsDisposed () && Surface::InMainThread ()) {
		// Dispose can be called multiple times, but Emit only once. When DestroyedEvent was in the dtor, 
		// it could only ever be emitted once, don't change that behaviour.
		Emit (DestroyedEvent); // TODO: Rename to DisposedEvent
	}
		
	SetSurface (NULL);
	// Remove attached flag and set the disposed flag.
	flags = (Flags) (flags & ~Attached);
	flags = (Flags) (flags | Disposed);
}

bool
EventObject::IsDisposed ()
{
	return (flags & Disposed) != 0;
}

void
EventObject::ref ()
{
	int v = g_atomic_int_exchange_and_add (&refcount, 1);
		
#if DEBUG		
	if (deployment != Deployment::GetCurrent ()) {
		printf ("EventObject::ref (): the type '%s' whose id is %i was created on a deployment (%p) different from the current deployment (%p).\n", GetTypeName (), GET_OBJ_ID (this), deployment, Deployment::GetCurrent ());
		// print_stack_trace ();
	}
#endif

	if (v == 0) {
		// Here something really bad happened, this object is probably being reffed again because
		// of some action in the destructor. There is no way to recover from this now, no matter what 
		// we do the code that called ref now will be accessing a deleted object later on, which may or 
		// may not crash. It might very well be an exploitable security problem. Anyways when unref is called, we 
		// have a second delete on the same object, which *will* crash. To make things easier and safer
		// lets just abort right away.
#if OBJECT_TRACKING
		PrintStackTrace ();
#endif
		// Due to our mixed usage of Dispose and dtor, currently there are valid cases of reffing
		// an object with refcount = 0. Use a warning instead of error until the mixed usage is
		// gone.
		g_warning ("Ref was called on an object with a refcount of 0.\n");

	} else if (v == 1 && toggleNotifyListener) {
		if (getenv ("MOONLIGHT_ENABLE_TOGGLEREF"))
			toggleNotifyListener->Invoke (false);
	}

	OBJECT_TRACK ("Ref", GetTypeName ());
}

void 
EventObject::unref ()
{
	// we need to retrieve all instance fields into locals before decreasing the refcount
	// TODO: do we need some sort of gcc foo (volatile variables, memory barries)
	// to ensure that gcc does not optimize the fetches below away
	ToggleNotifyListener *toggle_listener = this->toggleNotifyListener;
#if OBJECT_TRACKING
	Deployment *depl = this->deployment ? this->deployment : Deployment::GetCurrent ();
	const char *type_name = depl == NULL ? NULL : Type::Find (depl, GetObjectType ())->GetName ();
#endif	

	if (!IsMultiThreadedSafe () && !Surface::InMainThread ()) {
		unref_delayed ();
		return;
	}

	int v = g_atomic_int_exchange_and_add (&refcount, -1) - 1;
	
	// from now on we can't access any instance fields if v > 0
	// since another thread might have unreffed and caused our destruction

	if (v == 0 && events != NULL && events->emitting) {
		g_atomic_int_exchange_and_add (&refcount, 1);
		unref_delayed ();
		return;
	}

	OBJECT_TRACK ("Unref", type_name);

	if (v == 0) {
		// here we *can* access instance fields, since we know that we haven't been
		// desctructed already.
		Dispose ();
		
#if SANITY
		if ((flags & Disposed) == 0)
			printf ("EventObject::unref (): the type '%s' (or any of its parent types) forgot to call its base class' Dispose method.\n", GetTypeName ());
#endif

		// We need to check again if the refcount really is zero,
		// the object might have resurrected in the Dispose.
		// TODO: we should disallow resurrection, it's not thread-safe
		// if we got resurrected and unreffed, we'd be deleted by now
		// in which case we'll double free here.
		v = g_atomic_int_get (&refcount);
		if (v == 0)
			delete this;
			
	} else if (v == 1 && toggle_listener) {
		// we know that toggle_listener hasn't been freed, since if it exists, it will have a ref to us which would prevent our destruction
		// note that the instance field might point to garbage after decreasing the refcount above, so we access the local variable we 
		// retrieved before decreasing the refcount.
		if (getenv ("MOONLIGHT_ENABLE_TOGGLEREF"))
			toggle_listener->Invoke (true);
	}

#if SANITY
	if (v < 0) {
		g_warning ("EventObject::Unref (): NEGATIVE REFCOUNT id: %i v: %i refcount: %i", GET_OBJ_ID (this), v, refcount);
		print_stack_trace ();
	}
#endif
}

void
EventObject::AddToggleRefNotifier (ToggleNotifyHandler tr)
{
	if (toggleNotifyListener)
		return;

	this->ref ();
	toggleNotifyListener = new ToggleNotifyListener (this, tr);
}

void
EventObject::RemoveToggleRefNotifier ()
{
	if (!toggleNotifyListener)
		return;

	delete toggleNotifyListener;
	toggleNotifyListener = NULL;
	this->unref ();
}

#if OBJECT_TRACKING
// Define the ID of the object you want to track
// Object creation, destruction and all ref/unrefs
// are logged to the console, with a stacktrace.
static bool object_id_fetched = false;
static int object_id = -1;
static const char *track_object_type = NULL;
static bool use_visi_output = false;
static bool track_all = false;

#define OBJECT_TRACK_ID (0)

GHashTable* EventObject::objects_alive = NULL;

void
EventObject::Track (const char* done, const char* typname)
{
	int id = GetId ();
	if (!object_id_fetched) {
		object_id_fetched = true;
		char *sval = getenv ("MOONLIGHT_OBJECT_TRACK_ID");
		if (sval)
			object_id = atoi (sval);
		track_object_type = getenv ("MOONLIGHT_OBJECT_TRACK_TYPE");
		use_visi_output = (getenv ("MOONLIGHT_OBJECT_TRACK_VISI") != NULL);
		track_all = (getenv ("MOONLIGHT_OBJECT_TRACK_ALL") != NULL);
	}

	if (track_all)
		printf ("%p\t%s tracked object of type '%s': %i, current refcount: %i deployment: %p\n", this, done, typname, id, refcount, deployment);

	if (id == object_id || (track_object_type != NULL && typname != NULL && strcmp (typname, track_object_type) == 0)) {
		char *st = NULL;
		// load the stack trace before we print anything
		// this way there's a lot smaller chance of 
		// ending up with other output between the first line (tracked object of type...)
		// and the stack trace when using multiple threads.
		if (!use_visi_output)
			st = get_stack_trace ();
		
		if (!track_all)
			printf ("%p\t%s tracked object of type '%s': %i, current refcount: %i deployment: %p\n", this, done, typname, id, refcount, deployment);

		if (!use_visi_output) {
			printf("%s", st);
		} else {
			print_reftrace (done, typname, refcount, false);
		}
		g_free (st);
	}
}

char *
EventObject::GetStackTrace (const char* prefix)
{
	return get_stack_trace_prefix (prefix);
}

void
EventObject::PrintStackTrace ()
{
	print_stack_trace ();
}
#endif

int
EventObject::AddHandler (const char *event_name, EventHandler handler, gpointer data, GDestroyNotify data_dtor)
{
	int id = GetType()->LookupEvent (event_name);

	if (id == -1) {
		g_warning ("adding handler to event '%s', which has not been registered\n", event_name);
		return -1;
	}

	return AddHandler (id, handler, data, data_dtor);
}

int
EventObject::AddHandler (int event_id, EventHandler handler, gpointer data, GDestroyNotify data_dtor)
{ 
	if (GetType()->GetEventCount() <= 0) {
		g_warning ("adding handler to event with id %d, which has not been registered\n", event_id);
		return -1;
	}
	
	if (events == NULL)
		events = new EventLists (GetType ()->GetEventCount ());

	int token = events->lists [event_id].current_token++;
	
	events->lists [event_id].event_list->Append (new EventClosure (handler, data, data_dtor, token));
	
	return token;
}

void
EventObject::AddOnEventHandler (int event_id, EventHandler handler, gpointer data, GDestroyNotify data_dtor)
{
	if (GetType()->GetEventCount() <= event_id) {
		g_warning ("adding OnEvent handler to event with id %d, which has not been registered\n", event_id);
		return;
	}

	if (events == NULL)
		events = new EventLists (GetType ()->GetEventCount ());

	events->lists [event_id].onevent = new EventClosure (handler, data, data_dtor, 0);
}

void
EventObject::RemoveOnEventHandler (int event_id, EventHandler handler, gpointer data)
{
	if (events == NULL) {
#if SANITY
		fprintf (stderr, "EventObject::RemoveOnEventHandler (%i): no event handlers have been registered.\n", event_id);
#endif
		return;
	}

	if (GetType()->GetEventCount() <= event_id) {
		g_warning ("adding OnEvent handler to event with id %d, which has not been registered\n", event_id);
		return;
	}

	if (events->lists [event_id].onevent) {
		// FIXME check handler + data
		delete events->lists [event_id].onevent;
		events->lists [event_id].onevent = NULL;
	}
}

int
EventObject::AddXamlHandler (const char *event_name, EventHandler handler, gpointer data, GDestroyNotify data_dtor)
{
	int id = GetType ()->LookupEvent (event_name);
	
	if (id == -1) {
		g_warning ("adding xaml handler to event '%s', which has not been registered\n", event_name);
		return -1;
	}
	
	return AddXamlHandler (id, handler, data, data_dtor);
}

int
EventObject::AddXamlHandler (int event_id, EventHandler handler, gpointer data, GDestroyNotify data_dtor)
{ 
	if (GetType ()->GetEventCount () <= 0) {
		g_warning ("adding xaml handler to event with id %d, which has not been registered\n", event_id);
		return -1;
	}
	
	if (events == NULL)
		events = new EventLists (GetType ()->GetEventCount ());

	events->lists [event_id].event_list->Append (new EventClosure (handler, data, data_dtor, 0));
	
	return 0;
}

void
EventObject::RemoveHandler (const char *event_name, EventHandler handler, gpointer data)
{
	int id = GetType()->LookupEvent (event_name);

	if (id == -1) {
		g_warning ("removing handler for event '%s', which has not been registered\n", event_name);
		return;
	}

	RemoveHandler (id, handler, data);
}

int
EventObject::FindHandlerToken (int event_id, EventHandler handler, gpointer data)
{
	if (events == NULL)
		return -1;

	if (GetType()->GetEventCount() <= 0) {
		g_warning ("trying to find token for event with id %d, which has not been registered\n", event_id);
		return -1;
	}

	EventClosure *closure = (EventClosure *) events->lists [event_id].event_list->First ();
	while (closure) {
		if (closure->func == handler && closure->data == data)
			return closure->token;
		
		closure = (EventClosure *) closure->next;
	}
	return -1;
}

int
EventObject::RemoveHandler (int event_id, EventHandler handler, gpointer data)
{
	int token = -1;

	if (events == NULL)
		return token;

	if (GetType()->GetEventCount() <= 0) {
		g_warning ("removing handler for event with id %d, which has not been registered\n", event_id);
		return -1;
	}

	EventClosure *closure = (EventClosure *) events->lists [event_id].event_list->First ();
	while (closure) {
		if (closure->func == handler && closure->data == data) {
			token = closure->token;
 			if (!events->lists [event_id].context_stack->IsEmpty()) {
 				closure->pending_removal = true;
 			} else {
				events->lists [event_id].event_list->Remove (closure);
 			}
			break;
		}
		
		closure = (EventClosure *) closure->next;
	}
	return token;
}

void
EventObject::RemoveHandler (int event_id, int token)
{
	if (events == NULL) {
#if SANITY
		printf ("EventObject::RemoveHandler (%i, %i): no event handlers have been registered\n", event_id, token);
#endif
		return;
	}

	if (GetType()->GetEventCount() <= 0) {
		g_warning ("removing handler for event with id %d, which has not been registered\n", event_id);
		return;
	}

	EventClosure *closure = (EventClosure *) events->lists [event_id].event_list->First ();
	while (closure) {
		if (closure->token == token) {
			if (!events->lists [event_id].context_stack->IsEmpty()) {
				closure->pending_removal = true;
			} else {
				events->lists [event_id].event_list->Remove (closure);
			}
			break;
		}
		
		closure = (EventClosure *) closure->next;
	}
}

void
EventObject::RemoveAllHandlers (gpointer data)
{
	if (events == NULL)
		return;

	int count = GetType ()->GetEventCount ();
	
	for (int i = 0; i < count - 1; i++) {
		EventClosure *closure = (EventClosure *) events->lists [i].event_list->First ();
		while (closure) {
			if (closure->data == data) {
				if (!events->lists [i].context_stack->IsEmpty()) {
					closure->pending_removal = true;
				} else {
					events->lists [i].event_list->Remove (closure);
				}
				break;
			}
			
			closure = (EventClosure *) closure->next;
		}
	}
}

void
EventObject::RemoveMatchingHandlers (int event_id, bool (*predicate)(EventHandler cb_handler, gpointer cb_data, gpointer data), gpointer closure)
{
	if (events == NULL) {
#if SANITY
		fprintf (stderr, "EventObject::RemoveMatchingHandlers (): no handlers have been registered.\n");
#endif		
		return;
	}

	if (GetType()->GetEventCount() <= 0) {
		g_warning ("removing handler for event with id %d, which has not been registered\n", event_id);
		return;
	}

	EventClosure *c = (EventClosure *) events->lists [event_id].event_list->First ();
	while (c) {
		if (predicate (c->func, c->data, closure)) {
			if (!events->lists [event_id].context_stack->IsEmpty()) {
				c->pending_removal = true;
			} else {
				events->lists [event_id].event_list->Remove (c);
			}
			break;
		}
		
		c = (EventClosure *) c->next;
	}
}

void
EventObject::ForeachHandler (int event_id, bool only_new, HandlerMethod m, gpointer closure)
{
	if (events == NULL)
		return;

	EventClosure *event_closure = (EventClosure *) events->lists [event_id].event_list->First ();
	int last_foreach_generation = events->lists [event_id].last_foreach_generation;
	while (event_closure) {
		if (!only_new || event_closure->token >= last_foreach_generation)
			(*m) (this, event_closure->func, event_closure->data, closure);
		event_closure = (EventClosure *) event_closure->next;
	}
	events->lists [event_id].last_foreach_generation = GetEventGeneration (event_id);
}

void
EventObject::ClearForeachGeneration (int event_id)
{
	if (events == NULL)
		return;

	if (GetType()->GetEventCount() <= 0)
		return;

	events->lists [event_id].last_foreach_generation = -1;
}

void
EventObject::ForHandler (int event_id, int token, HandlerMethod m, gpointer closure)
{
	if (events == NULL)
		return;

	EventClosure *event_closure = (EventClosure *) events->lists [event_id].event_list->First ();
	while (event_closure) {
		if (event_closure->token == token) {
			(*m) (this, event_closure->func, event_closure->data, closure);
			break;
		}
		event_closure = (EventClosure *) event_closure->next;
	}
}

bool
EventObject::HasHandlers (int event_id, int newer_than_generation)
{
	if (events == NULL)
		return false;

	EventClosure *event_closure = (EventClosure *) events->lists [event_id].event_list->First ();
	while (event_closure) {
		if (newer_than_generation != -1 || event_closure->token >= newer_than_generation)
			return true;
		event_closure = (EventClosure *) event_closure->next;
	}
	return false;
}

int
EventObject::GetEventGeneration (int event_id)
{
	if (events == NULL)
		return 1; /* first generation */

	return events->lists [event_id].current_token;
}

bool
EventObject::CanEmitEvents (int event_id)
{
	if (IsDisposed ())
		return false;
	
	if (deployment == NULL)
		return false; /* how did this happen? */
	
	if (deployment == this)
		return true; /* Deployment::ShuttingDownEvent and Deployment::AppDomainUnloadedEvent */

	if (event_id == DestroyedEvent) {
		/* We need to allow this one too, namescopes listen to it to ensure
		 * that they detach from objects that are destroyed. Managed code 
		 * doesn't listen to this event, so it's safe. */
		return true;
	}

	if (deployment->IsShuttingDown ()) {
		/* Don't emit events after we've started shutting down, we might have
		 * managed event handlers, which could crash since the appdomain could
		 * have been unloaded */
		return false;
	}
	
	return true;
}

bool
EventObject::EmitAsync (const char *event_name, EventArgs *calldata, bool only_unemitted)
{
	int event_id;
	
	if ((event_id = GetType ()->LookupEvent (event_name)) == -1) {
		g_warning ("trying to emit event '%s', which has not been registered\n", event_name);
		if (calldata)
			calldata->unref ();
		return false;
	}

	if (!CanEmitEvents (event_id)) {
		if (calldata)
			calldata->unref ();
		return false;
	}
	
	return EmitAsync (event_id, calldata, only_unemitted);
}

bool
EventObject::Emit (const char *event_name, EventArgs *calldata, bool only_unemitted, int starting_generation)
{	
	int id = GetType ()->LookupEvent (event_name);

	if (id == -1) {
		g_warning ("trying to emit event '%s', which has not been registered\n", event_name);
		if (calldata)
			calldata->unref ();
		return false;
	}

	if (!CanEmitEvents (id)) {
		if (calldata)
			calldata->unref ();
		return false;
	}
	
	return Emit (id, calldata, only_unemitted, starting_generation);
}

class AsyncEventClosure : public EventObject {
 public:
	EventObject *sender;
	EventArgs *args;
	bool unemitted;
	int generation;
	int event_id;
	
	AsyncEventClosure (EventObject *sender, int event_id, EventArgs *args, bool unemitted, int generation)
	{
		this->sender = sender;
		this->event_id = event_id;
		this->args = args;
		this->unemitted = unemitted;
		this->generation = generation;
		
		sender->ref ();
	}
	
 protected:
	virtual ~AsyncEventClosure ()
	{
		sender->unref ();
	}
};

void
EventObject::emit_async (EventObject *calldata)
{
	AsyncEventClosure *async = (AsyncEventClosure *) calldata;
	
	async->sender->Emit (async->event_id, async->args, async->unemitted, async->generation);
	
	async->unref ();
}

bool
EventObject::EmitAsync (int event_id, EventArgs *calldata, bool only_unemitted)
{
	if (!CanEmitEvents (event_id)) {
		if (calldata)
			calldata->unref ();
		return false;
	}

	AddTickCall (EventObject::emit_async, new AsyncEventClosure (this, event_id, calldata, only_unemitted, GetEventGeneration (event_id)));
	
	return true;
}

struct EmitData {
	EventObject *sender;
	int event_id;
	EventArgs *calldata;
	bool only_unemitted;
};

gboolean
EventObject::EmitCallback (gpointer d)
{
	EmitData *data = (EmitData *) d;
	data->sender->SetCurrentDeployment ();
	data->sender->Emit (data->event_id, data->calldata, data->only_unemitted);
	data->sender->unref ();
	delete data;
#if SANITY
	Deployment::SetCurrent (NULL);
#endif
	return FALSE;
}

bool
EventObject::Emit (int event_id, EventArgs *calldata, bool only_unemitted, int starting_generation)
{
	if (events == NULL) {
		if (calldata)
			calldata->unref ();
		return false;
	}
	
	if (!CanEmitEvents (event_id)) {
		if (calldata)
			calldata->unref ();
		return false;
	}
	
	if (GetType()->GetEventCount() <= 0 || event_id >= GetType()->GetEventCount()) {
		g_warning ("trying to emit event with id %d, which has not been registered\n", event_id);
		if (calldata)
			calldata->unref ();
		return false;
	}

	if (events->lists [event_id].event_list->IsEmpty () && events->lists [event_id].onevent == NULL) {
		if (calldata)
			calldata->unref ();
		return false;
	}

	if (!Surface::InMainThread ()) {
		Surface *surface = deployment ? deployment->GetSurface () : NULL;
		
		if (surface == NULL) {
			printf ("EventObject::Emit (): could not emit event, the deployment %p does not have a surface.\n", deployment);
			return false;
		}
		
		EmitData *data = new EmitData ();
		data->sender = this;
		data->sender->ref ();
		data->event_id = event_id;
		data->calldata = calldata;
		data->only_unemitted = only_unemitted;
		surface->GetTimeManager ()->AddTimeout (MOON_PRIORITY_HIGH, 1, EmitCallback, data);
		return false;
	}

	EmitContext* ctx = StartEmit (event_id, only_unemitted, starting_generation);
	
	if (ctx == NULL)
		return false;
		
	DoEmit (event_id, calldata);

	FinishEmit (event_id, ctx);

	return true;
}

EmitContext*
EventObject::StartEmit (int event_id, bool only_unemitted, int starting_generation)
{
	if (events == NULL)
		return NULL;

	EmitContext *ctx = new EmitContext();
	ctx->only_unemitted = only_unemitted;
	ctx->starting_generation = starting_generation;
	EventClosure *closure;

	if (GetType()->GetEventCount() <= 0 || event_id >= GetType()->GetEventCount()) {
		g_warning ("trying to start emit with id %d, which has not been registered\n", event_id);
		return ctx;
	}

	events->emitting++;

	events->lists [event_id].context_stack->Prepend (new EmitContextNode (ctx));

	if (events->lists [event_id].event_list->IsEmpty ())
		return ctx;

	ctx->length = events->lists [event_id].event_list->Length();
	ctx->closures = g_new (EventClosure*, ctx->length);

	/* make a copy of the event list to use for emitting */
	closure = (EventClosure *) events->lists [event_id].event_list->First ();
	for (int i = 0; closure != NULL; i ++) {
		ctx->closures[i] = closure->pending_removal ? NULL : closure;
		closure = (EventClosure *) closure->next;
	}

	return ctx;
}

bool
EventObject::DoEmit (int event_id, EventArgs *calldata)
{
	if (events == NULL) {
		if (calldata)
			calldata->unref ();
		return false;
	}

	if (events->lists [event_id].context_stack->IsEmpty ()) {
		g_warning ("DoEmit called with no EmitContexts");
		return false;
	}

	EmitContext *ctx = ((EmitContextNode*)events->lists [event_id].context_stack->First())->GetEmitContext();

	if (events->lists [event_id].onevent) {
		EventClosure *closure = events->lists [event_id].onevent;
		closure->func (this, calldata, closure->data);
	}
	else {
		DoEmitCurrentContext (event_id, calldata);
	}

	if (calldata)
		calldata->unref ();

	return ctx->length > 0;
}

void
EventObject::DoEmitCurrentContext (int event_id, EventArgs *calldata)
{
	if (events == NULL)
		return;
		
	if (events->lists [event_id].context_stack->IsEmpty()) {
		g_warning ("DoEmitCurrentContext called with no EmitContexts");
		return;
	}

	EmitContext *ctx = ((EmitContextNode*)events->lists [event_id].context_stack->First())->GetEmitContext();

	/* emit the events using the copied list in the context*/
	for (int i = 0; i < ctx->length; i++) {
		if (calldata && calldata->Is (Type::ROUTEDEVENTARGS)) {
			RoutedEventArgs *rea = (RoutedEventArgs*)calldata;
			if (rea->GetHandled ())
				break;
		}

		EventClosure *closure = ctx->closures[i];

		if (closure && closure->func
		    && (!ctx->only_unemitted || closure->emit_count == 0)
		    && (ctx->starting_generation == -1 || closure->token < ctx->starting_generation)) {
			closure->func (this, calldata, closure->data);

			closure->emit_count ++;
		}
	}
}

void
EventObject::FinishEmit (int event_id, EmitContext *ctx)
{
	if (events == NULL || ctx == NULL)
		return;

	if (GetType()->GetEventCount() <= 0 || event_id >= GetType()->GetEventCount()) {
		g_warning ("trying to finish emit with id %d, which has not been registered\n", event_id);
		return;
	}

	if (events->lists [event_id].context_stack->IsEmpty()) {
		g_warning ("FinishEmit called with no EmitContexts");
		return;
	}

	EmitContextNode *first_node = (EmitContextNode*)events->lists [event_id].context_stack->First();
	EmitContext *first_ctx = first_node->GetEmitContext();

	if (first_ctx != ctx) {
		g_warning ("FinishEmit called out of order");
		return;
	}

	events->lists [event_id].context_stack->Unlink (first_node);

	delete first_node;
	events->emitting--;

	if (events->lists [event_id].context_stack->IsEmpty ()) {
		// Remove closures which are waiting for removal
		EventClosure *closure = (EventClosure *) events->lists [event_id].event_list->First ();
		while (closure != NULL) {
			EventClosure *next = (EventClosure *) closure->next;
			if (closure->pending_removal)
				events->lists [event_id].event_list->Remove (closure);
			closure = next;
		}
	}
}

void
EventObject::unref_delayed ()
{
	Deployment *depl;
	
	OBJECT_TRACK ("DelayedUnref", GetTypeName ());

	// access deployment as long as we have it (until Dispose has been called),
	// after that access the static deployment.
	depl = deployment ? deployment : Deployment::GetCurrent ();
	depl->UnrefDelayed (this);
}

class Listener {
public:
	virtual bool Matches (PropertyChangedEventArgs *args) = 0;
	virtual void Invoke (DependencyObject *sender, PropertyChangedEventArgs *args, MoonError *error) = 0;

	virtual gpointer GetListener () = 0;
	virtual gpointer GetProperty () = 0;
};

class WildcardListener : public Listener {
public:
	WildcardListener (DependencyObject *obj, DependencyProperty *prop)
	{
		this->obj = obj;
		this->prop = prop;
	}

	virtual bool Matches (PropertyChangedEventArgs *args) { return true; }
	virtual void Invoke (DependencyObject *sender, PropertyChangedEventArgs *args, MoonError *error)
	{
		// FIXME we ignore error here.
		obj->OnSubPropertyChanged (prop, sender, args);
	}

	virtual gpointer GetListener ()
	{
		return obj;
	}

	virtual gpointer GetProperty ()
	{
		return prop;
	}

private:

	DependencyObject *obj;
	DependencyProperty *prop;
};

class CallbackListener : public Listener {
public:
	CallbackListener (DependencyProperty *prop, PropertyChangeHandler cb, gpointer closure)
	{
		this->prop = prop;
		this->cb = cb;
		this->closure = closure;
	}

	virtual bool Matches (PropertyChangedEventArgs *args)
	{
		return prop == args->GetProperty ();
	}

	virtual void Invoke (DependencyObject *sender, PropertyChangedEventArgs *args, MoonError *error)
	{
		cb (sender, args, error, closure);
	}

	virtual gpointer GetListener ()
	{
		return (gpointer)cb;
	}

	virtual gpointer GetProperty ()
	{
		return prop;
	}

private:
	PropertyChangeHandler cb;
	DependencyProperty *prop;
	gpointer closure;
};

//
// Registers @listener as a listener on changes to @child_property of this DO.
//
void
DependencyObject::AddPropertyChangeListener (DependencyObject *listener, DependencyProperty *child_property)
{
	listener_list = g_slist_append (listener_list, new WildcardListener (listener, child_property));
}

void
DependencyObject::RemoveListener (gpointer listener, DependencyProperty *child_property)
{
	GSList *next;
	for (GSList *l = listener_list; l; l = next) {
		next = l->next;
		Listener *listen = (Listener *) l->data;
		
		if ((listen->GetListener() == listener)
		    && (child_property == NULL || listen->GetProperty() == child_property)) {
			listener_list = g_slist_delete_link (listener_list, l);
			delete listen;
		}
	}
}

//
// Unregisters @container as a listener on changes to @child_property of this DO.
//
void
DependencyObject::RemovePropertyChangeListener (DependencyObject *listener, DependencyProperty *child_property)
{
	RemoveListener (listener, child_property);
}

void
DependencyObject::AddPropertyChangeHandler (DependencyProperty *property, PropertyChangeHandler cb, gpointer closure)
{
	listener_list = g_slist_append (listener_list, new CallbackListener (property, cb, closure));
}

void
DependencyObject::RemovePropertyChangeHandler (DependencyProperty *property, PropertyChangeHandler cb)
{
	RemoveListener ((gpointer)cb, property);
}

static void
unregister_depobj_values (gpointer  key,
			  gpointer  value,
			  gpointer  user_data)
{
	DependencyObject *this_obj = (DependencyObject*)user_data;
	//DependencyProperty *prop = (DependencyProperty*)key;
	Value *v = (Value*)value;

	if (v != NULL && v->Is (this_obj->GetDeployment (), Type::DEPENDENCY_OBJECT) && v->AsDependencyObject() != NULL) {
		//printf ("unregistering from property %s\n", prop->name);
		DependencyObject *obj = v->AsDependencyObject ();
		obj->RemovePropertyChangeListener (this_obj);
		obj->SetParent (NULL, NULL);
	}
}

void
DependencyObject::RemoveAllListeners ()
{
	AutoCreatePropertyValueProvider *autocreate = (AutoCreatePropertyValueProvider *) providers[PropertyPrecedence_AutoCreate];
	
	if (autocreate)
		g_hash_table_foreach (autocreate->auto_values, unregister_depobj_values, this);
	
	g_hash_table_foreach (local_values, unregister_depobj_values, this);
}

static bool listeners_notified;

void
DependencyObject::NotifyListenersOfPropertyChange (PropertyChangedEventArgs *args, MoonError *error)
{
	g_return_if_fail (args);

	listeners_notified = true;

	for (GSList *l = listener_list; l != NULL; l = l->next){
		Listener *listener = (Listener*)l->data;

		if (listener->Matches (args))
			listener->Invoke (this, args, error);
		if (error && error->number)
			break;
	}
}

void
DependencyObject::NotifyListenersOfPropertyChange (int id, MoonError *error)
{
	if (IsDisposed ())
		return;
	NotifyListenersOfPropertyChange (GetDeployment ()->GetTypes ()->GetProperty (id), error);
}

void
DependencyObject::NotifyListenersOfPropertyChange (DependencyProperty *subproperty, MoonError *error)
{
	// XXX I really think this method should go away.  we only use it in
	// a couple of places, and it abuses things.

	Value *new_value = subproperty ? GetValue (subproperty) : NULL;

	PropertyChangedEventArgs *args = new PropertyChangedEventArgs (subproperty, subproperty->GetId (), NULL, new_value);

	NotifyListenersOfPropertyChange (args, error);

	args->unref ();
}

bool
DependencyObject::IsValueValid (DependencyProperty* property, Value* value, MoonError *error)
{
	if (property == NULL) {
		MoonError::FillIn (error, MoonError::ARGUMENT_NULL, 1001,
				   "NULL property passed to IsValueValid");
		return false;
	}

	if (value != NULL) {
		if (value->Is (GetDeployment (), Type::EVENTOBJECT) && !value->AsEventObject ()) {
			// if it's a null DependencyObject, it doesn't matter what type it is
			return true;
		}
		
		if (value->Is (GetDeployment (), Type::MANAGED)) {
			// This is a big hack, we do no type-checking if we try to set a managed type.
			// Given that for the moment we might not have the surface available, we can't
			// do any type checks since we can't access types registered on the surface.
			return true;
		}
		
		if (!Type::IsAssignableFrom (GetDeployment (), property->GetPropertyType(), value->GetKind())) {
			char *error_msg = g_strdup_printf ("DependencyObject::SetValue, value cannot be assigned to the "
							    "property %s::%s (property has type '%s', value has type '%s')",
							    GetTypeName (), property->GetName(), Type::Find (GetDeployment (), property->GetPropertyType())->GetName (),
							   Type::Find (GetDeployment (), value->GetKind ())->GetName ());
			MoonError::FillIn (error, MoonError::ARGUMENT, 1001, error_msg);
			g_free (error_msg);
			return false;
		}
	} else {
		// In 2.0, property->GetPropertyType() can return
		// something greater than Type::LASTTYPE.  Only check
		// built-in types for null Types registered on the
		// managed side has their own check there.
		if (!CanPropertyBeSetToNull (property)) {
			char *error_msg = g_strdup_printf ("Can not set a non-nullable scalar type to NULL (property: %s)",
							   property->GetName());
			MoonError::FillIn (error, MoonError::ARGUMENT, 1001, error_msg);
			g_free (error_msg);
			return false;
		}
	}

	return true;
}

bool
DependencyObject::CanPropertyBeSetToNull (DependencyProperty* property)
{
	Deployment *deployment;
	
	if (property->GetPropertyType () > Type::LASTTYPE)
		return true;

	if (property->IsNullable ())
		return true;

	deployment = Deployment::GetCurrent ();
	if (Type::IsSubclassOf (deployment, property->GetPropertyType(), Type::DEPENDENCY_OBJECT))
		return true;

	if (Type::IsSubclassOf (deployment, property->GetPropertyType (), Type::STRING))
		return true;

	return false;
}

bool
DependencyObject::SetValue (int id, Value *value)
{
	if (IsDisposed ())
		return false;
	return SetValue (GetDeployment ()->GetTypes ()->GetProperty (id), value);
}

bool
DependencyObject::SetValue (int id, Value value)
{
	if (IsDisposed ())
		return false;
	return SetValue (GetDeployment ()->GetTypes ()->GetProperty (id), value);
}

bool
DependencyObject::SetValue (DependencyProperty *property, Value *value)
{
	MoonError err;
	return SetValueWithError (property, value, &err);
}

bool
DependencyObject::SetValue (DependencyProperty *property, Value value)
{
	MoonError err;
	return SetValueWithError (property, &value, &err);
}

bool
DependencyObject::SetValueWithError (DependencyProperty* property, Value value, MoonError *error)
{
	return SetValueWithError (property, &value, error);
}

bool
DependencyObject::SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error)
{
	if (is_frozen) {
		char *error_msg = g_strdup_printf ("Cannot set value for property '%s' on frozen DependencyObject '%s'", property->GetName(), GetTypeName());
		MoonError::FillIn (error, MoonError::UNAUTHORIZED_ACCESS, error_msg);
		g_free (error_msg);
		return false;
	}
	
	AutoCreatePropertyValueProvider *autocreate = (AutoCreatePropertyValueProvider *) providers[PropertyPrecedence_AutoCreate];
	Value *current_value;
	bool equal = false;
	
	if (!(current_value = ReadLocalValue (property)))
		if (property->IsAutoCreated ())
			current_value = autocreate->ReadLocalValue (property);
	
	if (current_value != NULL && value != NULL) {
		equal = !property->AlwaysChange() && (*current_value == *value);
	} else {
		equal = (current_value == NULL) && (value == NULL);
	}

	if (!equal) {
		Value *new_value;
		
		// remove the old value
		g_hash_table_remove (local_values, property);
		
		if (property->IsAutoCreated ())
			autocreate->ClearValue (property);
		
		if (value && (!property->IsAutoCreated () || !value->Is (GetDeployment (), Type::DEPENDENCY_OBJECT) || value->AsDependencyObject () != NULL))
			new_value = new Value (*value);
		else
			new_value = NULL;
		
		// replace it with the new value
		if (new_value)
			g_hash_table_insert (local_values, property, new_value);
		
		ProviderValueChanged (PropertyPrecedence_LocalValue, property, current_value, new_value, true, true, error);
		
		if (current_value)
			delete current_value;
	}

	return true;
}

bool
DependencyObject::SetValueWithError (DependencyProperty *property, Value *value, MoonError *error)
{
	if (!IsValueValid (property, value, error))
		return false;
	if (!property->Validate (this, value, error))
		return false;
	
	return SetValueWithErrorImpl (property, value, error);
}

struct RegisterNamesClosure {
	NameScope *to_ns;
	MoonError *error;
};

static void
register_depobj_names (gpointer  key,
		       gpointer  value,
		       gpointer  user_data)
{
	RegisterNamesClosure *closure = (RegisterNamesClosure*)user_data;
	if (closure->error->number)
		return;

	Value *v = (Value*)value;

	if (v != NULL && v->Is (closure->to_ns->GetDeployment (), Type::DEPENDENCY_OBJECT) && v->AsDependencyObject() != NULL) {
		DependencyObject *obj = v->AsDependencyObject ();
		obj->RegisterAllNamesRootedAt (closure->to_ns, closure->error);
	}
}

void
DependencyObject::RegisterAllNamesRootedAt (NameScope *to_ns, MoonError *error)
{
	AutoCreatePropertyValueProvider *autocreate = (AutoCreatePropertyValueProvider *) providers[PropertyPrecedence_AutoCreate];
	
	if (error->number)
		return;

	bool merge_namescope = false;
	bool register_name = false;
	bool recurse = false;

	NameScope *this_ns = NameScope::GetNameScope(this);

	if (this_ns && this_ns->GetTemporary()) {
		merge_namescope = true;
	}
	else if (!this_ns) {
		recurse = true;
		register_name = true;
	}
	else if (IsHydratedFromXaml ()) {
		register_name = true;
	}


	if (merge_namescope) {
		to_ns->MergeTemporaryScope (this_ns, error);
		ClearValue (NameScope::NameScopeProperty, false);
	}

	if (register_name) {
		const char *n = GetName();
		
		if (n && *n) {
			DependencyObject *o = to_ns->FindName (n);
			if (o) {
				if (o != this) {
					char *error_msg = g_strdup_printf ("The name already exists in the tree: %s.", n);
					MoonError::FillIn (error, MoonError::ARGUMENT, 2028, error_msg);
					g_free (error_msg);
					return;
				}
			}
			else {
				to_ns->RegisterName (n, this);
			}
		}
	}

	if (recurse) {
		RegisterNamesClosure closure;
		closure.to_ns = to_ns;
		closure.error = error;
	
		if (autocreate)
			g_hash_table_foreach (autocreate->auto_values, register_depobj_names, &closure);
	
		g_hash_table_foreach (local_values, register_depobj_names, &closure);
	}
}

static void
unregister_depobj_names (gpointer  key,
			 gpointer  value,
			 gpointer  user_data)
{
	NameScope *from_ns = (NameScope*)user_data;
	Value *v = (Value*)value;
	DependencyProperty *property = (DependencyProperty*)key;

	if (property->GetId() != UIElement::TagProperty && v != NULL && v->Is (from_ns->GetDeployment (), Type::DEPENDENCY_OBJECT) && v->AsDependencyObject() != NULL) {
		DependencyObject *obj = v->AsDependencyObject ();
		obj->UnregisterAllNamesRootedAt (from_ns);
	}
}

void
DependencyObject::UnregisterAllNamesRootedAt (NameScope *from_ns)
{
	AutoCreatePropertyValueProvider *autocreate = (AutoCreatePropertyValueProvider *) providers[PropertyPrecedence_AutoCreate];

	NameScope *this_ns = NameScope::GetNameScope(this);
	if (IsHydratedFromXaml () || this_ns == NULL || this_ns->GetTemporary ()) {
		// Unregister in the parent scope
	
		const char *n = GetName();

		if (n && strlen (n) > 0)
			from_ns->UnregisterName (n);
	}

	if (this_ns && !this_ns->GetTemporary())
		return;
	
	if (autocreate)
		g_hash_table_foreach (autocreate->auto_values, unregister_depobj_names, from_ns);
	
	g_hash_table_foreach (local_values, unregister_depobj_names, from_ns);
}

bool
DependencyObject::SetName (const char* name, NameScope *scope)
{
	DependencyProperty *property = GetDeployment ()->GetTypes ()->GetProperty (NameProperty);

	if (scope->FindName (name))
		return false;

	Value *new_value = new Value (name);
	SetValue (property, new_value);
	scope->RegisterName (name, this);

	return true;
}

Value *
DependencyObject::ReadLocalValue (int id)
{
	if (IsDisposed ())
		return NULL;
	return ReadLocalValue (GetDeployment ()->GetTypes ()->GetProperty (id));
}

Value *
DependencyObject::ReadLocalValue (DependencyProperty *property)
{
	return (Value *) g_hash_table_lookup (local_values, property);
}

Value *
DependencyObject::ReadLocalValueWithError (DependencyProperty *property, MoonError *error)
{
	if (!HasProperty (Type::INVALID, property, true)) {
		Type *pt = Type::Find (GetDeployment (), property->GetOwnerType ());
		char *error_msg = g_strdup_printf ("Cannot get the DependencyProperty %s.%s on an object of type %s", pt ? pt->GetName () : "<unknown>", property->GetName (), GetTypeName ());
		MoonError::FillIn (error, MoonError::EXCEPTION, error_msg);
		g_free (error_msg);
		return NULL;
	}
	return ReadLocalValue (property);
}

Value *
DependencyObject::GetValueWithError (Type::Kind whatami, DependencyProperty *property, MoonError *error)
{
	if (!HasProperty (whatami, property, true)) {
		Type *pt = Type::Find (GetDeployment (), property->GetOwnerType ());
		char *error_msg = g_strdup_printf ("Cannot get the DependencyProperty %s.%s on an object of type %s", pt ? pt->GetName () : "<unknown>", property->GetName (), GetTypeName ());
		MoonError::FillIn (error, MoonError::EXCEPTION, error_msg);
		g_free (error_msg);
		return NULL;
	}
	return GetValue (property);
}

Value *
DependencyObject::GetValue (int id)
{
	if (IsDisposed ())
		return NULL;
	return GetValue (GetDeployment ()->GetTypes ()->GetProperty (id));
}

Value *
DependencyObject::GetValue (DependencyProperty *property)
{
	return GetValue (property, PropertyPrecedence_Highest, PropertyPrecedence_Lowest);
}

Value *
DependencyObject::GetValue (DependencyProperty *property, PropertyPrecedence startingAtPrecedence)
{
	return GetValue (property, startingAtPrecedence, PropertyPrecedence_Lowest);
}

Value *
DependencyObject::GetValue (DependencyProperty *property, PropertyPrecedence startingAtPrecedence, PropertyPrecedence endingAtPrecedence)
{
	for (int i = startingAtPrecedence; i <= endingAtPrecedence; i ++) {
		if (!providers[i])
			continue;
		Value *value = providers[i]->GetPropertyValue (property);
		if (value) return value;
	}
	return NULL;
}

Value *
DependencyObject::GetValueNoDefault (int id)
{
	if (IsDisposed ())
		return NULL;
	return GetValueNoDefault (GetDeployment ()->GetTypes ()->GetProperty (id));
}

Value *
DependencyObject::GetValueNoDefault (DependencyProperty *property)
{
	Value *value = NULL;

	for (int i = 0; i < PropertyPrecedence_DefaultValue; i ++) {
		if (!providers[i])
			continue;
		value = providers[i]->GetPropertyValue (property);
		if (value) break;
	}
	return value && !value->GetIsNull () ? value : NULL;
}

Value *
DependencyObject::GetValueNoDefaultWithError (DependencyProperty *property, MoonError *error)
{
	if (!HasProperty (Type::INVALID, property, true)) {
		Type *pt = Type::Find (GetDeployment (), property->GetOwnerType ());
		char *error_msg = g_strdup_printf ("Cannot get the DependencyProperty %s.%s on an object of type %s", pt ? pt->GetName () : "<unknown>", property->GetName (), GetTypeName ());
		MoonError::FillIn (error, MoonError::EXCEPTION, error_msg);
		g_free (error_msg);
		return NULL;
	}
	return GetValueNoDefault (property);	
}

void
DependencyObject::ProviderValueChanged (PropertyPrecedence providerPrecedence,
					DependencyProperty *property,
					Value *old_provider_value, Value *new_provider_value,
					bool notify_listeners, bool set_parent, MoonError *error)
{
	int p;

	// first we look for a value higher in precedence for this property
	for (p = providerPrecedence - 1; p >= PropertyPrecedence_Highest; p --) {
		if (providers[p] && providers[p]->GetPropertyValue (property)) {
			// a provider higher in precedence already has
			// a value for this property, so the one
			// that's changing isn't visible anyway.
			return;
		}
	}

	Value *old_value = NULL;
	Value *new_value = NULL;

	if (!old_provider_value || !new_provider_value) {
		Value *lower_priority_value = GetValue (property, (PropertyPrecedence)(providerPrecedence + 1));

		if (new_provider_value == NULL) {
			// we're changing from @old_provider_value to whatever the
			// value lower on the priority list is.
			old_value = old_provider_value;
			new_value = lower_priority_value;
		}
		else if (old_provider_value == NULL) {
			// we're changing from the old value (from a lower
			// priority provider) to @new_provider_value.
			old_value = lower_priority_value;
			new_value = new_provider_value;
		}
	}
	else {
		old_value = old_provider_value;
		new_value = new_provider_value;
	}

	bool equal = false;
	
	if (old_value != NULL && new_value != NULL) {
		equal = !property->AlwaysChange() && (*old_value == *new_value);
	} else {
		equal = false;
	}

 	if (!equal) {
		DependencyObject *old_as_dep = NULL;
		DependencyObject *new_as_dep = NULL;

		// XXX this flag should be part of the DP metadata.
		// we also need to audit other "typeof (object)" DP's
		// to make sure they set parent when they should (and
		// don't when they shouldn't.)
		bool setsParent = set_parent && !property->IsCustom ();

		if (old_value && old_value->Is (GetDeployment (), Type::DEPENDENCY_OBJECT))
			old_as_dep = old_value->AsDependencyObject ();
		if (new_value && new_value->Is (GetDeployment (), Type::DEPENDENCY_OBJECT))
			new_as_dep = new_value->AsDependencyObject ();

		if (old_as_dep && setsParent) {
			old_as_dep->SetSurface (NULL);
			
			// unset its parent
			old_as_dep->SetParent (NULL, NULL);
			
			// remove ourselves as a target
			old_as_dep->RemoveTarget (this);
			
			// unregister from the existing value
			old_as_dep->RemovePropertyChangeListener (this, property);
			
			if (old_as_dep->Is(Type::COLLECTION)) {
				old_as_dep->RemoveHandler (Collection::ChangedEvent, collection_changed, this);
				old_as_dep->RemoveHandler (Collection::ItemChangedEvent, collection_item_changed, this);
			}
		}

		if (new_as_dep && setsParent) {
			new_as_dep->SetSurface (GetSurface ());

			new_as_dep->SetParent (this, error);
			if (error->number)
				return;

			new_as_dep->SetResourceBase (GetResourceBase());
			
			if (new_as_dep->Is(Type::COLLECTION)) {
				new_as_dep->AddHandler (Collection::ChangedEvent, collection_changed, this);
				new_as_dep->AddHandler (Collection::ItemChangedEvent, collection_item_changed, this);
			}
			
			// listen for property changes on the new object
			new_as_dep->AddPropertyChangeListener (this, property);
			
			// add ourselves as a target
			new_as_dep->AddTarget (this);
		}
		
		// we need to make this optional, as doing it for NameScope
		// merging is killing performance (and noone should ever care
		// about that property changing)
		if (notify_listeners) {
			Value *old_value_copy = old_value == NULL ? NULL : new Value (*old_value);
			Value *new_value_copy = new_value == NULL ? NULL : new Value (*new_value);
			
			PropertyChangedEventArgs *args = new PropertyChangedEventArgs (property, property->GetId (), old_value_copy, new_value_copy);

			listeners_notified = false;
		
			OnPropertyChanged (args, error);

			if (!listeners_notified) {
				g_warning ("setting property %s::%s on object of type %s didn't result in listeners being notified",
					   Type::Find (GetDeployment (), property->GetOwnerType())->GetName (), property->GetName(), GetTypeName ());
				if (error->number)
					g_warning ("the error was: %s", error->message);
			}

			if (property && property->GetChangedCallback () != NULL) {
				PropertyChangeHandler callback = property->GetChangedCallback ();
				callback (this, args, error, NULL);
			}


			if (InheritedPropertyValueProvider::IsPropertyInherited (property->GetId ()))
				InheritedPropertyValueProvider::PropagateInheritedProperty (this, property, old_value_copy, new_value_copy);

			args->unref ();
			
			delete old_value_copy;
			delete new_value_copy;
		}
 	}
}

void
DependencyObject::ClearValue (int id, bool notify_listeners)
{
	if (IsDisposed ())
		return;
	ClearValue (GetDeployment ()->GetTypes ()->GetProperty (id), notify_listeners);
}

void
DependencyObject::ClearValue (DependencyProperty *property, bool notify_listeners)
{
	ClearValue(property, notify_listeners, NULL);
}

void
DependencyObject::ClearValue (DependencyProperty *property, bool notify_listeners, MoonError *error)
{
	AutoCreatePropertyValueProvider *autocreate = (AutoCreatePropertyValueProvider *) providers[PropertyPrecedence_AutoCreate];
	Value *old_local_value;
	
	if (!(old_local_value = ReadLocalValue (property)))
		if (property->IsAutoCreated ())
			old_local_value = autocreate->ReadLocalValue (property);
	
	// detach from the existing value
	if (old_local_value != NULL && old_local_value->Is (GetDeployment (), Type::DEPENDENCY_OBJECT)) {
		DependencyObject *dob = old_local_value->AsDependencyObject();

		// Custom properties are non-parenting and so shouldn't clear the parent (same code
		// as found in the SetValue path)
		if (dob != NULL && !property->IsCustom ()) {
			// unset its parent
			dob->SetParent (NULL, NULL);

			// unregister from the existing value
			dob->RemovePropertyChangeListener (this, property);
			dob->SetSurface (NULL);
			if (dob->Is(Type::COLLECTION)) {
				dob->RemoveHandler (Collection::ChangedEvent, collection_changed, this);
				dob->RemoveHandler (Collection::ItemChangedEvent, collection_item_changed, this);
			}
		}
	}
	
	g_hash_table_remove (local_values, property);
	
	if (property->IsAutoCreated ())
		autocreate->ClearValue (property);
	
	// this is... yeah, it's disgusting
	for (int p = PropertyPrecedence_LocalValue + 1; p < PropertyPrecedence_Count; p ++) {
		if (providers[p])
			providers[p]->RecomputePropertyValue (property);
	}

	ProviderValueChanged (PropertyPrecedence_LocalValue, property, old_local_value, NULL, notify_listeners, true, error);
	
	delete old_local_value;
}

gboolean
DependencyObject::dispose_value (gpointer key, gpointer value, gpointer data)
{
	DependencyObject *_this = (DependencyObject*)data;

	Value *v = (Value *) value;
	
	if (!value)
		return TRUE;

	// detach from the existing value
	if (v->Is (_this->GetDeployment (), Type::DEPENDENCY_OBJECT)){
		DependencyObject *dob = v->AsDependencyObject();
		
		if (dob != NULL) {
			if (_this == dob->GetParent()) {
				// unset its logical parent
				dob->SetParent (NULL, NULL);
			}

			// unregister from the existing value
			dob->RemovePropertyChangeListener ((DependencyObject*)data, NULL);

			if (dob->Is(Type::COLLECTION)) {
				dob->RemoveHandler (Collection::ChangedEvent, collection_changed, _this);
				dob->RemoveHandler (Collection::ItemChangedEvent, collection_item_changed, _this);
			}
		}
	}
	
	delete (Value *) value;
	
	return TRUE;
}

void
DependencyObject::collection_changed (EventObject *sender, EventArgs *args, gpointer closure)
{
	DependencyObject *obj = (DependencyObject*)closure;
	obj->OnCollectionChanged ((Collection*)sender, (CollectionChangedEventArgs*)args);
}

void
DependencyObject::collection_item_changed (EventObject *sender, EventArgs *args, gpointer closure)
{
	DependencyObject *obj = (DependencyObject*)closure;
	CollectionItemChangedEventArgs* itemArgs = (CollectionItemChangedEventArgs*)args;

	PropertyChangedEventArgs *propChangedArgs = new PropertyChangedEventArgs (itemArgs->GetProperty(),
										  itemArgs->GetProperty()->GetId (),
										  itemArgs->GetOldValue(),
										  itemArgs->GetNewValue());

	obj->OnCollectionItemChanged ((Collection*)sender,
				      itemArgs->GetCollectionItem(),
				      propChangedArgs);

	propChangedArgs->unref ();
}

DependencyObject::DependencyObject ()
	: EventObject (Type::DEPENDENCY_OBJECT)
{
	Initialize ();
}

DependencyObject::DependencyObject (Deployment *deployment, Type::Kind object_type)
	: EventObject (deployment, object_type)
{
	Initialize ();
}

DependencyObject::DependencyObject (Type::Kind object_type)
	: EventObject (object_type)
{
	Initialize ();
}

void
DependencyObject::Initialize ()
{
	providers = new PropertyValueProvider*[PropertyPrecedence_Count];

	providers[PropertyPrecedence_LocalValue] = new LocalPropertyValueProvider (this, PropertyPrecedence_LocalValue);
	providers[PropertyPrecedence_DynamicValue] = NULL;  // subclasses will set this if they need it.

	providers[PropertyPrecedence_LocalStyle] = NULL;  // this is a frameworkelement specific thing
	providers[PropertyPrecedence_DefaultStyle] = NULL;  // this is a frameworkelement specific thing

	providers[PropertyPrecedence_Inherited] = new InheritedPropertyValueProvider (this, PropertyPrecedence_Inherited);
	providers[PropertyPrecedence_DefaultValue] = new DefaultValuePropertyValueProvider (this, PropertyPrecedence_DefaultValue);
	providers[PropertyPrecedence_AutoCreate] = new AutoCreatePropertyValueProvider (this, PropertyPrecedence_AutoCreate);
	
	local_values = g_hash_table_new (g_direct_hash, g_direct_equal);
	listener_list = NULL;
	parent = NULL;
	is_hydrated = false;
	is_frozen = false;
	resource_base = NULL;
	storage_hash = NULL; // Create it on first usage request
	template_owner = NULL;
}

void
DependencyObject::SetTemplateOwner (DependencyObject *value)
{
	// you should only set template owner once.
	g_return_if_fail (template_owner == NULL);
	template_owner = value;
	if (template_owner)
		template_owner->AddHandler (EventObject::DestroyedEvent, DependencyObject::TemplateOwnerDestroyedEvent, this);
}

DependencyObject *
DependencyObject::GetTemplateOwner ()
{
	return template_owner;
}

void
DependencyObject::TemplateOwnerDestroyedEvent (EventObject *sender, EventArgs *args, gpointer closure)
{
	DependencyObject *o = (DependencyObject *) closure;
	o->DetachTemplateOwnerDestroyed ();
}

void
DependencyObject::DetachTemplateOwnerDestroyed ()
{
	if (template_owner) {
		template_owner->RemoveHandler (EventObject::DestroyedEvent, DependencyObject::TemplateOwnerDestroyedEvent, this);
		template_owner = NULL;
	}
}

void
DependencyObject::Freeze()
{
	is_frozen = true;
}

struct CloneClosure {
	Types *types;
	DependencyObject *old_do;
	DependencyObject *new_do;
};

void
DependencyObject::clone_local_value (DependencyProperty *key, Value *value, gpointer data)
{
	CloneClosure *closure = (CloneClosure*)data;

	// don't clone the name property, or we end up with nasty
	// duplicate name errors.
 	if (key->GetId() == DependencyObject::NameProperty)
 		return;

	Value *cv = Value::Clone (value, closure->types);

	closure->new_do->SetValue (key, cv);

	delete cv;
}

void
DependencyObject::clone_autocreated_value (DependencyProperty *key, Value *value, gpointer data)
{
	Deployment *deployment = Deployment::GetCurrent ();
	CloneClosure *closure = (CloneClosure*)data;

	Value *old_value = closure->old_do->GetValue (key, PropertyPrecedence_AutoCreate);

	// this should create the new object
	Value *new_value = closure->new_do->GetValue (key, PropertyPrecedence_AutoCreate);

	if (old_value && !old_value->GetIsNull() && old_value->Is (deployment, Type::DEPENDENCY_OBJECT) && 
	    new_value && !new_value->GetIsNull() && new_value->Is (deployment, Type::DEPENDENCY_OBJECT)) {
		DependencyObject *new_obj = new_value->AsDependencyObject(closure->types);
		DependencyObject *old_obj = old_value->AsDependencyObject(closure->types);
		
		new_obj->CloneCore (closure->types, old_obj);
	}
}

void
DependencyObject::clone_animation_storage_list (DependencyProperty *key, List *list, gpointer data)
{
	if (!list || list->IsEmpty ())
		return;
	DependencyObject *d = (DependencyObject*)data;
	d->CloneAnimationStorageList (key, list);
}

void
DependencyObject::CloneAnimationStorageList (DependencyProperty *key, List *list)
{

	List *newlist = new List();

	AnimationStorage::Node *node = (AnimationStorage::Node *) list->First ();
	while (node) {
		node->storage->SwitchTarget (this);
		newlist->Append (node->Clone ());
		node = (AnimationStorage::Node*)node->next;
	}
	list->Clear (true);
	if (!storage_hash)
		storage_hash = g_hash_table_new (g_direct_hash, g_direct_equal);
	g_hash_table_insert (storage_hash, key, newlist);
}

DependencyObject*
DependencyObject::Clone (Types *types)
{
	Type *t = types->Find (GetObjectType());

	DependencyObject *new_do = t->CreateInstance();

	if (new_do)
		new_do->CloneCore (types, (DependencyObject*)this); // this cast should be unnecessary.  but C++ const behavior sucks.

	return new_do;
}

void
DependencyObject::CloneCore (Types *types, DependencyObject* fromObj)
{
	CloneClosure closure;
	closure.types = types;
	closure.old_do = fromObj;
	closure.new_do = this;

	AutoCreatePropertyValueProvider *autocreate = (AutoCreatePropertyValueProvider *) fromObj->providers[PropertyPrecedence_AutoCreate];

	g_hash_table_foreach (autocreate->auto_values, (GHFunc)DependencyObject::clone_autocreated_value, &closure);
	g_hash_table_foreach (fromObj->local_values, (GHFunc)DependencyObject::clone_local_value, &closure);
	if (fromObj->storage_hash) {
		g_hash_table_foreach (fromObj->storage_hash, (GHFunc)DependencyObject::clone_animation_storage_list, this);
	}
}

static void
clear_storage_list (DependencyProperty *key, List *list, gpointer unused)
{
	List::Node *node = list->First ();
	while (node) {
		delete ((AnimationStorage::Node*)node)->storage;
		node = node->next;
	}
	delete list;
}

DependencyObject::~DependencyObject ()
{
	DetachTemplateOwnerDestroyed ();
	g_hash_table_destroy (local_values);
	local_values = NULL;
	delete[] providers;
	providers = NULL;
	g_free (resource_base);
}

static void
free_listener (gpointer data, gpointer user_data)
{
	Listener* listener = (Listener*) data;
	delete listener;
}

void
DependencyObject::Dispose ()
{
	AutoCreatePropertyValueProvider *autocreate = (AutoCreatePropertyValueProvider *) providers[PropertyPrecedence_AutoCreate];
	
	if (listener_list != NULL) {
		g_slist_foreach (listener_list, free_listener, NULL);
		g_slist_free (listener_list);
		listener_list = NULL;
	}

	RemoveAllListeners();
	
	if (autocreate)
		g_hash_table_foreach_remove (autocreate->auto_values, dispose_value, this);
	
	g_hash_table_foreach_remove (local_values, dispose_value, this);
	
	for (int i = 0; i < PropertyPrecedence_Count; i ++) {
		delete providers[i];
		providers [i] = NULL;
	}
	
	if (storage_hash) {
		GHashTable *tmphash = storage_hash; // animation storages may call back to DetachAnimationStorage
		storage_hash = NULL;
		g_hash_table_foreach (tmphash, (GHFunc)clear_storage_list, NULL);
		g_hash_table_destroy (tmphash);
	}

	parent = NULL;
	
	EventObject::Dispose ();
}

static void
get_attached_props (gpointer key, gpointer value, gpointer user_data)
{
	DependencyProperty *prop = (DependencyProperty *) key;
	GHashTable *props = (GHashTable *) user_data;
	
	if (!(g_hash_table_lookup (props, (gpointer) prop->GetHashKey ())))
		g_hash_table_insert (props, (gpointer) prop->GetHashKey (), prop);
}

static void
hash_keys_to_array (gpointer key, gpointer value, gpointer user_data)
{
	g_ptr_array_add ((GPtrArray *) user_data, key);
}

static void
hash_values_to_array (gpointer key, gpointer value, gpointer user_data)
{
	g_ptr_array_add ((GPtrArray *) user_data, value);
}

DependencyProperty **
DependencyObject::GetProperties (bool only_changed)
{
	AutoCreatePropertyValueProvider *autocreate = (AutoCreatePropertyValueProvider *) providers[PropertyPrecedence_AutoCreate];
	DependencyProperty **props;
	GHashTable *table;
	GPtrArray *array;
	
	array = g_ptr_array_new ();
	
	if (!only_changed) {
		// get our class/inherited DependencyProperties
		table = GetType ()->CopyProperties (true);
		
		// find any attached properties that have been set
		g_hash_table_foreach (local_values, get_attached_props, table);
		
		// dump them to an array
		g_hash_table_foreach (table, hash_values_to_array, array);
		g_hash_table_destroy (table);
	} else {
		g_hash_table_foreach (local_values, hash_keys_to_array, array);
		g_hash_table_foreach (autocreate->auto_values, hash_keys_to_array, array);
	}
	
	g_ptr_array_add (array, NULL);
	props = (DependencyProperty **) array->pdata;
	g_ptr_array_free (array, false);
	
	return props;
}

DependencyProperty *
DependencyObject::GetDependencyProperty (const char *name)
{
	return DependencyProperty::GetDependencyProperty (GetType (), name);
}

bool
DependencyObject::HasProperty (const char *name, bool inherits)
{
	return DependencyProperty::GetDependencyProperty (GetType (), name, inherits) != NULL;
}

bool
DependencyObject::HasProperty (Type::Kind whatami, DependencyProperty *property, bool inherits)
{
	Type::Kind this_type = whatami == Type::INVALID ? GetObjectType () : whatami;
	
	// TODO: Handle attached properties correctly.
	
	if (property->IsAttached ())
		return true;
	
	/*
	printf ("DependencyObject::HasProperty (%p, %i (%s), %p (%i %s.%s), %i)..\n", 
		
		whatami, Type::Find (whatami)->name,
		property, property->GetOwnerType (), Type::Find (property->GetOwnerType ())->name, property->GetName (), 
		inherits);
	*/
	
	if (property == NULL)
		return false;
		
	if (property->GetOwnerType () == this_type)
		return true;
		
	if (!inherits)
		return false;

	if (!Type::IsSubclassOf (GetDeployment (), this_type, property->GetOwnerType ())) {
		bool is_prop_custom = property->IsCustom ();
		bool is_owner_custom = property->GetOwnerType () > Type::LASTTYPE;
		bool is_this_custom = this_type > Type::LASTTYPE;
		bool accept = false;

		// Yuck. 
		// This looks very wrong, but it's what SL seems to do.
		if (is_prop_custom) {
			if (!is_owner_custom && !is_this_custom) {
				// SL does not throw errors for custom properties defined on a builtin type 
				// and then used on another (unrelated) builtin type (DO.GetValue usage at least)
				accept = true;
			} else if (is_owner_custom) {
				// And this is a custom property defined on a custom type and used anywhere.
				accept = true;
			}
		}
		return accept;
	}
	
	return true;
}

DependencyObject *
DependencyObject::FindName (const char *name)
{
	return FindName (name, Control::GetIsTemplateItem (this));
}

DependencyObject *
DependencyObject::FindName (const char *name, bool template_item)
{
	NameScope *scope = NameScope::GetNameScope (this);
	
	if (scope && (template_item == scope->GetIsLocked ()))
		return scope->FindName (name);
	
	if (parent)
		return parent->FindName (name, template_item);
	
	return NULL;
}

NameScope *
DependencyObject::FindNameScope ()
{
	return FindNameScope (Control::GetIsTemplateItem (this));
}

NameScope*
DependencyObject::FindNameScope (bool template_namescope)
{
	NameScope *scope = NameScope::GetNameScope (this);

	// Only template namescopes are locked (for the moment)
	if (scope && (template_namescope == scope->GetIsLocked ()))
		return scope;

	if (parent)
		return parent->FindNameScope (template_namescope);

	return NULL;
}
		 
DependencyObject *
DependencyObject::FindName (const char *name, Type::Kind *element_kind)
{
	//printf ("Looking up in %p the string %p\n", obj, name);
	//printf ("        String: %s\n", name);
	DependencyObject *ret = FindName (name);

	if (ret == NULL)
		return NULL;

	*element_kind = ret->GetObjectType ();

	return ret;
}

AnimationStorage*
DependencyObject::GetAnimationStorageFor (DependencyProperty *prop)
{
	if (!storage_hash)
		return NULL;

	List *list = (List*) g_hash_table_lookup (storage_hash, prop);
	if (!list || !list->IsEmpty ())
		return NULL;

	return ((AnimationStorage::Node *) list->Last())->storage;
}

AnimationStorage*
DependencyObject::AttachAnimationStorage (DependencyProperty *prop, AnimationStorage *storage)
{
	AnimationStorage* attached_storage = NULL;
	// Create hash on first access to save some mem
	if (!storage_hash)
		storage_hash = g_hash_table_new (g_direct_hash, g_direct_equal);

	List *list = (List*) g_hash_table_lookup (storage_hash, prop);
	if (!list) {
		list = new List();
		g_hash_table_insert (storage_hash, prop, list);
	}
	else if (!list->IsEmpty ()) {
		attached_storage = ((AnimationStorage::Node*) list->Last())->storage;
		attached_storage->Disable ();
	}

	list->Append (new AnimationStorage::Node (prop, storage));
	return attached_storage;
}

void
DependencyObject::DetachAnimationStorage (DependencyProperty *prop, AnimationStorage *storage)
{
	if (!storage_hash)
		return;

	List *list = (List *) g_hash_table_lookup (storage_hash, prop);
	if (!list || list->IsEmpty ())
		return;

	// if the storage to be removed is the last one, activate the previous one (if any)
	if (((AnimationStorage::Node*)list->Last ())->storage == storage) {
		list->Remove (list->Last ());
		if (!list->IsEmpty ()) {
			((AnimationStorage::Node*)list->Last ())->storage->Enable ();
		}

	} else { // if there's more than one storage, that means the storage that was added after
		// the one we're removing is configured to reset back to the value of the previous
		// storage, so update the stop value so it resets back to the proper value
		List::Node *node = list->First ();
		while (node) {
			if (((AnimationStorage::Node*)node)->storage == storage) {
				List::Node *remove = node;
				node = node->next;
				((AnimationStorage::Node*)node)->storage->SetStopValue (storage->GetResetValue ());
				list->Remove (remove);
				break;
			}
			node = node->next;
		}
	}
}

//
//  A helper debugging routine for C#
//
const char *
dependency_object_get_name (DependencyObject *obj)
{
	return obj->GetName ();
}

Type::Kind
dependency_object_get_object_type (DependencyObject *obj)
{
	return obj->GetObjectType ();
}

const char *
dependency_object_get_type_name (DependencyObject *obj)
{
	return obj->GetTypeName ();
}

// Used by routines which need to create DO from code
void
dependency_object_set_name (DependencyObject *obj, const char *name)
{
	obj->SetValue (DependencyObject::NameProperty, Value (name));
}

static void
set_surface (gpointer key, gpointer value, gpointer data)
{
	Surface *s = (Surface *) data;
	Deployment *deployment = s ? s->GetDeployment () : Deployment::GetCurrent ();
	Value *v = (Value *) value;
	
	if (v && v->Is (deployment, Type::DEPENDENCY_OBJECT)) {
		DependencyObject *dob = v->AsDependencyObject();
		if (dob)
			dob->SetSurface (s);
	}
}

void
DependencyObject::SetSurface (Surface *s)
{
	AutoCreatePropertyValueProvider *autocreate = (AutoCreatePropertyValueProvider *) providers[PropertyPrecedence_AutoCreate];
	
	if (GetSurface() == s)
		return;
	
	EventObject::SetSurface (s);
	
	if (autocreate)
		g_hash_table_foreach (autocreate->auto_values, set_surface, s);
	
	g_hash_table_foreach (local_values, set_surface, s);
}

void
DependencyObject::SetParent (DependencyObject *parent, MoonError *error)
{
	if (parent == this->parent)
		return;

#if DEBUG
	// Check for circular families
	DependencyObject *current = parent;
	while (current != NULL) {
		if (current == this) {
			g_warning ("cycle found in logical tree.  bailing out");
			return;
		}
		current = current->GetParent ();
	}
#endif

	if (!this->parent) {
		if (parent) {
			NameScope *this_scope = NameScope::GetNameScope(this);
			NameScope *parent_scope = parent->FindNameScope();
			if (this_scope) {
				if (this_scope->GetTemporary()) {
					// if we have a temporary name scope, merge it into the
					// closest one up the hierarchy.
					if (parent_scope) {
						parent_scope->MergeTemporaryScope (this_scope, error);
						ClearValue (NameScope::NameScopeProperty, false);
					}
					else {
						// oddly enough, if
						// there's no parent
						// namescope, we don't
						// do anything
					}
				}
				else {
					// we have a non-temporary scope.  we still have to register the name
					// of this element (not the ones in the subtree rooted at this element)
					// in the new parent scope.  we only register the name in the parent scope
					// if the element was hydrated, not when it was created from a string.
					if (IsHydratedFromXaml()) {
						const char *name = GetName();
						if (parent_scope && name && *name) {
							DependencyObject *existing_obj = parent_scope->FindName (name);
							if (existing_obj != this) {
								if (existing_obj) {
									char *error_msg = g_strdup_printf ("name `%s' is already registered in new parent namescope.", name);
									MoonError::FillIn (error, MoonError::ARGUMENT, error_msg);
									return;
								}
								parent_scope->RegisterName (name, this);
							}
						}
					}
				}
			}
			else {
				// we don't have a namescope at all,
				// we have to iterate over the subtree
				// rooted at this object, and merge
				// the names into the parent
				// namescope.

				if (parent_scope) {
					NameScope *temp_scope = new NameScope();
					temp_scope->SetTemporary (true);

					RegisterAllNamesRootedAt (temp_scope, error);

					if (error->number) {
						temp_scope->unref ();
						return;
					}

					parent_scope->MergeTemporaryScope (temp_scope, error);

					temp_scope->unref ();
				}
			}
		}
	}
	else {
		if (!parent) {
			NameScope *parent_scope = this->parent->FindNameScope ();
			if (parent_scope)
				UnregisterAllNamesRootedAt (parent_scope);
		}
	}

	if (!error || error->number == 0)
		this->parent = parent;
}

Value *
dependency_object_get_value (DependencyObject *object, DependencyProperty *prop)
{
	if (object == NULL)
		return NULL;

	return object->GetValue (prop);
}

Value *
dependency_object_get_value_no_default (DependencyObject *object, DependencyProperty *prop)
{
	if (object == NULL)
		return NULL;
	
	return object->GetValueNoDefault (prop);
}

void
dependency_object_set_value (DependencyObject *object, DependencyProperty *prop, Value *val)
{
	if (object == NULL)
		return;

	object->SetValue (prop, val);
}

void
DependencyObject::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (DependencyObject::NameProperty == args->GetId ()) {
		NameScope *scope = FindNameScope ();
		if (scope && args->GetNewValue()) {
			if (args->GetOldValue ())
				scope->UnregisterName (args->GetOldValue ()->AsString ());
			scope->RegisterName (args->GetNewValue()->AsString (), this);

			if (IsHydratedFromXaml () && parent) {
				// we also need to update any parent
				// namescope about our name change

				scope = parent->FindNameScope ();
				if (scope) {
					if (args->GetOldValue ())
						scope->UnregisterName (args->GetOldValue ()->AsString ());
					scope->RegisterName (args->GetNewValue()->AsString (), this);
				}
			}
		}
	}

	NotifyListenersOfPropertyChange (args, error);
}

DependencyObject*
DependencyObject::GetContent()
{
	const char *content_property_name = GetType()->GetContentPropertyName();
	if (!content_property_name)
		return NULL;

	DependencyProperty *content_property = GetDependencyProperty (content_property_name);
	if (!content_property)
		return NULL;

	Value *content_value = GetValue(content_property);

	if (!content_value)
		return NULL;

	return content_value->AsDependencyObject();
}
