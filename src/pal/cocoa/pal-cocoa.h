/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_PAL_COCOA_H
#define MOON_PAL_COCOA_H

#include "pal.h"

#include "mutex.h"

namespace Moonlight {

class MoonWindowingSystemCocoa : public MoonWindowingSystem {
public:
	MoonWindowingSystemCocoa (bool out_of_browser);
	virtual ~MoonWindowingSystemCocoa ();

	// creates a platform/windowing system specific surface
	virtual cairo_surface_t *CreateSurface ();
	virtual void ExitApplication ();

	MoonWindow *CreateWindow (MoonWindowType windowType, int width, int height, MoonWindow *parentWindow, Surface *surface);
	MoonWindow *CreateWindowless (int width, int height, PluginInstance *forPlugin);

	virtual MoonMessageBoxResult ShowMessageBox (MoonMessageBoxType message_type, const char *caption, const char *text, MoonMessageBoxButton button);

	virtual bool ShowConsentDialog (const char *question, const char *detail, const char *website, bool *remember);

	virtual gchar** ShowOpenFileDialog (const char *title, bool multsel, const char *filter, int idx);
	virtual char* ShowSaveFileDialog (const char *title, const char *filter, int idx);
	
	virtual Color *GetSystemColor (SystemColor id);

	virtual guint AddTimeout (gint priority, gint ms, MoonSourceFunc timeout, gpointer data);
	virtual void RemoveTimeout (guint timeoutId);

	virtual guint AddIdle (MoonSourceFunc idle, gpointer data);
	virtual void RemoveIdle (guint idle_id);

	virtual MoonIMContext* CreateIMContext ();

	virtual MoonEvent* CreateEventFromPlatformEvent (gpointer platformEvent);

	virtual guint GetCursorBlinkTimeout (MoonWindow *window);

	virtual MoonPixbufLoader* CreatePixbufLoader (const char *imageType);

	virtual void RunMainLoop (MoonWindow *main_window = NULL, bool quit_on_window_close = true);

	void UnregisterWindow (MoonWindow *window);

	virtual void ShowCodecsUnavailableMessage ();

	virtual guint32 GetScreenHeight (MoonWindow *moon_window);

	virtual guint32 GetScreenWidth (MoonWindow *moon_window);

	virtual bool ConvertJPEGToBGRA (void *jpeg, guint32 jpeg_size, guint8 *buffer, guint32 buffer_stride, guint32 buffer_height);

	virtual gchar *GetTemporaryFolder ();

	virtual gchar *GetUserConfigFolder ();

	void OnTick ();

private:
	void RemoveSource (guint sourceId);
	void AddCocoaTimer ();

	void *pool;
	int stride;

	// glib terminology here.  we mean "timeouts and idles"
	GList *sources;
	guint source_id;
	bool emitting_sources;
	Mutex sourceMutex;
	guint32 before;
};

class MoonInstallerServiceCocoa : public MoonInstallerService {
	char *base_install_dir;
	
protected:
	virtual const char *GetBaseInstallDir ();
	
public:
	MoonInstallerServiceCocoa ();
	virtual ~MoonInstallerServiceCocoa ();
	
	virtual bool Install (Deployment *deployment, bool unattended);
	virtual bool Uninstall (Deployment *deployment);
};

class MoonFontServiceCocoa : public MoonFontService {
	GPtrArray *system_fonts;
	
public:
	MoonFontServiceCocoa ();
	~MoonFontServiceCocoa ();
	
	virtual void ForeachFont (MoonForeachFontCallback foreach, gpointer user_data);
	virtual MoonFont *FindFont (const FontStyleInfo *pattern);
};
};
#endif /* MOON_PAL_COCOA_H */
