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

struct KeyboardEventArgs {
	int state;
	int platformcode;
	int key;
};

class MouseEventArgs : public EventObject {
public:
	MouseEventArgs (GdkEvent *event);
	~MouseEventArgs ();

	int GetState ();
	void GetPosition (UIElement *relative_to, double *x, double *y);
	StylusInfo *GetStylusInfo ();
	StylusPointCollection *GetStylusPoints (UIElement *ink_presenter);

 private:
	GdkEvent *event;
};

G_BEGIN_DECLS

int                    mouse_event_args_get_state         (MouseEventArgs *args);
void                   mouse_event_args_get_position      (MouseEventArgs *args, UIElement *relative_to, double *x, double *y);
StylusInfo*            mouse_event_args_get_stylus_info   (MouseEventArgs *args);
StylusPointCollection* mouse_event_args_get_stylus_points (MouseEventArgs *args, UIElement *ink_presenter);

G_END_DECLS

#endif /* __MOON_EVENTARGS_H__ */

