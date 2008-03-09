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

typedef void (* EventHandler) (EventObject *sender, EventArgs *args, gpointer closure);

struct EventList {
	int current_token;
	List *event_list;
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

#if OBJECT_TRACKING
	void weak_ref (EventObject* base);
	void weak_unref (EventObject* base);
#endif

	void ref ()
	{
		g_atomic_int_inc (&refcount);
		OBJECT_TRACK ("Ref", GetTypeName ());
	}
	
	void unref ()
	{
		OBJECT_TRACK ("Unref", GetTypeName ());

		if (g_atomic_int_dec_and_test (&refcount)) {
			Emit (DestroyedEvent);
			if (refcount != 0) {
#if OBJECT_TRACKING
				g_warning ("Object %i of type %s has been woken up from the dead.\n", id, GetTypeName ());
#else
				g_warning ("Object %p of type %s has been woken up from the dead.\n", this, GetTypeName ());
#endif
			}
			delete this;
		}
	}

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
	
	char *GetTypeName ()
	{
		return Type::Find (GetObjectType ())->name;
	}

	int AddHandler (const char *event_name, EventHandler handler, gpointer data);
	void RemoveHandler (const char *event_name, EventHandler handler, gpointer data);
	void RemoveHandler (const char *event_name, int token);

	int AddHandler (int event_id, EventHandler handler, gpointer data);
	void RemoveHandler (int event_id, EventHandler handler, gpointer data);
	void RemoveHandler (int event_id, int token);

	virtual Type::Kind GetObjectType () { return Type::EVENTOBJECT; }

	static int DestroyedEvent;


 protected:
	virtual ~EventObject ();

	// To enable scenarios like Emit ("Event", new EventArgs ())
	// Emit will call unref on the calldata.
	bool Emit (char *event_name, EventArgs *calldata = NULL);
	bool Emit (int event_id, EventArgs *calldata = NULL);

 private:

	gint32 refcount;

	void FreeHandlers ();
	
	EventList *events;

#if OBJECT_TRACKING
	static int objects_created;
	static int objects_destroyed;
	static GHashTable* objects_alive;
	int id;
	GHashTable* weak_refs;

	char* GetStackTrace (const char* prefix);
	char* GetStackTrace () { return GetStackTrace (""); }
	void PrintStackTrace ();
	void Track (const char* done, const char* typname);
#endif
};


struct PropertyChangedEventArgs {
	PropertyChangedEventArgs (DependencyProperty *p, Value *ov, Value *nv) : property(p), old_value(ov), new_value (nv) { }

	DependencyProperty *property;
	Value *old_value;
	Value *new_value;
};

class DependencyObject : public EventObject {
 protected:
	virtual ~DependencyObject ();

 public:

	DependencyObject ();
	static DependencyProperty *Register (Type::Kind type, const char *name, Value *default_value);
	static DependencyProperty *Register (Type::Kind type, const char *name, Type::Kind vtype);
	static DependencyProperty *Register (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype);
	static DependencyProperty *RegisterNullable (Type::Kind type, const char *name, Type::Kind vtype);
	static DependencyProperty *RegisterFull (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached, bool readonly);
	
	static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name);
	static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name, bool inherits);
	DependencyProperty *GetDependencyProperty (const char *name);

	virtual void SetValue (DependencyProperty *property, Value *value);
	virtual void SetValue (DependencyProperty *property, Value value);
	void SetValue (const char *name, Value *value);
	void SetValue (const char *name, Value value);

	virtual Value *GetDefaultValue (DependencyProperty *property);
	virtual Value *GetValue (DependencyProperty *property);
	Value *GetValueNoDefault (DependencyProperty *property);
	Value *GetValue (const char *name);

	void ClearValue (DependencyProperty *property);
	bool HasProperty (const char *name, bool inherits);

	static GHashTable *properties;

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	// See the comment below about AddPropertyChangeListener for
	// the meaning of the @prop arg in this method.  it's not what
	// you might think it is.
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args) { }

	//
	// OnCollectionChanged:
	//
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args) { }

	virtual Type::Kind GetObjectType ();

	DependencyObject *FindName (const char *name);
	NameScope *FindNameScope ();

	const char *GetName ()
	{
		Value *v = GetValue (DependencyObject::NameProperty);
		return v ? v->AsString () : NULL;
	}
	
	void SetLogicalParent (DependencyObject *logical_parent);
	DependencyObject* GetLogicalParent ();

	static void Shutdown ();

	// These methods are a little confusing.  @child_property is
	// *not* the property you're interested in receiving change
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
	
	virtual Surface *GetSurface () { return NULL; }
	virtual void SetSurface (Surface *surface) { } // Do nothing here.

	static DependencyProperty *NameProperty;

 protected:
	void NotifyListenersOfPropertyChange (PropertyChangedEventArgs *args);
	void NotifyListenersOfPropertyChange (DependencyProperty *property);

	void RemoveAllListeners ();

 private:
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
	DependencyProperty (Type::Kind type, const char *name, Value *default_value, Type::Kind value_type, bool attached, bool readonly);

	char *name;
	Value *default_value;
	Type::Kind type;
	bool is_attached_property;
	Type::Kind value_type;
	bool is_nullable;

	bool IsNullable () { return is_nullable; }
	bool IsReadOnly () { return is_readonly; }

 private:
	bool is_readonly;
};

G_BEGIN_DECLS

void base_ref (EventObject *obj);
void base_unref (EventObject *obj);
void base_unref_delayed (EventObject *obj);

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


void event_object_add_event_handler (EventObject *o, const char *event, EventHandler handler, gpointer closure);
void event_object_remove_event_handler (EventObject *o, const char *event, EventHandler handler, gpointer closure);

G_END_DECLS

#endif /* __MONO_DEPOBJECT_H__ */

