/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * dependencyobject.h: 
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


#ifdef DEBUG
#define d(x) x
#else
#define d(x)
#endif



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
	id = ++objects_created;
	if (objects_alive == NULL)
		objects_alive = g_hash_table_new (g_direct_hash, g_direct_equal);
	// Helgrind correctly reports a race here. Given this is for debugging, ignore it.
	g_hash_table_insert (objects_alive, this, GINT_TO_POINTER (1));
	weak_refs = NULL;

	Track ("Created", "");
#elif DEBUG
	objects_created++;
#endif
}

EventObject::~EventObject()
{
#if OBJECT_TRACKING
	objects_destroyed++;
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
	objects_destroyed++;
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
		d (print_stack_trace ());
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
		d (print_stack_trace ());
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
		d(printf ("EventObject::AddTickCall (): Could not add tick call, no surface\n"));
		return;
	}
	
	timemanager = surface->GetTimeManager ();
	
	if (!timemanager) {
		d(printf ("EventObject::AddTickCall (): Could not add tick call, no time manager\n"));
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

	if (delete_me) {
		Dispose ();
		delete this;
	}
}

#if DEBUG
int EventObject::objects_created = 0;
int EventObject::objects_destroyed = 0;
#endif

#if OBJECT_TRACKING
// Define the ID of the object you want to track
// Object creation, destruction and all ref/unrefs
// are logged to the console, with a stacktrace.
static bool object_id_fetched = false;
static int object_id = -1;

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
	}
	if (id == object_id) {
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
	EventClosure (EventHandler func, gpointer data, int token) {
		this->func = func;
		this->data = data;
		this->token = token;

		pending_removal = false;
		emit_count = 0;
	}
	
	EventHandler func;
	gpointer data;
	int token;
	bool pending_removal;
	int emit_count;
};

int
EventObject::AddHandler (const char *event_name, EventHandler handler, gpointer data)
{
	int id = GetType()->LookupEvent (event_name);

	if (id == -1) {
		g_warning ("adding handler to event '%s', which has not been registered\n", event_name);
		return -1;
	}

	return AddHandler (id, handler, data);
}

int
EventObject::AddHandler (int event_id, EventHandler handler, gpointer data)
{ 
	if (GetType()->GetEventCount() <= 0) {
		g_warning ("adding handler to event with id %d, which has not been registered\n", event_id);
		return -1;
	}
	
	if (events == NULL)
		events = new EventLists (GetType ()->GetEventCount ());
	
	int token = events->lists [event_id].current_token++;
	
	events->lists [event_id].event_list->Append (new EventClosure (handler, data, token));
	
	return token;
}

int
EventObject::AddXamlHandler (const char *event_name, EventHandler handler, gpointer data)
{
	int id = GetType ()->LookupEvent (event_name);
	
	if (id == -1) {
		g_warning ("adding xaml handler to event '%s', which has not been registered\n", event_name);
		return -1;
	}
	
	return AddHandler (id, handler, data);
}

