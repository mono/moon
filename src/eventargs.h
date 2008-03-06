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

#include <stdint.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include "dependencyobject.h"

class StylusInfo;
class StylusPointCollection;
class UIElement;

/*
 * EventArgs needs to be ref counted since js can keep objects around until
 * after the event has been emitted.
 */

class EventArgs : public EventObject {
protected:
	virtual ~EventArgs () {};

public:
	EventArgs () {}
	virtual const char *GetTypeName () = 0;
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

	int state;
	int platformcode;
	int key;

	virtual const char *GetTypeName () { return "KeyboardEventArgs"; }
};

class MouseEventArgs : public EventArgs {
protected:
	virtual ~MouseEventArgs ();

public:
	MouseEventArgs (GdkEvent *event);

	int GetState ();
	void GetPosition (UIElement *relative_to, double *x, double *y);
	StylusInfo *GetStylusInfo ();
	StylusPointCollection *GetStylusPoints (UIElement *ink_presenter);

	virtual const char *GetTypeName () { return "MouseEventArgs"; }

 private:
	GdkEvent *event;
};

class MarkerReachedEventArgs : public EventArgs {
private:
	TimelineMarker *marker;

protected:
	virtual ~MarkerReachedEventArgs ();

public:
	MarkerReachedEventArgs (TimelineMarker *marker);

	TimelineMarker *GetMarker () { return marker; }

	virtual const char *GetTypeName () { return "MarkerReachedEventArgs"; }
};

G_BEGIN_DECLS

int                    mouse_event_args_get_state         (MouseEventArgs *args);
void                   mouse_event_args_get_position      (MouseEventArgs *args, UIElement *relative_to, double *x, double *y);
StylusInfo*            mouse_event_args_get_stylus_info   (MouseEventArgs *args);
StylusPointCollection* mouse_event_args_get_stylus_points (MouseEventArgs *args, UIElement *ink_presenter);

G_END_DECLS

#endif /* __MOON_EVENTARGS_H__ */

