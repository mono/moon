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
	item_set_transform (r, (double *) (&trans));
	surface_repaint (t, 0, 0, 300, 300);

#if VIDEO_DEMO
	Item *v; *v2

	v = video_new ("/home/everaldo/BoxerSmacksdownInhoffe.wmv", 0, 0);
	item_transform_set (v, (double *) (&trans));
	panel_child_add (t, v);

	v2 = video_new ("/home/everaldo/sawamu.wmv", 100, 30);
	panel_child_add (t, v2);
#endif	
	panel_child_add (t, r);

	gtk_widget_show_all (box);
	//gtk_timeout_add (60, repaint, w);
}


NPError 
moon_plugin_initialise ()
{
	gtk_init (0, 0);

	return NPERR_NO_ERROR;
}

NPError 
moon_plugin_shutdown ()
{
	return NPERR_NO_ERROR;
}

NPError 
moon_plugin_get_value (void *instance, NPPVariable variable, void *result)
{
	DEBUG ("GetValue %d (%x)", variable, variable);

	NPError err = NPERR_NO_ERROR;

	switch (variable) {
		case NPPVpluginNameString:
			*((char **)result) = PLUGIN_NAME;
			break;
		case NPPVpluginDescriptionString:
			*((char **)result) = PLUGIN_DESCRIPTION;
			break;
		case NPPVpluginNeedsXEmbed:
			*((PRBool *)result) = PR_TRUE;
			break;
		default:
			err = NPERR_GENERIC_ERROR;
	}
	return err;
}

NPError 
moon_plugin_new (NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
	DEBUG ("New");

	PluginInstance* This;

	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

    instance->pdata = NPN_MemAlloc (sizeof (PluginInstance));

    This = (PluginInstance*) instance->pdata;

    if (This == NULL) 
    {
        return NPERR_OUT_OF_MEMORY_ERROR;
    }

    memset (This, 0, sizeof (PluginInstance));

    /* mode is NP_EMBED, NP_FULL, or NP_BACKGROUND (see npapi.h) */
    This->mode = mode;
    This->instance = instance;
    This->dialog_up = 0;
    This->windowless = FALSE;

	return NPERR_NO_ERROR;
}

NPError 
moon_plugin_destroy (NPP instance, NPSavedData** save)
{
	DEBUG ("Destroy");
	return NPERR_NO_ERROR;
}

