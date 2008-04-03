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

#include <gtk/gtk.h>
#include <pthread.h>

#include "debug.h"
#include "namescope.h"
#include "list.h"
#include "collection.h"
#include "dependencyobject.h"
#include "clock.h"
#include "runtime.h"
#include "uielement.h"

EventObject::EventObject ()
{
	surface = NULL;
	refcount = 1;
	events = NULL;

#if OBJECT_TRACKING
	id = ++objects_created;
	if (objects_alive == NULL)
		objects_alive = g_hash_table_new (g_direct_hash, g_direct_equal);
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

		FreeHandlers ();
}

void 
EventObject::unref ()
{
	bool delete_me;

	if (!Surface::InMainThread ()) {
		unref_delayed ();
		return;
	}

	delete_me = g_atomic_int_dec_and_test (&refcount);

	OBJECT_TRACK ("Unref", GetTypeName ());

	if (delete_me) {
		Emit (DestroyedEvent);
#if DEBUG
		if (refcount != 0) {
			g_warning ("Object %p (id: %i) of type %s has been woken up from the dead.\n", this, GET_OBJ_ID (this), GetTypeName ());
		}
#endif
		SetSurface (NULL);

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

char* 
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
	EventClosure (EventHandler func, gpointer data, int token) { this->func = func; this->data = data; this->token = token; }
	
	EventHandler func;
	gpointer data;
	int token;
};

int EventObject::DestroyedEvent = -1;


void
EventObject::FreeHandlers ()
{
	if (events) {
		int i, n = GetType ()->GetEventCount ();
		
		for (i = 0; i < n; i++)
			delete events[i].event_list;
		
		delete [] events;
	}
}

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
	
	if (events == NULL) {
		int i, n = GetType ()->GetEventCount ();
		
		events = new EventList [n];
		for (i = 0; i < n; i++) {
			events[i].current_token = 0;
			events[i].event_list = new List ();
		}
	}
	
	int token = events[event_id].current_token++;
	
	events[event_id].event_list->Append (new EventClosure (handler, data, token));
	
	return token;
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
EventObject::RemoveHandler (const char *event_name, int token)
{
	int id = GetType()->LookupEvent (event_name);

	if (id == -1) {
		g_warning ("removing handler for event '%s', which has not been registered\n", event_name);
		return;
	}

	RemoveHandler (id, token);
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
	
	EventClosure *closure = (EventClosure *) events[event_id].event_list->First ();
	while (closure) {
		if (closure->func == handler && closure->data == data) {
			events[event_id].event_list->Unlink (closure);
			delete closure;
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
	
	EventClosure *closure = (EventClosure *) events[event_id].event_list->First ();
	while (closure) {
		if (closure->token == token) {
			events[event_id].event_list->Unlink (closure);
			delete closure;
			break;
		}
		
		closure = (EventClosure *) closure->next;
	}
}

void
EventObject::RemoveMatchingHandlers (const char *event_name, bool (*predicate)(EventHandler cb_handler, gpointer cb_data, gpointer data), gpointer closure)
{
	int id = GetType()->LookupEvent (event_name);

	if (id == -1) {
		g_warning ("removing handler for event '%s', which has not been registered\n", event_name);
		return;
	}

	RemoveMatchingHandlers (id, predicate, closure);
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

	EventClosure *c = (EventClosure *) events[event_id].event_list->First ();
	while (c) {
		if (predicate (c->func, c->data, closure)) {
			events[event_id].event_list->Unlink (c);
			delete c;
			break;
		}
		
		c = (EventClosure *) c->next;
	}
}

bool
EventObject::Emit (char *event_name, EventArgs *calldata)
{
	int id = GetType()->LookupEvent (event_name);

	if (id == -1) {
		g_warning ("trying to emit event '%s', which has not been registered\n", event_name);
		return false;
	}

	return Emit (id, calldata);
}

bool
EventObject::Emit (int event_id, EventArgs *calldata)
{
	if (GetType()->GetEventCount() <= 0) {
		g_warning ("trying to emit event with id %d, which has not been registered\n", event_id);
		if (calldata)
			calldata->unref ();
		return false;
	}

	if (events == NULL || events[event_id].event_list->IsEmpty ()) {
		if (calldata)
			calldata->unref ();
		return false;
	}
	
	EventClosure *next, *closure = (EventClosure *) events[event_id].event_list->First ();
	List *event_list = new List ();
	
	/* make a copy of the event-list to use for emitting */
	while (closure) {
		event_list->Append (new EventClosure (closure->func, closure->data, closure->token));
		
		closure = (EventClosure *) closure->next;
	}
	
	/* emit the events using the copied list */
	closure = (EventClosure *) event_list->First ();
	while (closure) {
		next = (EventClosure *) closure->next;
		
		if (closure->func)
			closure->func (this, calldata, closure->data);
		
		event_list->Unlink (closure);
		delete closure;
		
		closure = next;
	}
	
	delete event_list;

	if (calldata)
		calldata->unref ();

	return true;
}

Surface*
event_object_get_surface (EventObject *o)
{
	return o->GetSurface ();
}

void
event_object_add_event_handler (EventObject *o, const char *event, EventHandler handler, gpointer closure)
{
	o->AddHandler (event, handler, closure);
}

void
event_object_remove_event_handler (EventObject *o, const char *event, EventHandler handler, gpointer closure)
{
	o->RemoveHandler (event, handler, closure);
}

void 
base_ref (EventObject *obj)
{
	if (obj != NULL)
		obj->ref ();
}

void
base_unref (EventObject *obj)
{
	if (obj != NULL)
		obj->unref ();
}

static pthread_mutex_t delayed_unref_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool drain_tick_call_added = false;
static GSList *pending_unrefs = NULL;

static gboolean
drain_unrefs_idle_call (gpointer data)
{
	drain_unrefs ();
	return FALSE;
}

void
drain_unrefs ()
{
	GSList *list;

	// We need to unlock our mutex before unreffing the objects,
	// since unreffing any object might cause unref_delayed to be
	// called (on the same thread), which will then try to lock the
	// mutex again, causing a dead-lock.
	pthread_mutex_lock (&delayed_unref_mutex);
	list = pending_unrefs;
	pending_unrefs = NULL;
	drain_tick_call_added = false;
	pthread_mutex_unlock (&delayed_unref_mutex);

	g_slist_foreach (list, (GFunc) base_unref, NULL);
	g_slist_free (list);
}

void
EventObject::unref_delayed ()
{
	OBJECT_TRACK ("DelayedUnref", GetTypeName ());

#if DEBUG
	if (Surface::InMainThread ())
		printf ("EventObject::unref_delayed () is being called on the main thread.\n");
#endif
	
	pthread_mutex_lock (&delayed_unref_mutex);
	pending_unrefs = g_slist_prepend (pending_unrefs, this);

	if (!drain_tick_call_added) {
		g_idle_add (drain_unrefs_idle_call, NULL);
		drain_tick_call_added = true;
	}
	pthread_mutex_unlock (&delayed_unref_mutex);
}


GHashTable *DependencyObject::properties = NULL;

typedef struct {
	DependencyObject *dob;
	DependencyProperty *prop;
} Listener;

//
// Registers @listener as a listener on changes to @child_property of this DO.
//
void
DependencyObject::AddPropertyChangeListener (DependencyObject *listener, DependencyProperty *child_property)
{
	Listener *listen = new Listener ();
	listen->dob = listener;
	listen->prop = child_property;
	listener_list = g_slist_append (listener_list, listen);
}

//
// Unregisters @container as a listener on changes to @child_property of this DO.
//
void
DependencyObject::RemovePropertyChangeListener (DependencyObject *listener, DependencyProperty *child_property)
{
	for (GSList *l = listener_list; l; l = l->next) {
		Listener *listen = (Listener*)l->data;

		if ((listen->dob == listener) && (child_property == NULL || listen->prop == child_property)) {
			listener_list = g_slist_remove_link (listener_list, l);
			delete listen;
		}
	}
}

static void
unregister_depobj_values (gpointer  key,
			  gpointer  value,
			  gpointer  user_data)
{
	DependencyObject *this_obj = (DependencyObject*)user_data;
	//DependencyProperty *prop = (DependencyProperty*)key;
	Value *v = (Value*)value;

	if (v != NULL && v->GetKind() >= Type::DEPENDENCY_OBJECT && v->AsDependencyObject() != NULL) {
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

		listener->dob->OnSubPropertyChanged (listener->prop, this, args);
		if (listener->dob == logical_parent)
			notified_parent = true;
	}

	// attached properties are implicitly listened to by the
	// object's logical parent.  Notify them, but sure not to do
	// it twice.
	if (args->property->is_attached_property && !notified_parent) {
		if (logical_parent /*&& args->property->type == logical_parent->GetObjectType ()*/)
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

void
DependencyObject::SetValue (DependencyProperty *property, Value *value)
{
	g_return_if_fail (property != NULL);

	if (value != NULL){
		if (!Type::Find (value->GetKind ())->IsSubclassOf (property->value_type)) {
			g_warning ("DependencyObject::SetValue, value cannot be assigned to the property %s::%s (property has type '%s', value has type '%s')\n", GetTypeName (), property->name, Type::Find (property->value_type)->name, Type::Find (value->GetKind ())->name);
			return;
		}
	} else {
		if (!(property->value_type >= Type::DEPENDENCY_OBJECT) && !property->IsNullable ()) {
			g_warning ("Can not set a non-nullable scalar type to NULL (property: %s)", property->name);
			return;
		}
	}

	Value *current_value = (Value*)g_hash_table_lookup (current_values, property);

	if (current_value != NULL && current_value->GetKind () >= Type::DEPENDENCY_OBJECT) {
		DependencyObject *current_as_dep = current_value->AsDependencyObject ();
		if (current_as_dep)
			current_as_dep->SetLogicalParent (NULL);
	}
	if (value != NULL && value->GetKind () >= Type::DEPENDENCY_OBJECT) {
		DependencyObject *new_as_dep = value->AsDependencyObject ();
		
		new_as_dep->SetLogicalParent (this);
	}

	if ((current_value == NULL && value != NULL) ||
	    (current_value != NULL && value == NULL) ||
	    (current_value != NULL && value != NULL && *current_value != *value)) {

		if (current_value) {
			// unregister from the existing value
			if (current_value->GetKind () >= Type::DEPENDENCY_OBJECT){
				DependencyObject *dob = current_value->AsDependencyObject();

				if (dob != NULL) {
					dob->RemovePropertyChangeListener (this, property);
					dob->SetSurface (NULL);
				}
			}

			// and remove its closure.
			if (Type::Find(current_value->GetKind())->IsSubclassOf (Type::COLLECTION)) {
				Collection *col = current_value->AsCollection ();
				if (col)
					col->closure = NULL;
			}
		}

		Value *new_value = value ? new Value (*value) : NULL;

		// store the new value in the hash
		g_hash_table_insert (current_values, property, new_value);

		if (new_value) {
			// listen for property changes on the new object
			if (new_value->GetKind () >= Type::DEPENDENCY_OBJECT){
				DependencyObject *dob = new_value->AsDependencyObject();
				
				if (dob != NULL) {
					dob->AddPropertyChangeListener (this, property);
					dob->SetSurface (GetSurface ());
				}
			}

			// and set its closure to us.
			if (Type::Find(new_value->GetKind())->IsSubclassOf (Type::COLLECTION)) {
				Collection *col = new_value->AsCollection ();
				if (col) {
					if (col->closure)
						g_warning ("Collection added as property of more than 1 dependency object");
					col->closure = this;
					MergeTemporaryNameScopes (col);
				}
			}
		}

		listeners_notified = false;

		PropertyChangedEventArgs args (property, current_value, new_value ? new_value : GetDefaultValue (property));

		OnPropertyChanged (&args);

		if (!listeners_notified)
			g_warning ("setting property %s::%s on object of type %s didn't result in listeners being notified\n",
				   Type::Find(property->type)->name, property->name, Type::Find(GetObjectType())->name);

		if (current_value)
			delete current_value;
	}
}

void
DependencyObject::MergeTemporaryNameScopes (Collection *c)
{
	NameScope *ns = NameScope::GetNameScope (this);
	Collection::Node *cn;
	for (cn = (Collection::Node *) c->list->First () ; cn != NULL; cn = (Collection::Node *) cn->next) {
		NameScope *c_ns = NameScope::GetNameScope (cn->obj);
		if (c_ns && c_ns->GetTemporary()) {
			if (!ns) {
				ns = new NameScope ();
				ns->SetTemporary (true);
				NameScope::SetNameScope (this, ns);
			}

			ns->MergeTemporaryScope (c_ns);
			// remove the child's temporary namescope
			cn->obj->ClearValue (NameScope::NameScopeProperty, false);
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

	if (v != NULL && v->GetKind() >= Type::DEPENDENCY_OBJECT && v->AsDependencyObject() != NULL) {
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

	if (v != NULL && v->GetKind() >= Type::DEPENDENCY_OBJECT && v->AsDependencyObject() != NULL) {
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

// 	g_hash_table_foreach (current_values, register_depobj_names, to_ns);
}

void
DependencyObject::SetValue (DependencyProperty *property, Value value)
{
	SetValue (property, &value);
}

Value *
DependencyObject::GetDefaultValue (DependencyProperty *property)
{
	return property->default_value;
}

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

Value *
DependencyObject::GetValue (const char *name)
{
	DependencyProperty *property;
	property = GetDependencyProperty (name);

	if (property == NULL) {
		g_warning ("This object (of type '%s') doesn't have a property called '%s'\n", GetType ()->name, name);
		return NULL;
	}

	return GetValue (property);
}

void
DependencyObject::ClearValue (DependencyProperty *property, bool notify_listeners)
{
	Value *current_value = GetValueNoDefault (property);

	if (current_value == NULL) {
		/* the property has the default value, nothing to do */
		return;
	}

	// detach from the existing value
	if (current_value->GetKind () >= Type::DEPENDENCY_OBJECT){
		DependencyObject *dob = current_value->AsDependencyObject();

		if (dob != NULL)
			dob->RemovePropertyChangeListener (this, property);

		dob->SetSurface (NULL);
	}

	// and remove it's closure
	if (Type::Find(current_value->GetKind())->IsSubclassOf (Type::COLLECTION)) {
		Collection *col = current_value->AsCollection ();
		if (col)
			col->closure = NULL;
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
				   Type::Find(property->type)->name, property->name, Type::Find(GetObjectType())->name);
	}

	delete current_value;
}

void 
DependencyObject::SetValue (const char *name, Value value)
{
	SetValue (name, &value);
}

void
DependencyObject::SetValue (const char *name, Value *value)
{
	DependencyProperty *property;
	property = GetDependencyProperty (name);

	if (property == NULL) {
		g_warning ("This object (of type '%s') doesn't have a property called '%s'\n", GetType ()->name, name);
		return;
	}

	SetValue (property, value);
}

static gboolean
free_value (gpointer key, gpointer value, gpointer data)
{
	Value *v = (Value*)value;

	// detach from the existing value
	if (v->GetKind () >= Type::DEPENDENCY_OBJECT){
		DependencyObject *dob = v->AsDependencyObject();

		if (dob != NULL)
			dob->RemovePropertyChangeListener ((DependencyObject*)data, NULL);

	}

	// and remove it's closure
	if (Type::Find(v->GetKind())->IsSubclassOf (Type::COLLECTION)) {
		Collection *col = v->AsCollection ();
		if (col)
			col->closure = NULL;
	}

	delete (Value*)value;
	return TRUE;
}

DependencyObject::DependencyObject ()
{
	current_values = g_hash_table_new (g_direct_hash, g_direct_equal);
	this->listener_list = NULL;
	this->logical_parent = NULL;
}

static void
dump (gpointer key, gpointer value, gpointer data)
{
	printf ("%s\n", (char*)key);
}

Type::Kind
DependencyObject::GetObjectType ()
{
	g_critical ("%p This class is missing an override of GetObjectType ()", this);
#if DEBUG
	print_stack_trace ();
#endif
	g_hash_table_foreach (current_values, dump, NULL);
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
	return DependencyObject::GetDependencyProperty (GetObjectType (), name);
}

DependencyProperty *
DependencyObject::GetDependencyProperty (Type::Kind type, const char *name)
{
	return GetDependencyProperty (type, name, true);
}

DependencyProperty *
DependencyObject::GetDependencyProperty (Type::Kind type, const char *name, bool inherits)
{
	GHashTable *table;
	DependencyProperty *property = NULL;

	if (properties == NULL)
		return NULL;

	table = (GHashTable*) g_hash_table_lookup (properties, &type);

	if (table)
		property = (DependencyProperty*) g_hash_table_lookup (table, name);

	if (property != NULL)
		return property;

	if (!inherits)
		return NULL;	

	// Look in the parent type
	Type *current_type;
	current_type = Type::Find (type);
	
	if (current_type == NULL)
		return NULL;
	
	if (current_type->parent == Type::INVALID)
		return NULL;

	return GetDependencyProperty (current_type->parent, name);
}

bool
DependencyObject::HasProperty (const char *name, bool inherits)
{
	return GetDependencyProperty (GetObjectType (), name, inherits) != NULL;
}

DependencyObject*
DependencyObject::FindName (const char *name)
{
	NameScope *scope = NameScope::GetNameScope (this);
	DependencyObject *rv = NULL;

	if (scope)
		rv = scope->FindName (name);

	if (rv)
		return rv;
	else if (logical_parent)
		return logical_parent->FindName (name);
	else {
		Surface *surface = GetSurface ();
		if (surface) {
			UIElement *toplevel = surface->GetToplevel ();
			if (toplevel && toplevel != this)
				return toplevel->FindName (name);
		}

		return NULL;
	}
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
dependency_object_find_name (DependencyObject *obj, const char *name, Type::Kind *element_kind)
{
	if (obj == NULL)
		return NULL;
	//printf ("Looking up in %p the string %p\n", obj, name);
	//printf ("        String: %s\n", name);
	DependencyObject *ret = obj->FindName (name);

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

//
// Use this for values that can be null
//
DependencyProperty *
DependencyObject::Register (Type::Kind type, const char *name, Type::Kind vtype)
{
	g_return_val_if_fail (name != NULL, NULL);

	return RegisterFull (type, name, NULL, vtype, false, false);
}

//
// DependencyObject takes ownership of the Value * for default_value
//
DependencyProperty *
DependencyObject::Register (Type::Kind type, const char *name, Value *default_value)
{
	g_return_val_if_fail (default_value != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	return RegisterFull (type, name, default_value, default_value->GetKind (), false, false);
}

static void
free_property_hash (gpointer v)
{
	g_hash_table_destroy ((GHashTable*)v);
}

static void
free_property (gpointer v)
{
	delete (DependencyProperty*)v;
}

//
// DependencyObject takes ownership of the Value * for default_value
// This overload can be used to set the type of the property to a different type
// than the default value (the default value can for instance be a SolidColorBrush
// while the property type can be a Brush).
//
DependencyProperty *
DependencyObject::Register (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype)
{
	g_return_val_if_fail (default_value != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	return RegisterFull (type, name, default_value, vtype, false, false);
}

DependencyProperty *
DependencyObject::RegisterNullable (Type::Kind type, const char *name, Type::Kind vtype)
{
	DependencyProperty *property;
	property = Register (type, name, vtype);
	property->is_nullable = true;
	return property;
}

//
// Register the dependency property that belongs to @type with the name @name
// The default value is @default_value (if provided) and the type that can be
// stored in the dependency property is of type @vtype
//
DependencyProperty *
DependencyObject::RegisterFull (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly)
{
	GHashTable *table;

	DependencyProperty *property = new DependencyProperty (type, name, default_value, vtype, attached, readonly);
	
	/* first add the property to the global 2 level property hash */
	if (properties == NULL)
		properties = g_hash_table_new_full (g_int_hash, g_int_equal,
						    NULL, free_property_hash);

	table = (GHashTable*) g_hash_table_lookup (properties, &property->type);

	if (table == NULL) {
		table = g_hash_table_new_full (strcase_hash, strcase_equal,
					       NULL, free_property);
		g_hash_table_insert (properties, &property->type, table);
	}

	g_hash_table_insert (table, property->name, property);

	return property;
}

void
DependencyObject::Shutdown ()
{
	drain_unrefs ();
	g_hash_table_destroy (DependencyObject::properties);
	DependencyObject::properties = NULL;
}

static void
set_surface (gpointer key, gpointer value, gpointer data)
{
	Surface *s = (Surface*)data;
	Value *v = (Value*)value;
	if (v->GetKind () >= Type::DEPENDENCY_OBJECT) {
		DependencyObject *dob = v->AsDependencyObject();
		dob->SetSurface (s);
	}
}

void
DependencyObject::SetSurface (Surface *s)
{
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
		g_assert (current != this);
		current = current->GetLogicalParent ();
	} 
#endif
	this->logical_parent = logical_parent;
}

DependencyObject*
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

/*
 *	DependencyProperty
 */
DependencyProperty::DependencyProperty (Type::Kind type, const char *name, Value *default_value, Type::Kind value_type, bool attached, bool readonly)
{
	this->type = type;
	this->name = g_strdup (name);
	this->default_value = default_value;
	this->value_type = value_type;
	this->is_attached_property = attached;
	this->is_readonly = readonly;
}

DependencyProperty::~DependencyProperty ()
{
	g_free (name);
	if (default_value != NULL)
		delete default_value;
}

DependencyProperty *dependency_property_lookup (Type::Kind type, char *name)
{
	return DependencyObject::GetDependencyProperty (type, name);
}

char*
dependency_property_get_name (DependencyProperty *property)
{
	return property->name;
}

Type::Kind
dependency_property_get_value_type (DependencyProperty *property)
{
	return property->value_type;
}

bool
dependency_property_is_nullable (DependencyProperty *property)
{
	return property->IsNullable ();
}


//
// Everything inside of a ( ) resolves to a DependencyProperty, if there is a
// '.' after the property, we get the object, and continue resolving from there
// if there is a [n] after the property, we convert the property to a collection
// and grab the nth item.
//
// Dependency properties can be specified as (PropertyName) of the current object
// or they can be specified as (DependencyObject.PropertyName).
//
// Returns NULL on any error
//
DependencyProperty *
resolve_property_path (DependencyObject **o, const char *path)
{
	g_return_val_if_fail (o != NULL, NULL);
	g_return_val_if_fail (path != NULL, NULL);

	int c;
	int len = strlen (path);
	char *typen = NULL;
	char *propn = NULL;
	bool expression_found = false;
	DependencyProperty *res = NULL;
	DependencyObject *lu = *o;
	const char *prop = path;
	
	for (int i = 0; i < len; i++) {
		switch (path [i]) {
		case '(':
		{
			expression_found = true;

			typen = NULL;
			propn = NULL;
			int estart = i + 1;
			for (c = estart; c < len; c++) {
				if (path [c] == '.') {
					typen = strndup (path + estart, c - estart);
					estart = c + 1;
					continue;
				}
				if (path [c] == ')') {
					propn = strndup (path + estart, c - estart);
					break;
				}
			}

			i = c;
			
			Type *t = NULL;
			if (typen) {
				t = Type::Find (typen);
			} else
				t = Type::Find (lu->GetObjectType ());

			if (!t) {
				g_free (propn);
				if (typen)
					g_free (typen);
				*o = NULL;
				return NULL;
			}
	
			res = DependencyObject::GetDependencyProperty (t->type, propn);
			if (!res) {
				g_warning ("Can't find %s property in %s", propn, typen);
				*o = NULL;
				return NULL;
			}

			if (! res->is_attached_property && ! lu->Is (t->type)) {
				// We try to be gracefull here and do something smart...
				res = DependencyObject::GetDependencyProperty (lu->GetObjectType (), propn);

				if (! res) {
					g_warning ("Got %s but expected a type of %s!", typen, lu->GetTypeName ());
					*o = NULL;
					return NULL;
				} else {
					g_warning ("Got %s but expected a type of %s ! "
						   "Did you mean %s.%s ? Using that.",
						   typen, lu->GetTypeName (), lu->GetTypeName (), propn);
				}
			}

			g_free (propn);
			if (typen)
				g_free (typen);
			break;
		}
		case '.':
			lu = lu->GetValue (res)->AsDependencyObject ();
			expression_found = false;
			prop = path + (i + 1);
			// we can ignore this, since we pull the lookup object when we finish a ( ) block
			break;
		case '[':
		{
			int indexer = 0;

			// Need to be a little more loving
			if (path [i + 1] == 0)
				break;

			char *p;

			indexer = strtol (path + i + 1, &p, 10);
			i = p - path;

			if (path [i] != ']'
			    || path [i + 1] != '.')
				break;

			Collection *col = lu->GetValue (res)->AsCollection ();
			List::Node *n = col->list->Index (indexer);
			if (n)
				lu = ((Collection::Node *) n)->obj;
			else {
				g_warning ("%s collection doesn't have element %d!", lu->GetTypeName (), indexer);
				*o = NULL;
				return NULL;
			}

			i += 1;
			break;
		}
		}
	}

	if (!expression_found)
		res = DependencyObject::GetDependencyProperty (lu->GetObjectType (), prop);

	*o = lu;
	return res;
}

DependencyProperty* DependencyObject::NameProperty;

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

void
dependencyobject_init(void)
{
	Type *t = Type::Find(Type::EVENTOBJECT);
	EventObject::DestroyedEvent = t->LookupEvent ("Destroyed");

	DependencyObject::NameProperty = DependencyObject::Register (Type::DEPENDENCY_OBJECT, "Name", Type::STRING);
}
