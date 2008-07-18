#include "window-gtk.h"

MoonWindowGtk::MoonWindowGtk (bool fullscreen, int w, int h)
	: MoonWindow (w, h)
{
	this->fullscreen = fullscreen;

	if (fullscreen)
		InitializeFullScreen();
	else
		InitializeNormal();
}

MoonWindowGtk::~MoonWindowGtk ()
{
	DisableEvents ();
	gtk_widget_destroy (widget);
}


void
MoonWindowGtk::InitializeFullScreen ()
{
	widget = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	// Get the screen size
	width = gdk_screen_get_width (gdk_screen_get_default ());
	height = gdk_screen_get_height (gdk_screen_get_default ());

	gtk_window_fullscreen (GTK_WINDOW (widget));

	InitializeCommon ();

	Show();
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
			       //GDK_POINTER_MOTION_HINT_MASK |
			       GDK_KEY_PRESS_MASK |
			       GDK_KEY_RELEASE_MASK |
			       GDK_BUTTON_PRESS_MASK |
			       GDK_BUTTON_RELEASE_MASK |
			       GDK_FOCUS_CHANGE_MASK);
	
	GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_FOCUS);
}

void
MoonWindowGtk::Resize (int width, int height)
{
	gtk_widget_set_size_request (widget, width, height);
	gtk_widget_queue_resize (widget);
}

void
MoonWindowGtk::SetCursor (GdkCursor *cursor)
{
	if (widget->window)
		gdk_window_set_cursor (widget->window, cursor);
}

void
MoonWindowGtk::Invalidate (Rect r)
{
	gtk_widget_queue_draw_area (widget,
				    (int) (widget->allocation.x + r.x), 
				    (int) (widget->allocation.y + r.y), 
				    (int) r.w, (int)r.h);
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

	if (event->button != 1)
		return false;

	if (window->surface)
		window->surface->HandleUIButtonPress (event);
	// ignore HandleUIButtonPress's return value, and always
	// return true here, or it gets bubbled up to firefox.
	return true;
}

gboolean
MoonWindowGtk::button_release (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)data;

	if (window->surface)
		window->surface->HandleUIButtonRelease (event);
	// ignore HandleUIButtonRelease's return value, and always
	// return true here, or it gets bubbled up to firefox.
	return true;
}

gboolean
MoonWindowGtk::motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)user_data;

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

	return window->surface ? window->surface->HandleUICrossing (event) : false;
}

gboolean
MoonWindowGtk::focus_in (GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)user_data;

	return window->surface ? window->surface->HandleUIFocusIn (event) : false;
}

gboolean
MoonWindowGtk::focus_out (GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)user_data;

	return window->surface ? window->surface->HandleUIFocusIn (event) : false;
}

gboolean
MoonWindowGtk::key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)user_data;
	return window->surface ? window->surface->HandleUIKeyPress (event) : false;
}

gboolean
MoonWindowGtk::key_release (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)user_data;
	return window->surface ? window->surface->HandleUIKeyRelease (event) : false;
}

void
MoonWindowGtk::widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer data)
{
	MoonWindowGtk *window = (MoonWindowGtk*)data;

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

	if (window->surface)
		window->surface->HandleUIWindowAvailable ();

	return true;
}

gboolean
MoonWindowGtk::unrealized (GtkWidget *widget, gpointer user_data)
{
	MoonWindowGtk* window = (MoonWindowGtk*)user_data;

	if (window->surface)
		window->surface->HandleUIWindowUnavailable ();

	return true;
}

MoonWindowGtk*
moon_window_gtk_new (bool fullscreen, int width, int height)
{
	return new MoonWindowGtk (fullscreen, width, height);
}

GtkWidget*
moon_window_gtk_get_widget (MoonWindowGtk *window)
{
	return window->GetWidget();
}
