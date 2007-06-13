/*
 * moon-plugin.cpp: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "plugin.h"
#include "moon-mono.h"

void 
plugin_menu_about (PluginInstance *plugin)
{
	DEBUGMSG ("*** plugin_menu_about");
}

gboolean
plugin_show_menu (PluginInstance *plugin)
{
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu = gtk_menu_new();

	menu_item = gtk_menu_item_new_with_label (g_strdup_printf ("%s %s", PLUGIN_OURNAME, PLUGIN_OURVERSION));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect_swapped (G_OBJECT(menu_item), "activate", G_CALLBACK (plugin_menu_about), plugin);

	gtk_widget_show_all (menu);
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
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
				plugin_show_menu (plugin);
			}
			handled = 1;
			break;

		default:
			break;
	}

	return handled;
}

/*** PluginInstance:: *********************************************************/

static PluginRootClass* rootclass = NULL;

PluginInstance::PluginInstance (NPP instance, uint16 mode)
{
	this->mode = mode;
	this->instance = instance;
	this->window = NULL;
	this->rootobject = NULL;

	this->container = NULL;
	this->canvas = NULL;
	this->surface = NULL;

	// Property fields
	this->isloaded = false;
	this->source = NULL;
}

PluginInstance::~PluginInstance ()
{
	// Container must be destroyed or we have segfault when browser's closes.
	if (this->container != NULL)
		gtk_widget_destroy (this->container);
}

void 
PluginInstance::Initialize (int argc, char* const argn[], char* const argv[])
{
	char *url = NULL;

	for (int i = 0; i < argc; i++) {
		if (argn[i] == NULL)
			continue;

		// Source url handle.
		if (!strcasecmp (argn[i], "src") || !strcasecmp (argn[i], "source")) {
			this->source = argv[i];
		}
	}

	if (this->source) {
		NPN_GetURL (this->instance, this->source, NULL);
	}
}

NPError 
PluginInstance::GetValue (NPPVariable variable, void *result)
{
	NPError err = NPERR_NO_ERROR;

	switch (variable) {
		case NPPVpluginNeedsXEmbed:
			*((PRBool *)result) = PR_TRUE;
			break;

#ifdef SCRIPTING
		case NPPVpluginScriptableNPObject:
			if (!rootclass)
				rootclass = new PluginRootClass (this->instance);

			if (!this->rootobject)
				this->rootobject = NPN_CreateObject (this->instance, rootclass);
			else
				NPN_RetainObject (this->rootobject);

			if (!this->rootobject)
				err = NPERR_OUT_OF_MEMORY_ERROR;
			else
				*((NPObject **) result) =  this->rootobject;

			break;
#endif
		default:
			err = NPERR_INVALID_PARAM;
	}

	return err;
}

NPError
PluginInstance::SetValue (NPNVariable variable, void *value)
{
	return NPERR_NO_ERROR;
}

NPError 
PluginInstance::SetWindow (NPWindow* window)
{
	if (window == this->window)
		return NPERR_NO_ERROR;

	NPN_GetValue(this->instance, NPNVSupportsXEmbedBool, &this->xembed_supported);
	if (!this->xembed_supported)
	{
		DEBUGMSG ("*** XEmbed not supported");
		return NPERR_GENERIC_ERROR;
	}

	this->window = window;
	this->CreateWindow ();

	return NPERR_NO_ERROR;
}

void 
PluginInstance::CreateWindow ()
{
	DEBUGMSG ("*** creating window (%d,%d,%d,%d)", window->x, window->y, window->width, window->height);

	//  GtkPlug container and surface inside
	this->container = gtk_plug_new (reinterpret_cast <GdkNativeWindow> (window->window));

	// Connect signals to container
	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (this->container), GTK_CAN_FOCUS);

	gtk_widget_add_events (
		this->container,
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

	g_signal_connect (G_OBJECT(this->container), "event", G_CALLBACK (plugin_event_callback), this);

	this->surface = surface_new (window->width, window->height);
#ifndef RUNTIME
	this->canvas = new Canvas ();
	surface_attach (this->surface, canvas);
#endif
	gtk_container_add (GTK_CONTAINER (container), this->surface->drawing_area);
	gtk_widget_show_all (this->container);
}

NPError
PluginInstance::NewStream (NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
	*stype = NP_ASFILEONLY;

	return NPERR_NO_ERROR;
}

NPError
PluginInstance::DestroyStream (NPStream* stream, NPError reason)
{
	return NPERR_NO_ERROR;
}

void
PluginInstance::StreamAsFile (NPStream* stream, const char* fname)
{
	DEBUGMSG ("StreamAsFile: %s", fname);

	// TODO
	//   1. Call a helper method in managed code that would create the new AppDomain
	//   2. Call a helper method (maybe the same) that would process the input parameters
	//   3. Remove the call here below, and let managed code load the XAML

#ifdef RUNTIME
	vm_load_xaml (this->surface, fname);
#else
	this->isloaded = true;
	panel_child_add (this->canvas, xaml_create_from_file (fname, NULL));
#endif
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
	return 0;
}
