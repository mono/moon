/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_PAL_GTK_H
#define MOON_PAL_GTK_H

#include "pal.h"

class MoonWindowingSystemGtk : public MoonWindowingSystem {
public:
	// creates a platform/windowing system specific surface
	virtual cairo_surface_t *CreateSurface ();

	MoonWindow *CreateWindow (bool fullscreen, int width, int height, MoonWindow *parentWindow, Surface *surface);
	MoonWindow *CreateWindowless (int width, int height, PluginInstance *forPlugin);

	virtual guint AddTimeout (gint priority, gint ms, MoonSourceFunc timeout, gpointer data);
	virtual void RemoveTimeout (guint timeoutId);

	virtual MoonEvent* CreateEventFromPlatformEvent (gpointer platformEvent);

	virtual MoonIMContext* CreateIMContext ();

	void UnregisterWindow (MoonWindow *window);

	guint GetCursorBlinkTimeout (MoonWindow *window);

private:
	void RegisterWindow (MoonWindow *window);
};

#endif
