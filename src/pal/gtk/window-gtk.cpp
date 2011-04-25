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
#include <glib/gstdio.h>

#include "window-gtk.h"
#include "clipboard-gtk.h"
#include "config-dialog-gtk.h"
#include "pixbuf-gtk.h"
#include "install-dialog-gtk.h"
#include "deployment.h"
#include "timemanager.h"
#include "enums.h"
#include "context-cairo.h"
#ifdef USE_GALLIUM
#define __MOON_GALLIUM__
#include "context-gallium.h"
#ifdef CLAMP
#undef CLAMP
#endif
#include "util/u_inlines.h"
#endif
#define Visual _XxVisual
#define Region _XxRegion
#define Window _XxWindow
#include <gdk/gdkx.h>
#include <cairo-xlib.h>
#ifdef USE_GLX
#include <GL/glx.h>
#endif
#undef Visual
#undef Region
#undef Window

#ifdef USE_GLX
#include "context-opengl.h"
typedef void (* PFNGLXCOPYSUBBUFFERPROC) (Display *dpy, GLXDrawable drawable, int x, int y, int width, int height);
#endif

// change this to "1" if you want fullscreen redraws to allocate a new
// pixmap per redraw just at the size of the expose area.
//
#define FULLSCREEN_BACKING_STORE_SOPTIMIZATION 0

// Gallium context cache size.
//
#define CONTEXT_CACHE_SIZE 1

using namespace Moonlight;

#ifdef USE_GALLIUM
int MoonWindowGtk::gctxn = 0;
#endif

#ifdef USE_GLX

