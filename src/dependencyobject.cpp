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
#include "textbox.h"

namespace Moonlight {

#if PROPERTY_LOOKUP_DIAGNOSTICS
gint64 provider_property_lookups = 0;

struct totals
{
	GHashTable *get_values;
	int properties;
	int lookups;
};
#endif

// event handlers for c++
class EventClosure : public List::Node {
public:
	EventClosure (EventHandler func, gpointer data, bool handledEventsToo, GDestroyNotify data_dtor, bool managed_data_dtor, int token) {
		this->func = func;
		this->data = data;
		this->handledEventsToo = handledEventsToo;
		this->data_dtor = data_dtor;
		this->token = token;
		this->managed_data_dtor = managed_data_dtor;
	       
		pending_removal = false;
		emit_count = 0;
	}

	void InvokeDataDtor (bool is_shutting_down)
	{
		if (is_shutting_down && managed_data_dtor)
			return;

		if (data_dtor) {
			data_dtor (data);
			data_dtor = NULL;
			data = NULL;
		}
	}

	~EventClosure ()
	{
		if (!managed_data_dtor && data_dtor)
			data_dtor (data);
	}

	EventHandler func;
	gpointer data;
	bool handledEventsToo;
	GDestroyNotify data_dtor;
	int token;
	bool pending_removal;
	int emit_count;
	/* Managed data dtor aren't called when we're destroyed, nor when we're shutting down.
	 * In both cases becuase the managed delegate might have been collected, and we'd abort. */
	bool managed_data_dtor;
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
	
#if SANITY
	g_assert (depl != NULL); /* #if SANITY */
#endif

	object_type = type;
	deployment = depl;
	if (deployment != NULL && this != deployment) {
		deployment->ref ();
	}
	flags = g_atomic_int_exchange_and_add (&current_id, 1);
	refcount = 1;
	events = NULL;
	addManagedRef = NULL;
	clearManagedRef = NULL;
	mentorChanged = NULL;
	attached = NULL;
	detached = NULL;
	hadManagedPeer = false;

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
#endif
	
#if DEBUG || OBJECT_TRACKING
	if (object_type != Type::DEPLOYMENT)
		deployment->TrackObjectCreated (this);
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
#if DEBUG || OBJECT_TRACKING
	if (object_type != Type::DEPLOYMENT)
		deployment->TrackObjectDestroyed (this);
#endif
#if OBJECT_TRACKING
	Track ("Destroyed", "");
#endif

#if SANITY
	if (refcount != 0) {
		g_warning ("EventObject::~EventObject () #%i was deleted before its refcount reached 0 (current refcount: %i)\n", GetId (), refcount);
	}
	/* A special value, indicating that we're accessing a deleted object
	 * having a refcount of 0 doesn't necessarily mean we've been deleted
	 * already, we might be in the Disposed handler */
	refcount = -666;

	if (managed_handle.IsAllocated ()) {
		g_warning ("EventObject::~EventObject () was called with a non-null managed_handle\n");
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

void
EventObject::SetIsAttached (bool value)
{
	VERIFY_MAIN_THREAD;

	if (IsAttached() == value)
		return;

	if (value) {
		flags |= Attached;
		if (attached && deployment && !deployment->IsShuttingDown())
			attached (this);
	} else {
		flags &= ~Attached;
		if (detached && deployment && !deployment->IsShuttingDown())
			detached (this);
	}

	OnIsAttachedChanged (value);
}

void
EventObject::ClearWeakRef (EventObject *sender, EventArgs *args, gpointer closure)
{
#if DEBUG_WEAKREF
	printf ("EventOject::ClearWeakRef () %p = %s = %i\n", sender, sender->GetTypeName (), GET_OBJ_ID (sender));
#endif
	EventObject **eo_ptr = (EventObject**)closure;
	*eo_ptr = NULL;
}

void
EventObject::SetManagedPeerCallbacks (ManagedRefCallback add_strong_ref,
				      ManagedRefCallback clear_strong_ref,
				      MentorChangedCallback mentor_changed,
				      AttachCallback attached,
				      AttachCallback detached)
{
	hadManagedPeer = true;

	this->addManagedRef = add_strong_ref;
	this->clearManagedRef = clear_strong_ref;
	this->mentorChanged = mentor_changed;
	this->attached = attached;
	this->detached = detached;
}

void
EventObject::EnsureManagedPeer ()
{
	deployment->EnsureManagedPeer (this);
}

void
EventObject::AddTickCall (TickCallHandler handler, EventObject *data)
{
	AddTickCallInternal (handler, data);
}

void
EventObject::AddTickCallInternal (TickCallHandler handler, EventObject *data)
{
	Surface *surface = NULL;
	TimeManager *timemanager = NULL;
	
	/* This method is called from several threads, so it must be thread-safe.
	 * It's safe to get the deployment field, since it's only written to in the ctor
	 * or dtor. We use a thread-safe surface getter, and a thread-safe timemanager getter  */

	surface = GetDeployment ()->GetSurfaceReffed ();
	
	if (!surface) {
		LOG_DP ("EventObject::AddTickCall (): Could not add tick call, no surface\n");
		goto cleanup;
	}
	
	timemanager = surface->GetTimeManagerReffed ();
	
	if (!timemanager) {
		LOG_DP ("EventObject::AddTickCall (): Could not add tick call, no time manager\n");
		goto cleanup;
	}

	timemanager->AddTickCall (handler, data ? data : this);
	
cleanup:
	if (surface)
		surface->unref ();
	
	if (timemanager)
		timemanager->unref ();
}

#if SANITY
Deployment *
EventObject::GetDeployment ()
{
	if (deployment == NULL)
		g_warning ("EventObject::GetDeployment () should not be reached with a null deployment");
	
	if (deployment != Deployment::GetCurrent () && Deployment::GetCurrent () != NULL && object_type != Type::DEPLOYMENT) {
		g_warning ("EventObject::GetDeployment () our deployment %p doesn't match Deployment::GetCurrent () %p", deployment, Deployment::GetCurrent ());
		print_stack_trace ();
	}

	return deployment;
}
#endif

void
EventObject::SetCurrentDeployment (bool domain)
{
	if (deployment != NULL) {
		Deployment::SetCurrent (deployment, domain);
	}
}

void
EventObject::Dispose ()
{
#if DEBUG_WEAKREF
	printf ("EventObject::Dispose (this = %p/%s/%i) ", this, GetTypeName (), GET_OBJ_ID (this));
#endif
	if (!IsDisposed () && Surface::InMainThread () && deployment) {
		// Dispose can be called multiple times, but Emit only once. When DestroyedEvent was in the dtor, 
		// it could only ever be emitted once, don't change that behaviour.
		Emit (DestroyedEvent); // TODO: Rename to DisposedEvent
	}

	// Set the disposed flag.
	flags = (Flags) (flags | Disposed);
}

bool
EventObject::IsDisposed ()
{
	return (flags & Disposed) != 0;
}

bool
EventObject::IsDisposing ()
{
	return (flags & Disposing) != 0;
}

bool
EventObject::IsAttached ()
{
	return (flags & Attached) != 0;
}

void
EventObject::Resurrect ()
{
	int v = g_atomic_int_exchange_and_add (&refcount, 1);

#if SANITY
	g_assert (v >= 0); //  #if SANITY
#endif

	OBJECT_TRACK ("Resurrect [Ref]", GetTypeName ());
}

void
EventObject::ref ()
{
	int v = g_atomic_int_exchange_and_add (&refcount, 1);
		
#if DEBUG		
	if (deployment != Deployment::GetCurrent () && object_type != Type::DEPLOYMENT) {
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

	} else if (v == 1 && managed_handle.IsAllocated ()) {
		if (moonlight_flags & RUNTIME_INIT_ENABLE_TOGGLEREFS) {
			/* It's possible for a managed object to die at any stage, so there's no way to
			* guarantee that calling ref () on an object of refcount 1 will actually have a living
			* managed peer to GCHandle. This is just something we'll have to cope with without
			* blowing up. So, if the managed peer is dead when we ref a native peer, don't bother
			* swapping the existing GCHandle.
			*
			* One scenario where we hit this is when animating a DO where nothing holds a ref to
			* the DO except the storyboard.
			*/

			GCHandle old_handle = managed_handle;
			void *man_ob = deployment->GetGCHandleTarget (old_handle);
			if (man_ob) {
				this->managed_handle = deployment->CreateGCHandle (man_ob);
				deployment->FreeGCHandle (old_handle);
			}
		}
	}

	OBJECT_TRACK ("Ref", GetTypeName ());
}

void
EventObject::unref_static (EventObject *obj)
{
	if (obj == NULL)
		return;
	obj->unref ();
}

void 
EventObject::unref ()
{
	// we need to retrieve all instance fields into locals before decreasing the refcount
	// TODO: do we need some sort of gcc foo (volatile variables, memory barries)
	// to ensure that gcc does not optimize the fetches below away
	GCHandle managed_handle = this->managed_handle;
#if OBJECT_TRACKING
	Deployment *depl = this->deployment ? this->deployment : Deployment::GetCurrent ();
	const char *type_name = depl == NULL ? NULL : (object_type == Type::DEPLOYMENT ? "Deployment" : Type::Find (depl, GetObjectType ())->GetName ());
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
		flags |= Disposing;
		
		// here we *can* access instance fields, since we know that we haven't been
		// destructed already.
		Dispose ();

		flags &= ~Disposing;
		
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
			
	} else if (v == 1 && managed_handle.IsAllocated ()) {
		/* As per comment in DependencyObject::ref (), it's possible we reffed a native DO
		* whose managed peer had died before we reffed. In this scenario, the target of the
		* GCHandle will be null even though technically the peer should have a strong gchandle
		* attached. This is not an error condition. In this scenario, don't bother switching
		* the GCHandle to a weak one.
		*/

		GCHandle old_handle = managed_handle;
		void *man_ob = deployment->GetGCHandleTarget (old_handle);
		if (man_ob) {
			this->managed_handle = deployment->CreateWeakGCHandle (man_ob);
			deployment->FreeGCHandle (old_handle);
		}
#if 0
		printf ("%s::unref (): %p switching from strong %p to weak %p (strong obj: %p weak obj: %p) deployment: %p\n",
			GetTypeName (), this, old_handle.ToIntPtr (), this->managed_handle.ToIntPtr (), deployment->GetGCHandleTarget (old_handle),
			deployment->GetGCHandleTarget (this->managed_handle), deployment);
#endif
	}

#if SANITY
	if (v < 0) {
		g_warning ("EventObject::Unref (): NEGATIVE REFCOUNT id: %i v: %i refcount: %i", GET_OBJ_ID (this), v, refcount);
		print_stack_trace ();
		abort (); /*  #if SANITY */
	}
#endif
}

void
EventObject::SetManagedHandle (GCHandle managed_handle)
{
	// This method should be called exactly twice. Once to set the
	// managed handle when the managed peer is created and once to
	// clear the handle when the managed peer has died. Ensure that
	// we free the weak gchandle when the managed peer dies.
#if SANITY
	g_assert (this->managed_handle.IsAllocated () != managed_handle.IsAllocated ()); // #if SANITY
#endif

	deployment->FreeGCHandle (this->managed_handle);

	this->managed_handle = managed_handle;
	if (managed_handle.IsAllocated ())
		ref ();
	else
		unref ();
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
static bool track_store = false;

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
		track_store = (getenv ("MOONLIGHT_OBJECT_TRACK_STORE") != NULL);
	}

