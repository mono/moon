/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * dependencyobject.h: 
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MONO_DEPOBJECT_H__
#define __MONO_DEPOBJECT_H__

#ifndef HEAPVIZ
#define EVENT_ARG_REUSE 1
#endif
//#define PROPERTY_LOOKUP_DIAGNOSTICS 1

#include <glib.h>
#include <stdio.h>

#include "provider.h"
#include "weakrefmanager.h"
#include "dependencyproperty.h"
#include "value.h"
#include "enums.h"
#include "list.h"
#include "gchandle.h"

namespace Moonlight {

#define EVENTHANDLER(type, event, objtype, argtype)	\
	static void event##Callback (EventObject *sender, EventArgs *calldata, gpointer closure)	\
	{	\
		g_return_if_fail (sender != NULL);	\
		((type *) closure)->event##Handler ((objtype *) sender, (argtype *) calldata); \
	}	\
	void event##Handler (objtype *sender, argtype *calldata);

class CollectionChangedEventArgs;
class EventObject;
class EventArgs;
struct EmitContext;
class MoonError;

/* @CBindingRequisite */
typedef void (* TickCallHandler) (EventObject *object);
/* @CBindingRequisite */
typedef void (* UnmanagedEventHandlerInvoker) (EventObject *sender, int event_id, int token, EventArgs *args, gpointer closure);
/* @CBindingRequisite */
typedef void (* EventHandler) (EventObject *sender, EventArgs *args, gpointer closure);
typedef bool (* EventHandlerPredicate) (int token, EventHandler cb_handler, gpointer cb_data, gpointer data);
/* @CBindingRequisite */
typedef void (* DestroyUnmanagedEvent) (EventObject *object, int event_id, int token, void *data);
typedef void (* HandlerMethod) (EventObject *object, int token, gpointer data);

/* @CBindingRequisite */
typedef void (* ManagedRefCallback) (EventObject *referer, GCHandle referent, const void *id);
/* @CBindingRequisite */
typedef void (* MentorChangedCallback) (EventObject *object, EventObject *mentor);

class EventLists;


// 
// An EventObject starts out with a reference count of 1 (no need to
// ref it after creation), and will be deleted once the count reaches
// 0.
//
// DEBUGGING
// 
// To log all creation/destruction/ref/unref of an object,
// define OBJECT_TRACK_ID to that object's in dependencyobject.cpp.
// (this will require that you first run the program once to print
// the id of the object you're interested in).
//

#define GET_OBJ_ID(x) (x ? x->GetId () : 0)

/* @CBindingRequisite */
typedef void (* ToggleNotifyHandler) (gpointer sender, bool isLastRef);
class ToggleNotifyListener {
public:
	ToggleNotifyListener (gpointer sender, ToggleNotifyHandler callback)
	{
		this->callback = callback;
		this->sender = sender;
	}
	
	virtual ~ToggleNotifyListener () { }
	
	virtual void Invoke (bool isLastRef)
	{
		callback (sender, isLastRef);
	}

private:
	gpointer sender;
	ToggleNotifyHandler callback;
};

/* @Namespace=None,ManagedEvents=Manual */
class MOON_API EventObject {
private:
	enum Flags {
		MultiThreadedSafe = 1 << 28, // if the dtor can be called on any thread
		Attached = 1 << 29,
		Disposing = 1 << 30,
		Disposed = 1 << 31,
		IdMask = ~(Attached | Disposed | Disposing | MultiThreadedSafe),
	};
public:
#if OBJECT_TRACKING
	static GHashTable *objects_alive;
	static void PrintStackTrace ();
	void Track (const char *done, const char *typname);
#endif
	GCHandle GetManagedHandle () { return managed_handle; }
	/* @GeneratePInvoke */
	void ref ();
	
	/* @GeneratePInvoke */
	void unref ();
	static void unref_static (EventObject *obj); /* suitable for glib callbacks, hashtable free methods, etc */
	static void unref_eventhandler_data (EventObject *obj, int event_id, int token, void *data); /* suitable for dtors for eventhandlers */

