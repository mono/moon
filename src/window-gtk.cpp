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
 
#include "window-gtk.h"
#include "deployment.h"
#include "timemanager.h"

MoonWindowGtk::MoonWindowGtk (bool fullscreen, int w, int h, MoonWindow *parent, Surface *surface)
	: MoonWindow (w, h, surface)
{
	this->fullscreen = fullscreen;

	if (fullscreen)
		InitializeFullScreen(parent);
	else
		InitializeNormal();
}

MoonWindowGtk::~MoonWindowGtk ()
{
	/* gtk_widget_destroy can cause reentry (into another plugin if this destruction causes layout changes) */
	DeploymentStack deployment_push_pop;
	DisableEvents ();
	if (widget != NULL)
		gtk_widget_destroy (widget);
}

GdkWindow *
MoonWindowGtk::GetGdkWindow ()
{
	GdkWindow *parent_window = gtk_widget_get_parent_window (widget);
	if (parent_window == NULL)
		parent_window = widget->window;
	
	g_object_ref (parent_window);
	return parent_window;
}

void
MoonWindowGtk::InitializeFullScreen (MoonWindow *parent)
{
	widget = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	// only fullscreen on the monitor the plugin is on
	GdkWindow *gdk = parent->GetGdkWindow ();
	int monitor = gdk_screen_get_monitor_at_window (gdk_screen_get_default (), gdk);
	GdkRectangle bounds;
	gdk_screen_get_monitor_geometry (gdk_screen_get_default (), monitor, &bounds);
	width = bounds.width;
	height = bounds.height;
	gtk_window_move (GTK_WINDOW (widget), bounds.x, bounds.y);

	gtk_window_fullscreen (GTK_WINDOW (widget));

	InitializeCommon ();

	Show();

	g_object_unref (gdk);
}

void
MoonWindowGtk::InitializeNormal ()
{
	if (width == -1 || height == -1) {
		g_warning ("you must specify width and height when creating a non-fullscreen gtk window");
		width = 0;
		height = 0;
	}

	widget = gtk_event_box_new ();

	gtk_event_box_set_visible_window (GTK_EVENT_BOX (widget), false);

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
			       ((moonlight_flags & RUNTIME_INIT_DESKTOP_EXTENSIONS) != 0 ? GDK_SCROLL_MASK : 0) |
			       GDK_FOCUS_CHANGE_MASK);
	
	GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_FOCUS);
}

void
MoonWindowGtk::Resize (int width, int height)
{
	gtk_widget_set_size_request (widget, width, height);
	gtk_widget_queue_resize (widget);
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
MoonWindowGtk::SetCursor (MouseCursor cursor)
{
	if (widget->window) {

		GdkCursor *c = NULL;
		switch (cursor) {
		case MouseCursorDefault:
			c = NULL;
			break;
		case MouseCursorArrow:
			c = gdk_cursor_new (GDK_LEFT_PTR);
			break;
		case MouseCursorHand:
			c = gdk_cursor_new (GDK_HAND2);
			break;
		case MouseCursorWait:
			c = gdk_cursor_new (GDK_WATCH);
			break;
		case MouseCursorIBeam:
			c = gdk_cursor_new (GDK_XTERM);
			break;
		case MouseCursorStylus:
			c = gdk_cursor_new_from_pixbuf (gdk_display_get_default (), gdk_pixbuf_new_from_xpm_data ((const char**) dot), 0, 0);
			break;
		case MouseCursorEraser:
			c = gdk_cursor_new_from_pixbuf (gdk_display_get_default (), gdk_pixbuf_new_from_xpm_data ((const char**) eraser), 8, 8);
			break;
		case MouseCursorSizeNS:
			c = gdk_cursor_new (GDK_SB_V_DOUBLE_ARROW);
			break;
		case MouseCursorSizeWE:
			c = gdk_cursor_new (GDK_SB_H_DOUBLE_ARROW);
			break;
		case MouseCursorNone:
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
MoonWindowGtk::HandleEvent (XEvent *event)
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
	g_signal_connect (widget, "expose-event", G_CALLBACK (expose_event), this);
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

	window->SetCurrentDeployment ();
	
	if (!window->surface)
		return true;

	// we draw to a backbuffer pixmap, then transfer the contents
	// to the widget's window.
	GdkPixmap *pixmap = gdk_pixmap_new (widget->window,
					    MAX (event->area.width, 1), MAX (event->area.height, 1), -1);

	window->surface->PaintToDrawable (pixmap,
					  gdk_drawable_get_visual (widget->window),
					  event,
					  widget->allocation.x,
					  widget->allocation.y,
					  window->GetTransparent (),
					  true);

	GdkGC *gc = gdk_gc_new (pixmap);

	gdk_gc_set_clip_region (gc, event->region);

	gdk_draw_drawable (widget->window, gc, pixmap,
			   0, 0,
			   event->area.x, event->area.y,
			   event->area.width, event->area.height);
	
	g_object_unref (pixmap);
	g_object_unref (gc);

	return true;
}

gboolean
MoonWindowGtk::button_press (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)data;

	window->SetCurrentDeployment ();

	if (event->button != 1 && event->button != 3)
		return false;

	if (window->surface)
		window->surface->HandleUIButtonPress (event);
	
	// If we don't support right clicks (i.e. inside the browser)
	// return false here
	if (event->button == 3 && (moonlight_flags & RUNTIME_INIT_DESKTOP_EXTENSIONS) == 0)
		return false;

	// ignore HandleUIButtonPress's return value, and always
	// return true here, or it gets bubbled up to firefox.
	return true;
}

gboolean
MoonWindowGtk::button_release (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)data;

	window->SetCurrentDeployment ();

	if (window->surface)
		window->surface->HandleUIButtonRelease (event);
	// ignore HandleUIButtonRelease's return value, and always
	// return true here, or it gets bubbled up to firefox.
	return true;
}

gboolean
MoonWindowGtk::scroll (GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)data;

	window->SetCurrentDeployment ();

	if (window->surface)
		window->surface->HandleUIScroll (event);
	// ignore HandleUIScroll's return value, and always
	// return true here, or it gets bubbled up to firefox.
	return true;
}

gboolean
MoonWindowGtk::motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)user_data;

	window->SetCurrentDeployment ();

	if (window->surface)
		window->surface->HandleUIMotion (event);
	// ignore HandleUIMotion's return value, and always
	// return true here, or it gets bubbled up to firefox.
	return true;
}

gboolean
MoonWindowGtk::crossing_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)user_data;

	window->SetCurrentDeployment ();

	if (window->surface) {
		window->surface->HandleUICrossing (event);
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
		window->surface->HandleUIFocusIn (event);
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
		window->surface->HandleUIFocusOut (event);
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
		window->surface->HandleUIKeyPress (event);
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
		window->surface->HandleUIKeyRelease (event);
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

#if SANITY
	Deployment::SetCurrent (NULL);
#endif

	return true;
}

gboolean
MoonWindowGtk::unrealized (GtkWidget *widget, gpointer user_data)
{
	MoonWindowGtk* window = (MoonWindowGtk*)user_data;

	window->SetCurrentDeployment ();
	
	if (window->surface)
		window->surface->HandleUIWindowUnavailable ();

#if SANITY
	Deployment::SetCurrent (NULL);
#endif

	return true;
}
