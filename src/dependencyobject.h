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

#include <glib.h>

#include "provider.h"
#include "dependencyproperty.h"
#include "value.h"
#include "enums.h"
#include "list.h"

//#define OBJECT_TRACKING 1

#if OBJECT_TRACKING
#define GET_OBJ_ID(x) (x ? x->id : 0)
#else
#define GET_OBJ_ID(x) (-1)
#endif

class CollectionChangedEventArgs;
class EventObject;
class EventArgs;
struct EmitContext;
class MoonError;

typedef void (* TickCallHandler) (EventObject *object);
typedef void (* EventHandler) (EventObject *sender, EventArgs *args, gpointer closure);
typedef bool (* EventHandlerPredicate) (EventHandler cb_handler, gpointer cb_data, gpointer data);

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

#if OBJECT_TRACKING
#define OBJECT_TRACK(x,y) Track((x),(y))
#else
#define OBJECT_TRACK(x,y)
#endif

/* @IncludeInKinds */	
class EventObject {
public:
	EventObject ();
	
	static gint objects_created;
	static gint objects_destroyed;
	
#if OBJECT_TRACKING
	static GHashTable *objects_alive;
	
	int id;
	
	char *GetStackTrace (const char *prefix);
	char *GetStackTrace () { return GetStackTrace (""); }
	void PrintStackTrace ();
	void Track (const char *done, const char *typname);
#endif
	
	/* @GenerateCBinding,GeneratePInvoke */
	void ref ()
	{
		int v = g_atomic_int_exchange_and_add (&refcount, 1);
		
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
			g_error ("Ref was called on an object with a refcount of 0.\n"); // g_error valid, see comment above.
		}
		
		OBJECT_TRACK ("Ref", GetTypeName ());
	}
	
	/* @GenerateCBinding,GeneratePInvoke */
	void unref ();
	
	/* @GenerateCBinding */
	static void DrainUnrefs ();

	int GetRefCount () { return refcount; }
	
	//
	// Is:
	//    Similar to C#'s is: it checks if this object is of this kind or 
	//    a derived class.
	
	bool Is (Type::Kind k)
	{
		return GetType ()->IsSubclassOf (k);
	}
	
	Type *GetType ()
	{
		return Type::Find (GetObjectType ());
	}

	/* @GenerateCBinding,GeneratePInvoke */
	virtual const char *GetTypeName ()
	{
		return Type::Find (GetObjectType ())->GetName ();
	}	
	
	/* @GenerateCBinding,GeneratePInvoke */
 	int AddHandler (const char *event_name, EventHandler handler, gpointer data, GDestroyNotify data_dtor = NULL);
	/* @GenerateCBinding,GeneratePInvoke */
 	int AddXamlHandler (const char *event_name, EventHandler handler, gpointer data, GDestroyNotify data_dtor = NULL);
	/* @GenerateCBinding,GeneratePInvoke */
 	void RemoveHandler (const char *event_name, EventHandler handler, gpointer data);

	int AddHandler (int event_id, EventHandler handler, gpointer data, GDestroyNotify data_dtor = NULL);
	int AddXamlHandler (int event_id, EventHandler handler, gpointer data, GDestroyNotify data_dtor = NULL);
	void RemoveHandler (int event_id, EventHandler handler, gpointer data);
	void RemoveHandler (int event_id, int token);
	void RemoveMatchingHandlers (int event_id, EventHandlerPredicate predicate, gpointer closure);
	
	/* @GenerateCBinding */
	Surface *GetSurface () { return surface; }
	virtual void SetSurface (Surface *surface);
	// SetSurfaceLock/Unlock
	//  If AddTickCallSafe is called on a type, that type must override SetSurface and surround the call to its base SetSurface implementation
	//  with Lock/Unlock. Catch: none of the base implementation can cause SetSurfaceLock to be called again, it might cause a dead-lock.
	//  (This could happen if a MediaElement could contain another MediaElement, in which case DependencyObject::SetSurface would cause 
	//  the contained MediaElement's SetSurface(Lock) to be called).
	bool SetSurfaceLock ();
	void SetSurfaceUnlock ();
	
	// AddTickCall*: 
	//  Queues a delegate which will be called on the main thread.
	//  The delegate's parameter will be the 'this' pointer.
	//  Only AddTickCallSafe is safe to call on threads other than the main thread,
	//  and only if the type on which it is called overrides SetSurface and surrounds
	//  the call to the base type's SetSurface with SetSurfaceLock/Unlock.
	void AddTickCall (TickCallHandler handler);
	void AddTickCallSafe (TickCallHandler handler);

	/* @GenerateCBinding,GeneratePInvoke */
	void SetObjectType (Type::Kind value) { object_type = value; }

	/* @GenerateCBinding,GeneratePInvoke */
	virtual Type::Kind GetObjectType () { return object_type; }

	const static int DestroyedEvent;
	
	void unref_delayed ();
	
	EmitContext *StartEmit (int event_id);
	bool DoEmit (int event_id, EmitContext *ctx, EventArgs *calldata = NULL, bool only_unemitted = false);
	void FinishEmit (int event_id, EmitContext *ctx);
	
	virtual void Dispose ();
	