int
EventObject::AddXamlHandler (int event_id, EventHandler handler, gpointer data)
{ 
	if (GetType ()->GetEventCount () <= 0) {
		g_warning ("adding xaml handler to event with id %d, which has not been registered\n", event_id);
		return -1;
	}
	
	if (events == NULL)
		events = new EventLists (GetType ()->GetEventCount ());
	
	events->lists [event_id].event_list->Append (new EventClosure (handler, data, 0));
	
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

//#define DEBUG_EVENTS

bool
EventObject::Emit (int event_id, EventArgs *calldata, bool only_unemitted)
{
#ifdef DEBUG_EVENTS
	const char *event_name = GetType ()->LookupEventName (event_id);
	if (GetObjectType () == Type::MEDIAELEMENT)
	printf ("%s::Emit (%s (%i), %p, %i). Listeners: %i\n", GetTypeName (), event_name, event_id, calldata, only_unemitted, events == NULL ? 0 : events->lists [event_id].event_list->Length ());
#endif

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
unref_object (EventObject *obj)
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
	for (GSList *l = listener_list; l; l = l->next) {
		Listener *listen = (Listener *) l->data;
		
		if ((listen->GetListener() == listener)
		    && (child_property == NULL || listen->GetProperty() == child_property)) {
			listener_list = g_slist_remove_link (listener_list, l);
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
		obj->SetLogicalParent (NULL);
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
		
#if SL_2_0
		if (value->Is (additional_types, Type::MANAGED)) {
			// This is a big hack, we do no type-checking if we try to set a managed type.
			// Given that for the moment we might not have the surface available, we can't
			// do any type checks since we can't access types registered on the surface.
			return true;
		}
#endif
		
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
		if (scope && value && !scope->GetTemporary ()) {
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
	return SetValueWithError ((Types*)NULL, property, value, NULL);
}

bool
DependencyObject::SetValue (DependencyProperty *property, Value value)
{
	return SetValueWithError ((Types*)NULL, property, &value, NULL);
}

bool
DependencyObject::SetValueWithError (Types *additional_types, DependencyProperty* property, Value value, MoonError *error)
{
	return SetValueWithError (additional_types, property, &value, error);
}

bool
DependencyObject::SetValueWithError (Types *additional_types, DependencyProperty* property, Value* value, MoonError *error)
{
	if (!IsValueValid (additional_types, property, value, error))
		return false;

	Value *current_value = (Value*)g_hash_table_lookup (current_values, property);
	bool equal = false;

	if (current_value != NULL && value != NULL) {
		equal = !property->AlwaysChange() && (*current_value == *value);
	} else {
		equal = (current_value == NULL) && (value == NULL);
	}

	if (!equal) {
		DependencyObject *current_as_dep = NULL;
		DependencyObject *new_as_dep = NULL;

		if (current_value && current_value->Is (Type::DEPENDENCY_OBJECT))
			current_as_dep = current_value->AsDependencyObject ();
		if (value && value->Is (Type::DEPENDENCY_OBJECT))
			new_as_dep = value->AsDependencyObject ();

		if (current_as_dep) {
			// unset its logical parent
			current_as_dep->SetLogicalParent (NULL);

			// unregister from the existing value
			current_as_dep->RemovePropertyChangeListener (this, property);
			current_as_dep->SetSurface (NULL);
		}

		Value *new_value = value ? new Value (*value) : NULL;

		// store the new value in the hash
		g_hash_table_insert (current_values, property, new_value);

		if (new_as_dep) {
			// set its logical parent
			if (new_as_dep->GetLogicalParent() != NULL && new_as_dep->GetLogicalParent() != this)
				g_warning ("DependencyObject already has a logical parent");
			new_as_dep->SetLogicalParent (this);
			
			// listen for property changes on the new object
			new_as_dep->AddPropertyChangeListener (this, property);
			new_as_dep->SetSurface (GetSurface ());

			// merge any temporary namescopes in its
			// subtree.
			MergeTemporaryNameScopes (new_as_dep);
		}
		
		listeners_notified = false;

		PropertyChangedEventArgs args (property, current_value, new_value ? new_value : GetDefaultValue (property));

		OnPropertyChanged (&args);

		if (!listeners_notified)
			g_warning ("setting property %s::%s on object of type %s didn't result in listeners being notified\n",
				   Type::Find(property->GetOwnerType())->GetName (), property->GetName(), GetTypeName ());

#if SL_2_0
		if (property && property->GetChangedCallback () != NULL) {
			NativePropertyChangedHandler *callback = property->GetChangedCallback ();
			callback (property, this, current_value, new_value);
		}
#endif

		if (current_value)
			delete current_value;
	}

	return true;
}

static NameScope *
create_temp_namescope (DependencyObject *o)
{
	NameScope *ns = new NameScope ();
	ns->SetTemporary (true);
	NameScope::SetNameScope (o, ns);
	ns->unref ();

	return ns;
}

static NameScope *
merge_namescope (NameScope *parent_ns, NameScope *child_ns, DependencyObject *owner)
{
	if (!parent_ns)
		parent_ns = create_temp_namescope (owner);
	
	parent_ns->MergeTemporaryScope (child_ns);
	
	// remove the child's temporary namescope
	child_ns->ClearValue (NameScope::NameScopeProperty, false);
	
	return parent_ns;
}

void
DependencyObject::MergeTemporaryNameScopes (DependencyObject *dob)
{
	NameScope *dob_ns = NameScope::GetNameScope (dob);
	NameScope *ns = FindNameScope ();
	
	if (dob_ns) {
		if (dob_ns->GetTemporary ())
			ns = merge_namescope (ns, dob_ns, this);
	}
#if false
	else {
		if (!ns)
			ns = create_temp_namescope (this);
		dob->RegisterAllNamesRootedAt (ns);
	}
#endif
	
	if (dob->Is (Type::DEPENDENCY_OBJECT_COLLECTION)) {
		Collection *c = (Collection *) dob;
		
		for (int i = 0; i < c->GetCount (); i++) {
			NameScope *c_ns = NameScope::GetNameScope (c->GetValueAt (i)->AsDependencyObject ());
			if (c_ns && c_ns->GetTemporary ())
				ns = merge_namescope (ns, c_ns, this);
		}
	}
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

static void
register_depobj_names (gpointer  key,
		       gpointer  value,
		       gpointer  user_data)
{
	NameScope *to_ns = (NameScope*)user_data;
	Value *v = (Value*)value;

	if (v != NULL && v->Is (Type::DEPENDENCY_OBJECT) && v->AsDependencyObject() != NULL) {
		DependencyObject *obj = v->AsDependencyObject ();
		obj->RegisterAllNamesRootedAt (to_ns);
	}
}

void
DependencyObject::UnregisterAllNamesRootedAt (NameScope *from_ns)
{
	const char *n = GetName();
	
	if (n)
		from_ns->UnregisterName (n);
	
	g_hash_table_foreach (current_values, unregister_depobj_names, from_ns);
}

void
DependencyObject::RegisterAllNamesRootedAt (NameScope *to_ns)
{
	const char *n = GetName();
	
	if (n)
		to_ns->RegisterName (n, this);

 	g_hash_table_foreach (current_values, register_depobj_names, to_ns);
}

Value *
DependencyObject::GetDefaultValue (DependencyProperty *property)
{
	return property->GetDefaultValue();
}

#if SL_2_0
Value *
DependencyObject::GetDefaultValueWithError (Types *additional_types, DependencyProperty *property, MoonError *error)
{
	if (!HasProperty (additional_types, Type::INVALID, property, true)) {
		Type *pt = Type::Find (additional_types, property->GetOwnerType ());
		MoonError::FillIn (error, MoonError::EXCEPTION, g_strdup_printf ("Cannot get the DependencyProperty %s.%s on an object of type %s", pt ? pt->name : "<unknown>", property->GetName (), GetTypeName ()));
		return NULL;
	}
	return GetDefaultValue (property);
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
#endif

Value *
DependencyObject::GetValue (DependencyProperty *property)
{
	void *value = NULL;
	
	if (g_hash_table_lookup_extended (current_values, property, NULL, &value))
		return (Value *) value;
	
	return GetDefaultValue (property);
}

Value *
DependencyObject::GetValueNoDefault (DependencyProperty *property)
{
	return (Value *) g_hash_table_lookup (current_values, property);
}

#if SL_2_0
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
#endif

void
DependencyObject::ClearValue (DependencyProperty *property, bool notify_listeners)
{
	Value *current_value = GetValueNoDefault (property);

	if (current_value == NULL) {
		/* the property has the default value, nothing to do */
		return;
	}

	// detach from the existing value
	if (current_value->Is (Type::DEPENDENCY_OBJECT)){
		DependencyObject *dob = current_value->AsDependencyObject();

		if (dob != NULL) {
			// unset its logical parent
			dob->SetLogicalParent (NULL);

			// unregister from the existing value
			dob->RemovePropertyChangeListener (this, property);
			dob->SetSurface (NULL);
		}
	}

	g_hash_table_remove (current_values, property);

	// we need to make this optional, as doing it for NameScope
	// merging is killing performance (and noone should ever care
	// about that property changing)
	if (notify_listeners) {
		listeners_notified = false;

		PropertyChangedEventArgs args (property, current_value, NULL);

		OnPropertyChanged (&args);

		if (!listeners_notified)
			g_warning ("setting property %s::%s on object of type %s didn't result in listeners being notified\n",
				   Type::Find(property->GetOwnerType())->GetName (), property->GetName(), GetTypeName ());
	}

	delete current_value;
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
			dob->SetLogicalParent (NULL);

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
	current_values = g_hash_table_new (g_direct_hash, g_direct_equal);
	listener_list = NULL;
	logical_parent = NULL;
}

Type::Kind
DependencyObject::GetObjectType ()
{
	return Type::DEPENDENCY_OBJECT; 
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

#if SL_2_0
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
#endif

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
DependencyObject::SetLogicalParent (DependencyObject *logical_parent)
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
	
	this->logical_parent = logical_parent;
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