#define GETPROCADDR(type, name)				\
	(name) = (type) surface->GetProcAddress (# name)

class MoonGLXSurface : public OpenGLSurface {
public:
	MoonGLXSurface () : OpenGLSurface () {
		GETPROCADDR (PFNGLCREATESHADERPROC, glCreateShader);
		GETPROCADDR (PFNGLSHADERSOURCEPROC, glShaderSource);
		GETPROCADDR (PFNGLCOMPILESHADERPROC, glCompileShader);
		GETPROCADDR (PFNGLGETSHADERIVPROC, glGetShaderiv);
		GETPROCADDR (PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);
		GETPROCADDR (PFNGLDELETESHADERPROC, glDeleteShader);
		GETPROCADDR (PFNGLCREATEPROGRAMPROC, glCreateProgram);
		GETPROCADDR (PFNGLATTACHSHADERPROC, glAttachShader);
		GETPROCADDR (PFNGLBINDATTRIBLOCATIONPROC, glBindAttribLocation);
		GETPROCADDR (PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
		GETPROCADDR (PFNGLUNIFORM4FPROC, glUniform4f);
		GETPROCADDR (PFNGLUNIFORM4FVPROC, glUniform4fv);
		GETPROCADDR (PFNGLUNIFORM1IPROC, glUniform1i);
		GETPROCADDR (PFNGLLINKPROGRAMPROC, glLinkProgram);
		GETPROCADDR (PFNGLUSEPROGRAMPROC, glUseProgram);
		GETPROCADDR (PFNGLDELETEPROGRAMPROC, glDeleteProgram);
		GETPROCADDR (PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
		GETPROCADDR (PFNGLENABLEVERTEXATTRIBARRAYARBPROC,
			     glEnableVertexAttribArray);
		GETPROCADDR (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC,
			     glDisableVertexAttribArray);
		GETPROCADDR (PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers);
		GETPROCADDR (PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers);
		GETPROCADDR (PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);
		GETPROCADDR (PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);
		GETPROCADDR (PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers);
		GETPROCADDR (PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);
		GETPROCADDR (PFNGLCHECKFRAMEBUFFERSTATUSPROC,
			     glCheckFramebufferStatus);
	};
	__GLFuncPtr GetProcAddress (const char *procname) { return glXGetProcAddressARB ((const GLubyte *) procname); }
};
class MoonGLXContext : public OpenGLContext {
public:
	MoonGLXContext (OpenGLSurface *surface, Display *dpy, GLXContext ctx) : OpenGLContext (surface) { this->ctx = ctx; this->dpy = dpy; };
	GLXContext GetGLXContext () { return ctx; }
	Display *GetGLXDisplay () { return dpy; }
private:
	Display *dpy;
	GLXContext ctx;
protected:
	PFNGLCREATESHADERPROC glCreateShader;
	PFNGLSHADERSOURCEPROC glShaderSource;
	PFNGLCOMPILESHADERPROC glCompileShader;
	PFNGLGETSHADERIVPROC glGetShaderiv;
	PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
	PFNGLDELETESHADERPROC glDeleteShader;
	PFNGLCREATEPROGRAMPROC glCreateProgram;
	PFNGLATTACHSHADERPROC glAttachShader;
	PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
	PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
	PFNGLUNIFORM4FPROC glUniform4f;
	PFNGLUNIFORM4FVPROC glUniform4fv;
	PFNGLUNIFORM1IPROC glUniform1i;
	PFNGLLINKPROGRAMPROC glLinkProgram;
	PFNGLUSEPROGRAMPROC glUseProgram;
	PFNGLDELETEPROGRAMPROC glDeleteProgram;
	PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
	PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArray;
	PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArray;
	PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
	PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
	PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
	PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
	PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
	PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
	PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
};
#endif

MoonWindowGtk::MoonWindowGtk (MoonWindowType windowType, int w, int h, MoonWindow *parent, Surface *surface)
	: MoonWindow (windowType, w, h, parent, surface)
{
	this->windowType = windowType;

	backing_image_data = NULL;
	backing_store = NULL;
	backing_store_gc = NULL;
	backing_store_width = backing_store_height = 0;

	switch (windowType) {
	case MoonWindowType_FullScreen:
		InitializeFullScreen (parent);
		break;
	case MoonWindowType_Desktop:
		InitializeDesktop (parent);
		break;
	case MoonWindowType_Plugin:
		InitializePlugin();
		break;
	}

	native = NULL;

#ifdef USE_GALLIUM
	screen = NULL;
	gctx = NULL;
#endif

#ifdef USE_GLX
	glxtarget = NULL;
	glxctx = NULL;
	glxcopysubbuffer = NULL;
#endif

}

MoonWindowGtk::~MoonWindowGtk ()
{
	/* gtk_widget_destroy can cause reentry (into another plugin if this destruction causes layout changes) */
	DeploymentStack deployment_push_pop;
	DisableEvents ();
	if (widget != NULL)
		gtk_widget_destroy (widget);

	if (backing_store)
		g_object_unref (backing_store);
	if (backing_store_gc)
		g_object_unref (backing_store_gc);

	if (backing_image_data)
		g_free (backing_image_data);

	if (native)
		cairo_surface_destroy (native);

#ifdef USE_GALLIUM
	if (gctx) {
		delete gctx;
		gctxn--;
	}
#endif

#ifdef USE_GLX
	if (glxctx) {
		glXDestroyContext (static_cast<MoonGLXContext *> (glxctx)->GetGLXDisplay (),
				   static_cast<MoonGLXContext *> (glxctx)->GetGLXContext ());
		delete glxctx;
	}
	if (glxtarget)
		glxtarget->unref ();
#endif

}

void
MoonWindowGtk::ConnectToContainerPlatformWindow (gpointer container_window)
{
	//  GtkPlug container and surface inside
	container = gtk_plug_new ((GdkNativeWindow) container_window);

	// Connect signals to container
	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (container), GTK_CAN_FOCUS);

	gtk_widget_add_events (container,
			       GDK_BUTTON_PRESS_MASK |
			       GDK_BUTTON_RELEASE_MASK |
			       GDK_KEY_PRESS_MASK |
			       GDK_KEY_RELEASE_MASK |
			       GDK_POINTER_MOTION_MASK |
			       GDK_SCROLL_MASK |
			       GDK_EXPOSURE_MASK |
			       GDK_VISIBILITY_NOTIFY_MASK |
			       GDK_ENTER_NOTIFY_MASK |
			       GDK_LEAVE_NOTIFY_MASK |
			       GDK_FOCUS_CHANGE_MASK
			       );

	gtk_container_add (GTK_CONTAINER (container), widget);
	gtk_widget_show_all (container);
}

MoonClipboard*
MoonWindowGtk::GetClipboard (MoonClipboardType clipboardType)
{
	return new MoonClipboardGtk (this, clipboardType);
}

gpointer
MoonWindowGtk::GetPlatformWindow ()
{
	GtkWidget *w = widget;
	while (w->parent)
		w = w->parent;

	return w->window;
}

void
MoonWindowGtk::InitializeFullScreen (MoonWindow *parent)
{
	widget = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	// only fullscreen on the monitor the plugin is on
	GdkWindow *gdk = GDK_WINDOW (parent->GetPlatformWindow ());
	int monitor = gdk_screen_get_monitor_at_window (gdk_screen_get_default (), gdk);
	GdkRectangle bounds;
	gdk_screen_get_monitor_geometry (gdk_screen_get_default (), monitor, &bounds);
	width = bounds.width;
	height = bounds.height;
	gtk_window_move (GTK_WINDOW (widget), bounds.x, bounds.y);

	gtk_window_fullscreen (GTK_WINDOW (widget));

	InitializeCommon ();

	Show();
}

void
MoonWindowGtk::InitializeDesktop (MoonWindow *parent)
{
	widget = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_widget_set_app_paintable (widget, true);

	InitializeCommon ();
}

void
MoonWindowGtk::InitializePlugin ()
{
	if (width == -1 || height == -1) {
		g_warning ("you must specify width and height when creating a non-fullscreen gtk window");
		width = 0;
		height = 0;
	}

	widget = gtk_event_box_new ();

	gtk_event_box_set_visible_window (GTK_EVENT_BOX (widget), true);

	InitializeCommon ();

	Show ();
}

void
MoonWindowGtk::InitializeCommon ()
{
	// don't let gtk clear the window we'll do all the drawing.
	//gtk_widget_set_app_paintable (widget, true);
	gtk_widget_set_double_buffered (widget, false);
	gtk_widget_set_size_request (widget, width, height);

	g_signal_connect (widget, "size-allocate", G_CALLBACK (widget_size_allocate), this);
	g_signal_connect (widget, "destroy", G_CALLBACK (widget_destroyed), this);
	
	gtk_widget_add_events (widget, 
			       GDK_POINTER_MOTION_MASK |
#if !DEBUG
			       GDK_POINTER_MOTION_HINT_MASK |
#endif
			       GDK_KEY_PRESS_MASK |
			       GDK_KEY_RELEASE_MASK |
			       GDK_BUTTON_PRESS_MASK |
			       GDK_BUTTON_RELEASE_MASK |
			       GDK_SCROLL_MASK |
			       GDK_FOCUS_CHANGE_MASK);
	
	GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_FOCUS);
}

void
MoonWindowGtk::Resize (int width, int height)
{
	switch (windowType) {
	case MoonWindowType_Desktop:
		gtk_window_resize ((GtkWindow*)widget, width, height);
		break;
	default:
		gtk_widget_set_size_request (widget, width, height);
		gtk_widget_queue_resize (widget);
	}
}

/* XPM */
static const char *dot[] = {
	"18 18 4 1",
	"       c None",
	".      c #808080",
	"+      c #303030",
	"@      c #000000",
	".+.               ",
	"@@@               ",
	".@.               ",
	"                  ",
	"                  ",
	"                  ",
	"                  ",
	"                  ",
	"                  ",
	"                  ",
	"                  ",
	"                  ",
	"                  ",
	"                  ",
	"                  ",
	"                  ",
	"                  ",
	"                  "
};

/* XPM */
static const char *eraser[] = {
	"20 20 49 1",
	"       c None",
	".      c #000000",
	"+      c #858585",
	"@      c #E8E8E8",
	"#      c #E9E9E9",
	"$      c #E7E7E7",
	"%      c #E2E2E2",
	"&      c #D6D6D6",
	"*      c #7D7D7D",
	"=      c #565656",
	"-      c #E1E1E1",
	";      c #E0E0E0",
	">      c #DEDEDE",
	",      c #DFDFDF",
	"'      c #474747",
	")      c #6C6C6C",
	"!      c #B0B0B0",
	"~      c #E3E3E3",
	"{      c #4E4E4E",
	"]      c #636363",
	"^      c #E6E6E6",
	"/      c #505050",
	"(      c #4A4A4A",
	"_      c #C7C7C7",
	":      c #272727",
	"<      c #797979",
	"[      c #E5E5E5",
	"}      c #DDDDDD",
	"|      c #9C9C9C",
	"1      c #232323",
	"2      c #E4E4E4",
	"3      c #656565",
	"4      c #313131",
	"5      c #EAEAEA",
	"6      c #ECECEC",
	"7      c #EEEEEE",
	"8      c #EFEFEF",
	"9      c #F0F0F0",
	"0      c #999999",
	"a      c #5D5D5D",
	"b      c #343434",
	"c      c #757575",
	"d      c #383838",
	"e      c #CECECE",
	"f      c #A9A9A9",
	"g      c #6F6F6F",
	"h      c #B3B3B3",
	"i      c #787878",
	"j      c #3F3F3F",
	"                    ",
	"                    ",
	"                    ",
	"                    ",
	"                    ",
	"                    ",
	"       ...........  ",
	"      .+@#@@@$$%&*. ",
	"      =-;%>>>>>>>,' ",
	"     )!~>>>>>>>>>>{ ",
	"     ]^>>>>>>>>>,,/ ",
	"    (_;>>>>>>>>,>&: ",
	"    <[,}>>>>>>>-,|  ",
	"   1[-;>>>>>>>$2,3  ",
	"   45678999998550a  ",
	"   b~,,,,,,,,,;$c   ",
	"   de-,,,,,,,,,fg   ",
	"   bh%%,,;}}}>>ij   ",
	"    ............    ",
	"                    "
};

void
MoonWindowGtk::SetBackgroundColor (Color *color)
{
	GdkColor gdk_color;
	gdk_color.red = color->r * 0xffff;
	gdk_color.green = color->g * 0xffff;
	gdk_color.blue = color->b * 0xffff;
	
	gtk_widget_modify_bg (widget, GTK_STATE_NORMAL, &gdk_color);

	MoonWindow::SetBackgroundColor (color);
}

void
MoonWindowGtk::SetCursor (CursorType cursor)
{
	if (widget->window) {

		GdkCursor *c = NULL;
		switch (cursor) {
		case CursorTypeDefault:
			c = NULL;
			break;
		case CursorTypeArrow:
			c = gdk_cursor_new (GDK_LEFT_PTR);
			break;
		case CursorTypeHand:
			c = gdk_cursor_new (GDK_HAND2);
			break;
		case CursorTypeWait:
			c = gdk_cursor_new (GDK_WATCH);
			break;
		case CursorTypeIBeam:
			c = gdk_cursor_new (GDK_XTERM);
			break;
		case CursorTypeStylus:
			c = gdk_cursor_new_from_pixbuf (gdk_display_get_default (), gdk_pixbuf_new_from_xpm_data ((const char**) dot), 0, 0);
			break;
		case CursorTypeEraser:
			c = gdk_cursor_new_from_pixbuf (gdk_display_get_default (), gdk_pixbuf_new_from_xpm_data ((const char**) eraser), 8, 8);
			break;
		case CursorTypeSizeNS:
			c = gdk_cursor_new (GDK_SB_V_DOUBLE_ARROW);
			break;
		case CursorTypeSizeWE:
			c = gdk_cursor_new (GDK_SB_H_DOUBLE_ARROW);
			break;
		case CursorTypeNone:
			// Silverlight display no cursor if the enumeration value is invalid (e.g. -1)
		default:
			//from gdk-cursor doc :"To make the cursor invisible, use gdk_cursor_new_from_pixmap() to create a cursor with no pixels in it."
			GdkPixmap *empty = gdk_bitmap_create_from_data (NULL, "0x00", 1, 1);
			GdkColor empty_color = {0, 0, 0, 0};
			c = gdk_cursor_new_from_pixmap (empty, empty, &empty_color, &empty_color, 0, 0);
			g_object_unref (empty);
			break;
		}


		gdk_window_set_cursor (widget->window, c);

		if (c)
			gdk_cursor_unref (c);
	}
}

void
MoonWindowGtk::Invalidate (Rect r)
{
	gtk_widget_queue_draw_area (widget,
				    (int) (widget->allocation.x + r.x), 
				    (int) (widget->allocation.y + r.y), 
				    (int) r.width, (int)r.height);
}

void
MoonWindowGtk::ProcessUpdates ()
{
	if (widget->window)
		gdk_window_process_updates (widget->window, false);
}

gboolean
MoonWindowGtk::HandleEvent (gpointer platformEvent)
{
	// nothing to do here, since we don't pump events into the gtk
	// window, gtk calls our signal handlers directly.
	return TRUE;
}

void
MoonWindowGtk::Show ()
{
	gtk_widget_show (widget);

	// The window has to be realized for this call to work
	gtk_widget_set_extension_events (widget, GDK_EXTENSION_EVENTS_CURSOR);
	/* we need to explicitly enable the devices */
	for (GList *l = gdk_devices_list(); l; l = l->next) {
#if THIS_NOLONGER_BREAKS_LARRYS_MOUSE
		GdkDevice *device = GDK_DEVICE(l->data);
		//if (!device->has_cursor)
		gdk_device_set_mode (device, GDK_MODE_SCREEN);
#endif
	}

	GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_FOCUS);
}