	/* @GeneratePInvoke */
	int GetRefCount () { return refcount; }
	int GetId () { return flags & IdMask; }
	
	//
	// Is:
	//    Similar to C#'s is: it checks if this object is of this kind or 
	//    a derived class.
	
	bool Is (Type::Kind k)
	{
		return Type::IsSubclassOf (GetDeployment (), GetObjectType (), k);
	}
	
	Type *GetType ()
	{
		return Type::Find (GetDeployment (), GetObjectType ());
	}

	/* @GeneratePInvoke */
	virtual const char *GetTypeName ()
	{
		if (object_type == Type::DEPLOYMENT)
			return "Deployment";
		return Type::Find (GetDeployment (), GetObjectType ())->GetName ();
	}	
	
	int AddHandler (const char *event_name, EventHandler handler, gpointer data, DestroyUnmanagedEvent data_dtor = NULL, bool managed_data_dtor = false, bool handledEventsToo = false);
	int AddXamlHandler (const char *event_name, EventHandler handler, gpointer data, DestroyUnmanagedEvent data_dtor = NULL, bool managed_data_dtor = false, bool handledEventsToo = false);
 	void RemoveHandler (const char *event_name, EventHandler handler, gpointer data);

	/* @GeneratePInvoke */
	void AddOnEventHandler (int event_id, EventHandler handler, gpointer data, DestroyUnmanagedEvent data_dtor = NULL, bool managed_data_dtor = false, bool handledEventsToo = false);
	/* @GeneratePInvoke */
 	void RemoveOnEventHandler (int event_id, EventHandler handler, gpointer data);

	// called from the managed layer (Control.cs).
	/* @GeneratePInvoke */
	void DoEmitCurrentContext (int event_id, EventArgs *calldata);

	/* @GeneratePInvoke */
	virtual int AddManagedHandler (int event_id, UnmanagedEventHandlerInvoker handler, gpointer data, DestroyUnmanagedEvent data_dtor = NULL, bool managed_data_dtor = false, bool handledEventsToo = false);

	virtual int AddHandler (int event_id, EventHandler handler, gpointer data, DestroyUnmanagedEvent data_dtor = NULL, bool managed_data_dtor = false, bool handledEventsToo = false);
	/* @GeneratePInvoke */
	int AddXamlHandler (int event_id, EventHandler handler, gpointer data, DestroyUnmanagedEvent data_dtor = NULL, bool managed_data_dtor = false, bool handledEventsToo = false);
	virtual int RemoveHandler (int event_id, EventHandler handler, gpointer data);
	/* @GeneratePInvoke */
	virtual void RemoveHandler (int event_id, int token);
	void RemoveAllHandlers (gpointer data);
	// A NULL predicate means all handlers match
	void RemoveMatchingHandlers (int event_id, EventHandlerPredicate predicate, gpointer closure);

	int FindHandlerToken (int event_id, EventHandler handler, gpointer data);

	void ForeachHandler (int event_id, bool only_new, HandlerMethod m, gpointer closure);
	void ClearForeachGeneration (int event_id);
	void ForHandler (int event_id, int token, HandlerMethod m, gpointer closure);
	bool HasHandlers (int event_id, int newer_than_generation = -1);

	// AddTickCall*: 
	//  Queues a delegate which will be called on the main thread.
	//  The delegate's parameter will be the 'this' pointer.
	//  This method is thread-safe.
	void AddTickCall (TickCallHandler handler, EventObject *data = NULL);

	/* @GeneratePInvoke */
	void SetObjectType (Type::Kind value) { object_type = value; }

	/* @GeneratePInvoke */
	void SetManagedPeerCallbacks (ManagedRefCallback add_strong_ref,
				      ManagedRefCallback clear_strong_ref,
				      MentorChangedCallback mentor_changed);

	virtual void EnsureManagedPeer ();

	/* @GeneratePInvoke */
	Type::Kind GetObjectType () { return object_type; }

	const static int DestroyedEvent;
	
