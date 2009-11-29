/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_PAL_GTK_H
#define MOON_PAL_GTK_H

#include "pal.h"

class MoonWindowingSystemGtk : public MoonWindowingSystem {
public:
	MoonWindowingSystemGtk ();
	virtual ~MoonWindowingSystemGtk ();

	// creates a platform/windowing system specific surface
	virtual cairo_surface_t *CreateSurface ();

	MoonWindow *CreateWindow (bool fullscreen, int width, int height, MoonWindow *parentWindow, Surface *surface);
	MoonWindow *CreateWindowless (int width, int height, PluginInstance *forPlugin);

	virtual int ShowMessageBox (const char *caption, const char *text, int buttons);

	virtual gchar** ShowOpenFileDialog (const char *title, bool multsel, const char *filter, int idx);
	virtual char* ShowSaveFileDialog (const char *title, const char *filter, int idx);

	virtual guint AddTimeout (gint priority, gint ms, MoonSourceFunc timeout, gpointer data);
	virtual void RemoveTimeout (guint timeoutId);

	virtual MoonIMContext* CreateIMContext ();

	virtual MoonEvent* CreateEventFromPlatformEvent (gpointer platformEvent);

	virtual guint GetCursorBlinkTimeout (MoonWindow *window);

	virtual MoonPixbufLoader* CreatePixbufLoader (const char *imageType);


	void UnregisterWindow (MoonWindow *window);

private:
	void RegisterWindow (MoonWindow *window);

	bool RunningOnNvidia ();
};

#endif