void
MoonWindowGtk::Hide ()
{
	gtk_widget_hide (widget);
}

void
MoonWindowGtk::EnableEvents (bool first)
{
	g_signal_connect (widget, "motion-notify-event", G_CALLBACK (motion_notify), this);
	g_signal_connect (widget, "enter-notify-event", G_CALLBACK (crossing_notify), this);
	g_signal_connect (widget, "leave-notify-event", G_CALLBACK (crossing_notify), this);
	g_signal_connect (widget, "key-press-event", G_CALLBACK (key_press), this);
	g_signal_connect (widget, "key-release-event", G_CALLBACK (key_release), this);
	g_signal_connect (widget, "button-press-event", G_CALLBACK (button_press), this);
	g_signal_connect (widget, "button-release-event", G_CALLBACK (button_release), this);
	g_signal_connect (widget, "scroll-event", G_CALLBACK (scroll), this);
	g_signal_connect (widget, "focus-in-event", G_CALLBACK (focus_in), this);
	g_signal_connect (widget, "focus-out-event", G_CALLBACK (focus_out), this);

	g_signal_connect (widget, "expose-event", G_CALLBACK (expose_event), this);
	if (first) {
		g_signal_connect (widget, "realize", G_CALLBACK (realized), this);
		g_signal_connect (widget, "unrealize", G_CALLBACK (unrealized), this);
		
		if (GTK_WIDGET_REALIZED (widget))
			realized (widget, this);
	}
}

