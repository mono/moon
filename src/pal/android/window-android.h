/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * window-android.h: MoonWindow implementation using android widgets.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_WINDOW_ANDROID_H__
#define __MOON_WINDOW_ANDROID_H__

#include <glib.h>

#include "window.h"
#include "runtime.h"

#include "pal-android.h"

#ifdef USE_GALLIUM
struct pipe_screen;
#endif

namespace Moonlight {

#if USE_EGL
class MoonEGLSurface;
class MoonEGLContext;
#endif

/* @Namespace=System.Windows */
class MoonWindowAndroid : public MoonWindow {
public:
	MoonWindowAndroid (MoonWindowType windowType, int w = -1, int h = -1, MoonWindow* parent = NULL, Surface *surface = NULL);

	virtual ~MoonWindowAndroid ();

	virtual void ConnectToContainerPlatformWindow (gpointer container_window);

	virtual void Resize (int width, int height);
	virtual void SetCursor (CursorType cursor);
	virtual void SetBackgroundColor (Color *color);
	virtual void Invalidate (Rect r);
	virtual void ProcessUpdates ();
	virtual gboolean HandleEvent (gpointer platformEvent);
	virtual void Show ();
	virtual void Hide ();
	virtual void EnableEvents (bool first);
	virtual void DisableEvents ();

	virtual void SetLeft (double left);
	virtual double GetLeft ();

	virtual void SetTop (double top);
	virtual double GetTop ();

	virtual void SetWidth (double width);

	virtual void SetHeight (double height);

	virtual void SetTitle (const char *title);

	virtual void SetIconFromPixbuf (MoonPixbuf *pixbuf);

	virtual void SetStyle (WindowStyle style);

	virtual void GrabFocus ();
	virtual bool HasFocus ();

	virtual MoonClipboard *GetClipboard (MoonClipboardType clipboardType);

	virtual gpointer GetPlatformWindow ();

	// FIXME: These shouldn't really be public
	void Paint (android_app* app);
	void ClearPlatformContext ();

private:
#if !USE_EGL
	void CreateCairoContext ();

	cairo_surface_t *native;
	CairoContext *ctx;
	unsigned char *backing_image_data;
#endif
	Region *damage;

#if USE_EGL
	MoonEGLSurface *egltarget;
	MoonEGLContext *eglctx;
	bool has_swap_rect;
#endif

	int left;
	int top;
};

};
#endif /* __MOON_WINDOW_ANDROID_H__ */
