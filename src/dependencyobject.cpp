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

struct EventList {
	int current_token;
	int emitting;
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
			lists [i].emitting = 0;
			lists [i].event_list = new List ();
		}
	}

	~EventLists ()
	{
		for (int i = 0; i < size; i++) {
			delete lists [i].event_list;
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
	Track ("Created", "");
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
EventObject::AddTickCallSafe (TickCallHandler handler)
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

	AddTickCallInternal (handler);
 	
	pthread_rwlock_unlock (&surface_lock);
}

void
EventObject::AddTickCall (TickCallHandler handler)
{
	if (!Surface::InMainThread ()) {
		g_warning ("EventObject::AddTickCall (): This method must not be called on any other than the main thread! Tick call won't be added.\n");
#if DEBUG
		if (debug_flags & RUNTIME_DEBUG_DP)
			print_stack_trace ();
#endif
		return;
	}
	
	AddTickCallInternal (handler);
}

void
EventObject::AddTickCallInternal (TickCallHandler handler)
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

	timemanager->AddTickCall (handler, this);
}

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

void
EventObject::SetCurrentDeployment (bool domain)
{	
	if (deployment != NULL)
		Deployment::SetCurrent (deployment, domain);
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
	if (!IsDisposed ()) {
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
	if (GetObjectType () != object_type)
		printf ("EventObject::ref (): the type '%s' did not call SetObjectType, object_type is '%s'\n", Type::Find (GetObjectType ())->GetName (), Type::Find (object_type)->GetName ());
		
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
#if DEBUG
	if (GetObjectType () != object_type)
		printf ("EventObject::unref (): the type '%s' did not call SetObjectType, object_type is '%s'\n", Type::Find (GetObjectType ())->GetName (), Type::Find (object_type)->GetName ());
#endif

	if (!IsMultiThreadedSafe () && !Surface::InMainThread ()) {
		unref_delayed ();
		return;
	}

	if (refcount == 1 && events != NULL && events->emitting) {
		unref_delayed ();
		return;
	}

	int v = g_atomic_int_exchange_and_add (&refcount, -1) -1;

	OBJECT_TRACK ("Unref", (deployment == NULL && Deployment::GetCurrent () == NULL) ? "<unknown>" : GetTypeName ());

	if (v == 0) {
		Dispose ();
		
#if DEBUG
		if ((flags & Disposed) == 0)
			printf ("EventObject::unref (): the type '%s' (or any of its parent types) forgot to call its base class' Dispose method.\n", GetTypeName ());
#endif
	} else if (v == 1 && toggleNotifyListener) {
		if (getenv ("MOONLIGHT_ENABLE_TOGGLEREF"))
			toggleNotifyListener->Invoke (true);
	}
	
	// We need to check again the the refcount really is zero,
	// the object might have resurrected in the Dispose.
	if (refcount == 0)
		delete this;
	
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
		if (!track_all)
			printf ("%p\t%s tracked object of type '%s': %i, current refcount: %i deployment: %p\n", this, done, typname, id, refcount, deployment);

		if (!use_visi_output) {
			char *st = get_stack_trace ();
			printf("%s", st);
			g_free (st);
		} else
			print_reftrace (done, typname, refcount, false);
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

int
EventObject::AddXamlHandler (const char *event_name, EventHandler handler, gpointer data, GDestroyNotify data_dtor)
{
	int id = GetType ()->LookupEvent (event_name);
	
	if (id == -1) {
		g_warning ("adding xaml handler to event '%s', which has not been registered\n", event_name);
		return -1;
	}
	
	return AddHandler (id, handler, data, data_dtor);
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

void
EventObject::RemoveHandler (int event_id, EventHandler handler, gpointer data)
{
	if (GetType()->GetEventCount() <= 0) {
		g_warning ("removing handler for event with id %d, which has not been registered\n", event_id);
		return;
	}
	
	if (events == NULL)
		return;
	
	EventClosure *closure = (EventClosure *) events->lists [event_id].event_list->First ();
	while (closure) {
		if (closure->func == handler && closure->data == data) {
 			if (events->lists [event_id].emitting > 0) {
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
EventObject::RemoveHandler (int event_id, int token)
{
	if (GetType()->GetEventCount() <= 0) {
		g_warning ("removing handler for event with id %d, which has not been registered\n", event_id);
		return;
	}
	
	if (events == NULL)
		return;
	
	EventClosure *closure = (EventClosure *) events->lists [event_id].event_list->First ();
	while (closure) {
		if (closure->token == token) {
			if (events->lists [event_id].emitting > 0) {
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
				if (events->lists [i].emitting > 0) {
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
	if (GetType()->GetEventCount() <= 0) {
		g_warning ("removing handler for event with id %d, which has not been registered\n", event_id);
		return;
	}
	
	if (events == NULL)
		return;

	EventClosure *c = (EventClosure *) events->lists [event_id].event_list->First ();
	while (c) {
		if (predicate (c->func, c->data, closure)) {
			if (events->lists [event_id].emitting > 0) {
				c->pending_removal = true;
			} else {
				events->lists [event_id].event_list->Remove (c);
			}
			break;
		}
		
		c = (EventClosure *) c->next;
	}
}

bool
EventObject::Emit (char *event_name, EventArgs *calldata, bool only_unemitted)
{
	int id = GetType()->LookupEvent (event_name);

	if (id == -1) {
		g_warning ("trying to emit event '%s', which has not been registered\n", event_name);
		return false;
	}

	return Emit (id, calldata, only_unemitted);
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
EventObject::Emit (int event_id, EventArgs *calldata, bool only_unemitted)
{
	if (IsDisposed ())
		return false;
	
	if (GetType()->GetEventCount() <= 0 || event_id >= GetType()->GetEventCount()) {
		g_warning ("trying to emit event with id %d, which has not been registered\n", event_id);
		if (calldata)
			calldata->unref ();
		return false;
	}

	if (events == NULL || events->lists [event_id].event_list->IsEmpty ()) {
		if (calldata)
			calldata->unref ();
		return false;
	}

	if (!Surface::InMainThread ()) {
		Deployment *deployment = GetDeployment ();
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

	EmitContext* ctx = StartEmit (event_id);

	DoEmit (event_id, ctx, calldata, only_unemitted);

	if (calldata)
		calldata->unref ();

	FinishEmit (event_id, ctx);

	return true;
}

struct EmitContext {
  int length;
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

EmitContext*
EventObject::StartEmit (int event_id)
{
	EmitContext *ctx = new EmitContext();
	EventClosure *closure;

	if (GetType()->GetEventCount() <= 0 || event_id >= GetType()->GetEventCount()) {
		g_warning ("trying to start emit with id %d, which has not been registered\n", event_id);
		return ctx;
	}

	if (events == NULL || events->lists [event_id].event_list->IsEmpty ()) {
		return ctx;
	}

	events->emitting++;
	events->lists [event_id].emitting++;

	ctx->length = events->lists [event_id].event_list->Length();
	ctx->closures = g_new (EventClosure*, ctx->length);

	/* make a copy of the event list to use for emitting */
	closure = (EventClosure *) events->lists [event_id].event_list->First ();
	for (int i = 0; closure != NULL; i++) {
		ctx->closures [i] = closure;
		closure = (EventClosure *) closure->next;
	}

	return ctx;
}

bool
EventObject::DoEmit (int event_id, EmitContext *ctx, EventArgs *calldata, bool only_unemitted)
{
	/* emit the events using the copied list */
	for (int i = 0; i < ctx->length; i++) {
		EventClosure *closure = ctx->closures[i];
		if (closure && closure->func
		    && (!only_unemitted || closure->emit_count == 0)) {
			closure->func (this, calldata, closure->data);
			closure->emit_count ++;
		}
	}

	return ctx->length > 0;
}

void
EventObject::FinishEmit (int event_id, EmitContext *ctx)
{
	if (GetType()->GetEventCount() <= 0 || event_id >= GetType()->GetEventCount()) {
		g_warning ("trying to finish emit with id %d, which has not been registered\n", event_id);
		goto delete_ctx;
	}

	if (ctx->length == 0)
		goto delete_ctx;

	events->lists [event_id].emitting--;
	events->emitting--;

	if (events->lists [event_id].emitting == 0) {
		// Remove closures which are waiting for removal
		EventClosure *closure = (EventClosure *) events->lists [event_id].event_list->First ();
		while (closure != NULL) {
			EventClosure *next = (EventClosure *) closure->next;
			if (closure->pending_removal)
				events->lists [event_id].event_list->Remove (closure);
			closure = next;
		}
	}

 delete_ctx:
	delete ctx;
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
	virtual void Invoke (DependencyObject *sender, PropertyChangedEventArgs *args) = 0;

	virtual gpointer GetListener () = 0;
	virtual gpointer GetProperty () = 0;
};

class WildcardListener {
public:
	WildcardListener (DependencyObject *obj, DependencyProperty *prop)
	{
		this->obj = obj;
		this->prop = prop;
	}

	virtual bool Matches (PropertyChangedEventArgs *args) { return true; }
	virtual void Invoke (DependencyObject *sender, PropertyChangedEventArgs *args)
	{
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

class CallbackListener {
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

	virtual void Invoke (DependencyObject *sender, PropertyChangedEventArgs *args)
	{
		cb (sender, args, closure);
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

	if (v != NULL && v->Is (Type::DEPENDENCY_OBJECT) && v->AsDependencyObject() != NULL) {
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
DependencyObject::NotifyListenersOfPropertyChange (PropertyChangedEventArgs *args)
{
	g_return_if_fail (args);

	listeners_notified = true;

	for (GSList *l = listener_list; l != NULL; l = l->next){
		Listener *listener = (Listener*)l->data;

		if (listener->Matches (args))
			listener->Invoke (this, args);
	}
}

void
DependencyObject::NotifyListenersOfPropertyChange (int id)
{
	if (IsDisposed ())
		return;
	NotifyListenersOfPropertyChange (GetDeployment ()->GetTypes ()->GetProperty (id));
}

void
DependencyObject::NotifyListenersOfPropertyChange (DependencyProperty *subproperty)
{
	// XXX I really think this method should go away.  we only use it in
	// a couple of places, and it abuses things.

	Value *new_value = subproperty ? GetValue (subproperty) : NULL;

	PropertyChangedEventArgs args (subproperty, subproperty->GetId (), NULL, new_value);

	NotifyListenersOfPropertyChange (&args);
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
		if (value->Is (Type::EVENTOBJECT) && !value->AsEventObject ()) {
			// if it's a null DependencyObject, it doesn't matter what type it is
			return true;
		}
		
		if (value->Is (Type::MANAGED)) {
			// This is a big hack, we do no type-checking if we try to set a managed type.
			// Given that for the moment we might not have the surface available, we can't
			// do any type checks since we can't access types registered on the surface.
			return true;
		}
		
		if (!value->Is (property->GetPropertyType())) {
			MoonError::FillIn (error, MoonError::ARGUMENT, 1001,
					   g_strdup_printf ("DependencyObject::SetValue, value cannot be assigned to the "
							    "property %s::%s (property has type '%s', value has type '%s')",
							    GetTypeName (), property->GetName(), Type::Find (property->GetPropertyType())->GetName (),
							    Type::Find (value->GetKind ())->GetName ()));
			return false;
		}
	} else {
		// In 2.0, property->GetPropertyType() can return
		// something greater than Type::LASTTYPE.  Only check
		// built-in types for null Types registered on the
		// managed side has their own check there.
		if (property->GetPropertyType () < Type::LASTTYPE && !(Type::IsSubclassOf (property->GetPropertyType(), Type::DEPENDENCY_OBJECT)) && !property->IsNullable ()) {
			MoonError::FillIn (error, MoonError::ARGUMENT, 1001,
					   g_strdup_printf ("Can not set a non-nullable scalar type to NULL (property: %s)",
							    property->GetName()));
			return false;
		}
	}

	if (DependencyObject::NameProperty == property->GetId ()) {
		NameScope *scope = FindNameScope ();
		if (scope && value) {
			DependencyObject *o = scope->FindName (value->AsString ());
			if (o && o != this) {
				MoonError::FillIn (error, MoonError::ARGUMENT, 2028,
						   g_strdup_printf ("The name already exists in the tree: %s (%p %p).",
								    value->AsString (), o, this));
				return false;
			}
		}
		// TODO: Name validation
		// This doesn't happen in 1.0 or 2.0b according to my tests, but according to the 2.0 docs
		// names need to start with a '_' or letter.  They can't start with a _.  Also characters
		// should be limited to a-z A-Z 0-9 and _.  Once a newer beta actually enforces this
		// I'll implement the validation method.
	}

	return true;
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
		MoonError::FillIn (error, MoonError::UNAUTHORIZED_ACCESS, g_strdup_printf ("Cannot set value for property '%s' on frozen DependencyObject '%s'", property->GetName(), GetTypeName()));
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
		
		if (value && (!property->IsAutoCreated () || !value->Is (Type::DEPENDENCY_OBJECT) || value->AsDependencyObject () != NULL))
			new_value = new Value (*value);
		else
			new_value = NULL;
		
		// replace it with the new value
		if (new_value)
			g_hash_table_insert (local_values, property, new_value);
		
		ProviderValueChanged (PropertyPrecedence_LocalValue, property, current_value, new_value, true, error);
		
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

	if (v != NULL && v->Is (Type::DEPENDENCY_OBJECT) && v->AsDependencyObject() != NULL) {
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

	NameScope *this_ns = NameScope::GetNameScope(this);
	if (this_ns) {
		if (this_ns->GetTemporary()) {
			to_ns->MergeTemporaryScope (this_ns, error);
			ClearValue (NameScope::NameScopeProperty, false);
		}
		return;
	}

	const char *n = GetName();
		
	if (n && *n) {
		DependencyObject *o = to_ns->FindName (n);
		if (o && o != this) {
			MoonError::FillIn (error, MoonError::ARGUMENT, 2028,
					   g_strdup_printf ("The name already exists in the tree: %s.",
							    n));
			return;
		} else if (o == this)
			return;
		to_ns->RegisterName (n, this);
	}

	RegisterNamesClosure closure;
	closure.to_ns = to_ns;
	closure.error = error;
	
	if (autocreate)
		g_hash_table_foreach (autocreate->auto_values, register_depobj_names, &closure);
	
	g_hash_table_foreach (local_values, register_depobj_names, &closure);
}

static void
unregister_depobj_names (gpointer  key,
			 gpointer  value,
			 gpointer  user_data)
{
	NameScope *from_ns = (NameScope*)user_data;
	Value *v = (Value*)value;
	DependencyProperty *property = (DependencyProperty*)key;

	if (property->GetId() != UIElement::TagProperty && v != NULL && v->Is (Type::DEPENDENCY_OBJECT) && v->AsDependencyObject() != NULL) {
		DependencyObject *obj = v->AsDependencyObject ();
		obj->UnregisterAllNamesRootedAt (from_ns);
	}
}

void
DependencyObject::UnregisterAllNamesRootedAt (NameScope *from_ns)
{
	AutoCreatePropertyValueProvider *autocreate = (AutoCreatePropertyValueProvider *) providers[PropertyPrecedence_AutoCreate];
	NameScope *this_ns = NameScope::GetNameScope(this);
	if (this_ns && !this_ns->GetTemporary())
		return;
	
	const char *n = GetName();
	
	if (n && strlen (n) > 0)
		from_ns->UnregisterName (n);
	
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
		Type *pt = Type::Find (property->GetOwnerType ());
		MoonError::FillIn (error, MoonError::EXCEPTION, g_strdup_printf ("Cannot get the DependencyProperty %s.%s on an object of type %s", pt ? pt->GetName () : "<unknown>", property->GetName (), GetTypeName ()));
		return NULL;
	}
	return ReadLocalValue (property);
}

Value *
DependencyObject::GetValueWithError (Type::Kind whatami, DependencyProperty *property, MoonError *error)
{
	if (!HasProperty (whatami, property, true)) {
		Type *pt = Type::Find (property->GetOwnerType ());
		MoonError::FillIn (error, MoonError::EXCEPTION, g_strdup_printf ("Cannot get the DependencyProperty %s.%s on an object of type %s", pt ? pt->GetName () : "<unknown>", property->GetName (), GetTypeName ()));
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
	return GetValue (property, PropertyPrecedence_Highest);
}

Value *
DependencyObject::GetValue (DependencyProperty *property, PropertyPrecedence startingAtPrecedence)
{
	for (int i = startingAtPrecedence; i < PropertyPrecedence_Count; i ++) {
		if (!providers[i])
			continue;
		Value *value = providers[i]->GetPropertyValue (property);
		if (value) return value;
	}
	return NULL;
}

Value *
DependencyObject::GetValueSkippingPrecedence (DependencyProperty *property, PropertyPrecedence toSkip)
{
	for (int i = 0; i < PropertyPrecedence_Count; i ++) {
		if (i == toSkip)
			continue;
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
		Type *pt = Type::Find (property->GetOwnerType ());
		MoonError::FillIn (error, MoonError::EXCEPTION, g_strdup_printf ("Cannot get the DependencyProperty %s.%s on an object of type %s", pt ? pt->GetName () : "<unknown>", property->GetName (), GetTypeName ()));
		return NULL;
	}
	return GetValueNoDefault (property);	
}

void
DependencyObject::ProviderValueChanged (PropertyPrecedence providerPrecedence,
					DependencyProperty *property,
					Value *old_provider_value, Value *new_provider_value,
					bool notify_listeners, MoonError *error)
{
	int p;

	// if they're both NULL, get out of here.
	if (!old_provider_value && !new_provider_value)
		return;

	// first we look for a value higher in precedence for this property
	for (p = providerPrecedence - 1; p >= PropertyPrecedence_Highest; p --) {
		if (providers[p] && providers[p]->GetPropertyValue (property)) {
			// a provider higher in precedence already has
			// a value for this property, so the one
			// that's changing isn't visible anyway.
			return;
		}
	}

	Value *old_value;
	Value *new_value;

	if (!old_provider_value || !new_provider_value) {
		Value *lower_priority_value = GetValue (property, (PropertyPrecedence)(providerPrecedence + 1));

		if (old_provider_value == NULL) {
			// we're changing from the old value (from a lower
			// priority provider) to @new_provider_value.
			old_value = lower_priority_value;
			new_value = new_provider_value;
		}
		else if (new_provider_value == NULL) {
			// we're changing from @old_provider_value to whatever the
			// value lower on the priority list is.
			old_value = old_provider_value;
			new_value = lower_priority_value;
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
		bool setsParent = property->GetId() != UIElement::TagProperty;

		if (old_value && old_value->Is (Type::DEPENDENCY_OBJECT))
			old_as_dep = old_value->AsDependencyObject ();
		if (new_value && new_value->Is (Type::DEPENDENCY_OBJECT))
			new_as_dep = new_value->AsDependencyObject ();

		if (old_as_dep) {
			old_as_dep->SetSurface (NULL);

			if (setsParent) {
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
		}

		if (new_as_dep) {
			new_as_dep->SetSurface (GetSurface ());

			if (setsParent) {
				MoonError error;
				new_as_dep->SetParent (this, &error);
				if (error.number)
					return;

				if (new_as_dep->Is(Type::COLLECTION)) {
					new_as_dep->AddHandler (Collection::ChangedEvent, collection_changed, this);
					new_as_dep->AddHandler (Collection::ItemChangedEvent, collection_item_changed, this);
				}
			
				// listen for property changes on the new object
				new_as_dep->AddPropertyChangeListener (this, property);

				// add ourselves as a target
				new_as_dep->AddTarget (this);
			}
		}


		// we need to make this optional, as doing it for NameScope
		// merging is killing performance (and noone should ever care
		// about that property changing)
		if (notify_listeners) {
			listeners_notified = false;
		
			PropertyChangedEventArgs args (property, property->GetId (), old_value, new_value);

			OnPropertyChanged (&args, error);

			if (!listeners_notified) {
				g_warning ("setting property %s::%s on object of type %s didn't result in listeners being notified",
					   Type::Find(property->GetOwnerType())->GetName (), property->GetName(), GetTypeName ());
				if (error->number)
					g_warning ("the error was: %s", error->message);
			}
		}

		if (property && property->GetChangedCallback () != NULL) {
			NativePropertyChangedHandler *callback = property->GetChangedCallback ();
			callback (property, this, old_value, new_value, error);
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
	
	if (old_local_value == NULL) {
		// there wasn't a local value set.  don't do anything
		return;
	}

	// detach from the existing value
	if (old_local_value->Is (Type::DEPENDENCY_OBJECT)) {
		DependencyObject *dob = old_local_value->AsDependencyObject();

		if (dob != NULL) {
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

	ProviderValueChanged (PropertyPrecedence_LocalValue, property, old_local_value, NULL, notify_listeners, error);
	
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
	if (v->Is (Type::DEPENDENCY_OBJECT)){
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

	PropertyChangedEventArgs propChangedArgs (itemArgs->GetProperty(),
						  itemArgs->GetProperty()->GetId (),
						  itemArgs->GetOldValue(),
						  itemArgs->GetNewValue());

	obj->OnCollectionItemChanged ((Collection*)sender,
				      itemArgs->GetCollectionItem(),
				      &propChangedArgs);
}

DependencyObject::DependencyObject ()
{
	SetObjectType (Type::DEPENDENCY_OBJECT);
	Initialize ();
}
/*
DependencyObject::DependencyObject (Deployment *deployment)
	: EventObject (deployment)
{
	SetObjectType (Type::DEPENDENCY_OBJECT);
	Initialize ();
}
*/
DependencyObject::DependencyObject (Deployment *deployment, Type::Kind object_type)
	: EventObject (deployment, object_type)
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
	is_frozen = false;
}

void
DependencyObject::Freeze()
{
	is_frozen = true;
}

static void
free_listener (gpointer data, gpointer user_data)
{
	Listener* listener = (Listener*) data;
	delete listener;
}

DependencyObject::~DependencyObject ()
{
	g_hash_table_destroy (local_values);
	local_values = NULL;
	delete[] providers;
	providers = NULL;
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
	return DependencyProperty::GetDependencyProperty (GetObjectType (), name);
}

bool
DependencyObject::HasProperty (const char *name, bool inherits)
{
	return DependencyProperty::GetDependencyProperty (GetObjectType (), name, inherits) != NULL;
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

	if (!Type::Find (this_type)->IsSubclassOf (property->GetOwnerType ())) {
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
	NameScope *scope = NameScope::GetNameScope (this);
	DependencyObject *rv = NULL;
	
	if (scope && (rv = scope->FindName (name)))
		return rv;
	
	if (parent)
		return parent->FindName (name);
	
	Surface *surface = GetSurface ();
	if (surface) {
		UIElement *toplevel = surface->GetToplevel ();
		if (toplevel && toplevel != this)
			return toplevel->FindName (name);
	}
	
	return NULL;
}

NameScope*
DependencyObject::FindNameScope ()
{
	NameScope *scope = NameScope::GetNameScope (this);

	if (scope)
		return scope;

	if (parent)
		return parent->FindNameScope ();

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
	Value *v = (Value *) value;
	
	if (v && v->Is (Type::DEPENDENCY_OBJECT)) {
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
	
	if (parent != this->parent) {
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

		// the unregistration has to happen after the
		// registration, in order to make sure
		// namescope-corner-cases.html (test1) passes.  don't
		// ask me, it's crazy.
		if (this->parent) {
			NameScope *parent_scope = this->parent->FindNameScope ();
			if (parent_scope)
				UnregisterAllNamesRootedAt (parent_scope);
		}

		if (!error || error->number == 0)
			this->parent = parent;
	}
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
		}
	}

	NotifyListenersOfPropertyChange (args);
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