	void unref_delayed ();
	
	EmitContext *StartEmit (int event_id, int only_token, bool only_unemitted = false, int starting_generation = -1);
	bool DoEmit (int event_id, EventArgs *calldata = NULL);
	void FinishEmit (int event_id, EmitContext *ctx);
	static bool EmitCallback (gpointer d);
	
	virtual void Dispose ();
	
	/* 
	 * Looking for GetSurface/SetSurface?
	 * - If you want to know if the object is attached, use IsAttached ()
	 * - If you really want the surface, use GetDeployment ()->GetSurface ()
	 */
	bool IsAttached ();
	void SetIsAttached (bool value);
	virtual void OnIsAttachedChanged (bool newValue) { }

	bool IsDisposed ();
	bool IsDisposing ();
	bool IsMultiThreadedSafe () { return (flags & MultiThreadedSafe) != 0; }
	
	Deployment *GetDeployment ()
#if !SANITY
	{ return deployment; }
#endif
	;
	
	/* @GenerateCBinding */
	void SetCurrentDeployment (bool domain = true);

	// a public deployment getter for sanity checking without the warnings in GetDeployment.
	// it may also be used whenever it is known that the current deployment might be wrong
	// and we want the deployment on this object.
	Deployment *GetUnsafeDeployment () { return deployment; }

	/* @GeneratePInvoke */
	void SetManagedHandle (GCHandle managed_handle);

	ManagedRefCallback addManagedRef;
	ManagedRefCallback clearManagedRef;
	MentorChangedCallback mentorChanged;

	bool hadManagedPeer;

	static void ClearWeakRef (EventObject *sender, EventArgs *args, gpointer closure);
	bool EmitOnly (int event_id, int token, EventArgs *calldata = NULL, bool only_unemitted = false, int starting_generation = -1);

protected:
	virtual ~EventObject ();
	/* @SkipFactories */
	EventObject ();
	/* @SkipFactories */
	EventObject (Type::Kind type);
	/* @SkipFactories */
	EventObject (Type::Kind type, bool multi_threaded_safe);
	/* @SkipFactories */
	EventObject (Deployment *deployment);
	/* @SkipFactories */
	EventObject (Deployment *deployment, Type::Kind type);
	
	// To enable scenarios like Emit ("Event", new EventArgs ())
	// Emit will call unref on the calldata.
	bool Emit (const char *event_name, EventArgs *calldata = NULL, bool only_unemitted = false, int starting_generation = -1);
	bool Emit (int event_id, EventArgs *calldata = NULL, bool only_unemitted = false, int starting_generation = -1);
	
	bool EmitAsync (const char *event_name, EventArgs *calldata = NULL, bool only_unemitted = false);
	bool EmitAsync (int event_id, EventArgs *calldata = NULL, bool only_unemitted = false);
	
	int GetEventGeneration (int event_id);

	/* This method will increase the refcount by one, and it won't warn if refcount already is 0.
	 * There is at least one valid case for this: when something on the media thread realizes in
	 * the dtor that some cleanup is required, but the cleanup must happen on the main thread.
	 * In this case we need to stay alive until the cleanup on the main thread has finished.
	 * The only reason real reason for this method is to exist is to be able to keep a warning
	 * in ref () about reffing an object with a refcount of 0 (and trust that the warning means
	 * something really bad has happened) */
	void Resurrect ();

private:
	void AddTickCallInternal (TickCallHandler handler, EventObject *data = NULL);
	void Initialize (Deployment *deployment, Type::Kind type);
	
	static void emit_async (EventObject *calldata);
	bool CanEmitEvents (int event_id);
		
	EventLists *events;
	Deployment *deployment;
	gint32 refcount;
	gint32 flags; // Don't define as Flags, we need to keep this reliably at 32 bits.

	Type::Kind object_type;
	GCHandle managed_handle;

