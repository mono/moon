/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include "pal-android.h"

#include "runtime.h"
#include "window-android.h"
#include "pixbuf-android.h"
#include "debug.h"

#ifdef USE_GALLIUM
#define __MOON_GALLIUM__
#include "context-gallium.h"
extern "C" {
#include "pipe/p_screen.h"
#include "util/u_debug.h"
#define template templat
#include "state_tracker/sw_winsys.h"
#include "sw/null/null_sw_winsys.h"
#include "softpipe/sp_public.h"
#ifdef USE_LLVM
#include "llvmpipe/lp_public.h"
#endif
};
#endif

#include <glib.h>

#include <sys/stat.h>

#ifdef USE_GALLIUM
static struct pipe_screen *
swrast_screen_create (struct sw_winsys *ws)
{
	const char         *default_driver;
	const char         *driver;
	struct pipe_screen *screen = NULL;

#ifdef USE_LLVM
	default_driver = "llvmpipe";
#else
	default_driver = "softpipe";
#endif

	driver = debug_get_option ("GALLIUM_DRIVER", default_driver);

#ifdef USE_LLVM
	if (screen == NULL && strcmp (driver, "llvmpipe") == 0)
		screen = llvmpipe_create_screen (ws);
#endif

	if (screen == NULL)
		screen = softpipe_create_screen (ws);

	return screen;
}
#endif

using namespace Moonlight;


/// our windowing system

MoonWindowingSystemAndroid::MoonWindowingSystemAndroid (bool out_of_browser)
{
	if (out_of_browser) {
		g_thread_init (NULL);
	}

	LoadSystemColors ();

#ifdef USE_GALLIUM
	gscreen = swrast_screen_create (null_sw_create ());
#endif

}

MoonWindowingSystemAndroid::~MoonWindowingSystemAndroid ()
{

#ifdef USE_GALLIUM
	gscreen->destroy (gscreen);
#endif

	for (int i = 0; i < (int) NumSystemColors; i++)
		delete system_colors[i];
}

void
MoonWindowingSystemAndroid::ShowCodecsUnavailableMessage ()
{
	// FIXME
}

cairo_surface_t *
MoonWindowingSystemAndroid::CreateSurface ()
{
	// FIXME...
	g_assert_not_reached ();
}

void
MoonWindowingSystemAndroid::ExitApplication ()
{
	// FIXME
}

MoonWindow *
MoonWindowingSystemAndroid::CreateWindow (MoonWindowType windowType, int width, int height, MoonWindow *parentWindow, Surface *surface)
{
	MoonWindowAndroid *gtkwindow = new MoonWindowAndroid (windowType, width, height, parentWindow, surface);
	RegisterWindow (gtkwindow);
#ifdef USE_GALLIUM
	gtkwindow->SetGalliumScreen (gscreen);
#endif
	return gtkwindow;
}

MoonWindow *
MoonWindowingSystemAndroid::CreateWindowless (int width, int height, PluginInstance *forPlugin)
{
	MoonWindowAndroid *gtkwindow = (MoonWindowAndroid*)MoonWindowingSystem::CreateWindowless (width, height, forPlugin);
	if (gtkwindow)
		RegisterWindow (gtkwindow);
#ifdef USE_GALLIUM
	gtkwindow->SetGalliumScreen (gscreen);
#endif
	return gtkwindow;
}

MoonMessageBoxResult
MoonWindowingSystemAndroid::ShowMessageBox (MoonMessageBoxType message_type, const char *caption, const char *text, MoonMessageBoxButton button)
{
	// FIXME
	return MessageBoxResultNone;
}

char**
MoonWindowingSystemAndroid::ShowOpenFileDialog (const char *title, bool multsel, const char *filter, int idx)
{
	// FIXME
	return NULL;
}

char*
MoonWindowingSystemAndroid::ShowSaveFileDialog (const char *title, const char *filter, int idx)
{
	// FIXME
	return NULL;
}


