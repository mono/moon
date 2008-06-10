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

#include "value.h"
#include "enums.h"
#include "list.h"

//#define OBJECT_TRACKING 1

#if OBJECT_TRACKING
#define GET_OBJ_ID(x) (x ? x->id : 0)
#else
#define GET_OBJ_ID(x) (-1)
#endif

class EventObject;
class EventArgs;
struct EmitContext;

typedef void (* EventHandler) (EventObject *sender, EventArgs *args, gpointer closure);

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
	
	EventLists (int n);
	~EventLists ();
};

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
// To ensure that an object is destroyed before another object,
// you can use weak_ref/unref. For instance to ensure that child
// objects are always destroyed before their parent, make the
// child call weak_ref/unref on their parent. This won't prevent
// the parent from being destructed, but if the parent has any 
// weak refs upon destruction, a printf will be shown to the console
// (as long as OBJECT_TRACKING is defined). After that, you'll
// most likely crash when the child tries to call weak_unref on 
// its destructed parent.
//
#if OBJECT_TRACKING
#define OBJECT_TRACK(x,y) Track((x),(y))
#else
#define OBJECT_TRACK(x,y)
#endif

class EventObject {
 public:
	EventObject ();

#if DEBUG
	static int objects_created;
	static int objects_destroyed;
#endif

#if OBJECT_TRACKING
	void weak_ref (EventObject *base);
	void weak_unref (EventObject *base);
	
	static GHashTable *objects_alive;
	
	GHashTable *weak_refs;
	int id;

	char *GetStackTrace (const char *prefix);
	char *GetStackTrace () { return GetStackTrace (""); }
	void PrintStackTrace ();
	void Track (const char *done, const char *typname);
#endif
	
	void ref ()
	{
		if (refcount == 0) {
			// Here something really bad happened, this object is probably being reffed again because
			// of some action in the destructor. There is no way to recover from this now, no matter what 
			// we do the code that called ref now will be accessing a deleted object later on, which may or 
			// may not crash. It might very well be an exploitable security problem. Anyways when unref is called, we 
			// have a second delete on the same object, which *will* crash. To make things easier and safer
			// lets just abort right away.
			g_error ("Ref was called an object with a refcount of 0.\n"); // g_error valid, see comment above.
		}

		g_atomic_int_inc (&refcount);
		OBJECT_TRACK ("Ref", GetTypeName ());
	}
	
	void unref ();

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
	
	const char *GetTypeName ()
	{
		return Type::Find (GetObjectType ())->GetName ();
	}
	
	int AddHandler (const char *event_name, EventHandler handler, gpointer data);
	void RemoveHandler (const char *event_name, EventHandler handler, gpointer data);
	void RemoveHandler (const char *event_name, int token);
	void RemoveMatchingHandlers (const char *event_name, bool (*predicate)(EventHandler cb_handler, gpointer cb_data, gpointer data), gpointer closure);

	int AddHandler (int event_id, EventHandler handler, gpointer data);
	void RemoveHandler (int event_id, EventHandler handler, gpointer data);
	void RemoveHandler (int event_id, int token);
	void RemoveMatchingHandlers (int event_id, bool (*predicate)(EventHandler cb_handler, gpointer cb_data, gpointer data), gpointer closure);

	virtual Surface *GetSurface () { return surface; }
	virtual void SetSurface (Surface *surface);

	// This method is safe to call from other than the main thread.
	// This object is reffed before adding the tick call, the callback must unref.
	void AddTickCall (void (*func)(gpointer));
	
	virtual Type::Kind GetObjectType () { return Type::EVENTOBJECT; }
	
	const static int DestroyedEvent;
	
	void unref_delayed ();

	EmitContext* StartEmit (int event_id);
	bool DoEmit (int event_id, EmitContext* ctx, EventArgs *calldata = NULL, bool only_unemitted = false);
	void FinishEmit (int event_id, EmitContext* ctx);

 protected:
	virtual ~EventObject ();
	
	// To enable scenarios like Emit ("Event", new EventArgs ())
	// Emit will call unref on the calldata.
	bool Emit (char *event_name, EventArgs *calldata = NULL, bool only_unemitted = false);
	bool Emit (int event_id, EventArgs *calldata = NULL, bool only_unemitted = false);

 private:

	Surface *surface;
	gint32 refcount;
	EventLists *events;
};


struct PropertyChangedEventArgs {
	PropertyChangedEventArgs (DependencyProperty *p, Value *ov, Value *nv) : property(p), old_value(ov), new_value (nv) { }

	DependencyProperty *property;
	Value *old_value;
	Value *new_value;
};

class DependencyObject : public EventObject {
 public:
	DependencyObject ();

	DependencyProperty *GetDependencyProperty (const char *name);

	//
	// Returns true if a value is valid.  If the value is invalid return false.
	// If error is non NULL and the value is not valid, error will be given an error code and error message that should be
	// propogated to OnError
	//
	virtual bool IsValueValid (DependencyProperty *property, Value *value, GError **error);
	