void
MoonWindowGtk::DisableEvents ()
{
	if (widget == NULL)
		return;

	g_signal_handlers_disconnect_matched (widget, G_SIGNAL_MATCH_DATA,
					      0, 0, NULL, NULL, this);
}

void
MoonWindowGtk::GrabFocus ()
{
	gtk_widget_grab_focus (widget);
}

bool
MoonWindowGtk::HasFocus ()
{
	return GTK_WIDGET_HAS_FOCUS (widget);
}

gboolean
MoonWindowGtk::expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)data;

	return window->ExposeEvent (widget, event);
}

gboolean
MoonWindowGtk::ExposeEvent (GtkWidget *w, GdkEventExpose *event)
{
	SetCurrentDeployment ();

	if (!surface)
		return true;

#ifdef USE_GLX
	Display   *dpy = gdk_x11_drawable_get_xdisplay (w->window);
	XID       win = gdk_x11_drawable_get_xid (w->window);
	_XxVisual *visual = GDK_VISUAL_XVISUAL (gdk_drawable_get_visual (w->window));
	int       width, height;

	gdk_drawable_get_size (w->window, &width, &height);

	if (!glxtarget && (moonlight_flags & RUNTIME_INIT_HW_ACCELERATION)) {
		GLXContext  ctx = (GLXContext) 0;
		XVisualInfo templ, *visinfo;
		int         n;

		templ.visualid = XVisualIDFromVisual (visual);
		visinfo = XGetVisualInfo (dpy, VisualIDMask, &templ, &n);
		g_assert (visinfo);

		ctx = glXCreateContext (dpy, visinfo, 0, True);
		if (ctx) {
			gdk_error_trap_push ();
			glXMakeCurrent (dpy, win, ctx);
			gdk_flush ();
			if (gdk_error_trap_pop ()) {
				g_warning ("Failed to make GLX context current for window: "
					   "0x%x", (int) win);
				glXDestroyContext (dpy, ctx);
				ctx = (GLXContext) 0;
			}
		}
		else {
			g_warning ("Failed to create GLX context for VisualID: 0x%x",
				   (int) XVisualIDFromVisual (visual));
		}

		glxtarget = new MoonGLXSurface ();

		if (ctx) {
			OpenGLContext *context = new MoonGLXContext (glxtarget, dpy, ctx);

			if (context->Initialize ()) {
				const char *extensions = (const char *)
					glXGetClientString (dpy, GLX_EXTENSIONS);

				if (strstr (extensions, "GLX_MESA_copy_sub_buffer"))
					glxcopysubbuffer = glxtarget->GetProcAddress
						("glXCopySubBufferMESA");

				glxctx = context;
			}
			else {
				glXDestroyContext (dpy, ctx);
				delete context;
			}
		}
	}

	if (glxtarget && glxctx) {
		Rect r0 = Rect (0, 0, width, height);
		Rect r = Rect (event->area.x,
			       event->area.y,
			       event->area.width,
			       event->area.height);
		Region *region = new Region (r);
		int    y = height - (event->area.y + event->area.height);

		glXMakeCurrent (dpy,
				win,
				static_cast<MoonGLXContext *> (glxctx)->GetGLXContext ());

		glxtarget->Reshape (width, height);

		glxctx->Push (Context::Clip (r));
		surface->Paint (glxctx, region, GetTransparent (), true);
		glxctx->Pop ();

		glxctx->Flush ();

		if (region->RectIn (r0) == CAIRO_REGION_OVERLAP_IN) {
			glXSwapBuffers (dpy, win);
		}
		else if (glxcopysubbuffer) {
			PFNGLXCOPYSUBBUFFERPROC glXCopySubBufferMESA =
				(PFNGLXCOPYSUBBUFFERPROC) glxcopysubbuffer;

			glXCopySubBufferMESA (dpy,
					      win,
					      event->area.x,
					      y,
					      event->area.width,
					      event->area.height);
		}
		else {
			glDrawBuffer (GL_FRONT);
			glViewport (-1, -1, 2, 2);
			glRasterPos2f (0, 0);
			glViewport (0, 0, width, height);

			glBitmap (0, 0, 0, 0,
				  event->area.x,
				  y,
				  NULL);

			glCopyPixels (event->area.x, y,
				      event->area.width,
				      event->area.height,
				      GL_COLOR);

			glDrawBuffer (GL_BACK);
			glFlush ();
		}

		return true;
	}
#endif

	// we draw to a backbuffer pixmap, then transfer the contents
	// to the widget's window.
	if (backing_store == NULL ||
	    backing_store_width < (event->area.x + event->area.width) ||
	    backing_store_height < (event->area.y + event->area.height)) {
		if (backing_store)
			g_object_unref (backing_store);
		if (backing_store_gc)
			g_object_unref (backing_store_gc);
		if (backing_image_data) {
			g_free (backing_image_data);
			backing_image_data = NULL;
		}
#if FULLSCREEN_BACKING_STORE_SOPTIMIZATION
		if (IsFullScreen ()) {
			backing_store_width = MAX (event->area.x + event->area.width, 1);
			backing_store_height = MAX (event->area.y + event->area.height, 1);
		}
		else
#endif
		{
			backing_store_width = MAX (GetWidth(), 1);
			backing_store_height = MAX (GetHeight(), 1);
		}

		backing_store = gdk_pixmap_new (w->window,
						backing_store_width, backing_store_height, -1);

		backing_store_gc = gdk_gc_new (backing_store);
	}

	PaintToDrawable (backing_store,
			 gdk_drawable_get_visual (w->window),
			 event,
			 - event->area.x,
			 - event->area.y,
			 GetTransparent (),
			 true);

	gdk_gc_set_clip_region (backing_store_gc, event->region);

	gdk_draw_drawable (w->window, backing_store_gc, backing_store,
			   0, 0,
			   event->area.x, event->area.y,
			   event->area.width, event->area.height);

#if FULLSCREEN_BACKING_STORE_SOPTIMIZATION
	if (IsFullScreen ()) {
		g_object_unref (backing_store); backing_store = NULL;
		g_object_unref (backing_store_gc); backing_store_gc = NULL;
		backing_store_width = backing_store_height = 0;
	}
#endif

	return true;
}