	friend class WeakRefBase;
};

template<typename EO>
class EventObjectNode : public List::GenericNode<EO*> {
public:
	EventObjectNode (EO *obj)
		: List::GenericNode<EO*> (obj)
	{
		if (obj)
			obj->ref ();
	}
	EventObjectNode ()
		: List::GenericNode<EO*> (NULL)
	{
	}
	virtual ~EventObjectNode ()
	{
		if (this->element)
			this->element->unref ();
	}
};

/* @Namespace=System.Windows */
class MOON_API DependencyObject : public EventObject {
public:
	virtual void Dispose ();

	void Freeze ();

	DependencyObject* Clone (Types *types);

	DependencyProperty **GetProperties (bool only_changed);
	
	/* @GeneratePInvoke */
	DependencyObject *GetMentor ();
	void SetMentor (DependencyObject *value);

	/* @GeneratePInvoke */
	void SetTemplateOwner (DependencyObject *value);
	/* @GeneratePInvoke */
	DependencyObject *GetTemplateOwner ();

	// Gets the content property from this object's type, and
	// returns the value of that dependency property.
	//
	DependencyObject *GetContent ();
	DependencyProperty *GetDependencyProperty (const char *name);
	
	bool SetValue (DependencyProperty *property, const Value *value);
	bool SetValue (DependencyProperty *property, const Value &value);
	
	bool SetValue (int property, const Value *value);
	bool SetValue (int property, const Value &value);

	/* @GeneratePInvoke */
	bool SetValueWithError (DependencyProperty *property, const Value *value, MoonError *error);
	bool SetValueWithError (DependencyProperty *property, const Value &value, MoonError *error);

	bool PropagateInheritedValue (InheritedPropertyValueProvider::Inheritable inheritableProperty,
				      DependencyObject *source, Value *new_value);

	Value *GetValueWithError (DependencyProperty *property, MoonError *error);
	Value *GetValue (DependencyProperty *property);
	Value *GetValue (int id);
	Value *GetValue (int id, PropertyPrecedence startingAtPrecedence);
	Value *GetValue (int id, PropertyPrecedence startingAtPrecedence, PropertyPrecedence endingAtPrecedence);

	Value *GetValueNoAutoCreate (int id);
	Value *GetValueNoAutoCreate (DependencyProperty *property);

	Value *GetAnimationBaseValueWithError (int id, MoonError *error);
	/* @GeneratePInvoke */
	Value *GetAnimationBaseValueWithError (DependencyProperty *property, MoonError *error);

	bool PropertyHasValueNoAutoCreate (int property_id, DependencyObject *obj);

	void ProviderValueChanged (PropertyPrecedence providerPrecedence, DependencyProperty *property, Value *old_value, Value *new_value, bool notify_listeners, bool set_parent, bool merge_names_on_set_parent, MoonError *error);
	Value *GetValue (DependencyProperty *property, PropertyPrecedence startingAtPrecedence);
	Value *GetValue (DependencyProperty *property, PropertyPrecedence startingAtPrecedence, PropertyPrecedence endingAtPrecedence);
	
	/* @GeneratePInvoke */
	Value *ReadLocalValueWithError (DependencyProperty *property, MoonError *error);
	virtual Value *ReadLocalValue (DependencyProperty *property);
	virtual Value *ReadLocalValue (int id);
	
	/* @GeneratePInvoke */
	Value *GetValueNoDefaultWithError (DependencyProperty *property, MoonError *error);
	Value *GetValueNoDefault (DependencyProperty *property);
	Value *GetValueNoDefault (int id);
	
	
	/* @GeneratePInvoke */
	virtual void ClearValue (DependencyProperty *property, bool notify_listeners, MoonError *error);
	void ClearValue (int id, bool notify_listeners, MoonError *error);
	void ClearValue (DependencyProperty *property, bool notify_listeners = true /*, error = NULL */);
	void ClearValue (int id, bool notify_listeners = true);
	bool HasProperty (const char *name, bool inherits);
	bool HasProperty (Type::Kind whatami, DependencyProperty *property, bool inherits);

