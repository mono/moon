/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * window-gtk.cpp: MoonWindow implementation using gtk widgets.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include <glib.h>

#include "android_native_app_glue.h"

#include "window-android.h"
#include "clipboard-android.h"
#include "pixbuf-android.h"
#include "deployment.h"
#include "timemanager.h"
#include "enums.h"
#include "context-cairo.h"

#define __MOON_EGL__
#define EGL_EGLEXT_PROTOTYPES 1

#ifdef USE_EGL
#include "context-egl.h"
#endif

using namespace Moonlight;

MoonWindowAndroid::MoonWindowAndroid (MoonWindowType windowType, int w, int h, MoonWindow *parent, Surface *surface)
	: MoonWindow (windowType, w, h, parent, surface)
{
	this->width = w;
	this->height = h;
	damage = new Region ();

#ifdef USE_EGL
	egltarget = NULL;
	eglctx = NULL;
	has_swap_rect = FALSE;
#else
	CreateCairoContext ();
#endif
}

MoonWindowAndroid::~MoonWindowAndroid ()
{
#ifdef USE_EGL
	if (eglctx)
		delete eglctx;
	if (egltarget)
		egltarget->unref ();
#endif
}

void
MoonWindowAndroid::ConnectToContainerPlatformWindow (gpointer container_window)
{
}

MoonClipboard*
MoonWindowAndroid::GetClipboard (MoonClipboardType clipboardType)
{
	return new MoonClipboardAndroid (this, clipboardType);
}

gpointer
MoonWindowAndroid::GetPlatformWindow ()
{
	// FIXME
	return NULL;
}

void
MoonWindowAndroid::Resize (int width, int height)
{
	if (this->width == width && this->height == height)
		return;
	
	g_warning ("buffer = (%d,%d) surface = (%d,%d)", width, height, this->width, this->height);
	
	this->width = width;
	this->height = height;

	
	delete damage;
	damage = new Region (0.0, 0.0, width, height);

#if !defined (USE_EGL)
	CreateCairoContext();
#endif
	
	if (surface)
		surface->HandleUIWindowAllocation (true);
	
}	

void
MoonWindowAndroid::SetBackgroundColor (Color *color)
{
	// FIXME
}

void
MoonWindowAndroid::SetCursor (CursorType cursor)
{
	// FIXME
}

void
MoonWindowAndroid::Invalidate (Rect r)
{
	// FIXME
	damage->Union (r);
}

void
MoonWindowAndroid::ProcessUpdates ()
{
	android_app* state = (android_app*) Runtime::GetWindowingSystem()->GetPlatformWindowingSystemData ();
	Paint (state);
}

gboolean
MoonWindowAndroid::HandleEvent (gpointer platformEvent)
{
	// FIXME
	return TRUE;
}

void
MoonWindowAndroid::Show ()
{
	if (surface) {
		surface->HandleUIWindowUnavailable ();
		surface->HandleUIWindowAvailable ();
	}
}

void
MoonWindowAndroid::Hide ()
{
	if (surface)
		surface->HandleUIWindowAvailable ();
}

void
MoonWindowAndroid::EnableEvents (bool first)
{
	// FIXME
}

void
MoonWindowAndroid::DisableEvents ()
{
	// FIXME
}

void
MoonWindowAndroid::GrabFocus ()
{
	// FIXME
}

bool
MoonWindowAndroid::HasFocus ()
{
	// FIXME
	return false;
}

void
MoonWindowAndroid::SetLeft (double left)
{
	// FIXME
}

double
MoonWindowAndroid::GetLeft ()
{
	return left;
}

void
MoonWindowAndroid::SetTop (double top)
{
	// FIXME
}

double
MoonWindowAndroid::GetTop ()
{
	return top;
}

void
MoonWindowAndroid::SetWidth (double width)
{
	if (this->width == width)
		return;
	
	Resize (width, this->height);
}

void
MoonWindowAndroid::SetHeight (double height)
{
	if (this->height == height)
		return;

	Resize (this->width, height);
}

void
MoonWindowAndroid::SetTitle (const char *title)
{
	// FIXME
}

void
MoonWindowAndroid::SetIconFromPixbuf (MoonPixbuf *pixbuf)
{
	// FIXME
}

void
MoonWindowAndroid::SetStyle (WindowStyle style)
{
	// FIXME
}

void
MoonWindowAndroid::ClearPlatformContext ()
{
#ifdef USE_EGL
	if (eglctx)
		delete eglctx;
	if (egltarget)
		egltarget->unref ();

	eglctx = NULL;
	egltarget = NULL;
#endif
}

#ifdef USE_EGL

