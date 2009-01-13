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
#include "clock.h"
#include "runtime.h"
#include "uielement.h"
#include "animation.h"

#include "text.h"

struct EventList {
	int current_token;
	int emitting;
	int event_count;
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
 *
 */

EventObject::EventObject ()
{
	surface = NULL;
	refcount = 1;
	events = NULL;
	
#if OBJECT_TRACKING
	id = g_atomic_int_exchange_and_add (&objects_created, 1);
	if (objects_alive == NULL)
		objects_alive = g_hash_table_new (g_direct_hash, g_direct_equal);
	// Helgrind correctly reports a race here. Given this is for debugging, ignore it.
	g_hash_table_insert (objects_alive, this, GINT_TO_POINTER (1));
	weak_refs = NULL;

	Track ("Created", "");
#elif DEBUG
	g_atomic_int_inc (&objects_created);
#endif
}

EventObject::~EventObject()
{
#if OBJECT_TRACKING
	g_atomic_int_inc (&objects_destroyed);
	g_hash_table_remove (objects_alive, this);
	if (weak_refs) {
		int length = g_hash_table_size (weak_refs);
		if (length > 0) {
			printf ("Destroying id=%i with %i weak refs.\n", id, length);
			GList* list = g_hash_table_get_values (weak_refs);
			GList* first = list;
			while (list != NULL) {
				EventObject* eo = (EventObject*) list->data;
				printf ("\t%s %i.\n", eo->GetTypeName (), eo->id);
				list = list->next;
			}
			g_list_free (first);
		}
		g_hash_table_destroy (weak_refs);
		weak_refs = NULL;
	}

	if (refcount != 0) {
		printf ("Object #%i was deleted before its refcount reached 0 (current refcount: %i)\n", id, refcount);
		//print_stack_trace ();
	}

	Track ("Destroyed", "");
#elif DEBUG
	g_atomic_int_inc (&objects_destroyed);
#endif

	delete events;
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
	if (!Surface::InMainThread ()) {
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

void
EventObject::Dispose ()
{
	SetSurface (NULL);
}

void 
EventObject::unref ()
{
	bool delete_me;

	if (!Surface::InMainThread ()) {
		unref_delayed ();
		return;
	}

	if (refcount == 1 && events != NULL && events->emitting) {
		unref_delayed ();
		return;
	}

	if (refcount == 1)
		Emit (DestroyedEvent);

	delete_me = g_atomic_int_dec_and_test (&refcount);

	OBJECT_TRACK ("Unref", GetTypeName ());

	if (delete_me)
		Dispose ();
	
	// We need to check again the the refcount really is zero,
	// the object might have resurrected in the Dispose. Should
	// never happen, but checking avoids a crash, so...
	if (refcount == 0)
		delete this;
	
}

#if DEBUG
gint EventObject::objects_created = 0;
gint EventObject::objects_destroyed = 0;
#endif

#if OBJECT_TRACKING
// Define the ID of the object you want to track
// Object creation, destruction and all ref/unrefs
// are logged to the console, with a stacktrace.
static bool object_id_fetched = false;
static int object_id = -1;
static const char *object_type = NULL;

#define OBJECT_TRACK_ID (0)

GHashTable* EventObject::objects_alive = NULL;

void
EventObject::Track (const char* done, const char* typname)
{
	if (!object_id_fetched) {
		object_id_fetched = true;
		char *sval = getenv ("MOONLIGHT_OBJECT_TRACK_ID");
		if (sval)
			object_id = atoi (sval);
		object_type = getenv ("MOONLIGHT_OBJECT_TRACK_TYPE");
	}
	if (id == object_id || (object_type != NULL && typname != NULL && strcmp (typname, object_type) == 0)) {
		char *st = get_stack_trace ();
		printf ("%s tracked object of type '%s': %i, current refcount: %i\n%s", done, typname, id, refcount, st);
		g_free (st);
	}
}

void
EventObject::weak_ref (EventObject* base)
{
	if (weak_refs == NULL)
		weak_refs = g_hash_table_new (g_direct_hash, g_direct_equal);
	g_hash_table_insert (weak_refs, base, base);
}

void
EventObject::weak_unref (EventObject* base)
{
	// if you get a glib assertion here, you're most likely accessing an already 
	// destroyed object (or you have mismatched ref/unrefs).
	g_hash_table_remove (weak_refs, base);
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

bool
EventObject::Emit (int event_id, EventArgs *calldata, bool only_unemitted)
{
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

	if (events == NULL || events->lists [event_id].event_list->IsEmpty ()) {
		goto delete_ctx;
	}

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
	// make sure to pass false here, as the lists are shallow copied and the nodes are shared
	delete ctx;
}

static pthread_mutex_t delayed_unref_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool drain_tick_call_added = false;
static GSList *pending_unrefs = NULL;

static gboolean
drain_unrefs_idle_call (gpointer data)
{
	EventObject::DrainUnrefs ();
	return false;
}

static void
unref_object (EventObject *obj, gpointer unused)
{
	obj->unref ();
}

void
EventObject::DrainUnrefs ()
{
	GSList *list;

	// TODO: make pending_unrefs a lock free structure (or just make the
	// access to the variable lock free, the structure itself doesn't need
	// to be lock free given that we get exclusive access to it here
	// by nulling out the global variable before using it).
	do {
		// We need to unlock our mutex before unreffing the objects,
		// since unreffing any object might cause unref_delayed to be
		// called (on the same thread), which will then try to lock the
		// mutex again, causing a dead-lock.
		pthread_mutex_lock (&delayed_unref_mutex);
		list = pending_unrefs;
		pending_unrefs = NULL;
		drain_tick_call_added = false;
		pthread_mutex_unlock (&delayed_unref_mutex);
	
		g_slist_foreach (list, (GFunc) unref_object, NULL);
		g_slist_free (list);
	} while (pending_unrefs != NULL);
}

void
EventObject::unref_delayed ()
{
	OBJECT_TRACK ("DelayedUnref", GetTypeName ());

	pthread_mutex_lock (&delayed_unref_mutex);
	pending_unrefs = g_slist_prepend (pending_unrefs, this);

	if (!drain_tick_call_added) {
		g_idle_add (drain_unrefs_idle_call, NULL);
		drain_tick_call_added = true;
	}
	pthread_mutex_unlock (&delayed_unref_mutex);
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
		return prop == args->property;
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
		obj->SetLogicalParent (NULL, NULL);
	}
}

void
DependencyObject::RemoveAllListeners ()
{
	g_hash_table_foreach (current_values, unregister_depobj_values, this);
}

static bool listeners_notified;

void
DependencyObject::NotifyListenersOfPropertyChange (PropertyChangedEventArgs *args)
{
	g_return_if_fail (args);

	DependencyObject *logical_parent = GetLogicalParent ();
	bool notified_parent = false;

	listeners_notified = true;

	for (GSList *l = listener_list; l != NULL; l = l->next){
		Listener *listener = (Listener*)l->data;

		if (listener->Matches (args))
			listener->Invoke (this, args);

		if (listener->GetListener() == logical_parent)
			notified_parent = true;
	}

	// attached properties are implicitly listened to by the
	// object's logical parent.  Notify them, but sure not to do
	// it twice.
	if (args->property->IsAttached() && !notified_parent) {
		if (logical_parent /*&& args->property->GetOwnerType() == logical_parent->GetObjectType ()*/)
			logical_parent->OnSubPropertyChanged (NULL, this, args);
	}
}

void
DependencyObject::NotifyListenersOfPropertyChange (DependencyProperty *subproperty)
{
	// XXX I really think this method should go away.  we only use it in
	// a couple of places, and it abuses things.

	Value *new_value = subproperty ? GetValue (subproperty) : NULL;

	PropertyChangedEventArgs args (subproperty, NULL, new_value);

	NotifyListenersOfPropertyChange (&args);
}

bool
DependencyObject::IsValueValid (Types *additional_types, DependencyProperty* property, Value* value, MoonError *error)
{
	if (property == NULL) {
		MoonError::FillIn (error, MoonError::ARGUMENT_NULL, 1001,
				   "NULL property passed to IsValueValid");
		return false;
	}

	if (value != NULL) {
		if (value->Is (additional_types, Type::EVENTOBJECT) && !value->AsEventObject ()) {
			// if it's a null DependencyObject, it doesn't matter what type it is
			return true;
		}
		
		if (value->Is (additional_types, Type::MANAGED)) {
			// This is a big hack, we do no type-checking if we try to set a managed type.
			// Given that for the moment we might not have the surface available, we can't
			// do any type checks since we can't access types registered on the surface.
			return true;
		}
		
		if (!value->Is (additional_types, property->GetPropertyType())) {
			MoonError::FillIn (error, MoonError::ARGUMENT, 1001,
					   g_strdup_printf ("DependencyObject::SetValue, value cannot be assigned to the "
							    "property %s::%s (property has type '%s', value has type '%s')",
							    GetTypeName (), property->GetName(), Type::Find (additional_types, property->GetPropertyType())->name,
							    Type::Find (additional_types, value->GetKind ())->name));
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

	if (DependencyObject::NameProperty == property) {
		NameScope *scope = FindNameScope ();
		if (scope && value) {
			DependencyObject *o = scope->FindName (value->AsString ());
			if (o && o != this) {
				MoonError::FillIn (error, MoonError::ARGUMENT, 2028,
						   g_strdup_printf ("The name already exists in the tree: %s.",
								    value->AsString ()));
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
DependencyObject::SetValue (DependencyProperty *property, Value *value)
{
	MoonError err;
	return SetValueWithError ((Types*)NULL, property, value, &err);
}

bool
DependencyObject::SetValue (DependencyProperty *property, Value value)
{
	MoonError err;
	return SetValueWithError ((Types*)NULL, property, &value, &err);
}

bool
DependencyObject::SetValueWithError (Types *additional_types, DependencyProperty* property, Value value, MoonError *error)
{
	return SetValueWithError (additional_types, property, &value, error);
}

bool
DependencyObject::SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error)
{
	if (is_frozen) {
		MoonError::FillIn (error, MoonError::UNAUTHORIZED_ACCESS, "Cannot set value on frozen DependencyObject");
		return false;
	}

	Value *current_value = GetLocalValue (property);

	bool equal = false;
	
	if (current_value != NULL && value != NULL) {
		equal = (*current_value == *value);
	} else {
		equal = (current_value == NULL) && (value == NULL);
	}

	if (!equal) {
		Value *new_value = value ? new Value (*value) : NULL;

		// store the new value in the hash
		g_hash_table_insert (current_values, property, new_value);

		ProviderValueChanged (PropertyPrecedence_LocalValue, property, current_value, new_value, true);

		if (current_value)
			delete current_value;
	}

	return true;
}

bool
DependencyObject::SetValueWithError (Types *additional_types, DependencyProperty *property, Value *value, MoonError *error)
{
	if (!IsValueValid (additional_types, property, value, error))
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
		
	if (n && strlen (n) > 0) {
		if (to_ns->FindName (n)) {
			MoonError::FillIn (error, MoonError::ARGUMENT, 2028,
					   g_strdup_printf ("The name already exists in the tree: %s.",
							    n));
			return;
		}
		to_ns->RegisterName (n, this);
	}

	RegisterNamesClosure closure;
	closure.to_ns = to_ns;
	closure.error = error;

	g_hash_table_foreach (current_values, register_depobj_names, &closure);
}

static void
unregister_depobj_names (gpointer  key,
			 gpointer  value,
			 gpointer  user_data)
{
	NameScope *from_ns = (NameScope*)user_data;
	Value *v = (Value*)value;

	if (v != NULL && v->Is (Type::DEPENDENCY_OBJECT) && v->AsDependencyObject() != NULL) {
		DependencyObject *obj = v->AsDependencyObject ();
		obj->UnregisterAllNamesRootedAt (from_ns);
	}
}

void
DependencyObject::UnregisterAllNamesRootedAt (NameScope *from_ns)
{
	NameScope *this_ns = NameScope::GetNameScope(this);
	if (this_ns && !this_ns->GetTemporary())
		return;
	
	const char *n = GetName();
	
	if (n && strlen (n) > 0)
		from_ns->UnregisterName (n);
	
	g_hash_table_foreach (current_values, unregister_depobj_names, from_ns);
}

Value *
DependencyObject::GetLocalValue (DependencyProperty *property)
{
	return providers[PropertyPrecedence_LocalValue]->GetPropertyValue (property);
}

Value *
DependencyObject::GetLocalValueWithError (Types *additional_types, DependencyProperty *property, MoonError *error)
{
	if (!HasProperty (additional_types, Type::INVALID, property, true)) {
		Type *pt = Type::Find (additional_types, property->GetOwnerType ());
		MoonError::FillIn (error, MoonError::EXCEPTION, g_strdup_printf ("Cannot get the DependencyProperty %s.%s on an object of type %s", pt ? pt->name : "<unknown>", property->GetName (), GetTypeName ()));
		return NULL;
	}
	return GetLocalValue (property);
}

Value *
DependencyObject::GetValueWithError (Types *additional_types, Type::Kind whatami, DependencyProperty *property, MoonError *error)
{
	if (!HasProperty (additional_types, whatami, property, true)) {
		Type *pt = Type::Find (additional_types, property->GetOwnerType ());
		MoonError::FillIn (error, MoonError::EXCEPTION, g_strdup_printf ("Cannot get the DependencyProperty %s.%s on an object of type %s", pt ? pt->name : "<unknown>", property->GetName (), GetTypeName ()));
		return NULL;
	}
	return GetValue (property);
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
DependencyObject::GetValueNoDefaultWithError (Types *additional_types, DependencyProperty *property, MoonError *error)
{
	if (!HasProperty (additional_types, Type::INVALID, property, true)) {
		Type *pt = Type::Find (additional_types, property->GetOwnerType ());
		MoonError::FillIn (error, MoonError::EXCEPTION, g_strdup_printf ("Cannot get the DependencyProperty %s.%s on an object of type %s", pt ? pt->name : "<unknown>", property->GetName (), GetTypeName ()));
		return NULL;
	}
	return GetValueNoDefault (property);	
}

void
DependencyObject::ProviderValueChanged (PropertyPrecedence providerPrecedence,
					DependencyProperty *property,
					Value *old_provider_value, Value *new_provider_value,
					bool notify_listeners)
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

		if (old_value && old_value->Is (Type::DEPENDENCY_OBJECT))
			old_as_dep = old_value->AsDependencyObject ();
		if (new_value && new_value->Is (Type::DEPENDENCY_OBJECT))
			new_as_dep = new_value->AsDependencyObject ();

		if (old_as_dep) {
			// unset its logical parent
			old_as_dep->SetLogicalParent (NULL, NULL);
			
			// remove ourselves as a target
			old_as_dep->RemoveTarget (this);
			
			// unregister from the existing value
			old_as_dep->RemovePropertyChangeListener (this, property);
			old_as_dep->SetSurface (NULL);
		}

		if (new_as_dep) {
			new_as_dep->SetSurface (GetSurface ());

			// set its logical parent
			if (new_as_dep->GetLogicalParent() != NULL && new_as_dep->GetLogicalParent() != this)
				g_warning ("DependencyObject already has a logical parent");
			MoonError error;
			new_as_dep->SetLogicalParent (this, &error);
 			if (error.number)
 				return /*XXX false*/;
			
			// listen for property changes on the new object
			new_as_dep->AddPropertyChangeListener (this, property);
			
			// add ourselves as a target
			new_as_dep->AddTarget (this);
		}


		// we need to make this optional, as doing it for NameScope
		// merging is killing performance (and noone should ever care
		// about that property changing)
		if (notify_listeners) {
			listeners_notified = false;
		
			PropertyChangedEventArgs args (property, old_value, new_value);

			OnPropertyChanged (&args);

			if (!listeners_notified)
				g_warning ("setting property %s::%s on object of type %s didn't result in listeners being notified\n",
					   Type::Find(property->GetOwnerType())->GetName (), property->GetName(), GetTypeName ());
		}

		if (property && property->GetChangedCallback () != NULL) {
			NativePropertyChangedHandler *callback = property->GetChangedCallback ();
			callback (property, this, old_value, new_value);
		}
 	}
}

void
DependencyObject::ClearValue (DependencyProperty *property, bool notify_listeners)
{
	Value *old_local_value = providers[PropertyPrecedence_LocalValue]->GetPropertyValue (property);

	if (old_local_value == NULL) {
		// there wasn't a local value set.  don't do anything
		return;
	}

	// detach from the existing value
	if (old_local_value->Is (Type::DEPENDENCY_OBJECT)){
		DependencyObject *dob = old_local_value->AsDependencyObject();

		if (dob != NULL) {
			// unset its logical parent
			dob->SetLogicalParent (NULL, NULL);

			// unregister from the existing value
			dob->RemovePropertyChangeListener (this, property);
			dob->SetSurface (NULL);
		}
	}

	g_hash_table_remove (current_values, property);

	// this is... yeah, it's disgusting
	for (int p = PropertyPrecedence_LocalValue + 1; p < PropertyPrecedence_Count; p ++) {
		if (providers[p])
			providers[p]->RecomputePropertyValue (property);
	}

	ProviderValueChanged (PropertyPrecedence_LocalValue, property, old_local_value, NULL, notify_listeners);
	
	delete old_local_value;
}

static gboolean
free_value (gpointer key, gpointer value, gpointer data)
{
	Value *v = (Value *) value;
	
	if (!value)
		return true;
	
	// detach from the existing value
	if (v->Is (Type::DEPENDENCY_OBJECT)){
		DependencyObject *dob = v->AsDependencyObject();
		
		if (dob != NULL) {
			// unset its logical parent
			dob->SetLogicalParent (NULL, NULL);

			// unregister from the existing value
			dob->RemovePropertyChangeListener ((DependencyObject*)data, NULL);
			dob->SetSurface (NULL);
		}
	}
	
	delete (Value *) value;
	
	return true;
}

DependencyObject::DependencyObject ()
{
	providers = new PropertyValueProvider*[PropertyPrecedence_Count];

	providers[PropertyPrecedence_Animation] = new AnimationPropertyValueProvider (this);
	providers[PropertyPrecedence_LocalValue] = new LocalPropertyValueProvider (this);
	providers[PropertyPrecedence_DynamicValue] = NULL;  // subclasses will set this if they need it.
	providers[PropertyPrecedence_Style] = new StylePropertyValueProvider (this);
	providers[PropertyPrecedence_Inherited] = new InheritedPropertyValueProvider (this);
	providers[PropertyPrecedence_DefaultValue] = new DefaultValuePropertyValueProvider (this);

	current_values = g_hash_table_new (g_direct_hash, g_direct_equal);
	listener_list = NULL;
	logical_parent = NULL;
	is_frozen = false;
}

Type::Kind
DependencyObject::GetObjectType ()
{
	return Type::DEPENDENCY_OBJECT; 
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
	if (listener_list != NULL) {
		g_slist_foreach (listener_list, free_listener, NULL);
		g_slist_free (listener_list);
	}

	RemoveAllListeners();
	g_hash_table_foreach_remove (current_values, free_value, this);
	g_hash_table_destroy (current_values);

	for (int i = 0; i < PropertyPrecedence_Count; i ++)
		delete providers[i];
	delete[] providers;
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
DependencyObject::HasProperty (Types *additional_types, Type::Kind whatami, DependencyProperty *property, bool inherits)
{
	Type::Kind this_type = whatami == Type::INVALID ? GetObjectType () : whatami;
	
	// TODO: Handle attached properties correctly.
	
	if (property->IsAttached ())
		return true;
	
	/*
	printf ("DependencyObject::HasProperty (%p, %i (%s), %p (%i %s.%s), %i)..\n", 
		additional_types, 
		whatami, Type::Find (additional_types, whatami)->name,
		property, property->GetOwnerType (), Type::Find (additional_types, property->GetOwnerType ())->name, property->GetName (), 
		inherits);
	*/
	
	if (property == NULL)
		return false;
		
	if (property->GetOwnerType () == this_type)
		return true;
		
	if (!inherits)
		return false;

	if (!Type::Find (additional_types, this_type)->IsSubclassOf (additional_types, property->GetOwnerType ())) {
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
	
	if (logical_parent)
		return logical_parent->FindName (name);
	
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

	if (logical_parent)
		return logical_parent->FindNameScope ();

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

void
DependencyObject::Shutdown ()
{
	EventObject::DrainUnrefs ();
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
	if (GetSurface() == s)
		return;
	EventObject::SetSurface (s);
	g_hash_table_foreach (current_values, set_surface, s);
}

void
DependencyObject::SetLogicalParent (DependencyObject *logical_parent, MoonError *error)
{
#if DEBUG
	// Check for circular families
	DependencyObject *current = logical_parent;
	while (current != NULL) {
		if (current == this) {
			g_warning ("cycle found in logical tree.  bailing out");
			return;
		}
		current = current->GetLogicalParent ();
	}
#endif
	
	if (logical_parent != this->logical_parent) {
		if (logical_parent) {
			NameScope *this_scope = NameScope::GetNameScope(this);
			NameScope *parent_scope = logical_parent->FindNameScope();
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
		if (this->logical_parent) {
			NameScope *parent_scope = this->logical_parent->FindNameScope ();
			if (parent_scope)
				UnregisterAllNamesRootedAt (parent_scope);
		}

		if (!error || error->number == 0)
			this->logical_parent = logical_parent;
	}
}

DependencyObject *
DependencyObject::GetLogicalParent ()
{
	DependencyObject *res = logical_parent;
	while (res && Type::Find (res->GetObjectType ())->IsSubclassOf (Type::COLLECTION))
		res = res->GetLogicalParent ();
	return res;
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
DependencyObject::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (DependencyObject::NameProperty == args->property) {
		NameScope *scope = FindNameScope ();
		if (scope && args->new_value)
			scope->RegisterName (args->new_value->AsString (), this);
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
