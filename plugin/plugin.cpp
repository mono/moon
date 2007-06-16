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
#include "plugin-class.h"
#include "moon-mono.h"

#define NEW_STREAM_REQUEST_AS_FILE vm_missing_file

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

PluginInstance::PluginInstance (NPP instance, uint16 mode)
{
	this->mode = mode;
	this->instance = instance;
	this->window = NULL;
	this->rootobject = NULL;

	this->container = NULL;
	this->canvas = NULL;
	this->surface = NULL;

	this->sourceUrl = NULL;

	// Property fields
	this->initParams = false;
	this->isLoaded = false;
	this->source = NULL;

	this->windowless = false;
	
	this->vm_missing_url = NULL;
	this->vm_missing_file = NULL;
	this->mono_loader_object = NULL;
}

PluginInstance::~PluginInstance ()
{
	// finalization code is under Finalize (), it was moved because we cant
	// free resources, it causes browser reload problems. It must be checked
	// and fixed later.
}

void 
PluginInstance::Initialize (int argc, char* const argn[], char* const argv[])
{
	char *url = NULL;

	for (int i = 0; i < argc; i++) {
		if (argn[i] == NULL)
			continue;

		// initParams.
		if (!strcasecmp (argn[i], "initParams")) {
			this->initParams = argv[i];
		}

		// Source url handle.
		if (!strcasecmp (argn[i], "src") || !strcasecmp (argn[i], "source")) {
			this->source = argv[i];
		}
	}

	if (this->source) {
		NPN_GetURL (this->instance, this->source, NULL);
	}
}

void 
PluginInstance::Finalize ()
{
	// Container must be destroyed or we have segfault when browser's closes.
	if (this->container != NULL)
		gtk_widget_destroy (this->container);
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
				rootclass = new PluginRootClass ();

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
	this->canvas = new Canvas ();
	surface_attach (this->surface, canvas);
	gtk_container_add (GTK_CONTAINER (container), this->surface->drawing_area);
	gtk_widget_show_all (this->container);
}

NPError
PluginInstance::NewStream (NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
	DEBUGMSG ("NewStream (%s) %s", this->source, stream->url);

	if (!this->sourceUrl) {
		this->sourceUrl = stream->url;

	*stype = NP_ASFILEONLY;

	} else {
		if (vm_missing_url == NEW_STREAM_REQUEST_AS_FILE){
			*stype = NP_ASFILEONLY;
			vm_missing_url = stream->url;
		} else
			*stype = NP_NORMAL;
	}


	return NPERR_NO_ERROR;
}

NPError
PluginInstance::DestroyStream (NPStream* stream, NPError reason)
{
	return NPERR_NO_ERROR;
}

//
// Tries to load the XAML file, the parsing might fail because a
// required dependency is not available, so we need to queue the
// request to fetch the data.
//
void
PluginInstance::TryLoad ()
{
	int error = 0;

	vm_missing_file = vm_loader_try (mono_loader_object, &error);

	if (vm_missing_file != NULL){
		vm_missing_url = NEW_STREAM_REQUEST_AS_FILE;
		NPN_GetURL (instance, vm_missing_file, NULL);
		return;
	}

	//
	// missing file was NULL, if error is set, display some message
	//
}

void
PluginInstance::StreamAsFile (NPStream* stream, const char* fname)
{
	DEBUGMSG ("StreamAsFile: %s", fname);

	if (!strcasecmp (this->sourceUrl, stream->url)) {
		LoadFromXaml (fname);
		this->isLoaded = true;
	}

	if (vm_missing_url != NULL && vm_missing_url != NEW_STREAM_REQUEST_AS_FILE && !strcmp (stream->url, vm_missing_url)){
		//
		// We got the dependency requested
		//
		vm_insert_mapping (mono_loader_object, vm_missing_file, vm_missing_url);
		g_free (vm_missing_file);
		vm_missing_file = NULL;

		// retry to load
		TryLoad ();
	}
}

void
PluginInstance::LoadFromXaml (const char* fname)
{
	DEBUGMSG ("LoadFromXaml: %s", fname);
#ifdef RUNTIME
	mono_loader_object = vm_xaml_loader_new (this, this->surface, fname);
	TryLoad ();
#else	
	surface_attach (this->surface, xaml_create_from_file (fname, NULL));
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

/*** Getters and Setters ******************************************************/

void
PluginInstance::setSource (const char *value)
{
	if (!value || !strcasecmp (this->source, value))
		return;

	this->source = (char *) NPN_MemAlloc (strlen (value) + 1);
	strcpy (this->source, value);

	this->sourceUrl = NULL;
	NPN_GetURL(this->instance, value, NULL);
}

char *
PluginInstance::getBackground ()
{
	char *background = "";
	return background;
}

void
PluginInstance::setBackground (const char *value)
{
	// do nothing, our surface theres no backcolor at moment.
}

bool
PluginInstance::getEnableFramerateCounter ()
{
	return false;
}

bool
PluginInstance::getEnableRedrawRegions ()
{
	return false;
}

void
PluginInstance::setEnableRedrawRegions (bool value)
{
	// not implemented yet.
}

bool
PluginInstance::getEnableHtmlAccess ()
{
	return true;
}

bool
PluginInstance::getWindowless ()
{
	return this->windowless;
}

int32
PluginInstance::getActualHeight ()
{
	return window->height;
}

int32
PluginInstance::getActualWidth ()
{
	return window->width;
}
