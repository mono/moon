/*
 * moon-plugin.c: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#include "moon-plugin.h"
#include "runtime.h"

static void moon_plugin_demo (PluginInstance *plugin)
{
	DEBUG ("*** moon_plugin_demo");

	GtkWidget *w, *box, *button;
	cairo_matrix_t trans;

	Rectangle *r;

	box = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (plugin->container), box);
	gtk_widget_show_all (box);

	//button = gtk_button_new_with_label ("dingus");
	//gtk_container_add (GTK_CONTAINER (box), button);

	// Create our objects
	Surface *t = surface_new (300, 600);
	gtk_container_add (GTK_CONTAINER (box), t->drawing_area);

	r = rectangle_new (50, 50, 100, 100);
	Color c = Color (1.0, 0.0, 0.5, 0.5);
	shape_set_stroke (r, new SolidColorBrush (c));
	cairo_matrix_init_rotate (&trans, 0.4);
	//item_set_transform (r, (double *) (&trans));
	surface_repaint (t, 0, 0, 300, 300);

#if VIDEO_DEMO
	UIElement *v, *v2;

	v = video_new ("/home/everaldo/BoxerSmacksdownInhoffe.wmv", 0, 0);
	//item_transform_set (v, (double *) (&trans));
	panel_child_add (t, v);

	v2 = video_new ("/home/everaldo/sawamu.wmv", 100, 30);
	panel_child_add (t, v2);
#endif	
	panel_child_add (t, r);

	gtk_widget_show_all (box);
	//gtk_timeout_add (60, repaint, w);
}

static void moon_plugin_menu_about (PluginInstance *plugin)
{
	DEBUG ("moon_plugin_menu_about Clicked!");
	// TODO: Implement an about Window.
}

gboolean
moon_plugin_show_menu (PluginInstance *plugin)
{
	DEBUG ("moon_plugin_show_menu");

	GtkWidget *menu;
	GtkWidget *menu_item;

	menu = gtk_menu_new();

	menu_item = gtk_menu_item_new_with_label ("Help...");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);
	
	menu_item = gtk_separator_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);

	menu_item = gtk_menu_item_new_with_label (g_strdup_printf ("%s %s", PLUGIN_NAME, PLUGIN_VERSION));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect_swapped (G_OBJECT(menu_item), "activate", G_CALLBACK (moon_plugin_menu_about), plugin);
	gtk_widget_show (menu_item);

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
	    0, gtk_get_current_event_time());
}

gboolean
plugin_event_callback (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    gboolean handled = 0;

	PluginInstance *plugin = (PluginInstance *) user_data;
    GdkEventButton *event_button;

    switch (event->type) {

    case GDK_BUTTON_PRESS:
		event_button = (GdkEventButton *) event;
		if (event_button->button == 3) {
			moon_plugin_show_menu (plugin);
		}
		handled = 1;
		break;

	default:
		break;
	}

	return handled;
}

PluginInstance::PluginInstance ()
{
}

PluginInstance::~PluginInstance ()
{
}

NPError 
PluginInstance::GetValue (NPPVariable variable, void *result)
{
	return NPERR_NO_ERROR;
}

NPError 
PluginInstance::SetWindow (NPWindow* window)
{
	if (window == this->window) {
		// TODO: Implement resize and other things.
		return NPERR_NO_ERROR;
	}

	NPN_GetValue(this->instance, NPNVSupportsXEmbedBool, &this->xembed_supported);
	if (!this->xembed_supported)
	{
		DEBUG ("*** XEmbed not supported");
		return NPERR_GENERIC_ERROR;
	}

	this->window = window;
	this->CreateControls ();

	//moon_plugin_create_window (this->instance, window);

	return NPERR_NO_ERROR;
}

void 
PluginInstance::CreateControls ()
{
	DEBUG ("creating window (%d,%d,%d,%d)", window->x, window->y, window->width, window->height);

	box = gtk_hbox_new (FALSE, 0);
	
	/*  GtkPlug container and drawing canvas inside */
	this->container = gtk_plug_new ((GdkNativeWindow) window->window);
	this->canvas = gtk_drawing_area_new ();

	gtk_container_add (GTK_CONTAINER (this->container), this->canvas);

	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (this->canvas), GTK_CAN_FOCUS);

	gtk_widget_add_events (
		this->canvas,
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

	g_signal_connect (G_OBJECT(this->canvas), "event", G_CALLBACK (plugin_event_callback), this);

	//moon_plugin_demo (Instance);

	gtk_widget_show (this->canvas);
	gtk_widget_show (this->container);
}

NPError
PluginInstance::NewStream (NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
	return NPERR_NO_ERROR;
}

void
PluginInstance::StreamAsFile (NPStream* stream, const char* fname)
{
	// nothing to do.
}

NPError
PluginInstance::DestroyStream (NPStream* stream, NPError reason)
{
	return NPERR_NO_ERROR;
}

int32
PluginInstance::WriteReady (NPStream* stream)
{
	return -1L;
}

int32
PluginInstance::Write (NPStream* stream, int32 offset, int32 len, void* buffer)
{
	return -1L;
}

void
PluginInstance::UrlNotify (const char* url, NPReason reason, void* notifyData)
{
	// nothing to do.
}

void
PluginInstance::Print (NPPrint* platformPrint)
{
	// nothing to do.
}

int16
PluginInstance::EventHandle (void* event)
{
	/* Our plugin is a windowed so we dont need the windowless code */
	return 0;
}
