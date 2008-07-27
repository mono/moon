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
	CollectionChangedActionReset
};

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

class RoutedEventArgs : public EventArgs {

public:
	RoutedEventArgs ();

	virtual Type::Kind GetObjectType () { return Type::ROUTEDEVENTARGS; };

	DependencyObject* GetSource() { return source; }
	void SetSource(DependencyObject *el);

protected:
	virtual ~RoutedEventArgs ();

private:
	DependencyObject *source;
};

class KeyboardEventArgs : public RoutedEventArgs {
public:
	KeyboardEventArgs () { handled = false; }
	KeyboardEventArgs (int state_, int platformcode_, int key_) : 
		state (state_), platformcode (platformcode_), key (key_)
	{
	}
	virtual Type::Kind GetObjectType () { return Type::KEYBOARDEVENTARGS; };

	void SetHandled (bool handled) { this->handled = handled; }
	bool GetHandled () { return handled; }

	int state;
	int platformcode;
	int key;

protected:
	virtual ~KeyboardEventArgs () {}

private:
	bool handled;
};

class MouseEventArgs : public RoutedEventArgs {
public:
	MouseEventArgs ();
	MouseEventArgs (GdkEvent *event);
	virtual Type::Kind GetObjectType () { return Type::MOUSEEVENTARGS; };

	int GetState ();
	void GetPosition (UIElement *relative_to, double *x, double *y);
	StylusInfo *GetStylusInfo ();
	StylusPointCollection *GetStylusPoints (UIElement *ink_presenter);

	void SetHandled (bool handled) { this->handled = handled; }
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

MouseEventArgs*        mouse_event_args_new (void);
int                    mouse_event_args_get_state (MouseEventArgs *args);
void                   mouse_event_args_get_position (MouseEventArgs *args, UIElement *relative_to, double *x, double *y);
StylusInfo*            mouse_event_args_get_stylus_info (MouseEventArgs *args);
StylusPointCollection* mouse_event_args_get_stylus_points (MouseEventArgs *args, UIElement *ink_presenter);
bool                   mouse_event_args_get_handled (MouseEventArgs *args);
void                   mouse_event_args_set_handled (MouseEventArgs *args, bool handled);

bool                   keyboard_event_args_get_handled (KeyboardEventArgs *args);
void                   keyboard_event_args_set_handled (KeyboardEventArgs *args, bool handled);

ModifierKeys           keyboard_get_modifiers ();

RoutedEventArgs*  routed_event_args_new (void);
DependencyObject* routed_event_args_get_source (RoutedEventArgs *args);
void              routed_event_args_set_source (RoutedEventArgs *args, DependencyObject *source);

G_END_DECLS

#endif /* __MOON_EVENTARGS_H__ */
