/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * eventargs.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __EVENTARGS_H__
#define __EVENTARGS_H__

#include <glib.h>

#include <cairo.h>
#include <gdk/gdkevents.h>
#include "dependencyobject.h"
#include "keyboard.h"
#include "enums.h"

class StylusInfo;
class StylusPointCollection;
class UIElement;

/*
 * EventArgs needs to be ref counted since js can keep objects around until
 * after the event has been emitted.
 */

/* @Namespace=None */
class EventArgs : public DependencyObject {
 protected:
	virtual ~EventArgs () {}
	
 public:
	EventArgs () {}
	virtual Type::Kind GetObjectType () { return Type::EVENTARGS; }
};

enum CollectionChangedAction {
	CollectionChangedActionAdd,
	CollectionChangedActionRemove,
	CollectionChangedActionReplace,
	CollectionChangedActionClearing,
	CollectionChangedActionCleared,
};

/* @Namespace=None */
class CollectionChangedEventArgs : public EventArgs {
 protected:
	virtual ~CollectionChangedEventArgs () {}
	
 public:
	CollectionChangedAction action;
	Value *old_value;
	Value *new_value;
	int index;
	
	/* @GenerateCBinding,GeneratePInvoke */
	CollectionChangedEventArgs ()
	{
		action = CollectionChangedActionAdd;
		old_value = NULL;
		new_value = NULL;
		index = -1;
	}
	
	CollectionChangedEventArgs (CollectionChangedAction action, Value *new_value, Value *old_value, int index)
	{
		this->action = action;
		this->new_value = new_value;
		this->old_value = old_value;
		this->index = index;
	}
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetChangedAction (CollectionChangedAction action) { this->action = action; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	CollectionChangedAction GetChangedAction () { return action; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetNewItem (Value *item) { new_value = item; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	Value *GetNewItem () { return new_value; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetOldItem (Value *item) { old_value = item; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	Value *GetOldItem () { return old_value; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetIndex (int index) { this->index = index; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	int GetIndex () { return index; }
};

/* @Namespace=None */
class RoutedEventArgs : public EventArgs {
	DependencyObject *source;
	bool handled;
	
 protected:
	virtual ~RoutedEventArgs ();
	
 public:
 	/* @GenerateCBinding,GeneratePInvoke */
	RoutedEventArgs ();

	virtual Type::Kind GetObjectType () { return Type::ROUTEDEVENTARGS; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObject* GetSource() { return source; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetSource(DependencyObject *el);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetHandled (bool handled) { this->handled = handled; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	bool GetHandled () { return handled; }
};

/* @Namespace=None */
class KeyEventArgs : public RoutedEventArgs {
	GdkEventKey *event;
	
 protected:
	virtual ~KeyEventArgs ();
	
 public:
 	/* @GenerateCBinding,GeneratePInvoke */
	KeyEventArgs ();
	KeyEventArgs (GdkEventKey *event);
	virtual Type::Kind GetObjectType () { return Type::KEYEVENTARGS; };
	
	/* @GenerateCBinding,GeneratePInvoke */
	int GetKey ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	int GetPlatformKeyCode ();
	
	// accessors for the native GdkEventKey
	GdkModifierType GetModifiers ();
	gunichar GetUnicode ();
	guint GetKeyVal ();
	bool IsModifier ();
};

/* @Namespace=None */
class MouseEventArgs : public RoutedEventArgs {
	GdkEvent *event;
	
 protected:
	virtual ~MouseEventArgs ();
	
 public:
 	/* @GenerateCBinding,GeneratePInvoke */
	MouseEventArgs ();
	MouseEventArgs (GdkEvent *event);
	virtual Type::Kind GetObjectType () { return Type::MOUSEEVENTARGS; };
	
	int GetState ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void GetPosition (UIElement *relative_to, double *x, double *y);
	
	StylusInfo *GetStylusInfo ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	StylusPointCollection *GetStylusPoints (UIElement *ink_presenter);
};

#endif /* __EVENTARGS_H__ */