	int GetPropertyValueProvider (DependencyProperty *property);
	DependencyObject* GetInheritedValueSource (InheritedPropertyValueProvider::Inheritable inheritableProperty);
	void SetInheritedValueSource (InheritedPropertyValueProvider::Inheritable inheritableProperty, DependencyObject *source);

	DependencyObject *FindName (const char *name);
	DependencyObject *FindName (const char *name, bool template_item);
	/* @GeneratePInvoke */
	DependencyObject *FindName (const char *name, Type::Kind *element_kind);
	NameScope *FindNameScope ();
	NameScope *FindNameScope (bool template_namescope);

	AnimationStorage *AttachAnimationStorage (DependencyProperty *prop, AnimationStorage *storage);
	void DetachAnimationStorage (DependencyProperty *prop, AnimationStorage *storage);
	AnimationStorage *GetAnimationStorageFor (DependencyProperty *prop);

	/* @GeneratePInvoke */
	const char *GetName ();
	/* @GeneratePInvoke */
	void SetName (const char *name);
	/* @GeneratePInvoke */
	bool SetNameOnScope (const char *name, NameScope *scope);

	virtual void OnIsAttachedChanged (bool value);

	void SetParent (DependencyObject *parent, MoonError *error);
	void SetParent (DependencyObject *parent, bool merge_names_from_subtree, MoonError *error);
	DependencyObject* GetParent () { return parent; }

	void SetSecondaryParent (DependencyObject *value) { secondary_parent = value; }
	DependencyObject* GetSecondaryParent () { return secondary_parent; }

	virtual bool PermitsMultipleParents () { return true; }

	/* @GeneratePInvoke */
	void SetResourceBase (const Uri *resourceBase);
	/* @GeneratePInvoke */
	const Uri *GetResourceBase ();
	const Uri *GetResourceBaseRecursive ();

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	
	// See the comment below about AddPropertyChangeListener for
	// the meaning of the @prop arg in this method.  it's not what
	// you might think it is.
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args) { }
	
	//
	// OnCollectionChanged:
	//
	// This method is invoked when a change has happened in the @col
	// collection, the kind of change is described in @type (change start,
	// change end, adding, removing, or altering an existing item).
	//
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args) { }
	
	//
	// OnCollectionItemChanged:
	//
	// This method is invoked when an item in the collection has had a property changed.
	//
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args) { }
	
	// These methods get called on DependencyObjects when they are
	// set/unset as property values on another DependencyObject.
	virtual void AddTarget (DependencyObject *obj) { }
	virtual void RemoveTarget (DependencyObject *obj) { }
	
	// These two methods are a little confusing.  @child_property
	// is *not* the property you're interested in receiving change
	// notifications on.  Listeners are always notified of all
	// changes.  What @child_property is used for is so that the
	// listener can look at it and know which of its *own*
	// properties is reporting the change.  So, if a object wants
	// to listen for changes on its BackgroundProperty, it would
	// essentially do:
	//
	// background = GetValue(BackgroundProperty)->AsDependencyObject();
	// background->AddPropertyChangeListener (this, BackgroundProperty);
	//
	// then in its OnSubPropertyChanged method, it could check prop to
	// see if it's == BackgroundProperty and act accordingly.  The
	// child's changed property is contained in the @subobj_args
	// argument of OnSubPropertyChanged.

	void AddPropertyChangeListener (DependencyObject *listener, DependencyProperty *child_property = NULL);
	void RemovePropertyChangeListener (DependencyObject *listener, DependencyProperty *child_property = NULL);

	// *These* two methods do what you'd expect.  You provide this
	// dependencyobject with a callback and a closure to be
	// invoked when the given property changes.

	int AddPropertyChangeHandler (DependencyProperty *property, PropertyChangeHandler cb, gpointer closure);
	/* @GeneratePInvoke */
	int AddManagedPropertyChangeHandler (DependencyProperty *property, PropertyChangeHandlerInvoker cb, gpointer closure);
	/* @GeneratePInvoke */
	void RemovePropertyChangeHandler (int token);
	void RemovePropertyChangeHandler (DependencyProperty *property, PropertyChangeHandler cb);

	virtual void UnregisterAllNamesRootedAt (NameScope *from_ns);
	virtual void RegisterAllNamesRootedAt (NameScope *to_ns, MoonError *error);

	/* @PropertyType=string,GenerateAccessors,ManagedDeclaringType=FrameworkElement,Validator=NameValidator,DefaultValue=\"\" */
	const static int NameProperty;
	
	// parser hook.  objects that are parsed using XamlReader.Load
	// behave differently than those parsed using LoadComponent in
	// terms of their name registration behavior.
	void SetIsHydratedFromXaml (bool flag) { is_hydrated = flag; }
	bool IsHydratedFromXaml () { return is_hydrated; }
	
	// if the xaml parser is currently parsing this object, then this
	// state will be set.
	void SetIsBeingParsed (bool v) { is_being_parsed = v; }
	bool IsBeingParsed () { return is_being_parsed; }