NPError 
moon_plugin_set_window (NPP instance, NPWindow* window)
{
    PluginInstance* This;
    NPSetWindowCallbackStruct *ws_info;

    int xembedSupported = 0;

	DEBUG ("SetWindow %d %d", window->width, window->height);

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    This = (PluginInstance*) instance->pdata;

    if (This == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    ws_info = (NPSetWindowCallbackStruct *)window->ws_info;

    /* Mozilla likes to re-run its greatest hits */
    /*if ((window == This->window) &&
        (window->x == This->x) &&
        (window->y == This->y) &&
        (window->width == This->width) &&
        (window->height == This->height)) {*/
if (window == This->window) {
        printf("  (window re-run; returning)\n");
        return NPERR_NO_ERROR;
    }

    NPN_GetValue(instance, NPNVSupportsXEmbedBool, &xembedSupported);
    if (!xembedSupported)
    {
        printf("DiamondX: XEmbed not supported\n");
        return NPERR_GENERIC_ERROR;
    }

    This->window = window;
    This->x = window->x;
    This->y = window->y;
    This->width = window->width;
    This->height = window->height;

	moon_plugin_create_window (instance, window);

	return NPERR_NO_ERROR;
}

NPError
moon_plugin_create_window (NPP instance, NPWindow* window)
{
	DEBUG ("moon_plugin_create_window %d %d", window->width, window->height);

    PluginInstance* Instance;
    NPSetWindowCallbackStruct *ws_info;

    int xembedSupported = 0;

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    instance->pdata = NPN_MemAlloc (sizeof (PluginInstance));

    Instance = (PluginInstance*) instance->pdata;

    if (Instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    NPN_GetValue (instance, NPNVSupportsXEmbedBool, &xembedSupported);
    if (!xembedSupported)
    {
        DEBUG ("XEmbed not supported\n");
        return NPERR_GENERIC_ERROR;
    }

    Instance->window = window;
    Instance->x = window->x;
    Instance->y = window->y;
    Instance->width = window->width;
    Instance->height = window->height;

    /*  GtkPlug container and drawing canvas inside */
    Instance->container = gtk_plug_new ((GdkNativeWindow) window->window);
    Instance->canvas = gtk_drawing_area_new ();

	//gtk_container_add (GTK_CONTAINER (Instance->container), Instance->canvas);

    GTK_WIDGET_SET_FLAGS (GTK_WIDGET (Instance->canvas), GTK_CAN_FOCUS);

    gtk_widget_add_events (
        Instance->canvas,
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

    g_signal_connect (G_OBJECT(Instance->canvas), "event", G_CALLBACK (moon_plugin_canvas_event), Instance);


	moon_plugin_demo (Instance);

	gtk_widget_show (Instance->canvas);
	gtk_widget_show (Instance->container);

    return NPERR_NO_ERROR;
}

NPError
moon_plugin_new_stream (NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
	DEBUG ("moon_plugin_new_stream");
	return NPERR_NO_ERROR;
}

void
moon_plugin_stream_as_file (NPP instance, NPStream* stream, const char* fname)
{
	DEBUG ("moon_plugin_stream_as_file");
}

NPError
moon_plugin_destroy_stream (NPP instance, NPStream* stream, NPError reason)
{
	DEBUG ("moon_plugin_destroy_stream");

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    return NPERR_NO_ERROR;
}

int32
moon_plugin_write_ready (NPP instance, NPStream* stream)
{
	DEBUG ("moon_plugin_write_ready");

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    NPN_DestroyStream(instance, stream, NPRES_DONE);

    return -1L;
}

int32
moon_plugin_write (NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer)
{
	DEBUG ("moon_plugin_write");

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    NPN_DestroyStream(instance, stream, NPRES_DONE);

    return -1L;
}

void
moon_plugin_url_notify (NPP instance, const char* url, NPReason reason, void* notifyData)
{
	DEBUG ("moon_plugin_url_notify");
}

void
moon_plugin_print (NPP instance, NPPrint* platformPrint)
{
	DEBUG ("moon_plugin_print");
}

int16 
moon_plugin_handle_event (NPP instance, void* event)
{
	XGraphicsExposeEvent exposeEvent;
	XEvent *nsEvent;

	nsEvent = (XEvent *)event;
	exposeEvent = nsEvent->xgraphicsexpose;

	DEBUG ("event: x, y, w, h = %d, %d, %d, %d; display @ %p, window/drawable = %d\n",
			exposeEvent.x,
			exposeEvent.y,
			exposeEvent.width,
			exposeEvent.height,
			exposeEvent.display,
			exposeEvent.drawable);

	windowlessWindow.window = exposeEvent.display;
	windowlessWindow.x = exposeEvent.x;
	windowlessWindow.y = exposeEvent.y;
	windowlessWindow.width = exposeEvent.width;
	windowlessWindow.height = exposeEvent.height;
	windowlessWindow.ws_info = (void *) exposeEvent.drawable;

	moon_plugin_set_window (instance, &windowlessWindow);

	return 0;
}

gboolean
moon_plugin_canvas_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	DEBUG ("moon_plugin_canvas_event");

    gboolean handled = 0;
    PluginInstance *plugin = (PluginInstance *)user_data;

    GdkEventButton *button;

    switch (event->type) {

    case GDK_BUTTON_PRESS:
        button = (GdkEventButton *) event;
        if (button->button == 3) {
			moon_plugin_show_menu (plugin);
		}
        handled = 1;
        break;

    default:
        break;
    }

    return handled;

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

static void moon_plugin_menu_about (PluginInstance *plugin)
{
	DEBUG ("moon_plugin_menu_about Clicked!");
	// TODO: Implement an about Window.
}