	if (track_store) {
		if (strcmp (done, "Destroyed") != 0) {
			store_reftrace (this, done, refcount);
		} else {
			free_reftrace (this);
		}
	} else {
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
}

void
EventObject::PrintStackTrace ()
{
	print_stack_trace ();
}
#endif

int
EventObject::AddHandler (const char *event_name, EventHandler handler, gpointer data, GDestroyNotify data_dtor, bool managed_data_dtor, bool handledEventsToo)
{
	int id = GetType()->LookupEvent (event_name);

	if (id == -1) {
		g_warning ("adding handler to event '%s', which has not been registered\n", event_name);
		return -1;
	}

	return AddHandler (id, handler, data, data_dtor, managed_data_dtor, handledEventsToo);
}

int
EventObject::AddHandler (int event_id, EventHandler handler, gpointer data, GDestroyNotify data_dtor, bool managed_data_dtor, bool handledEventsToo)
{ 
	if (GetType()->GetEventCount() <= 0) {
		g_warning ("adding handler to event with id %d, which has not been registered\n", event_id);
		return -1;
	}
	
	if (events == NULL)
		events = new EventLists (GetType ()->GetEventCount ());

	int token = events->lists [event_id].current_token++;
	
	events->lists [event_id].event_list->Append (new EventClosure (handler, data, handledEventsToo, data_dtor, managed_data_dtor, token));
	
	return token;
}

void
EventObject::AddOnEventHandler (int event_id, EventHandler handler, gpointer data, GDestroyNotify data_dtor, bool managed_data_dtor, bool handledEventsToo)
{
	if (GetType()->GetEventCount() <= event_id) {
		g_warning ("adding OnEvent handler to event with id %d, which has not been registered\n", event_id);
		return;
	}

	if (events == NULL)
		events = new EventLists (GetType ()->GetEventCount ());

	events->lists [event_id].onevent = new EventClosure (handler, data, handledEventsToo, data_dtor, managed_data_dtor, 0);
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
EventObject::AddXamlHandler (const char *event_name, EventHandler handler, gpointer data, GDestroyNotify data_dtor, bool managed_data_dtor, bool handledEventsToo)
{
	int id = GetType ()->LookupEvent (event_name);
	
	if (id == -1) {
		g_warning ("adding xaml handler to event '%s', which has not been registered\n", event_name);
		return -1;
	}
	
	return AddXamlHandler (id, handler, data, data_dtor, managed_data_dtor, handledEventsToo);
}

int
EventObject::AddXamlHandler (int event_id, EventHandler handler, gpointer data, GDestroyNotify data_dtor, bool managed_data_dtor, bool handledEventsToo)
{ 
	if (GetType ()->GetEventCount () <= 0) {
		g_warning ("adding xaml handler to event with id %d, which has not been registered\n", event_id);
		return -1;
	}
	
	if (events == NULL)
		events = new EventLists (GetType ()->GetEventCount ());

	events->lists [event_id].event_list->Append (new EventClosure (handler, data, handledEventsToo, data_dtor, managed_data_dtor, 0));
	
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

	bool is_shutting_down = GetDeployment ()->IsShuttingDown ();
	EventClosure *closure = (EventClosure *) events->lists [event_id].event_list->First ();
	EventClosure *next;
	while (closure) {
		next = (EventClosure *) closure->next;
		if (closure->func == handler && closure->data == data) {
			token = closure->token;
 			if (!events->lists [event_id].context_stack->IsEmpty()) {
 				closure->pending_removal = true;
 			} else {
				closure->InvokeDataDtor (is_shutting_down);
				events->lists [event_id].event_list->Remove (closure);
 			}
			break;
		}
		
		closure = next;
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

	bool is_shutting_down = GetDeployment ()->IsShuttingDown ();
	EventClosure *closure = (EventClosure *) events->lists [event_id].event_list->First ();
	EventClosure *next;
	while (closure) {
		next = (EventClosure *) closure->next;
		if (closure->token == token) {
			if (!events->lists [event_id].context_stack->IsEmpty()) {
				closure->pending_removal = true;
			} else {
				closure->InvokeDataDtor(is_shutting_down);
				events->lists [event_id].event_list->Remove (closure);
			}
			break;
		}
		
		closure = next;
	}
}

void
EventObject::RemoveAllHandlers (gpointer data)
{
	if (events == NULL)
		return;

	bool is_shutting_down = GetDeployment ()->IsShuttingDown ();
	int count = GetType ()->GetEventCount ();
	
	for (int i = 0; i < count; i++) {
		EventClosure *closure = (EventClosure *) events->lists [i].event_list->First ();
		EventClosure *next;
		while (closure) {
			next = (EventClosure *) closure->next;
			if (closure->data == data) {
				if (!events->lists [i].context_stack->IsEmpty()) {
					closure->pending_removal = true;
				} else {
					closure->InvokeDataDtor(is_shutting_down);
					events->lists [i].event_list->Remove (closure);
				}
				break;
			}
			
			closure = next;
		}
	}
}

void
EventObject::RemoveMatchingHandlers (int event_id, bool (*predicate)(int token, EventHandler cb_handler, gpointer cb_data, gpointer data), gpointer closure)
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
	EventClosure *next;
	bool is_shutting_down = GetDeployment ()->IsShuttingDown ();
	while (c) {
		next = (EventClosure *) c->next;
		if (!predicate || predicate (c->token, c->func, c->data, closure)) {
			if (!events->lists [event_id].context_stack->IsEmpty()) {
				c->pending_removal = true;
			} else {
				c->InvokeDataDtor (is_shutting_down);
				events->lists [event_id].event_list->Remove (c);
			}
		}
		
		c = next;
	}
}

void
EventObject::ForeachHandler (int event_id, bool only_new, HandlerMethod m, gpointer closure)
{
	if (events == NULL)
		return;

	EventClosure *event_closure = (EventClosure *) events->lists [event_id].event_list->First ();
	EventClosure *next;
	int last_foreach_generation = events->lists [event_id].last_foreach_generation;
	while (event_closure) {
		next = (EventClosure *) event_closure->next;
		if (!event_closure->pending_removal && (!only_new || event_closure->token >= last_foreach_generation))
			(*m) (this, event_closure->token, closure);
		event_closure = next;
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
	EventClosure *next;
	while (event_closure) {
		next = (EventClosure *) event_closure->next;
		if (event_closure->token == token) {
			(*m) (this, event_closure->token, closure);
			break;
		}
		event_closure = next;
	}
}

bool
EventObject::HasHandlers (int event_id, int newer_than_generation)
{
	// no events, trivially false
	if (events == NULL)
		return false;

	// if we have an onevent handler, trivially true
	if (events->lists [event_id].onevent != NULL)
		return true;

	// if we're not caring about generation, return true if there are any events
	if (newer_than_generation == -1 && !events->lists [event_id].event_list->IsEmpty ())
		return true;

	// otherwise we need to loop over them to find one newer than @newer_than_generation
	EventClosure *event_closure = (EventClosure *) events->lists [event_id].event_list->First ();
	while (event_closure) {
		if (event_closure->token >= newer_than_generation)
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

	if (hadManagedPeer && !managed_handle.IsAllocated ()) {
#if DEBUG
		/* We're doomed. Don't emit any more events. Note that we'll normally hit this condition once in a while. */
		printf ("Moonlight: Trying to emit event %i on %s after the managed object has been collected.\n", event_id, GetTypeName ());
#endif
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

		// FIXME: we need this to fix a leak, but enabling it crashes
		// Microsoft.SilverlightControls/site/ControlsExtended.Test.html
		//
		// if (args)
		// 	args->unref ();
	}
};

void
EventObject::emit_async (EventObject *calldata)
{
	AsyncEventClosure *async = (AsyncEventClosure *) calldata;
	
	async->sender->Emit (async->event_id, async->args, async->unemitted, async->generation);
}

bool
EventObject::EmitAsync (int event_id, EventArgs *calldata, bool only_unemitted)
{
	AsyncEventClosure *async;

	if (!CanEmitEvents (event_id)) {
		if (calldata)
			calldata->unref ();
		return false;
	}

	async = new AsyncEventClosure (this, event_id, calldata, only_unemitted, GetEventGeneration (event_id));
	AddTickCall (EventObject::emit_async, async);
	async->unref ();
	
	return true;
}

struct EmitData {
	EventObject *sender;
	int event_id;
	EventArgs *calldata;
	bool only_unemitted;
	int only_token;
};

bool
EventObject::EmitCallback (gpointer d)
{
	EmitData *data = (EmitData *) d;
	data->sender->SetCurrentDeployment ();
	data->sender->EmitOnly (data->event_id, data->only_token, data->calldata, data->only_unemitted);
	data->sender->unref ();
	delete data;
#if SANITY
	Deployment::SetCurrent (NULL);
#endif
	return false;
}

bool
EventObject::Emit (int event_id, EventArgs *calldata, bool only_unemitted, int starting_generation)
{
	return EmitOnly (event_id, -1, calldata, only_unemitted, starting_generation);
}
bool
EventObject::EmitOnly (int event_id, int token, EventArgs *calldata, bool only_unemitted, int starting_generation)
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
		if (calldata) {
#if DEBUG
			if (!(GetObjectType () == Type::TEXTBOX && (event_id == TextBox::SelectionChangedEvent || event_id == TextBox::TextChangedEvent))
			    && !(GetObjectType () == Type::PASSWORDBOX && event_id == PasswordBox::PasswordChangedEvent))
				printf ("EMIT OF EVENT %s(%d) ON OBJECT %s CALLED WITH NO LISTENERS AND NON-NULL CALLDATA\n", GetType ()->LookupEvent (event_id), event_id, GetTypeName());
#endif
			calldata->unref ();
		}
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
		data->only_token = token;
		data->calldata = calldata;
		data->only_unemitted = only_unemitted;
		surface->GetTimeManager ()->AddTimeout (MOON_PRIORITY_HIGH, 1, EmitCallback, data);
		return false;
	}

	EmitContext* ctx = StartEmit (event_id, token, only_unemitted, starting_generation);
	
	if (ctx == NULL)
		return false;
		
	DoEmit (event_id, calldata);

	FinishEmit (event_id, ctx);

	return true;
}

EmitContext*
EventObject::StartEmit (int event_id, int only_token, bool only_unemitted, int starting_generation)
{
	if (events == NULL)
		return NULL;

	if (GetType()->GetEventCount() <= 0 || event_id >= GetType()->GetEventCount()) {
		g_warning ("trying to start emit with id %d, which has not been registered\n", event_id);
		return NULL;
	}

	// if (events->lists [event_id].event_list->IsEmpty ())
	// 	return NULL;

	EmitContext *ctx = new EmitContext();
	ctx->only_unemitted = only_unemitted;
	ctx->starting_generation = starting_generation;
	EventClosure *closure;

	events->emitting++;

	events->lists [event_id].context_stack->Prepend (new EmitContextNode (ctx));

	ctx->length = events->lists [event_id].event_list->Length();
	ctx->closures = g_new (EventClosure*, ctx->length);

	/* make a copy of the event list to use for emitting */
	closure = (EventClosure *) events->lists [event_id].event_list->First ();
	for (int i = 0; closure != NULL; i ++) {
		if (only_token == -1 || closure->token == only_token)
			ctx->closures[i] = closure->pending_removal ? NULL : closure;
		else
			ctx->closures[i] = NULL;
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
		if (!calldata || closure->handledEventsToo || !calldata->Is (Type::ROUTEDEVENTARGS) || !((RoutedEventArgs *) calldata)->GetHandled ())
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
		EventClosure *closure = ctx->closures[i];

		// closures might be null if they were pending removal on StartEmit
		if (!closure)
			continue;

		if (!closure->handledEventsToo && calldata && calldata->Is (Type::ROUTEDEVENTARGS)) {
			RoutedEventArgs *rea = (RoutedEventArgs*)calldata;
			if (rea->GetHandled ())
				continue;
		}

		if (closure->func
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

	bool is_shutting_down = GetDeployment ()->IsShuttingDown ();
	if (events->lists [event_id].context_stack->IsEmpty ()) {
		// Remove closures which are waiting for removal
		EventClosure *closure = (EventClosure *) events->lists [event_id].event_list->First ();
		while (closure != NULL) {
			EventClosure *next = (EventClosure *) closure->next;
			if (closure->pending_removal) {
				closure->InvokeDataDtor(is_shutting_down);
				events->lists [event_id].event_list->Remove (closure);
			}
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
	virtual ~Listener () {}
};

class WildcardListener : public Listener {
public:
	WildcardListener (DependencyObject *obj, DependencyProperty *prop)
		: obj (NULL, 0, false)
	{
		this->obj = obj;
		this->prop = prop;
	}

	virtual bool Matches (PropertyChangedEventArgs *args) { return true; }
	virtual void Invoke (DependencyObject *sender, PropertyChangedEventArgs *args, MoonError *error)
	{
		// FIXME we ignore error here.
		if (obj)
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

	WeakRef<DependencyObject> obj;
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

	if (v != NULL && v->IsDependencyObject (this_obj->GetDeployment ()) && v->AsDependencyObject(this_obj->GetDeployment()->GetTypes()) != NULL) {
		//printf ("unregistering from property %s\n", prop->name);
		DependencyObject *obj = v->AsDependencyObject (this_obj->GetDeployment()->GetTypes());
		obj->RemovePropertyChangeListener (this_obj);
		obj->SetParent (NULL, NULL);
	}
}

void
DependencyObject::RemoveAllListeners ()
{
	if (providers.autocreate)
		providers.autocreate->ForeachValue (unregister_depobj_values, this);
	
	if (providers.localvalue)
		providers.localvalue->ForeachValue (unregister_depobj_values, this);
}

static bool listeners_notified;

void
DependencyObject::NotifyListenersOfPropertyChange (PropertyChangedEventArgs *args, MoonError *error)
{
	if (hadManagedPeer && !GetManagedHandle ().IsAllocated ()) {
		g_warning ("Calling NotifyListenersOfPropertyChange on an element whose managed peer has died\n");
		return;
	}

	GSList *list;
	GSList *node;
	GSList *n;

	g_return_if_fail (args);

	listeners_notified = true;

	/* The list we want to traverse, listener_list, can change when notifying listeners.
	 * So we clone it, and when traversing it, we check for each node we process that it
	 * still exists in listener_list. This way we will not process deleted nodes, or
	 * nodes added after we've started. I'm not sure this is the correct behavior though. */

	list = g_slist_copy (listener_list);

	node = list;
	while (node != NULL) {
		/* Find the matching node in listener_list */
		n = listener_list;
		while (n != NULL && n->data != node->data)
			n = n->next;

		/* Notify */
		if (n != NULL && n->data == node->data) {
			Listener *listener = (Listener *) node->data;

			if (listener->Matches (args))
				listener->Invoke (this, args, error);

			if (error && error->number)
				break;
		}

		/* Check if all nodes have been deleted */
		if (listener_list == NULL)
			break;

		node = node->next;
	}

	g_slist_free (list);
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

#if EVENT_ARG_REUSE
	PropertyChangedEventArgs *args = GetDeployment()->GetPropertyChangedEventArgs ();

	args->SetProperty (subproperty);
	args->SetId (subproperty->GetId());
	args->SetOldValue (NULL);
	args->SetNewValue (new_value);
#else
	PropertyChangedEventArgs *args = new PropertyChangedEventArgs (subproperty, subproperty->GetId (), NULL, new_value);
#endif
	NotifyListenersOfPropertyChange (args, error);

#if EVENT_ARG_REUSE
	GetDeployment()->ReleasePropertyChangedEventArgs (args);
#else
	args->unref ();
#endif
}

bool
DependencyObject::IsValueValid (DependencyProperty* property, Value* value, MoonError *error)
{
	if (property == NULL) {
		MoonError::FillIn (error, MoonError::ARGUMENT_NULL, 1001,
				   "NULL property passed to IsValueValid");
		return false;
	}

	if (value != NULL && !value->GetIsNull ()) {
		if (value->IsEventObject (GetDeployment ()) && !value->AsEventObject ()) {
			// if it's a null DependencyObject, it doesn't matter what type it is
			return true;
		}
		
		if (value->GetIsManaged ()) {
			// This is a big hack, we do no type-checking if we try to set a managed type.
			// Given that for the moment we might not have the surface available, we can't
			// do any type checks since we can't access types registered on the surface.
			return true;
		}
		
		if (property->GetPropertyType() != value->GetKind() && !Type::IsAssignableFrom (GetDeployment (), property->GetPropertyType(), value->GetKind())) {
			char *error_msg = g_strdup_printf ("DependencyObject::SetValue, value cannot be assigned to the "
							    "property %s::%s (property has type '%s', value has type '%s')",
							    GetTypeName (), property->GetName(), Type::Find (GetDeployment (), property->GetPropertyType())->GetName (),
							   Type::Find (GetDeployment (), value->GetKind ())->GetName ());
			printf (error_msg);
			MoonError::FillIn (error, MoonError::ARGUMENT, 1001, error_msg);
			g_free (error_msg);
			return false;
		}
	} else {
		if (!property->CanBeSetToNull ()) {
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
	
	Value *current_value;
	bool equal = false;
	
	if (!(current_value = ReadLocalValue (property)))
		if (property->IsAutoCreated ())
			current_value = providers.autocreate->ReadLocalValue (property);
	
	if (current_value != NULL && value != NULL) {
		equal = !property->AlwaysChange() && (*current_value == *value);
	} else {
		equal = (current_value == NULL) && (value == NULL);
	}

	if (!equal) {
		Deployment *deployment = GetDeployment ();

		Value *new_value;

		if (current_value)
			current_value = new Value(*current_value);
		
		// remove the old value
		providers.localvalue->ClearValue (property);
		if (property->IsAutoCreated ())
			providers.autocreate->ClearValue (property);
		
		if (value && (!property->IsAutoCreated () || !value->IsDependencyObject (deployment) || value->AsDependencyObject () != NULL))
			new_value = new Value (*value);
		else
			new_value = NULL;

		// clear out the current value from the managed side if there's a ref to it
		if (current_value) {
			if (clearManagedRef && current_value->HoldManagedRef (deployment) && !deployment->IsShuttingDown ()) {
				current_value->Strengthen (deployment);
				clearManagedRef (this, current_value, property);
			}
		}

		// replace it with the new value
		if (new_value) {
			if (addManagedRef && new_value->HoldManagedRef (deployment) && !deployment->IsShuttingDown ()) {
				addManagedRef (this, new_value, property);
				new_value->Weaken (deployment);
			}

			providers.localvalue->SetValue (property, new_value);
		}
		ProviderValueChanged (PropertyPrecedence_LocalValue, property, current_value, new_value, true, true, true, error);
		
		delete current_value;
	}

	return true;
}

bool
DependencyObject::SetValueWithError (DependencyProperty *property, Value *value, MoonError *error)
{
	bool ret;
	Value *coerced = value;
	bool hasCoercer = property->HasCoercer ();

	if ((hasCoercer && !property->Coerce (this, value, &coerced, error)) ||
		!IsValueValid (property, coerced, error) ||
		!property->Validate (this, coerced, error)) {
		ret = false;
	} else {
		ret = SetValueWithErrorImpl (property, coerced, error);
	}

	if (hasCoercer)
		delete coerced;
	return ret;
}

// sets the inherited property source on this object to @source.
// returns true if this->GetValue(inheritedProperty) is equal to the inherited value (i.e. false if the value is of a higher precedence)
bool
DependencyObject::PropagateInheritedValue (InheritedPropertyValueProvider::Inheritable inheritableProperty,
					   DependencyObject *source, Value *new_value)
{
	if (!providers.inherited) {
		// we don't have an inherited provider at all, so the
		// value must be coming from some other provider
		return true;
	}

	providers.inherited->SetPropertySource (inheritableProperty, source);

	int propertyId = InheritedPropertyValueProvider::InheritablePropertyToPropertyId (GetDeployment()->GetTypes(),
											  inheritableProperty,
											  GetObjectType());

	if (propertyId == -1) {
		// there can be no local value, so it has to be inherited
		return false;
	}

	DependencyProperty *property = GetDeployment()->GetTypes()->GetProperty (propertyId);
	MoonError unused;
	ProviderValueChanged (PropertyPrecedence_Inherited, property, NULL, new_value, true, false, false, &unused);

	return GetPropertyValueProvider (property) == PropertyPrecedence_Inherited;
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

	if (v != NULL && v->IsDependencyObject (closure->to_ns->GetDeployment ()) && v->AsDependencyObject() != NULL) {
		DependencyObject *obj = v->AsDependencyObject ();
		obj->RegisterAllNamesRootedAt (closure->to_ns, closure->error);
	}
}

void
DependencyObject::RegisterAllNamesRootedAt (NameScope *to_ns, MoonError *error)
{
	if (error->number || registering_names)
		return;

	registering_names = true;
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

		providers.autocreate->ForeachValue (register_depobj_names, &closure);
		providers.localvalue->ForeachValue (register_depobj_names, &closure);
	}

	registering_names = false;
}

static void
unregister_depobj_names (gpointer  key,
			 gpointer  value,
			 gpointer  user_data)
{
	NameScope *from_ns = (NameScope*)user_data;
	Value *v = (Value*)value;
	DependencyProperty *property = (DependencyProperty*)key;

	if (!property->IsCustom () && v != NULL && v->IsDependencyObject (from_ns->GetDeployment ()) && v->AsDependencyObject() != NULL) {
		DependencyObject *obj = v->AsDependencyObject ();
		obj->UnregisterAllNamesRootedAt (from_ns);
	}
}

void
DependencyObject::UnregisterAllNamesRootedAt (NameScope *from_ns)
{
	if (registering_names)
		return;
	registering_names = true;

	NameScope *this_ns = NameScope::GetNameScope(this);
	if (IsHydratedFromXaml () || this_ns == NULL || this_ns->GetTemporary ()) {
		// Unregister in the parent scope
	
		const char *n = GetName();

		if (n && strlen (n) > 0)
			from_ns->UnregisterName (n);
	}

	if (this_ns && !this_ns->GetTemporary()) {
		registering_names = false;
		return;
	}
	
	providers.autocreate->ForeachValue (unregister_depobj_names, from_ns);
	providers.localvalue->ForeachValue (unregister_depobj_names, from_ns);

	registering_names = false;
}

bool
DependencyObject::SetNameOnScope (const char* name, NameScope *scope)
{
	DependencyProperty *property = GetDeployment ()->GetTypes ()->GetProperty (NameProperty);

	if (scope->FindName (name))
		return false;

	SetValue (property, Value (name));
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
	return providers.localvalue->GetPropertyValue (property);
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
DependencyObject::GetValueWithError (DependencyProperty *property, MoonError *error)
{
	if (!HasProperty (GetObjectType (), property, true)) {
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
	return GetValue (GetDeployment ()->GetTypes ()->GetProperty (id), PropertyPrecedence_Highest, PropertyPrecedence_Lowest);
}

Value *
DependencyObject::GetValue (int id, PropertyPrecedence startingAtPrecedence)
{
	if (IsDisposed ())
		return NULL;
	return GetValue (GetDeployment ()->GetTypes ()->GetProperty (id), startingAtPrecedence, PropertyPrecedence_Lowest);
}

Value *
DependencyObject::GetValue (int id, PropertyPrecedence startingAtPrecedence, PropertyPrecedence endingAtPrecedence)
{
	if (IsDisposed ())
		return NULL;
	return GetValue (GetDeployment ()->GetTypes ()->GetProperty (id), startingAtPrecedence, endingAtPrecedence);
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
DependencyObject::GetValueNoAutoCreate (int id)
{
	if (IsDisposed ())
		return NULL;
	return GetValueNoAutoCreate (GetDeployment ()->GetTypes ()->GetProperty (id));
}

Value *
DependencyObject::GetValueNoAutoCreate (DependencyProperty *property)
{
	Value *v = GetValue (property, PropertyPrecedence_LocalValue, PropertyPrecedence_InheritedDataContext);
	if (v == NULL && property->IsAutoCreated())
		v = providers.autocreate->ReadLocalValue (property);

	return v;
}

Value *
DependencyObject::GetAnimationBaseValueWithError (int id, MoonError *error)
{
	if (IsDisposed ())
		return NULL;
	return GetAnimationBaseValueWithError (GetDeployment ()->GetTypes ()->GetProperty (id), error);
}

Value *
DependencyObject::GetAnimationBaseValueWithError (DependencyProperty *property, MoonError *error)
{
	AnimationStorage *storage = GetAnimationStorageFor (property);
	return storage ? storage->GetStopValue () : NULL;
}

bool
DependencyObject::PropertyHasValueNoAutoCreate (int property_id, DependencyObject *obj)
{
	Value *v = GetValueNoAutoCreate (property_id);
	return v ? v->AsDependencyObject() == obj : NULL == obj;
}

Value *
DependencyObject::GetValue (DependencyProperty *property, PropertyPrecedence startingAtPrecedence, PropertyPrecedence endingAtPrecedence)
{
#if PROPERTY_LOOKUP_DIAGNOSTICS
	g_hash_table_insert (get_values_per_property, property, GINT_TO_POINTER (GPOINTER_TO_INT (g_hash_table_lookup (get_values_per_property, property)) + 1));

	int lookups = GPOINTER_TO_INT (g_hash_table_lookup (hash_lookups_per_property, property));
#endif

	int provider_bitmask = GPOINTER_TO_INT (g_hash_table_lookup (provider_bitmasks, property));
	// providers we *always* consult
	provider_bitmask |= ((1 << PropertyPrecedence_Inherited) |
			     (1 << PropertyPrecedence_DynamicValue) |
			     (1 << PropertyPrecedence_AutoCreate));
#if PROPERTY_LOOKUP_DIAGNOSTICS
	lookups ++; // for the provider bitmask
#endif

	PropertyValueProvider **provider_array = (PropertyValueProvider**)&providers;

	for (int i = startingAtPrecedence; i <= endingAtPrecedence; i ++) {
// #if USE_PROVIDER_BITMASK
		if (!(provider_bitmask & (1 << i)))
			continue;
// #endif
		if (!provider_array[i])
			continue;
#if PROPERTY_LOOKUP_DIAGNOSTICS
		lookups ++;
		provider_property_lookups ++;
#endif
		Value *value = provider_array[i]->GetPropertyValue (property);
		if (value) {
#if PROPERTY_LOOKUP_DIAGNOSTICS
			g_hash_table_insert (hash_lookups_per_property, property, GINT_TO_POINTER (lookups));
#endif

			return value;
		}
	}

#if PROPERTY_LOOKUP_DIAGNOSTICS
	g_hash_table_insert (hash_lookups_per_property, property, GINT_TO_POINTER (lookups));
#endif
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

	PropertyValueProvider **provider_array = (PropertyValueProvider**)&providers;
	for (int i = 0; i < PropertyPrecedence_AutoCreate; i ++) {
		if (!provider_array[i])
			continue;
		value = provider_array[i]->GetPropertyValue (property);
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
DependencyObject::CallRecomputePropertyValueForProviders (DependencyProperty *property, int providerPrecedence, MoonError *error)
{
	PropertyValueProvider **provider_array = (PropertyValueProvider**)&providers;

	for (int i = 0; i < PropertyPrecedence_Count; i ++) {
		if (!provider_array[i])
			continue;

		if (i == providerPrecedence)
			continue;
		else if (i < providerPrecedence && (provider_array[i]->GetFlags () & ProviderFlags_RecomputesOnLowerPriorityChange) != 0)
			provider_array[i]->RecomputePropertyValue (property, ProviderFlags_RecomputesOnLowerPriorityChange, error);
		else if (i > providerPrecedence && (provider_array[i]->GetFlags () & ProviderFlags_RecomputesOnHigherPriorityChange) != 0)
			provider_array[i]->RecomputePropertyValue (property, ProviderFlags_RecomputesOnHigherPriorityChange, error);
	}
}

void
DependencyObject::ProviderValueChanged (PropertyPrecedence providerPrecedence,
					DependencyProperty *property,
					Value *old_provider_value, Value *new_provider_value,
					bool notify_listeners, bool set_parent, bool merge_names_on_set_parent,
					MoonError *error)
{
	int p;
	int provider_bitmask = GPOINTER_TO_INT (g_hash_table_lookup (provider_bitmasks, property));

	if (new_provider_value)
		provider_bitmask |= (1 << providerPrecedence);
	else
		provider_bitmask &= ~(1 << providerPrecedence);

	g_hash_table_insert (provider_bitmasks, property, GINT_TO_POINTER (provider_bitmask));

	int higher = 0;

	// first we look for a value higher in precedence for this property
	for (p = providerPrecedence - 1; p >= PropertyPrecedence_LocalValue; p --) {
		higher |= 1<<p;
	}

	higher &= provider_bitmask;

	higher |= ((1 << PropertyPrecedence_Inherited) |
		   (1 << PropertyPrecedence_DynamicValue) |
		   (1 << PropertyPrecedence_AutoCreate));

	PropertyValueProvider **provider_array = (PropertyValueProvider**)&providers;

	for (p = providerPrecedence - 1; p >= PropertyPrecedence_Highest; p --) {
// #if USE_PROVIDER_BITMASK
		if (!(higher & (1 << p)))
			continue;
// #endif

		if (!provider_array[p])
			continue;

#if PROPERTY_LOOKUP_DIAGNOSTICS
		provider_property_lookups ++;
#endif

		if (provider_array[p]->GetPropertyValue (property)) {
			// a provider higher in precedence already has
			// a value for this property, so the one
			// that's changing isn't visible, so we don't
			// do all the property change notification
			// below.

			// we *do* need to call RecomputePropertyValue
			// on providers, though.
			CallRecomputePropertyValueForProviders (property, providerPrecedence, error);

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
		if (providerPrecedence != PropertyPrecedence_IsEnabled && providers.isenabled && providers.isenabled->LocalValueChanged (property))
			return;

		// we need to let the providers above/below this one that things have changed.
		CallRecomputePropertyValueForProviders (property, providerPrecedence, error);

		DependencyObject *old_as_dep = NULL;
		DependencyObject *new_as_dep = NULL;

		// XXX this flag should be part of the DP metadata.
		// we also need to audit other "typeof (object)" DP's
		// to make sure they set parent when they should (and
		// don't when they shouldn't.)
		bool setsParent = set_parent && !property->IsCustom ();

		if (old_value && old_value->IsDependencyObject (GetDeployment ()))
			old_as_dep = old_value->AsDependencyObject ();
		if (new_value && new_value->IsDependencyObject (GetDeployment ()))
			new_as_dep = new_value->AsDependencyObject ();

		if (old_as_dep) {
			old_as_dep->SetMentor (NULL);
		}

		if (old_as_dep && setsParent) {
			old_as_dep->SetIsAttached (false);
			
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
			new_as_dep->SetIsAttached (IsAttached ());

			new_as_dep->SetParent (this, merge_names_on_set_parent, error);
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

		if (new_as_dep) {
			DependencyObject *e = this;
			while (e && !e->Is (Type::FRAMEWORKELEMENT))
				e = e->mentor;
			new_as_dep->SetMentor (e);
		}

		// we need to make this optional, as doing it for NameScope
		// merging is killing performance (and noone should ever care
		// about that property changing)
		if (notify_listeners && !GetDeployment()->IsShuttingDown()) {
			Value old_value_copy, new_value_copy;
			Value *old_value_ptr = NULL, *new_value_ptr = NULL;

			if (old_value != NULL) {
				old_value_copy = Value (*old_value);
				old_value_ptr = &old_value_copy;
			}
			if (new_value != NULL) {
				new_value_copy = Value (*new_value);
				new_value_ptr = &new_value_copy;
			}

#if EVENT_ARG_REUSE
			PropertyChangedEventArgs* args = GetDeployment()->GetPropertyChangedEventArgs ();

			args->SetProperty (property);
			args->SetId (property->GetId());
			args->SetOldValue (old_value_ptr);
			args->SetNewValue (new_value_ptr);
#else
			PropertyChangedEventArgs *args = new PropertyChangedEventArgs (property, property->GetId (), old_value_ptr, new_value_ptr);
#endif

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

#if EVENT_ARG_REUSE
			GetDeployment()->ReleasePropertyChangedEventArgs (args);
#else
			args->unref ();
#endif

			if (providers.inherited) {
				if (providerPrecedence == PropertyPrecedence_Inherited) {
					// we don't do anything here, it's handled in provider.cpp
				}
				else {
					if (InheritedPropertyValueProvider::IsPropertyInherited (this, property->GetId ())
					    && GetPropertyValueProvider (property) < PropertyPrecedence_Inherited)
						providers.inherited->PropagateInheritedProperty (property, this, this);
				}
			}
		}

		// if we have an active animation on this property,
		// make sure we tell the time manager we need another
		// clock tick
		if ((moonlight_flags & RUNTIME_INIT_USE_IDLE_HINT) && GetAnimationStorageFor (property) != NULL && IsAttached()) {
			GetDeployment()->GetSurface()->GetTimeManager()->NeedClockTick();
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
	MoonError error;
	ClearValue(property, notify_listeners, &error);
}

void
DependencyObject::ClearValue (DependencyProperty *property, bool notify_listeners, MoonError *error)
{
	Value *old_local_value;

	if (GetAnimationStorageFor (property) != NULL) {
		// DRT #508 has a test for clearing an animating
		// property which seems to indicate that ClearValue
		// doesn't work on them.  we have other tests that
		// show that there isn't another precedent for them
		// (and that promoted values become local values), so
		// the only explanation, as gross as it is, is that
		// ClearValue short-circuits out if we're animating
		// the property.
		return;
	}
	
	if (!(old_local_value = ReadLocalValue (property)))
		if (property->IsAutoCreated ())
			old_local_value = providers.autocreate->ReadLocalValue (property);
	
	if (old_local_value) {
		// detach from the existing value
		if (old_local_value->IsDependencyObject (GetDeployment ())) {
			DependencyObject *dob = old_local_value->AsDependencyObject();
			
			// Custom properties are non-parenting and so shouldn't clear the parent (same code
			// as found in the SetValue path)
			if (dob != NULL && !property->IsCustom ()) {
				// unset its parent
				dob->SetParent (NULL, NULL);
				
				// unregister from the existing value
				dob->RemovePropertyChangeListener (this, property);
				dob->SetIsAttached (false);
				if (dob->Is(Type::COLLECTION)) {
					dob->RemoveHandler (Collection::ChangedEvent, collection_changed, this);
					dob->RemoveHandler (Collection::ItemChangedEvent, collection_item_changed, this);
				}
			}
		}
		
		old_local_value = new Value (*old_local_value);
		
		providers.localvalue->ClearValue (property);
		if (property->IsAutoCreated ())
			providers.autocreate->ClearValue (property);
	}
	
	PropertyValueProvider **provider_array = (PropertyValueProvider**)&providers;
	for (int p = PropertyPrecedence_LocalValue + 1; p < PropertyPrecedence_Count; p ++) {
		if (provider_array[p] && (provider_array[p]->GetFlags () & ProviderFlags_RecomputesOnClear) != 0)
			provider_array[p]->RecomputePropertyValue (property, ProviderFlags_RecomputesOnClear, error);
	}
	
	if (old_local_value) {
		ProviderValueChanged (PropertyPrecedence_LocalValue, property, old_local_value, NULL, notify_listeners, true, false, error);
		delete old_local_value;
	}
}

gboolean
DependencyObject::dispose_value (gpointer key, gpointer value, gpointer data)
{
	DependencyObject *_this = (DependencyObject*)data;

	Value *v = (Value *) value;
	
	if (Value::IsNull (v))
		return TRUE;

	Deployment *deployment = _this->GetDeployment();

	Types *types = deployment->GetTypes();
	// detach from the existing value
	if (v->IsDependencyObject (deployment)){
		DependencyObject *dob = v->AsDependencyObject(types);
		
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

#if EVENT_ARG_REUSE
	PropertyChangedEventArgs* propChangedArgs = obj->GetDeployment()->GetPropertyChangedEventArgs ();

	propChangedArgs->SetProperty (itemArgs->GetProperty());
	propChangedArgs->SetId (itemArgs->GetProperty()->GetId());
	propChangedArgs->SetOldValue (itemArgs->GetOldValue());
	propChangedArgs->SetNewValue (itemArgs->GetNewValue());
#else
	PropertyChangedEventArgs *propChangedArgs = new PropertyChangedEventArgs (itemArgs->GetProperty(),
										  itemArgs->GetProperty()->GetId (),
										  itemArgs->GetOldValue(),
										  itemArgs->GetNewValue());
#endif
	obj->OnCollectionItemChanged ((Collection*)sender,
				      itemArgs->GetCollectionItem(),
				      propChangedArgs);

#if EVENT_ARG_REUSE
	obj->GetDeployment()->ReleasePropertyChangedEventArgs (propChangedArgs);
#else
	propChangedArgs->unref ();
#endif
}

DependencyObject::DependencyObject ()
	: EventObject (Type::DEPENDENCY_OBJECT),
	mentor (this, MentorWeakRef, false), parent (this, ParentWeakRef, false), template_owner (this, TemplateOwnerWeakRef, false), secondary_parent (this, SecondaryParentWeakRef, false)
{
	Initialize ();
}

DependencyObject::DependencyObject (Deployment *deployment, Type::Kind object_type)
	: EventObject (deployment, object_type),
	mentor (this, MentorWeakRef, false), parent (this, ParentWeakRef, false), template_owner (this, TemplateOwnerWeakRef, false), secondary_parent (this, SecondaryParentWeakRef, false)
{
	Initialize ();
}

DependencyObject::DependencyObject (Type::Kind object_type)
	: EventObject (object_type),
	mentor (this, MentorWeakRef, false), parent (this, ParentWeakRef, false), template_owner (this, TemplateOwnerWeakRef, false), secondary_parent (this, SecondaryParentWeakRef, false)
{
	Initialize ();
}

void
DependencyObject::Initialize ()
{
	// clear out our provider vtable
	memset (&providers, 0, sizeof (providers));

	// and install the ones all DO's have
	providers.localvalue = new LocalPropertyValueProvider (this, PropertyPrecedence_LocalValue, dispose_value);
	providers.autocreate = new AutoCreatePropertyValueProvider (this, PropertyPrecedence_AutoCreate, dispose_value);
	
	listener_list = NULL;
	is_hydrated = false;
	is_frozen = false;
	is_being_parsed = false;
	resource_base = NULL;
	registering_names = false;
#if PROPERTY_LOOKUP_DIAGNOSTICS
	hash_lookups_per_property = g_hash_table_new (g_direct_hash, g_direct_equal);
	get_values_per_property = g_hash_table_new (g_direct_hash, g_direct_equal);
#endif
	provider_bitmasks = g_hash_table_new (g_direct_hash, g_direct_equal);
	storage_hash = NULL; // Create it on first usage request
}

void
DependencyObject::SetMentor (DependencyObject *value)
{
	if (this->mentor == value)
		// easy, nothing to change
		return;

	if (GetDeployment ()->IsShuttingDown ()) {
		// for sanity's sake, we should verify that value is
		// NULL, but we don't care, we're going away anyway.
		mentor = NULL;
		return;
	}

#if DEBUG_WEAKREF
	printf ("Setting mentor for %p/%s to ", this, this->GetTypeName());

	if (!value)
		printf ("NULL\n");
	else
		printf ("%p/%s\n", value, value->GetTypeName());
#endif

	DependencyObject *old_mentor = this->mentor;

	mentor = value;

	OnMentorChanged (old_mentor, value);
}

void
DependencyObject::OnMentorChanged (DependencyObject *old_mentor, DependencyObject *new_mentor)
{
	// If this object is a framework element, it will already be the mentor for all its child values.
	// If it is *not* a FrameworkElement then 'new_mentor' will be the mentor for all the child
	// values of this object so we should propagate it here
	if (!this->Is (Type::FRAMEWORKELEMENT)) {
		providers.autocreate->ForeachValue ((GHFunc)DependencyObject::propagate_mentor, new_mentor);
		providers.localvalue->ForeachValue ((GHFunc)DependencyObject::propagate_mentor, new_mentor);
		if (providers.localstyle)
			providers.localstyle->ForeachValue ((GHFunc)DependencyObject::propagate_mentor, new_mentor);
		if (providers.defaultstyle)
			providers.defaultstyle->ForeachValue ((GHFunc)DependencyObject::propagate_mentor, new_mentor);
	}

	if (mentorChanged && !GetDeployment ()->IsShuttingDown ()) {
		/* We have to check if we're shutting down, since mentorChanged is a managed callback */
		mentorChanged (this, new_mentor);
	}
}


void
DependencyObject::propagate_mentor (DependencyProperty *key, Value *value, gpointer data)
{
	Deployment *d = Deployment::GetCurrent ();
	DependencyObject *mentor = (DependencyObject *) data;
	if (value->IsDependencyObject (d)) {
		DependencyObject *v = value->AsDependencyObject ();
		if (v)
			v->SetMentor (mentor);
	}
}

void
DependencyObject::SetTemplateOwner (DependencyObject *value)
{
	// FIXME -- you should only set template owner once.
	//g_return_if_fail (template_owner == NULL);
	this->template_owner = value;
}

DependencyObject *
DependencyObject::GetMentor ()
{
	return mentor;
}

DependencyObject *
DependencyObject::GetTemplateOwner ()
{
	return template_owner;
}

void
DependencyObject::SetResourceBase (const Uri *resourceBase)
{
	delete resource_base;
	resource_base = Uri::Clone (resourceBase);
}

const Uri *
DependencyObject::GetResourceBase ()
{
	return GetResourceBaseRecursive ();
}

const Uri *
DependencyObject::GetResourceBaseRecursive ()
{
	if (!Uri::IsNullOrEmpty(resource_base))
		return resource_base;
	if (parent != NULL)
		return parent->GetResourceBaseRecursive ();
	return resource_base;
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
	CloneClosure *closure = (CloneClosure*)data;

	Deployment *deployment = closure->old_do->GetDeployment();

	Value *old_value = closure->old_do->GetValue (key, PropertyPrecedence_AutoCreate);

	// this should create the new object
	Value *new_value = closure->new_do->GetValue (key, PropertyPrecedence_AutoCreate);

	if (old_value && !old_value->GetIsNull() && old_value->IsDependencyObject (deployment) &&
	    new_value && !new_value->GetIsNull() && new_value->IsDependencyObject (deployment)) {
		DependencyObject *new_obj = new_value->AsDependencyObject(closure->types);
		DependencyObject *old_obj = old_value->AsDependencyObject(closure->types);

		if (new_obj && old_obj)
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

	fromObj->providers.autocreate->ForeachValue ((GHFunc)DependencyObject::clone_autocreated_value, &closure);
	fromObj->providers.localvalue->ForeachValue ((GHFunc)DependencyObject::clone_local_value, &closure);
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

#if PROPERTY_LOOKUP_DIAGNOSTICS
static void
output_hash_lookups_per_property (DependencyProperty *key, int count, totals *ts)
{
	int get_values = GPOINTER_TO_INT (g_hash_table_lookup (ts->get_values, key));
	printf (" %s: %d (%d GetValue calls, %g hash lookups/GetValue)\n",
		key->GetName(),
		count, get_values,
		(double)count / (double)get_values);

	ts->properties ++;
	ts->lookups += count;
}
#endif

DependencyObject::~DependencyObject ()
{
	g_hash_table_destroy (provider_bitmasks);
	delete resource_base;

#if PROPERTY_LOOKUP_DIAGNOSTICS
	totals ts;
	ts.get_values = get_values_per_property;
	ts.properties = ts.lookups = 0;

	printf ("for object of type %s, hash lookups per property...\n", GetTypeName());
	g_hash_table_foreach (hash_lookups_per_property, (GHFunc)output_hash_lookups_per_property, &ts);
	if (ts.properties > 0)
		printf ("  average %g lookups per property\n", (double)ts.lookups / (double)ts.properties);

	g_hash_table_destroy (hash_lookups_per_property);
	g_hash_table_destroy (get_values_per_property);
#endif
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
#if SANITY
	/* Assert that if we still have a parent, it must be alive */
	g_assert (GetDeployment()->IsShuttingDown () || parent == NULL || parent->GetRefCount () >= 0); /* #if SANITY */
#endif
	if (parent != NULL) {
		SetParent (NULL, NULL);
	}

	if (listener_list != NULL) {
		g_slist_foreach (listener_list, free_listener, NULL);
		g_slist_free (listener_list);
		listener_list = NULL;
	}

	RemoveAllListeners();

	PropertyValueProvider **provider_array = (PropertyValueProvider**)&providers;
	for (int i = 0; i < PropertyPrecedence_Count; i ++) {
		delete provider_array[i];
		provider_array [i] = NULL;
	}
	
	if (storage_hash) {
		GHashTable *tmphash = storage_hash; // animation storages may call back to DetachAnimationStorage
		storage_hash = NULL;
		g_hash_table_foreach (tmphash, (GHFunc)clear_storage_list, NULL);
		g_hash_table_destroy (tmphash);
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
	DependencyProperty **props;
	GHashTable *table;
	GPtrArray *array;
	
	array = g_ptr_array_new ();
	
	if (!only_changed) {
		// get our class/inherited DependencyProperties
		table = GetType ()->CopyProperties (true);
		
		// find any attached properties that have been set

		providers.localvalue->ForeachValue (get_attached_props, table);
		
		// dump them to an array
		g_hash_table_foreach (table, hash_values_to_array, array);
		g_hash_table_destroy (table);
	} else {
		providers.localvalue->ForeachValue (hash_keys_to_array, array);
		providers.autocreate->ForeachValue (hash_keys_to_array, array);
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

int
DependencyObject::GetPropertyValueProvider (DependencyProperty *property)
{
	int provider_bitmask = GPOINTER_TO_INT (g_hash_table_lookup (provider_bitmasks, property));
	for (int i = 0; i < PropertyPrecedence_Lowest; i ++) {
		int p = 1 << i;
		if ((provider_bitmask & p) == p)
			return i;
		if (i == PropertyPrecedence_AutoCreate && (property->GetAutoCreator() || property->GetDefaultValue (GetObjectType())))
			return i;
	}

	return -1;
}

DependencyObject*
DependencyObject::GetInheritedValueSource (InheritedPropertyValueProvider::Inheritable inheritableProperty)
{
	if (!providers.inherited)
		return NULL;
	return providers.inherited->GetPropertySource (inheritableProperty);
}

void
DependencyObject::SetInheritedValueSource (InheritedPropertyValueProvider::Inheritable inheritableProperty,
					   DependencyObject *source)
{
	if (!providers.inherited)
		return;
	// if source is null, we need to remove the precedence from the provider_bitmask for this property
	if (source == NULL) {
		Types *types = GetDeployment()->GetTypes();
		int propertyId = InheritedPropertyValueProvider::InheritablePropertyToPropertyId (types,
												  inheritableProperty,
												  GetObjectType());

		if (propertyId == -1) {
			return;
		}

		DependencyProperty *property = types->GetProperty (propertyId);

		int provider_bitmask = GPOINTER_TO_INT (g_hash_table_lookup (provider_bitmasks, property));
		provider_bitmask &= ~(1 << PropertyPrecedence_Inherited);
		g_hash_table_insert (provider_bitmasks, property, GINT_TO_POINTER (provider_bitmask));
	}
	providers.inherited->SetPropertySource (inheritableProperty, source);
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

	if (parent) {
#if SANITY
		g_assert (parent->GetRefCount () >= 0); /* #if SANITY */
#endif
		return parent->FindNameScope (template_namescope);
	}

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
	if (!list || list->IsEmpty ())
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
				((AnimationStorage::Node*)node)->storage->SetStopValue (storage->GetStopValue ());
				list->Remove (remove);
				break;
			}
			node = node->next;
		}
	}
}

struct attach_data {
	Deployment *deployment;
	bool value;
};

static void
set_is_attached (gpointer key, gpointer value, gpointer data)
{
	if (((DependencyProperty *)key)->IsCustom ())
		return;

	attach_data *adata = (attach_data *) data;
	Value *v = (Value *) value;
	
	if (v && v->IsDependencyObject (adata->deployment)) {
		DependencyObject *dob = v->AsDependencyObject();
		if (dob)
			dob->SetIsAttached (adata->value);
	}
}

void
DependencyObject::OnIsAttachedChanged (bool value)
{
	EventObject::OnIsAttachedChanged (value);

	attach_data data;

	data.deployment = GetDeployment ();
	data.value = value;

	providers.localvalue->ForeachValue (set_is_attached, &data);
	providers.autocreate->ForeachValue (set_is_attached, &data);
}

void
DependencyObject::SetParent (DependencyObject *parent, MoonError *error)
{
	SetParent (parent, true, error);
}

void
DependencyObject::SetParent (DependencyObject *parent, bool merge_names_from_subtree, MoonError *error)
{
	if (parent == this->parent)
		return;

	if (GetDeployment()->IsShuttingDown ()) {
		// not much we can do here... @parent should always be
		// null, but we don't care
		this->parent = NULL;
		return;
	}
	
	// Check for circular families
	DependencyObject *current = parent;
	while (current != NULL) {
		if (current == this) {
			g_warning ("cycle found in logical tree.  bailing out");
			return;
		}
		current = current->GetParent ();
	}

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

				if (parent_scope && merge_names_from_subtree) {
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

	if (!error || error->number == 0) {
		this->parent = parent;
	}
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

};