bool
MoonWindowingSystemAndroid::ShowConsentDialog (const char *question, const char *detail, const char *website, bool *remember)
{
	// FIXME
	return false;
}

void
MoonWindowingSystemAndroid::RegisterWindow (MoonWindow *window)
{
}

void
MoonWindowingSystemAndroid::UnregisterWindow (MoonWindow *window)
{
}

void
MoonWindowingSystemAndroid::LoadSystemColors ()
{
	// FIXME
#if GTK_PAL_CODE_VERSION
	GtkSettings *settings = gtk_settings_get_default ();
	GtkWidget *widget;
	GtkStyle *style;
	
	widget = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_ensure_style (widget);
	style = gtk_widget_get_style (widget);
	
	// AppWorkspace colors (FIXME: wtf is an Application Workspace?)
	system_colors[AppWorkspaceColor] = color_from_gdk (style->bg[GTK_STATE_ACTIVE]);
	
	// Border colors (the Window's border - FIXME: get this from the WM?)
	system_colors[ActiveBorderColor] = color_from_gdk (style->bg[GTK_STATE_ACTIVE]);
	system_colors[InactiveBorderColor] = color_from_gdk (style->bg[GTK_STATE_INSENSITIVE]);
	
	// Caption colors (the Window's title bar - FIXME: get this from the WM?)
	system_colors[ActiveCaptionColor] = color_from_gdk (style->bg[GTK_STATE_ACTIVE]);
	system_colors[ActiveCaptionTextColor] = color_from_gdk (style->fg[GTK_STATE_ACTIVE]);
	system_colors[InactiveCaptionColor] = color_from_gdk (style->bg[GTK_STATE_INSENSITIVE]);
	system_colors[InactiveCaptionTextColor] = color_from_gdk (style->fg[GTK_STATE_INSENSITIVE]);
	
	// Desktop colors (FIXME: get this from gconf?)
	system_colors[DesktopColor] = color_from_gdk (style->bg[GTK_STATE_ACTIVE]);
	
	// Window colors (GtkWindow)
	system_colors[WindowColor] = color_from_gdk (style->bg[GTK_STATE_NORMAL]);
	system_colors[WindowFrameColor] = color_from_gdk (style->bg[GTK_STATE_NORMAL]);
	system_colors[WindowTextColor] = color_from_gdk (style->fg[GTK_STATE_NORMAL]);
	
	gtk_widget_destroy (widget);
	
	// Control colors (FIXME: what widget should we use? Does it matter?)
	widget = gtk_button_new ();
	gtk_widget_ensure_style (widget);
	style = gtk_widget_get_style (widget);
	
	system_colors[ControlColor] = color_from_gdk (style->bg[GTK_STATE_ACTIVE]);
	system_colors[ControlTextColor] = color_from_gdk (style->fg[GTK_STATE_ACTIVE]);
	
	system_colors[ControlDarkColor] = color_from_gdk (style->dark[GTK_STATE_ACTIVE]);
	system_colors[ControlDarkDarkColor] = color_from_gdk (style->dark[GTK_STATE_ACTIVE]);
	system_colors[ControlDarkDarkColor]->Darken ();
	
	system_colors[ControlLightColor] = color_from_gdk (style->light[GTK_STATE_ACTIVE]);
	system_colors[ControlLightLightColor] = color_from_gdk (style->light[GTK_STATE_ACTIVE]);
	system_colors[ControlLightLightColor]->Lighten ();
	
	// Gray Text colors (disabled text)
	system_colors[GrayTextColor] = color_from_gdk (style->fg[GTK_STATE_INSENSITIVE]);
	
	gtk_widget_destroy (widget);
	
	// Highlight colors (selected items - FIXME: what widget should we use? Does it matter?)
	widget = gtk_entry_new ();
	gtk_widget_ensure_style (widget);
	style = gtk_widget_get_style (widget);
	system_colors[HighlightColor] = color_from_gdk (style->bg[GTK_STATE_SELECTED]);
	system_colors[HighlightTextColor] = color_from_gdk (style->fg[GTK_STATE_SELECTED]);
	gtk_widget_destroy (widget);
	
	// Info colors (GtkTooltip)
	if (!(style = gtk_rc_get_style_by_paths (settings, "gtk-tooltip", "GtkWindow", GTK_TYPE_WINDOW))) {
		widget = gtk_window_new (GTK_WINDOW_POPUP);
		gtk_widget_ensure_style (widget);
		style = gtk_widget_get_style (widget);
	} else {
		widget = NULL;
	}
	system_colors[InfoColor] = color_from_gdk (style->bg[GTK_STATE_NORMAL]);
	system_colors[InfoTextColor] = color_from_gdk (style->fg[GTK_STATE_NORMAL]);
	if (widget)
		gtk_widget_destroy (widget);
	
	// Menu colors (GtkMenu)
	widget = gtk_menu_new ();
	gtk_widget_ensure_style (widget);
	style = gtk_widget_get_style (widget);
	system_colors[MenuColor] = color_from_gdk (style->bg[GTK_STATE_NORMAL]);
	system_colors[MenuTextColor] = color_from_gdk (style->fg[GTK_STATE_NORMAL]);
	gtk_widget_destroy (widget);
	
	// ScrollBar colors (GtkScrollbar)
	widget = gtk_vscrollbar_new (NULL);
	gtk_widget_ensure_style (widget);
	style = gtk_widget_get_style (widget);
	system_colors[ScrollBarColor] = color_from_gdk (style->bg[GTK_STATE_NORMAL]);
	gtk_widget_destroy (widget);
#endif
}

