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

namespace Moonlight {

class CollectionIterator;

//
// Collection: provides a collection that we can monitor for
// changes.   We expose this collection in a few classes to
// the managed world, and when a change happens we get a
// chance to reflect the changes
//
/* @Namespace=System.Windows */
/* @ManagedName=PresentationFrameworkCollection`1 */
/* @CallInitialize */
/* @ManagedDependencyProperties=Manual */
/* @ManagedEvents=Manual */
class Collection : public DependencyObject {
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
	
#if EVENT_ARG_REUSE
	CollectionItemChangedEventArgs *itemChangedEventArgs;
	CollectionChangedEventArgs *changedEventArgs;
#endif

	void EmitChanged (CollectionChangedAction action, Value *new_value, Value *old_value, int index);
	void EmitItemChanged (DependencyObject *object, DependencyProperty *property, Value *newValue, Value *oldValue);

	virtual bool CanAdd (Value *value);
	virtual bool AddedToCollection (Value *value, MoonError *error) { return true; }
	virtual void RemovedFromCollection (Value *value) {}
	
	void SetCount (int count);

	virtual void CloneCore (Types *types, DependencyObject* fromObj);

	/* @SkipFactories */
	Collection ();
	virtual ~Collection ();
	virtual void Dispose ();
	
};

/* @Namespace=System.Windows */
/* @ManagedName=DependencyObjectCollection`1 */
class DependencyObjectCollection : public Collection {
public:
	virtual Type::Kind GetElementType () { return Type::DEPENDENCY_OBJECT; }
	
	virtual void OnIsAttachedChanged (bool value);
	
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *args);
	virtual void UnregisterAllNamesRootedAt (NameScope *from_ns);
	virtual void RegisterAllNamesRootedAt (NameScope *to_ns, MoonError *error);
	
protected:
	virtual bool AddedToCollection (Value *value, MoonError *error);
	virtual void RemovedFromCollection (Value *value);
	
	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObjectCollection ();
	
	virtual void OnMentorChanged (DependencyObject *old_mentor, DependencyObject *new_mentor);
	virtual ~DependencyObjectCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media */
class DoubleCollection : public Collection {
public:
	virtual Type::Kind GetElementType () { return Type::DOUBLE; }

	static DoubleCollection* FromStr (const char *str);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	DoubleCollection ();
	
	virtual ~DoubleCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media */
class PointCollection : public Collection {
public:
	virtual Type::Kind GetElementType () { return Type::POINT; }

	static PointCollection* FromStr (const char *str);

protected:
	/* @GenerateCBinding,GeneratePInvoke */
	PointCollection ();

	virtual ~PointCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
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
	VisualTreeWalker (UIElement *item, VisualTreeWalkerDirection direction = Logical, bool ref_content = true, Types *types = NULL);

	~VisualTreeWalker ();

	UIElement *Step ();
	int GetCount ();

protected:
	DependencyObject *content;
	Collection *collection;
	Types *types;
	int index;
	VisualTreeWalkerDirection direction;
	bool ref_content;
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
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	TriggerCollection ();
	
	virtual ~TriggerCollection ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	virtual Type::Kind GetElementType () { return Type::EVENTTRIGGER; }
};

/* @Namespace=System.Windows */
class TriggerActionCollection : public DependencyObjectCollection {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	TriggerActionCollection ();
	
	virtual ~TriggerActionCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	/* this may seem wrong, but it's what the TriggerActionCollection mandates */
	virtual Type::Kind GetElementType () { return Type::BEGINSTORYBOARD; }
};

/* @Namespace=System.Windows.Documents */
class InlineCollection : public DependencyObjectCollection {
 protected:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	InlineCollection ();

	virtual ~InlineCollection ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
	virtual Type::Kind GetElementType () { return Type::INLINE; }
	
	bool Equals (InlineCollection *inlines);
};

/* @Namespace=System.Windows.Controls */
class UIElementCollection : public DependencyObjectCollection {
 protected:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	UIElementCollection ();

	virtual ~UIElementCollection ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	GPtrArray *z_sorted;
	
	virtual Type::Kind GetElementType () { return Type::UIELEMENT; }
	
	virtual bool Clear ();
	
	void ResortByZIndex ();
};

/* @Namespace=System.Windows.Controls */
class ItemCollection : public Collection {
 protected:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	ItemCollection ();
	
	virtual ~ItemCollection ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	virtual Type::Kind GetElementType () { return Type::OBJECT; }
};

/* @Namespace=System.Windows.Controls */
class MultiScaleSubImageCollection : public DependencyObjectCollection {
public:
	GPtrArray *z_sorted;

	virtual Type::Kind GetElementType () { return Type::MULTISCALESUBIMAGE; }

	virtual bool Clear ();

	void ResortByZIndex ();

protected:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	MultiScaleSubImageCollection ();
	
	virtual ~MultiScaleSubImageCollection ();
	
	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Controls */
class HitTestCollection : public UIElementCollection {
 protected:
	virtual bool AddedToCollection (Value *value, MoonError *error) { return true; }
	virtual void RemovedFromCollection (Value *value) {}

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	HitTestCollection ();

	virtual ~HitTestCollection () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows */
class ResourceDictionaryCollection : public DependencyObjectCollection {
 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	ResourceDictionaryCollection ();
	
	virtual ~ResourceDictionaryCollection ();

	virtual bool AddedToCollection (Value *value, MoonError *error);

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

 public:
	virtual Type::Kind GetElementType () { return Type::RESOURCE_DICTIONARY; }
};

/* @Namespace=System.Windows.Documents */
/* @ManagedName=TextElementCollection`1 */
class TextElementCollection : public DependencyObjectCollection {
protected:
	/* @GenerateCBinding,GeneratePInvoke */
	TextElementCollection ();

	virtual ~TextElementCollection () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
 
public:
	virtual Type::Kind GetElementType () { return Type::TEXTELEMENT; }
};

/* @Namespace=System.Windows.Documents */
class BlockCollection : public TextElementCollection {
protected:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	BlockCollection ();
 
	virtual ~BlockCollection () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
 
public:
	virtual Type::Kind GetElementType () { return Type::BLOCK; }
};

G_BEGIN_DECLS

Collection *collection_new (Type::Kind kind);

G_END_DECLS

};

#endif /* __MOON_COLLECTION_H__ */