gboolean
MoonWindowGtk::button_press (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)data;
	MoonEventStatus status = MoonEventNotHandled;

	window->SetCurrentDeployment ();

	if (event->button != 1 && event->button != 3)
		return false;
	
	if (window->surface) {
		MoonEvent *mevent = Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		status = mevent->DispatchToWindow (window);
		delete mevent;
	}
	if (status == MoonEventNotHandled)
		container_button_press_callback (widget, event, data);
	// ignore DispatchToWindow's return value, and always
	// return true here, or it gets bubbled up to firefox.
	return true;
}

gboolean
MoonWindowGtk::button_release (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	MoonWindowGtk *window = (MoonWindowGtk *) data;
	MoonEventStatus status = MoonEventNotHandled;
	
	window->SetCurrentDeployment ();

	if (window->surface) {
		MoonEvent *mevent = Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		status = mevent->DispatchToWindow (window);
		delete mevent;
	}
	// ignore DispatchToWindow's return value, and always
	// return true here, or it gets bubbled up to firefox.
	return true;
}

gboolean
MoonWindowGtk::scroll (GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)data;
	MoonEventStatus status = MoonEventNotHandled;
	
	window->SetCurrentDeployment ();

	if (window->surface) {
		MoonEvent *mevent = Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		status = mevent->DispatchToWindow (window);
		delete mevent;
	}
	// HandleUIScroll's return value is a special case: if it
	// returns NotSupported, then we need to bubble up to firefox.
	return status != MoonEventNotSupported;
}