Color *
MoonWindowingSystemAndroid::GetSystemColor (SystemColor id)
{
	if (id < 0 || id >= (int) NumSystemColors)
		return NULL;
	
	return system_colors[id];
}

guint
MoonWindowingSystemAndroid::AddTimeout (gint priority, gint ms, MoonSourceFunc timeout, gpointer data)
{
	// FIXME
	return 0;
#if GTK_PAL_CODE_VERSION
	return g_timeout_add_full (priority, ms, (GSourceFunc)timeout, data, NULL);
#endif
}

void
MoonWindowingSystemAndroid::RemoveTimeout (guint timeoutId)
{
	// FIXME
#if GTK_PAL_CODE_VERSION
	g_source_remove (timeoutId);
#endif
}

guint
MoonWindowingSystemAndroid::AddIdle (MoonSourceFunc idle, gpointer data)
{
	// FIXME
	return 0;
#if GTK_PAL_CODE_VERSION
	return g_idle_add ((GSourceFunc)idle, data);
#endif
}

void
MoonWindowingSystemAndroid::RemoveIdle (guint idle_id)
{
	// FIXME
#if GTK_PAL_CODE_VERSION
	g_source_remove (idle_id);
#endif
}

MoonIMContext*
MoonWindowingSystemAndroid::CreateIMContext ()
{
	// FIXME
	return NULL;
}

MoonEvent*
MoonWindowingSystemAndroid::CreateEventFromPlatformEvent (gpointer platformEvent)
{
	// FIXME
	return NULL;
#if GTK_PAL_CODE_VERSION
	GdkEvent *gdk = (GdkEvent*)platformEvent;

	switch (gdk->type) {
	case GDK_MOTION_NOTIFY: {
		GdkEventMotion *mev = (GdkEventMotion*)gdk;
		if (mev->is_hint) {
#if GTK_CHECK_VERSION(2,12,0)
			if (gtk_check_version (2, 12, 0)) {
				gdk_event_request_motions (mev);
			}
			else
#endif
			{
				int ix, iy;
				GdkModifierType state;
				gdk_window_get_pointer (mev->window, &ix, &iy, (GdkModifierType*)&state);
				mev->x = ix;
				mev->y = iy;
			}    
		}

		return new MoonMotionEventAndroid (gdk);
	}
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:
		return new MoonButtonEventAndroid (gdk);

	case GDK_KEY_PRESS:
	case GDK_KEY_RELEASE:
		return new MoonKeyEventAndroid (gdk);

	case GDK_ENTER_NOTIFY:
	case GDK_LEAVE_NOTIFY:
		return new MoonCrossingEventAndroid (gdk);

	case GDK_FOCUS_CHANGE:
		return new MoonFocusEventAndroid (gdk);

	case GDK_SCROLL:
		return new MoonScrollWheelEventAndroid (gdk);
	default:
		printf ("unhandled android event %d\n", gdk->type);
		return NULL;
	}
#endif
}

