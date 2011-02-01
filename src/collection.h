/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * collection.h: different types of collections
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_COLLECTION_H__
#define __MOON_COLLECTION_H__

#include <glib.h>

#include "dependencyobject.h"
#include "eventargs.h"
#include "point.h"

class CollectionIterator;

//
// Collection: provides a collection that we can monitor for
// changes.   We expose this collection in a few classes to
// the managed world, and when a change happens we get a
// chance to reflect the changes
//
/* @Namespace=System.Windows */
/* @ManagedName=PresentationFrameworkCollection`1 */
/* @ManagedDependencyProperties=Manual */
/* @ManagedEvents=Manual */
class MOON_API Collection : public DependencyObject {
public:
 	/* @PropertyType=gint32,DefaultValue=0,GenerateAccessors */
	const static int CountProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	virtual Type::Kind GetElementType () = 0;
	
	int Generation () { return generation; }
	GPtrArray *Array () { return array; }

	/* @GenerateCBinding,GeneratePInvoke */
	virtual CollectionIterator *GetIterator ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual int GetCount ();
	
	int Add (Value value);
	int Add (Value *value);
	
	/* @GenerateCBinding,GeneratePInvoke */
	virtual bool Clear ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	bool Contains (Value *value);
	/* @GenerateCBinding,GeneratePInvoke */
	int IndexOf (Value *value);
	
	bool Insert (int index, Value value);
	bool Insert (int index, Value *value);
	
	bool Remove (Value value);
	/* @GenerateCBinding,GeneratePInvoke */
	bool Remove (Value *value);
	
	bool RemoveAt (int index);
	
	bool SetValueAt (int index, Value *value);
	Value *GetValueAt (int index);

	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	virtual int AddWithError (Value *value, MoonError *error);
	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	virtual bool InsertWithError (int index, Value *value, MoonError *error);
	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	virtual Value *GetValueAtWithError (int index, MoonError *error);
	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	virtual bool SetValueAtWithError (int index, Value *value, MoonError *error);
	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	virtual bool RemoveAtWithError (int index, MoonError *error);

	const static int ChangedEvent;
	const static int ItemChangedEvent;

protected:
	GPtrArray *array;
	int generation;
	
	void EmitChanged (CollectionChangedAction action, Value *new_value, Value *old_value, int index);
	void EmitItemChanged (DependencyObject *object, DependencyProperty *property, Value *newValue, Value *oldValue);
	
	virtual bool CanAdd (Value *value);
	virtual bool AddedToCollection (Value *value, MoonError *error) { return true; }
	virtual void RemovedFromCollection (Value *value) {}
	
	void SetCount (int count);

	virtual void CloneCore (Types *types, DependencyObject* fromObj);

	Collection ();
	virtual ~Collection ();
	virtual void Dispose ();
	
};

/* @Namespace=None */
class DependencyObjectCollection : public Collection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObjectCollection ();
	
	virtual Type::Kind GetElementType () { return Type::DEPENDENCY_OBJECT; }
	
	virtual void SetSurface (Surface *surface);
	
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void UnregisterAllNamesRootedAt (NameScope *from_ns);
	virtual void RegisterAllNamesRootedAt (NameScope *to_ns, MoonError *error);
	
protected:
	virtual bool AddedToCollection (Value *value, MoonError *error);
	virtual void RemovedFromCollection (Value *value);
	
	virtual ~DependencyObjectCollection ();
};

/* @Namespace=System.Windows.Media */
class DoubleCollection : public Collection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	DoubleCollection ();
	
	virtual Type::Kind GetElementType () { return Type::DOUBLE; }

	static DoubleCollection* FromStr (const char *str);

protected:
	virtual ~DoubleCollection ();
};

/* @Namespace=System.Windows.Media */
class PointCollection : public Collection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	PointCollection ();
	
	virtual Type::Kind GetElementType () { return Type::POINT; }

	static PointCollection* FromStr (const char *str);

protected:
	virtual ~PointCollection ();
};

class CollectionIterator {
public:
	CollectionIterator (Collection *c);
	virtual ~CollectionIterator ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	virtual bool Next (MoonError *error);
	