gboolean
MoonWindowGtk::motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk *) user_data;
	MoonEventStatus status = MoonEventNotHandled;
	
	window->SetCurrentDeployment ();

	if (window->surface) {
		MoonEvent *mevent = Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		status = mevent->DispatchToWindow (window);
		delete mevent;
	}
	// ignore DispatchToWindows's return value, and always
	// return true here, or it gets bubbled up to firefox.
	return true;
}

gboolean
MoonWindowGtk::crossing_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)user_data;

	window->SetCurrentDeployment ();

	if (window->surface) {
		MoonEvent *mevent = Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		mevent->DispatchToWindow (window);
		delete mevent;
		return true;
	}

	return false;
}

gboolean
MoonWindowGtk::focus_in (GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)user_data;

	window->SetCurrentDeployment ();

	if (window->surface) {
		MoonEvent *mevent = Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		mevent->DispatchToWindow (window);
		delete mevent;
		return true;
	}

	return false;
}

gboolean
MoonWindowGtk::focus_out (GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)user_data;

	window->SetCurrentDeployment ();

	if (window->surface) {
		MoonEvent *mevent = Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		mevent->DispatchToWindow (window);
		delete mevent;
		return true;
	}

	return false;
}

gboolean
MoonWindowGtk::key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)user_data;

	window->SetCurrentDeployment ();

	if (window->surface) {
		MoonEvent *mevent = Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		mevent->DispatchToWindow (window);
		delete mevent;
		return true;
	}

	return false;
}

gboolean
MoonWindowGtk::key_release (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)user_data;

	window->SetCurrentDeployment ();

	if (window->surface) {
		MoonEvent *mevent = Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		mevent->DispatchToWindow (window);
		delete mevent;
		return true;
	}

	return false;
}

void
MoonWindowGtk::widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)data;

	window->SetCurrentDeployment ();

	//printf ("Surface::size-allocate callback: current = %dx%d; new = %dx%d\n",
	//	s->width, s->height, allocation->width, allocation->height);
	
	bool emit_resize = false;

        if (window->width != allocation->width || window->height != allocation->height) {
                window->width = allocation->width;
                window->height = allocation->height;
		
		emit_resize = true;
	}

	if (window->surface)
		window->surface->HandleUIWindowAllocation (emit_resize);
}

void
MoonWindowGtk::widget_destroyed (GtkWidget *widget, gpointer user_data)
{
	MoonWindowGtk* window = (MoonWindowGtk*)user_data;

	window->widget = NULL;
	if (window->surface)
		window->surface->HandleUIWindowDestroyed (window);

}

gboolean
MoonWindowGtk::realized (GtkWidget *widget, gpointer user_data)
{
	MoonWindowGtk* window = (MoonWindowGtk*)user_data;

#ifdef USE_XRANDR
#if INTEL_DRIVERS_STOP_SUCKING
	// apparently the i965 drivers blank external screens when
	// getting the screen info (um, ugh?).  needless to say, this
	// annoyance is worse than not using the monitor's refresh as
	// the upper bound for our fps.
	//
	// http://lists.freedesktop.org/archives/xorg/2007-August/027616.html
	int event_base, error_base;
	GdkWindow *gdk_root = gtk_widget_get_root_window (widget);
	Display *dpy = GDK_WINDOW_XDISPLAY(gdk_root);
	Window root = GDK_WINDOW_XID (gdk_root);
	if (XRRQueryExtension (dpy, &event_base, &error_base)) {
		XRRScreenConfiguration *info = XRRGetScreenInfo (dpy,
								 root);
		short rate = XRRConfigCurrentRate (info);
		printf ("screen refresh rate = %d\n", rate);
		if (window->surface)
			window->surface->GetTimeManager()->SetMaximumRefreshRate (rate);
		XRRFreeScreenConfigInfo (info);
	}
#endif
#endif

	window->SetCurrentDeployment ();
	
	if (window->surface) {
		window->surface->HandleUIWindowUnavailable ();
		window->surface->HandleUIWindowAvailable ();
	}

	return true;
}