guint
MoonWindowingSystemAndroid::GetCursorBlinkTimeout (MoonWindow *moon_window)
{
	// FIXME
	return CURSOR_BLINK_TIMEOUT_DEFAULT;
}


MoonPixbufLoader*
MoonWindowingSystemAndroid::CreatePixbufLoader (const char *imageType)
{
	// FIXME
	return NULL;
}

void
MoonWindowingSystemAndroid::RunMainLoop (MoonWindow *window, bool quit_on_window_close)
{
	// FIXME
#if GTK_PAL_CODE_VERSION
	if (window) {
		window->Show ();

		if (quit_on_window_close)
			g_signal_connect (((MoonWindowGtk*)window)->GetWidget (), "delete-event", G_CALLBACK (gtk_main_quit), NULL);
	}

	gdk_threads_enter ();
	gtk_main ();
	gdk_threads_leave ();
#endif
}

guint32
MoonWindowingSystemAndroid::GetScreenHeight (MoonWindow *moon_window)
{
	// FIXME
	return 100;
}

guint32
MoonWindowingSystemAndroid::GetScreenWidth (MoonWindow *moon_window)
{
	// FIXME
	return 100;
}

bool
MoonWindowingSystemAndroid::ConvertJPEGToBGRA (void *jpeg, guint32 jpeg_size, guint8 *buffer, guint32 buffer_stride, guint32 buffer_height)
{
	// FIXME
	return false;
#if GTK_PAL_CODE_VERSION
	bool result = false;
	GError *err = NULL;
	GdkPixbufLoader *loader;
	GdkPixbuf *pixbuf;
	guint32 gdk_stride;
	guint32 gdk_height;
	guint32 gdk_width;
	guint8 *gdk_pixels;
	guint8 *in;
	guint8 *out;

	if ((loader = gdk_pixbuf_loader_new_with_type ("jpeg", &err)) == NULL) {
		goto cleanup;
	}

	if (!gdk_pixbuf_loader_write (loader, (const guchar *) jpeg, jpeg_size, &err)) {
		goto cleanup;
	}

	if (!gdk_pixbuf_loader_close (loader, &err)) {
		goto cleanup;
	}

	if ((pixbuf = gdk_pixbuf_loader_get_pixbuf (loader)) == NULL) {
		fprintf (stderr, "Moonlight: Could not convert JPEG to BGRA: pixbufloader didn't create a pixbuf.\n");
		goto cleanup;
	}

	gdk_pixels = gdk_pixbuf_get_pixels (pixbuf);
	gdk_stride = gdk_pixbuf_get_rowstride (pixbuf);
	gdk_height = gdk_pixbuf_get_height (pixbuf);
	gdk_width = gdk_pixbuf_get_width (pixbuf);

	for (guint32 y = 0; y < MIN (gdk_height, buffer_height); y++) {
		out = buffer + buffer_stride * y;
		in = gdk_pixels + gdk_stride * y;
		for (guint32 x = 0; x < MIN (gdk_width, buffer_stride); x++) {
			out [0] = in [2];
			out [1] = in [1];
			out [2] = in [0];
			out [3] = 0xFF;;
			out += 4;
			in += 3;
		}
	}

	result = true;

cleanup:
	if (err) {
		fprintf (stderr, "Moonlight: could not convert jpeg to bgra: %s\n", err->message);
		g_error_free (err);
	}

	if (loader)
		g_object_unref (loader);

	return result;
#endif
}