	/* @GenerateCBinding,GeneratePInvoke */
	virtual bool Reset ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	virtual Value *GetCurrent (MoonError *error);
	
	/* @GenerateCBinding,GeneratePInvoke */
	static void Destroy (CollectionIterator *iterator);

protected:
	Collection *collection;
	int generation;
	int index;
};

enum VisualTreeWalkerDirection {
	Logical,
	LogicalReverse,
	ZForward,
	ZReverse
};

class VisualTreeWalker {
public:
	VisualTreeWalker (UIElement *item, VisualTreeWalkerDirection direction = Logical, Types *types = NULL);

	~VisualTreeWalker ();

	UIElement *Step ();
	int GetCount ();

protected:
	DependencyObject *content;
	Collection *collection;
	Types *types;
	int index;
	VisualTreeWalkerDirection direction;
};

class DeepTreeWalker {
public:
	DeepTreeWalker (UIElement *top, VisualTreeWalkerDirection direction = Logical, Types *types = NULL);
	UIElement *Step ();
	void SkipBranch ();
	~DeepTreeWalker ();
protected:
	List *walk_list;
	Types *types;
	UIElement *last;
	VisualTreeWalkerDirection direction;
};

/* @Namespace=System.Windows */
class TriggerCollection : public DependencyObjectCollection {
 protected:
	virtual ~TriggerCollection ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	TriggerCollection ();
	
	virtual Type::Kind GetElementType () { return Type::EVENTTRIGGER; }
};

/* @Namespace=System.Windows */
class TriggerActionCollection : public DependencyObjectCollection {
 protected:
	virtual ~TriggerActionCollection ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	TriggerActionCollection ();
	
	/* this may seem wrong, but it's what the TriggerActionCollection mandates */
	virtual Type::Kind GetElementType () { return Type::BEGINSTORYBOARD; }
};

/* @Namespace=System.Windows.Documents */
class InlineCollection : public DependencyObjectCollection {
 protected:
	virtual ~InlineCollection ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	InlineCollection ();
	
	virtual Type::Kind GetElementType () { return Type::INLINE; }
	
	bool Equals (InlineCollection *inlines);
};

/* @Namespace=System.Windows.Controls */
class UIElementCollection : public DependencyObjectCollection {
 protected:
	virtual ~UIElementCollection ();
	
 public:
	GPtrArray *z_sorted;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	UIElementCollection ();
	
	virtual Type::Kind GetElementType () { return Type::UIELEMENT; }
	
	virtual bool Clear ();
	
	void ResortByZIndex ();
};

/* @Namespace=System.Windows.Controls */
class ItemCollection : public Collection {
 protected:
	virtual ~ItemCollection ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	ItemCollection ();
	
	virtual Type::Kind GetElementType () { return Type::OBJECT; }
};

/* @Namespace=System.Windows.Controls */
class MultiScaleSubImageCollection : public DependencyObjectCollection {
public:
	GPtrArray *z_sorted;

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	MultiScaleSubImageCollection ();
	
	virtual Type::Kind GetElementType () { return Type::MULTISCALESUBIMAGE; }

	virtual bool Clear ();

	void ResortByZIndex ();

protected:
	virtual ~MultiScaleSubImageCollection ();
};

/* @Namespace=System.Windows.Controls */
class HitTestCollection : public UIElementCollection {
 protected:
	virtual bool AddedToCollection (Value *value, MoonError *error) { return true; }
	virtual void RemovedFromCollection (Value *value) {}
	virtual ~HitTestCollection () {}
 public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	HitTestCollection ();
};

/* @Namespace=System.Windows */
class ResourceDictionaryCollection : public DependencyObjectCollection {
 protected:
	virtual ~ResourceDictionaryCollection ();

 public:
	/* @GenerateCBinding,GeneratePInvoke */
	ResourceDictionaryCollection ();
	
	virtual Type::Kind GetElementType () { return Type::RESOURCE_DICTIONARY; }
};

G_BEGIN_DECLS

Collection *collection_new (Type::Kind kind);

G_END_DECLS

#endif /* __MOON_COLLECTION_H__ */