protected:
	virtual ~EventObject ();
	
	bool IsDisposed () { return refcount == 0; }
	
	// To enable scenarios like Emit ("Event", new EventArgs ())
	// Emit will call unref on the calldata.
	bool Emit (char *event_name, EventArgs *calldata = NULL, bool only_unemitted = false);
	bool Emit (int event_id, EventArgs *calldata = NULL, bool only_unemitted = false);

private:
	void AddTickCallInternal (TickCallHandler handler);

	EventLists *events;
	Surface *surface;
	gint32 refcount;

	Type::Kind object_type;
};


struct PropertyChangedEventArgs {
	PropertyChangedEventArgs (DependencyProperty *p, Value *ov, Value *nv) : property(p), old_value(ov), new_value (nv) { }

	DependencyProperty *property;
	Value *old_value;
	Value *new_value;
};

typedef void (* PropertyChangeHandler) (DependencyObject *sender, PropertyChangedEventArgs *args, gpointer closure);

/* @Namespace=System.Windows */
/* @IncludeInKinds */	
class DependencyObject : public EventObject {
public:
 	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObject ();

	void Freeze ();

	GHashTable* GetCurrentValues () { return current_values; }

	// Gets the content property from this object's type, and
	// returns the value of that dependency property.
	//
	DependencyObject *GetContent ();
	DependencyProperty *GetDependencyProperty (const char *name);
	
	bool SetValue (DependencyProperty *property, Value *value);
	bool SetValue (DependencyProperty *property, Value value);

	/* @GenerateCBinding,GeneratePInvoke */
	bool SetMarshalledValueWithError (DependencyProperty *property, Value *value, MoonError *error);
	bool SetValueWithError (DependencyProperty *property, Value *value, MoonError *error);
	bool SetValueWithError (DependencyProperty *property, Value value, MoonError *error);

	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	Value *GetValueWithError (Type::Kind whatami, DependencyProperty *property, MoonError *error);
	virtual Value *GetValue (DependencyProperty *property);

	void ProviderValueChanged (PropertyPrecedence providerPrecedence, DependencyProperty *property, Value *old_value, Value *new_value, bool notify_listeners);
	Value *GetValue (DependencyProperty *property, PropertyPrecedence startingAtPrecedence);
	Value *GetValueSkippingPrecedence (DependencyProperty *property, PropertyPrecedence toSkip);

	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	Value *GetLocalValueWithError  (DependencyProperty *property, MoonError *error);
	virtual Value *GetLocalValue (DependencyProperty *property);
	
	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	Value *GetValueNoDefaultWithError (DependencyProperty *property, MoonError *error);
	Value *GetValueNoDefault (DependencyProperty *property);

	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	virtual void ClearValue (DependencyProperty *property, bool notify_listeners = true);
	bool HasProperty (const char *name, bool inherits);
	bool HasProperty (Type::Kind whatami, DependencyProperty *property, bool inherits);

	DependencyObject *FindName (const char *name);
	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObject *FindName (const char *name, Type::Kind *element_kind);
	NameScope *FindNameScope ();

	/* @GenerateCBinding,GeneratePInvoke */
	const char *GetName ();
	/* @GenerateCBinding,GeneratePInvoke */
	void SetName (const char *name);

	virtual void SetSurface (Surface *surface);

	void SetLogicalParent (DependencyObject *logical_parent, MoonError *error);
	DependencyObject *GetLogicalParent ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
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
	void AddPropertyChangeHandler (DependencyProperty *property, PropertyChangeHandler cb, gpointer closure);
	void RemovePropertyChangeHandler (DependencyProperty *property, PropertyChangeHandler cb);

	virtual void UnregisterAllNamesRootedAt (NameScope *from_ns);
	virtual void RegisterAllNamesRootedAt (NameScope *to_ns, MoonError *error);

	/* @PropertyType=string,GenerateAccessors,ManagedDeclaringType=FrameworkElement,DefaultValue=\"\" */
	static DependencyProperty *NameProperty;

	static void Shutdown ();

protected:
	virtual ~DependencyObject ();
	
	//
	// Returns true if a value is valid.  If the value is invalid return false.
	// If error is non NULL and the value is not valid, error will be given an error code and error message that should be
	// propogated to OnError
	//
	bool IsValueValid (DependencyProperty *property, Value *value, MoonError *error);
	
	virtual bool SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error);
	
	void NotifyListenersOfPropertyChange (PropertyChangedEventArgs *args);
	void NotifyListenersOfPropertyChange (DependencyProperty *property);
	
	void RemoveAllListeners ();

	PropertyValueProvider **providers;

private:
	void RemoveListener (gpointer listener, DependencyProperty *child_property);

	GHashTable        *current_values;
	GSList            *listener_list;
	DependencyObject  *logical_parent;

	bool is_frozen;
};

#endif /* __MONO_DEPOBJECT_H__ */