	bool SetValue (DependencyProperty *property, Value *value, GError **error);
	bool SetValue (DependencyProperty *property, Value value, GError **error);
	void SetValue (DependencyProperty *property, Value *value);
	void SetValue (DependencyProperty *property, Value value);
	void SetValue (const char *name, Value *value);
	void SetValue (const char *name, Value value);

	virtual Value *GetDefaultValue (DependencyProperty *property);
	virtual Value *GetValue (DependencyProperty *property);
	Value *GetValueNoDefault (DependencyProperty *property);
	Value *GetValue (const char *name);

	void ClearValue (DependencyProperty *property, bool notify_listeners = true);
	bool HasProperty (const char *name, bool inherits);

	virtual Type::Kind GetObjectType ();

	DependencyObject *FindName (const char *name);
	NameScope *FindNameScope ();

	const char *GetName ()
	{
		Value *v = GetValue (DependencyObject::NameProperty);
		return v ? v->AsString () : NULL;
	}

	virtual void SetSurface (Surface *surface);

	void SetLogicalParent (DependencyObject *logical_parent);
	DependencyObject* GetLogicalParent ();


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
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args) { }

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

	void MergeTemporaryNameScopes (DependencyObject *dob);
	virtual void UnregisterAllNamesRootedAt (NameScope *from_ns);
	virtual void RegisterAllNamesRootedAt (NameScope *to_ns);

	static DependencyProperty *NameProperty;

	static DependencyProperty *Register (Type::Kind type, const char *name, Value *default_value);
	static DependencyProperty *Register (Type::Kind type, const char *name, Type::Kind vtype);
	static DependencyProperty *Register (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype);
	static DependencyProperty *RegisterNullable (Type::Kind type, const char *name, Type::Kind vtype);
	static DependencyProperty *RegisterFull (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly, bool always_change = false);
	static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name);
	static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name, bool inherits);

	static void Shutdown ();

 protected:
	virtual ~DependencyObject ();

	void NotifyListenersOfPropertyChange (PropertyChangedEventArgs *args);
	void NotifyListenersOfPropertyChange (DependencyProperty *property);

	void RemoveAllListeners ();

 private:
	static GHashTable *properties;
	GHashTable        *current_values;
	GSList            *listener_list;
	DependencyObject  *logical_parent;
};


//
// DependencyProperty
//
class DependencyProperty {
 public:
	DependencyProperty () {};
	~DependencyProperty ();
	DependencyProperty (Type::Kind type, const char *name, Value *default_value, Type::Kind value_type, bool attached, bool readonly, bool always_change);

	char *name;
	Value *default_value;
	Type::Kind type;
	bool is_attached_property;
	Type::Kind value_type;
	bool is_nullable;
	bool always_change; // determines if SetValue will do something if the current and new values are equal.

	bool IsNullable () { return is_nullable; }
	bool IsReadOnly () { return is_readonly; }

	AnimationStorage* AttachAnimationStorage (DependencyObject *obj, AnimationStorage *storage);
	void DetachAnimationStorage (DependencyObject *obj, AnimationStorage *storage);
	AnimationStorage* GetAnimationStorageFor (DependencyObject *obj);

 private:
	GHashTable *storage_hash; // keys: objects, values: animation storage's
	bool is_readonly;
};

G_BEGIN_DECLS

void base_ref (EventObject *obj);
void base_unref (EventObject *obj);

void drain_unrefs ();

Value *dependency_object_get_value (DependencyObject *object, DependencyProperty *prop);
Value *dependency_object_get_value_no_default (DependencyObject *object, DependencyProperty *prop);
void   dependency_object_set_value (DependencyObject *object, DependencyProperty *prop, Value *val);

DependencyObject *dependency_object_find_name (DependencyObject *obj, const char *name, Type::Kind *element_type);
const char       *dependency_object_get_name  (DependencyObject *obj);
void dependency_object_set_name (DependencyObject *obj, const char *name);

Type::Kind dependency_object_get_object_type (DependencyObject *obj);
const char *dependency_object_get_type_name (DependencyObject *obj);

DependencyProperty *dependency_property_lookup (Type::Kind type, char *name);
char *dependency_property_get_name (DependencyProperty* property);
bool  dependency_property_is_nullable (DependencyProperty* property);
Type::Kind dependency_property_get_value_type (DependencyProperty* property);
DependencyProperty *resolve_property_path (DependencyObject **o, const char *path);

void dependencyobject_init (void);

Surface *event_object_get_surface (EventObject *o);
void event_object_add_event_handler (EventObject *o, const char *event, EventHandler handler, gpointer closure);
void event_object_remove_event_handler (EventObject *o, const char *event, EventHandler handler, gpointer closure);

G_END_DECLS

#endif /* __MONO_DEPOBJECT_H__ */