void
MoonWindowAndroid::Paint (android_app *app)
{
	if (app->window == NULL)
		return;

	if (!egltarget) {
		const EGLint attribs[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_BLUE_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_RED_SIZE, 8,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_NONE
		};

		EGLDisplay native_display;
		EGLSurface native_surface;
		EGLint format;
		EGLint numConfigs;
		EGLConfig config;

		MoonEGLContext *context;
		EGLint r,g,b,a;
		EGLint native_width;
		EGLint native_height;

		native_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

		eglInitialize (native_display, 0, 0);

		eglChooseConfig (native_display, attribs, &config, 1, &numConfigs);
		eglGetConfigAttrib (native_display, config, EGL_NATIVE_VISUAL_ID, &format);

		eglGetConfigAttrib(native_display, config, EGL_RED_SIZE,   &r);
		eglGetConfigAttrib(native_display, config, EGL_GREEN_SIZE, &g);
		eglGetConfigAttrib(native_display, config, EGL_BLUE_SIZE,  &b);
		eglGetConfigAttrib(native_display, config, EGL_ALPHA_SIZE, &a);

		eglQuerySurface(native_display, native_surface, EGL_WIDTH,  &native_width);
		eglQuerySurface(native_display, native_surface, EGL_HEIGHT, &native_height);

		ANativeWindow_setBuffersGeometry (app->window, 0, 0, format);

		native_surface = eglCreateWindowSurface (native_display, config, app->window, NULL);

		const char* const egl_extensions = eglQueryString (native_display, EGL_EXTENSIONS);
			    
		g_warning ("EGL informations:");
		g_warning ("# of configs : %d", numConfigs);
		g_warning ("vendor    : %s", eglQueryString (native_display, EGL_VENDOR));
		g_warning ("version   : %s", eglQueryString (native_display, EGL_VERSION));
		g_warning ("extensions: %s", egl_extensions);
		g_warning ("Client API: %s", eglQueryString (native_display, EGL_CLIENT_APIS) ?: "Not Supported");
		g_warning ("EGLSurface: %d-%d-%d-%d, config=%p", r, g, b, a, config);
		g_warning ("Display: %p\n", native_display);
		g_warning ("Surface: %p\n", native_surface);

		egltarget = new MoonEGLSurface (native_display, native_surface);
		context = new MoonEGLContext (egltarget);

		if (eglSetSwapRectangleANDROID(native_display, native_surface, 0, 0, native_width, native_height) == EGL_TRUE) {
			has_swap_rect = TRUE;
		} else {
			g_warning ("Disabling eglSetSwapRectangleANDROID");
		}

		if (context->Initialize ()) {
			eglctx = context;
		} else {
			delete context;
		}


		// HACK HACK HACK Our size has 1,1 in EGL before, so lets force a resize
		Resize (ANativeWindow_getWidth (app->window), ANativeWindow_getHeight (app->window));
	}
	
	if (damage->IsEmpty ()) {
		//g_warning ("no damage");
		return;
	}

	if (egltarget && eglctx) {
		if (has_swap_rect) {
			damage->Intersect (Rect (0, 0, width, height));
			Rect extents = damage->GetExtents ();

			egltarget->Reshape (width, height);

			static_cast<Context *> (eglctx)->Push (Context::Clip (extents));
			surface->Paint (eglctx, damage, GetTransparent (), true);
			static_cast<Context *> (eglctx)->Pop ();

			eglSetSwapRectangleANDROID (egltarget->GetEGLDisplay (), egltarget->GetEGLSurface (), extents.x, extents.y, extents.width, extents.height);
		} else {
			Rect r0 = Rect (0, 0, width, height);
			Region r1 = Region (r0);

			egltarget->Reshape (width, height);

			static_cast<Context *> (eglctx)->Push (Context::Clip (r0));
			surface->Paint (eglctx, &r1, GetTransparent (), true);
			static_cast<Context *> (eglctx)->Pop ();
		}

		eglctx->Flush ();
		egltarget->SwapBuffers ();
	} else {
		g_warning ("uhoh");
	}
}
#else
static inline guint8
convert_color_channel (guint8 src, guint8 alpha)
{
	return alpha ? src / (alpha / 255.0) : 0;
}

static inline void
convert_bgra_to_rgba (guint8 const *src, guint8 *dst, gint stride, gint x, gint y, gint width, gint height)
{
	guint8 const *src_pixel = src;
	guint8 * dst_pixel = dst;
	guint8 const *src_row = src_pixel + (stride * y) + (x * 4);
	guint8 *dst_row = dst_pixel + (stride * y) + (x * 4);
	
	for (int i = 0; i < height; i++)
	{   
		int j;
		src_pixel = src_row;
		dst_pixel = dst_row;
		for (j = 0; j < width; j++)
		{   
			dst_pixel[0] = convert_color_channel (src_pixel[2], src_pixel[3]);
			dst_pixel[1] = convert_color_channel (src_pixel[1], src_pixel[3]);
			dst_pixel[2] = convert_color_channel (src_pixel[0], src_pixel[3]);
			dst_pixel[3] = src_pixel[3];

			dst_pixel += 4;
			src_pixel += 4;
		}
		src_row += stride;
		dst_row += stride;
	}   
}

void
MoonWindowAndroid::CreateCairoContext ()
{
	CairoSurface *target;

	target = new CairoSurface (width, height);
	ctx = new CairoContext (target);

	backing_image_data = target->GetData ();
	target->unref ();
}

void
MoonWindowAndroid::Paint (android_app *app)
{
	ANativeWindow_Buffer buffer;
	unsigned char *pixels;

	SetCurrentDeployment ();

	if (app->window == NULL)
		return;
		

	// Make sure our buffer is the right size
	Resize (ANativeWindow_getWidth (app->window), ANativeWindow_getHeight (app->window));

	if (damage->IsEmpty ()) {
		//g_warning ("no damage");
		return;
	}
	
	surface->Paint (ctx, damage, transparent, true);
	
	gint stride = width * 4;

	int count = damage->GetRectangleCount (); 
	for (int i = 0; i < count; i++) {
		Rect box = damage->GetRectangle (i);
		ARect dirty;
		dirty.left = box.x;
		dirty.top = box.y;
		dirty.right = box.x+box.width;
		dirty.bottom = box.y+box.height;

		g_warning ("Blit damage(%d) = (%g,%g,%g,%g)", i, box.x, box.y, box.width, box.height);
		if (ANativeWindow_lock (app->window, &buffer, &dirty) < 0)
			return;
		
		pixels = (unsigned char *) buffer.bits;
		convert_bgra_to_rgba (backing_image_data, pixels, stride, box.x, box.y, box.width, box.height);
	
		ANativeWindow_unlockAndPost (app->window);
	}
	
	delete damage;
	damage = new Region ();
}
#endif
