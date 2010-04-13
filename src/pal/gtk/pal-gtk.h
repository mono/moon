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
	
	virtual Color *GetSystemColor (SystemColor id);
	
	virtual guint AddTimeout (gint priority, gint ms, MoonSourceFunc timeout, gpointer data);
	virtual void RemoveTimeout (guint timeoutId);

	virtual MoonIMContext* CreateIMContext ();

	virtual MoonEvent* CreateEventFromPlatformEvent (gpointer platformEvent);

	virtual guint GetCursorBlinkTimeout (MoonWindow *window);

	virtual MoonPixbufLoader* CreatePixbufLoader (const char *imageType);


	void UnregisterWindow (MoonWindow *window);

private:
	Color *system_colors[NumSystemColors];
	
	void LoadSystemColors ();
	
	void RegisterWindow (MoonWindow *window);

	bool RunningOnNvidia ();
};

class MoonInstallerServiceGtk : public MoonInstallerService {
protected:
	virtual char *GetUpdateUri (Deployment *deployment);
	virtual time_t GetLastModified (Deployment *deployment);
	virtual char *GetTmpFilename (Deployment *deployment);
	virtual char *GetXapFilename (Deployment *deployment);
	
public:
	MoonInstallerServiceGtk () {}
	virtual ~MoonInstallerServiceGtk () {}
	
	virtual bool IsRunningOutOfBrowser (Deployment *deployment);
	virtual bool CheckInstalled (Deployment *deployment);
	virtual void Uninstall (Deployment *deployment);
	virtual bool Install (Deployment *deployment);
};

#endif /* MOON_PAL_GTK_H */