protected:
 	/* @GeneratePInvoke */
	DependencyObject ();

	virtual ~DependencyObject ();
	DependencyObject (Deployment *deployment, Type::Kind object_type = Type::DEPENDENCY_OBJECT);
	DependencyObject (Type::Kind object_type);

	//
	// Returns true if a value is valid.  If the value is invalid return false.
	// If error is non NULL and the value is not valid, error will be given an error code and error message that should be
	// propogated to OnError
	//
	bool IsValueValid (DependencyProperty *property, Value *value, MoonError *error);
	
	virtual bool SetValueWithErrorImpl (DependencyProperty *property, const Value *value, MoonError *error);
	
	void NotifyListenersOfPropertyChange (PropertyChangedEventArgs *args, MoonError *error);
	void NotifyListenersOfPropertyChange (DependencyProperty *property, MoonError *error);
	void NotifyListenersOfPropertyChange (int id, MoonError *error);
	virtual void OnMentorChanged (DependencyObject *old_mentor, DependencyObject *new_mentor);

	static void propagate_mentor (DependencyProperty *key, Value *value, gpointer data);
	void RemoveAllListeners ();

	virtual void CloneCore (Types *types, DependencyObject* from);

	PropertyValueProviderVTable providers;
	static gboolean dispose_value (gpointer key, gpointer value, gpointer data);

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:
	void CallRecomputePropertyValueForProviders (DependencyProperty *property, int providerPrecedence, MoonError *error);

	void DetachTemplateOwnerDestroyed ();
	void RemoveListener (gpointer listener, DependencyProperty *child_property);
	void Initialize ();

	static void collection_changed (EventObject *sender, EventArgs *args, gpointer closure);
	static void collection_item_changed (EventObject *sender, EventArgs *args, gpointer closure);

	static void clone_local_value (DependencyProperty *key, Value *value, gpointer data);
	static void clone_autocreated_value (DependencyProperty *key, Value *value, gpointer data);
	static void clone_animation_storage_list (DependencyProperty *key, List *list, gpointer data);
	void CloneAnimationStorageList (DependencyProperty *key, List *list);

	static void TemplateOwnerDestroyedEvent (EventObject *sender, EventArgs *args, gpointer closure);

#if PROPERTY_LOOKUP_DIAGNOSTICS
	GHashTable *hash_lookups_per_property;
	GHashTable *get_values_per_property;
#endif

	GHashTable *provider_bitmasks; // keys: DependencyProperty, values: bitmask of 1 << PropertyPrecedence values

	GHashTable *storage_hash; // keys: DependencyProperty, values: animation storage's

	GSList            *listener_list;
	WeakRef<DependencyObject> mentor;
	WeakRef<DependencyObject> parent;
	WeakRef<DependencyObject> template_owner;
	WeakRef<DependencyObject> secondary_parent;

	bool is_frozen;
	bool is_hydrated;
	bool is_being_parsed;
	bool registering_names;
	int property_change_token;
	
	Uri *resource_base;
};

};

#endif /* __MONO_DEPOBJECT_H__ */