gboolean
MoonWindowGtk::unrealized (GtkWidget *widget, gpointer user_data)
{
	MoonWindowGtk* window = (MoonWindowGtk*)user_data;

	window->SetCurrentDeployment ();
	
	if (window->surface)
		window->surface->HandleUIWindowUnavailable ();

	return true;
}

cairo_surface_t *
MoonWindowGtk::CreateCairoSurface (GdkWindow *drawable, GdkVisual *visual, bool native, int width, int height)
{
	cairo_surface_t *surface;

	if (native) {
#if DEBUG
		int nw,nh;
		gdk_drawable_get_size (drawable, &nw, &nh);

		if (nw != width || nh != height)
			g_printf ("size of drawable doesn't match requested size");
#endif

		surface = cairo_xlib_surface_create (gdk_x11_drawable_get_xdisplay (drawable),
						     gdk_x11_drawable_get_xid (drawable),
						     GDK_VISUAL_XVISUAL (visual),
						     width, height);
	}
	else {
		if (backing_image_data == NULL) {
			//printf ("allocating backing_image_data for %d x %d\n", width, height);
			
			backing_image_data = (unsigned char*)g_malloc (backing_store_height * cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, backing_store_width));
		}

		surface = cairo_image_surface_create_for_data (backing_image_data,
							       CAIRO_FORMAT_ARGB32,
							       width, height,
							       cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32,
											      width));
	}

	return surface;
}

void
MoonWindowGtk::PaintToDrawable (GdkDrawable *drawable, GdkVisual *visual, GdkEventExpose *event, int off_x, int off_y, bool transparent, bool clear_transparent)
{
// 	LOG_UI ("Surface::PaintToDrawable (%p, %p, (%d,%d %d,%d), %d, %d, %d, %d)\n",
// 		drawable, visual, event->area.x, event->area.y, event->area.width, event->area.height,
// 		off_x, off_y, transparent, clear_transparent);
	
	SetCurrentDeployment ();

#if 0
#if TIME_REDRAW
	STARTTIMER (expose, "redraw");
#endif
	if (cache_size_multiplier == -1)
		cache_size_multiplier = gdk_drawable_get_depth (drawable) / 8 + 1;
#endif
	int width, height;
	gdk_drawable_get_size (drawable, &width, &height);
	Context *ctx;
	GdkRectangle area = event->area;
	MoonSurface *src;
	Rect r = Rect (area.x, area.y, area.width, area.height);

	if (!native)
		native = CreateCairoSurface (drawable, visual, true, width, height);
	cairo_xlib_surface_set_drawable (native,
					 gdk_x11_drawable_get_xid (drawable),
					 width, height);
	cairo_surface_set_device_offset (native, off_x, off_y);

	Region *region = new Region ();
	int count = 0;
	GdkRectangle *rects;
	gdk_region_get_rectangles (event->region, &rects, &count);
	while (count--) {
		GdkRectangle c = rects[count];
		region->Union (Rect (c.x, c.y, c.width, c.height));
	}

#ifdef USE_GALLIUM
	if (gctx) {
		ctx = gctx;
	}
	else {
		struct pipe_resource pt, *texture;
		GalliumSurface       *target;

		memset (&pt, 0, sizeof (pt));
		pt.target = PIPE_TEXTURE_2D;
		pt.format = PIPE_FORMAT_B8G8R8A8_UNORM;
		pt.width0 = width;
		pt.height0 = height;
		pt.depth0 = 1;
		pt.last_level = 0;
		pt.bind = PIPE_BIND_RENDER_TARGET | PIPE_BIND_TRANSFER_WRITE |
			PIPE_BIND_TRANSFER_READ;

		g_assert (screen);

		texture = (*screen->resource_create) (screen, &pt);

		target = new GalliumSurface (texture);
		pipe_resource_reference (&texture, NULL);
		ctx = new GalliumContext (target);
		target->unref ();

		if (gctxn < CONTEXT_CACHE_SIZE) {
			gctxn++;
			gctx = ctx;
		}
	}
#else
	CairoSurface *target = new CairoSurface (1, 1);
	ctx = new CairoContext (target);
	target->unref ();
#endif

	ctx->Push (Context::Group (r));
	/* if we are redirecting to an image surface clear that first */
	surface->Paint (ctx, region, transparent, true);

	r = ctx->Pop (&src);
	if (!r.IsEmpty ()) {
		cairo_surface_t *image = src->Cairo ();
		cairo_t         *cr = cairo_create (native);

		cairo_surface_flush (image);

		cairo_set_source_surface (cr, image, r.x, r.y);
		cairo_set_operator (cr, clear_transparent ? CAIRO_OPERATOR_SOURCE : CAIRO_OPERATOR_OVER);

		region->Draw (cr);
		cairo_fill (cr);

		cairo_destroy (cr);
		cairo_surface_destroy (image);
		src->unref ();
	}

#ifdef USE_GALLIUM
	if (ctx != gctx)
#endif
		delete ctx;

	delete region;

#if TIME_REDRAW
	ENDTIMER (expose, "redraw");
#endif

}

