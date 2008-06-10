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

class StylusInfo;
class StylusPointCollection;
class UIElement;

/*
 * EventArgs needs to be ref counted since js can keep objects around until
 * after the event has been emitted.
 */

class EventArgs : public DependencyObject {
protected:
	virtual ~EventArgs () {};

public:
	EventArgs () {}
	virtual Type::Kind GetObjectType () { return Type::EVENTARGS; };
};


class KeyboardEventArgs : public EventArgs {
protected:
	virtual ~KeyboardEventArgs () {}

public:
	KeyboardEventArgs () {}
	KeyboardEventArgs (int state_, int platformcode_, int key_) : 
		state (state_), platformcode (platformcode_), key (key_)
	{
	}
	virtual Type::Kind GetObjectType () { return Type::KEYBOARDEVENTARGS; };

	int state;
	int platformcode;
	int key;

};

class MouseEventArgs : public EventArgs {
protected:
	virtual ~MouseEventArgs ();

public:
	MouseEventArgs ();
	MouseEventArgs (GdkEvent *event);
	virtual Type::Kind GetObjectType () { return Type::MOUSEEVENTARGS; };

	int GetState ();
	void GetPosition (UIElement *relative_to, double *x, double *y);
	StylusInfo *GetStylusInfo ();
	StylusPointCollection *GetStylusPoints (UIElement *ink_presenter);

 private:
	GdkEvent *event;
};

G_BEGIN_DECLS

MouseEventArgs*        mouse_event_args_new               (void);
int                    mouse_event_args_get_state         (MouseEventArgs *args);
void                   mouse_event_args_get_position      (MouseEventArgs *args, UIElement *relative_to, double *x, double *y);
StylusInfo*            mouse_event_args_get_stylus_info   (MouseEventArgs *args);
StylusPointCollection* mouse_event_args_get_stylus_points (MouseEventArgs *args, UIElement *ink_presenter);

G_END_DECLS

#endif /* __MOON_EVENTARGS_H__ */

