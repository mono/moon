/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * eventargs.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_EVENTARGS_H__
#define __MOON_EVENTARGS_H__

#include <glib.h>

#include <cairo.h>
#include <gdk/gdkevents.h>
#include "dependencyobject.h"
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
public:
	EventArgs () {}
	virtual Type::Kind GetObjectType () { return Type::EVENTARGS; };

protected:
	virtual ~EventArgs () {};
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
public:
	CollectionChangedAction action;
	Value *old_value;
	Value *new_value;
	int index;
	
	CollectionChangedEventArgs (CollectionChangedAction action, Value *new_value, Value *old_value, int index)
	{
		this->action = action;
		this->new_value = new_value;
		this->old_value = old_value;
		this->index = index;
	}
};

/* @Namespace=None */
class RoutedEventArgs : public EventArgs {

public:
 	/* @GenerateCBinding,GeneratePInvoke */
	RoutedEventArgs ();

	virtual Type::Kind GetObjectType () { return Type::ROUTEDEVENTARGS; };
	
	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObject* GetSource() { return source; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetSource(DependencyObject *el);

protected:
	virtual ~RoutedEventArgs ();

private:
	DependencyObject *source;
};

/* @Namespace=None */
class KeyEventArgs : public RoutedEventArgs {
public:
 	/* @GenerateCBinding,GeneratePInvoke */
	KeyEventArgs ();
	KeyEventArgs (GdkEventKey *event);
	virtual Type::Kind GetObjectType () { return Type::KEYEVENTARGS; };
	
	int GetState ();

	/* @GenerateCBinding,GeneratePInvoke */
	int GetKey ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	int GetPlatformKeyCode ();

	/* @GenerateCBinding,GeneratePInvoke */
	void SetHandled (bool handled) { this->handled = handled; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	bool GetHandled () { return handled; }
	
	static int gdk_keyval_to_key (guint keyval);

protected:
	virtual ~KeyEventArgs ();

private:
	GdkEventKey *event;
	bool handled;
};

/* @Namespace=None */
class MouseEventArgs : public RoutedEventArgs {
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
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetHandled (bool handled) { this->handled = handled; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	bool GetHandled () { return handled; }

protected:
	virtual ~MouseEventArgs ();

private:
	GdkEvent *event;
	bool handled;
};

class Keyboard {
public:
	static ModifierKeys Modifiers;
};

G_BEGIN_DECLS

ModifierKeys keyboard_get_modifiers (void);

G_END_DECLS

#endif /* __MOON_EVENTARGS_H__ */