void
MoonWindowGtk::install_application (MoonWindowGtk *window)
{
	window->SetCurrentDeployment ();
	Application *app = Application::GetCurrent ();
	
	app->Install ();
}

void
MoonWindowGtk::uninstall_application (MoonWindowGtk *window)
{
	window->SetCurrentDeployment ();
	Deployment *deployment = Deployment::GetCurrent ();
	Application *application = deployment->GetCurrentApplication ();
	
	application->Uninstall ();
}


void
MoonWindowGtk::ShowMoonlightDialog ()
{
	if (!surface)
		return;

	SetCurrentDeployment();
	
	Deployment *deployment = Deployment::GetCurrent();
	
	MoonConfigDialogGtk *dialog = new MoonConfigDialogGtk (this, surface, deployment);

	dialog->Show ();
}

void
MoonWindowGtk::show_moonlight_dialog (MoonWindowGtk *window)
{
	window->ShowMoonlightDialog ();
}

void
MoonWindowGtk::RightClickMenu ()
{
	Deployment *deployment = Deployment::GetCurrent ();
	OutOfBrowserSettings *settings;
	GtkWidget *menu_item;
	GtkWidget *menu;
	char *name;

	menu = gtk_menu_new();

	menu_item = gtk_menu_item_new_with_label ("Moonlight Settings");

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect_swapped (G_OBJECT(menu_item), "activate", G_CALLBACK (show_moonlight_dialog), this);

	if (deployment && (settings = deployment->GetOutOfBrowserSettings ())) {
		Application *application = deployment->GetCurrentApplication ();
		
		switch (application->GetInstallState ()) {
		case InstallStateNotInstalled:
			if (application->IsInstallable () && settings->GetShowInstallMenuItem ()) {
				name = g_strdup_printf ("Install %s onto this computer...", settings->GetShortName ());
				menu_item = gtk_menu_item_new_with_label (name);
				gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
				g_signal_connect_swapped (G_OBJECT(menu_item), "activate", G_CALLBACK (install_application), this);
				g_free (name);
			}
			break;
		case InstallStateInstalled:
			menu_item = gtk_menu_item_new_with_label ("Remove this application...");
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
			g_signal_connect_swapped (G_OBJECT(menu_item), "activate", G_CALLBACK (uninstall_application), this);
			break;
		default:
			break;
		}
	}

	gtk_widget_show_all (menu);
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

gboolean
MoonWindowGtk::container_button_press_callback (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk *) user_data;

	if (event->button == 3) {
		window->RightClickMenu ();
		return TRUE;
	}

	return FALSE;
}

void
MoonWindowGtk::SetLeft (double left)
{
	if (this->left == left)
		return;

	this->left = left;

	if (left > G_MININT32 && top > G_MININT32) {
		gtk_window_move (GTK_WINDOW (widget), top, left);
	}
	else {
		// should we do something here?  hide the window?
	}
}

double
MoonWindowGtk::GetLeft ()
{
	return left;
}

void
MoonWindowGtk::SetTop (double top)
{
	if (this->top == top)
		return;

	this->top = top;

	if (left > G_MININT32 && top > G_MININT32) {
		gtk_window_move (GTK_WINDOW (widget), top, left);
	}
	else {
		// should we do something here?  hide the window?
	}
}

double
MoonWindowGtk::GetTop ()
{
	return top;
}

void
MoonWindowGtk::SetWidth (double width)
{
	this->width = width;
	if (width > 0 && height > 0)
		gtk_window_resize (GTK_WINDOW (widget), width, height);
}

void
MoonWindowGtk::SetHeight (double height)
{
	this->height = height;
	if (width > 0 && height > 0)
		gtk_window_resize (GTK_WINDOW (widget), width, height);
}

void
MoonWindowGtk::SetTitle (const char *title)
{
	gtk_window_set_title (GTK_WINDOW (widget), title);
}

void
MoonWindowGtk::SetIconFromPixbuf (MoonPixbuf *pixbuf)
{
	MoonPixbufGtk* pixbuf_gtk = (MoonPixbufGtk*)pixbuf;

	GList *icon_list;
	
	icon_list = gtk_window_get_icon_list (GTK_WINDOW (widget));
	icon_list = g_list_prepend (icon_list, (GdkPixbuf*)pixbuf_gtk->GetPlatformPixbuf());
	gtk_window_set_icon_list (GTK_WINDOW (widget), icon_list);
}

void
MoonWindowGtk::SetStyle (WindowStyle style)
{
	switch (style) {
	case WindowStyleBorderlessRoundCornersWindow:
	case WindowStyleNone:
		gtk_window_set_decorated (GTK_WINDOW (widget), false);
		break;
	default:
		/* by default, gtk enables window decorations */
		break;
	}
}
