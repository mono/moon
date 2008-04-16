/*
 * moon-plugin.cpp: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (ecanuto@novell.com)
 *   Michael Dominic K. <mdk@mdk.am>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "plugin.h"
#include "plugin-class.h"
#include "plugin-debug.h"
#include "plstr.h"
#include "moon-mono.h"
#include "downloader.h"
#include "plugin-downloader.h"
#define Visual _XxVisual
#define Region _XxRegion
#include "gdk/gdkx.h"
#undef Visual
#undef Region

extern guint32 moonlight_flags;

/* gleaned from svn log of the moon module, as well as olive/class/{agclr,agmono,System.Silverlight} */
static const char *moonlight_authors[] = {
	"Andreia Gaita <avidigal@novell.com>",
	"Atsushi Enomoto <atsushi@ximian.com>",
	"Chris Toshok <toshok@ximian.com>",
	"Dick Porter <dick@ximian.com>",
	"Everaldo Canuto <ecanuto@novell.com>",
	"Jackson Harper <jackson@ximian.com>",
	"Jeffrey Stedfast <fejj@novell.com>",
	"Larry Ewing <lewing@novell.com>",
	"Marek Habersack <mhabersack@novell.com>",
	"Miguel de Icaza <miguel@novell.com>",
	"Rodrigo Kumpera <rkumpera@novell.com>",
	"Rolf Bjarne Kvinge <RKvinge@novell.com>",
	"Sebastien Pouliot <sebastien@ximian.com>",
	"Jb Evain <jbevain@novell.com>",
	"Geoff Norton <gnorton@novell.com>",
	NULL
};

void
plugin_menu_about (PluginInstance *plugin)
{
	GtkAboutDialog *about = GTK_ABOUT_DIALOG (gtk_about_dialog_new ());

	gtk_about_dialog_set_name (about, PLUGIN_OURNAME);
	gtk_about_dialog_set_version (about, VERSION);

	gtk_about_dialog_set_copyright (about, "Copyright 2007 Novell, Inc. (http://www.novell.com/)");
	gtk_about_dialog_set_website (about, "http://mono-project.com/Moonlight");
	gtk_about_dialog_set_website_label (about, "Project Website");

	gtk_about_dialog_set_authors (about, moonlight_authors);

	/* Newer gtk+ versions require this for the close button to work */
	g_signal_connect_swapped (about,
				  "response",
				  G_CALLBACK (gtk_widget_destroy),
				  about);

	gtk_dialog_run (GTK_DIALOG (about));
}

void
plugin_properties (PluginInstance *plugin)
{
	plugin->Properties ();
}

void
plugin_show_menu (PluginInstance *plugin)
{
	GtkWidget *menu;
	GtkWidget *menu_item;
	char *name;

	menu = gtk_menu_new();

	name = g_strdup_printf ("%s %s", PLUGIN_OURNAME, VERSION);
	menu_item = gtk_menu_item_new_with_label (name);
	g_free (name);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect_swapped (G_OBJECT(menu_item), "activate", G_CALLBACK (plugin_menu_about), plugin);

	menu_item = gtk_menu_item_new_with_label ("Properties");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect_swapped (G_OBJECT(menu_item), "activate", G_CALLBACK (plugin_properties), plugin);

#ifdef DEBUG
	menu_item = gtk_menu_item_new_with_label ("Debug");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect_swapped (G_OBJECT(menu_item), "activate", G_CALLBACK (plugin_debug), plugin);
	
	menu_item = gtk_menu_item_new_with_label ("Sources");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect_swapped (G_OBJECT(menu_item), "activate", G_CALLBACK (plugin_sources), plugin);
#endif

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
			if (event_button->button == 3)
				plugin_show_menu (plugin);
			
			handled = 1;
			break;

		default:
			break;
	}

	return handled;
}

/*** PluginInstance:: *********************************************************/

GSList *plugin_instances = NULL;

void
plugin_set_unload_callback (PluginInstance *plugin, plugin_unload_callback *puc)
{
	if (!plugin) {
		printf ("Trying to set plugin unload callback on a null plugin.\n");
		return;
	}

	plugin->SetUnloadCallback (puc);
}

static void
table_add (GtkWidget *table, const char *txt, int col, int row)
{
	GtkWidget *l = gtk_label_new (txt);

	gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE(table), l, col, col+1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) 0, 4, 0);
}

static GtkWidget *
title (const char *txt)
{
	char *fmt = g_strdup_printf ("<b>%s</b>", txt);
	GtkWidget *label = gtk_label_new (NULL);

	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_label_set_markup (GTK_LABEL (label), fmt);
	g_free (fmt);

	return label;
}

static void
expose_regions (GtkToggleButton *checkbox, gpointer user_data)
{
	if (gtk_toggle_button_get_active (checkbox))
		moonlight_flags |= RUNTIME_INIT_SHOW_EXPOSE;
	else
		moonlight_flags &= ~RUNTIME_INIT_SHOW_EXPOSE;
}

static void
clipping_regions (GtkToggleButton *checkbox, gpointer user_data)
{
	if (gtk_toggle_button_get_active (checkbox))
		moonlight_flags |= RUNTIME_INIT_SHOW_CLIPPING;
	else
		moonlight_flags &= ~RUNTIME_INIT_SHOW_CLIPPING;
}

static void
bounding_boxes (GtkToggleButton *checkbox, gpointer user_data)
{
	if (gtk_toggle_button_get_active (checkbox))
		moonlight_flags |= RUNTIME_INIT_SHOW_BOUNDING_BOXES;
	else
		moonlight_flags &= ~RUNTIME_INIT_SHOW_BOUNDING_BOXES;
}

static void
show_fps (GtkToggleButton *checkbox, gpointer user_data)
{
	if (gtk_toggle_button_get_active (checkbox))
		moonlight_flags |= RUNTIME_INIT_SHOW_FPS;
	else
		moonlight_flags &= ~RUNTIME_INIT_SHOW_FPS;
}

void
PluginInstance::properties_dialog_response (GtkWidget *dialog, int response, PluginInstance *plugin)
{
	plugin->properties_fps_label = NULL;
	plugin->properties_cache_label = NULL;
	gtk_widget_destroy (dialog);
}

void
PluginInstance::Properties ()
{
	GtkWidget *dialog, *table, *checkbox;
	char buffer[40];
	GtkBox *vbox;
	int row = 0;
	
	dialog = gtk_dialog_new_with_buttons ("Object Properties", NULL, (GtkDialogFlags)
					      GTK_DIALOG_NO_SEPARATOR,
					      GTK_STOCK_CLOSE, GTK_RESPONSE_NONE, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 8);
	
	vbox = GTK_BOX (GTK_DIALOG (dialog)->vbox);
	
	// Silverlight Application properties
	gtk_box_pack_start (vbox, title ("Properties"), FALSE, FALSE, 0);
	gtk_box_pack_start (vbox, gtk_hseparator_new (), FALSE, FALSE, 8);
	
	table = gtk_table_new (11, 2, FALSE);
	gtk_box_pack_start (vbox, table, TRUE, TRUE, 0);
	
	table_add (table, "Source:", 0, row++);
	table_add (table, "Width:", 0, row++);
	table_add (table, "Height:", 0, row++);
	table_add (table, "Background:", 0, row++);
	table_add (table, "Kind:", 0, row++);
	table_add (table, "Windowless:", 0, row++);
	table_add (table, "MaxFrameRate:", 0, row++);
	
	row = 0;
	table_add (table, source, 1, row++);
	snprintf (buffer, sizeof (buffer), "%dpx", getActualWidth ());
	table_add (table, buffer, 1, row++);
	snprintf (buffer, sizeof (buffer), "%dpx", getActualHeight ());
	table_add (table, buffer, 1, row++);
	table_add (table, background, 1, row++);
	table_add (table, xaml_loader == NULL ? "(Unknown)" : (xaml_loader->IsManaged () ? "1.1 (XAML + Managed Code)" : "1.0 (Pure XAML)"), 1, row++);
	table_add (table, windowless ? "yes" : "no", 1, row++);
	snprintf (buffer, sizeof (buffer), "%i", maxFrameRate);
	table_add (table, buffer, 1, row++);
	
	row++;
	properties_fps_label = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (properties_fps_label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE(table), properties_fps_label, 0, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) 0, 4, 0);

	row++;
	properties_cache_label = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (properties_cache_label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE(table), properties_cache_label, 0, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) 0, 4, 0);

	// Runtime debug options
	gtk_box_pack_start (vbox, title ("Runtime Debug Options"), FALSE, FALSE, 0);
	gtk_box_pack_start (vbox, gtk_hseparator_new (), FALSE, FALSE, 8);
	
	checkbox = gtk_check_button_new_with_label ("Show exposed regions");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), moonlight_flags & RUNTIME_INIT_SHOW_EXPOSE);
	g_signal_connect (checkbox, "toggled", G_CALLBACK (expose_regions), NULL);
	gtk_box_pack_start (vbox, checkbox, FALSE, FALSE, 0);
	
	checkbox = gtk_check_button_new_with_label ("Show clipping regions");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), moonlight_flags & RUNTIME_INIT_SHOW_CLIPPING);
	g_signal_connect (checkbox, "toggled", G_CALLBACK (clipping_regions), NULL);
	gtk_box_pack_start (vbox, checkbox, FALSE, FALSE, 0);
	
	checkbox = gtk_check_button_new_with_label ("Show bounding boxes");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), moonlight_flags & RUNTIME_INIT_SHOW_BOUNDING_BOXES);
	g_signal_connect (checkbox, "toggled", G_CALLBACK (bounding_boxes), NULL);
	gtk_box_pack_start (vbox, checkbox, FALSE, FALSE, 0);
	
	checkbox = gtk_check_button_new_with_label ("Show Frames Per Second");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), moonlight_flags & RUNTIME_INIT_SHOW_FPS);
	g_signal_connect (checkbox, "toggled", G_CALLBACK (show_fps), NULL);
	gtk_box_pack_start (vbox, checkbox, FALSE, FALSE, 0);

	g_signal_connect (dialog, "response", G_CALLBACK (properties_dialog_response), this);
	gtk_widget_show_all (dialog);
}

PluginInstance::PluginInstance (NPP instance, uint16_t mode)
{
	this->instance = instance;
	this->mode = mode;
	window = NULL;
	
	properties_fps_label = NULL;
	properties_cache_label = NULL;

	rootobject = NULL;

	container = NULL;
	surface = NULL;

	// Property fields
	initParams = false;
	isLoaded = false;
	source = NULL;
	source_idle = 0;
	onLoad = NULL;
	onError = NULL;
	background = NULL;

	windowless = false;
	
	// MSDN says the default is 24: http://msdn2.microsoft.com/en-us/library/bb979688.aspx
	// blog says the default is 60: http://blogs.msdn.com/seema/archive/2007/10/07/perf-debugging-tips-enableredrawregions-a-performance-bug-in-videobrush.aspx
	// testing seems to confirm that the default is 60.
	maxFrameRate = 60;

	vm_missing_file = NULL;
	xaml_loader = NULL;
	plugin_unload = NULL;

	timers = NULL;

	wrapped_objects = g_hash_table_new (g_direct_hash, g_direct_equal);

	plugin_instances = g_slist_append (plugin_instances, instance);

	/* back pointer to us */
	instance->pdata = this;
	
#if DEBUG
	moon_sources = NULL;
#endif	
}

PluginInstance::~PluginInstance ()
{
	// Kill timers
	GSList *p;
	for (p = timers; p != NULL; p = p->next){
		uint32_t source_id = GPOINTER_TO_INT (p->data);

		g_source_remove (source_id);
	}
	g_slist_free (p);

	g_hash_table_destroy (wrapped_objects);

	// Remove us from the list.
	plugin_instances = g_slist_remove (plugin_instances, instance);

	if (rootobject)
		NPN_ReleaseObject ((NPObject*)rootobject);

	g_free (background);

	delete xaml_loader;
	xaml_loader = NULL;

	g_free (source);

	if (source_idle)
		g_source_remove (source_idle);


	//
	// The code below was an attempt at fixing this, but we are still getting spurious errors
	// we might have another source of problems
	//
	//fprintf (stderr, "Destroying the surface: %p, plugin: %p\n", surface, this);
	if (surface != NULL){
		//gdk_error_trap_push ();
		surface->unref ();
		//gdk_display_sync (display);
		//gdk_error_trap_pop ();
	}

	if (plugin_unload)
		plugin_unload (this);
		
#if DEBUG
	delete moon_sources;
	moon_sources = NULL;
#endif
}

#if DEBUG
void
PluginInstance::AddSource (const char *uri, const char *filename)
{
	moon_source *src = new moon_source ();
	src->uri = g_strdup (uri);
	src->filename = g_strdup (filename);
	if (moon_sources == NULL)
		moon_sources = new List ();
	moon_sources->Append (src);
}

List*
PluginInstance::GetSources ()
{
	return moon_sources;
}
#endif

void
PluginInstance::SetUnloadCallback (plugin_unload_callback* puc)
{
	plugin_unload = puc;
}

void
PluginInstance::Initialize (int argc, char* const argn[], char* const argv[])
{
	for (int i = 0; i < argc; i++) {
		if (argn[i] == NULL)
			continue;

		// initParams.
		if (!g_ascii_strcasecmp (argn[i], "initParams")) {
			initParams = argv[i];
			continue;
		}

		// onLoad.
		if (!g_ascii_strcasecmp (argn[i], "onLoad")) {
			onLoad = argv[i];
			continue;
		}

		// onError.
		if (!g_ascii_strcasecmp (argn[i], "onError")) {
			onError = argv[i];
			continue;
		}

		// Source url handle.
		if (!g_ascii_strcasecmp (argn[i], "src") || !g_ascii_strcasecmp (argn[i], "source")) {
			source = g_strdup (argv[i]);
			continue;
		}

		if (!g_ascii_strcasecmp (argn[i], "background")) {
			background = g_strdup (argv[i]);
			continue;
		}
		
		if (!g_ascii_strcasecmp (argn [i], "windowless")) {
			windowless = !g_ascii_strcasecmp (argv [i], "true");
			continue;
		}
		
		if (!g_ascii_strcasecmp (argn [i], "maxFramerate")) {
			maxFrameRate = atoi (argv [i]);
			continue;
		}
	}

	NPBool supportsWindowless = FALSE;

	int plugin_major, plugin_minor;
	int netscape_major, netscape_minor;

	/* Find the version numbers. */
	NPN_Version(&plugin_major, &plugin_minor,
		    &netscape_major, &netscape_minor);

	printf(" Browser NPAPI version = %d.%d\n",
	       netscape_major, netscape_minor);

	if (netscape_major >= 1 || netscape_minor >= 18) {
		if (windowless)
			NPN_GetValue (instance, NPNVSupportsWindowless, &supportsWindowless);
	}

	if (windowless) {
		if (supportsWindowless) {
			NPN_SetValue (instance, NPPVpluginWindowBool, (void *) FALSE);
			NPN_SetValue (instance, NPPVpluginTransparentBool, (void *) TRUE);
			printf ("windowless mode\n");
		} else {
			printf ("browser doesn't support windowless mode.\n");
			windowless = FALSE;
		}
	}
}

void
PluginInstance::Finalize ()
{
}

NPError
PluginInstance::GetValue (NPPVariable variable, void *result)
{
	NPError err = NPERR_NO_ERROR;

	switch (variable) {
		case NPPVpluginNeedsXEmbed:
			*((PRBool *)result) = !windowless;
			break;

		case NPPVpluginScriptableNPObject:
			*((NPObject**) result) = getRootObject ();
			break;
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
PluginInstance::SetWindow (NPWindow *window)
{
 	if (window == this->window)
 		return NPERR_NO_ERROR;

	this->window = window;

	if (!windowless) {
		NPN_GetValue (instance, NPNVSupportsXEmbedBool, &xembed_supported);
		if (!xembed_supported) {
			DEBUGMSG ("*** XEmbed not supported");
			return NPERR_GENERIC_ERROR;
		}
	}

	CreateWindow ();
	
	return NPERR_NO_ERROR;
}

void
PluginInstance::SetPageURL ()
{
	// From: http://developer.mozilla.org/en/docs/Getting_the_page_URL_in_NPAPI_plugin
	NPIdentifier str_location = NPN_GetStringIdentifier ("location");
	NPIdentifier str_href = NPN_GetStringIdentifier ("href");
	NPVariant location_property;
	NPVariant location_object;
	NPObject *window;
	
	if (NPERR_NO_ERROR != NPN_GetValue (instance, NPNVWindowNPObject, &window))
		return;
	
	// Get the location property from the window object (which is another object).
	if (NPN_GetProperty (instance, window, str_location, &location_property)) {
		// Get the location property from the location object.
		if (NPN_GetProperty (instance, location_property.value.objectValue, str_href, &location_object )) {
			surface->SetSourceLocation (NPVARIANT_TO_STRING (location_object).utf8characters);
			NPN_ReleaseVariantValue (&location_object);
		}
		NPN_ReleaseVariantValue (&location_property);
	}
}

void
PluginInstance::RenderSurface (Surface *surface, void *user_data)
{
	PluginInstance *plugin = (PluginInstance *) user_data;

	NPN_ForceRedraw (plugin->instance);
}

void
PluginInstance::InvalidateSurface (Surface *surface, Rect rect, void *user_data)
{
	PluginInstance *plugin = (PluginInstance *) user_data;

	NPRect nprect;

	rect = rect.RoundOut();

	nprect.left = (uint16)rect.x;
	nprect.top = (uint16)rect.y;
	nprect.right = (uint16)(rect.x + rect.w);
	nprect.bottom = (uint16)(rect.y + rect.h);

	NPN_InvalidateRect (plugin->instance, &nprect);
}

void
PluginInstance::ReportFPS (Surface *surface, int nframes, float nsecs, void *user_data)
{
	PluginInstance *plugin = (PluginInstance *) user_data;
	char *msg;
	
	msg = g_strdup_printf ("Rendered %d frames in %.3fs = %.3f FPS",
			       nframes, nsecs, nframes / nsecs);
	
	NPN_Status (plugin->instance, msg);

	if (plugin->properties_fps_label)
		gtk_label_set_text (GTK_LABEL (plugin->properties_fps_label), msg);
	
	g_free (msg);
}

void
PluginInstance::ReportCache (Surface *surface, long bytes, void *user_data)
{
	PluginInstance *plugin = (PluginInstance *) user_data;
	char *msg;

	if (bytes < 1048576)
		msg = g_strdup_printf ("Cache size is ~%d KB", (int) (bytes / 1024));
	else
		msg = g_strdup_printf ("Cache size is ~%.2f MB", bytes / 1048576.0);

	NPN_Status (plugin->instance, msg);

	if (plugin->properties_cache_label)
		gtk_label_set_text (GTK_LABEL (plugin->properties_cache_label), msg);
	
	g_free (msg);
}

void
PluginInstance::CreateWindow ()
{
	//DEBUGMSG ("*** creating window2 (%d,%d,%d,%d)", window->x, window->y, window->width, window->height);
	
	surface = new Surface (window->width, window->height, windowless);

	if (windowless) {
		surface->SetInvalidateFunc (InvalidateSurface, this);
		surface->SetRenderFunc (RenderSurface, this);
		surface->SetTrans (true);
	}

	if (onError != NULL) {
		char *retval = PL_strdup (onError);
		NPVariant npvalue;

		STRINGZ_TO_NPVARIANT (retval, npvalue);
		NPIdentifier identifier = NPN_GetStringIdentifier ("onError");
		NPN_SetProperty (instance, getRootObject ()->content, 
				 identifier, &npvalue);
	}

	/*
	if (onLoad != NULL) {
		char *retval = PL_strdup (onLoad);
		NPVariant npvalue;

		STRINGZ_TO_NPVARIANT (retval, npvalue);
		NPIdentifier identifier = NPN_GetStringIdentifier ("onLoad");
		NPN_SetProperty (instance, getRootObject ()->content, 
				 identifier, &npvalue);
	}
	*/

	surface->SetFPSReportFunc (ReportFPS, this);
	surface->SetCacheReportFunc (ReportCache, this);
	surface->SetDownloaderContext (this);
	
	SetPageURL ();

	UpdateSource ();
	
	surface->GetTimeManager()->SetMaximumRefreshRate (maxFrameRate);
	
	if (background) {
		Color *c = color_from_str (background);
		
		if (c == NULL) {
			g_warning ("error setting background color");
			c = new Color (0x00FFFFFF);
		}
		
		surface->SetBackgroundColor (c);
		delete c;
	}
	
	if (!windowless) {
		//  GtkPlug container and surface inside
		container = gtk_plug_new (reinterpret_cast <GdkNativeWindow> (window->window));

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

		g_signal_connect (G_OBJECT(container), "event", G_CALLBACK (plugin_event_callback), this);

		gtk_container_add (GTK_CONTAINER (container), surface->GetWidget());
		//display = gdk_drawable_get_display (surface->GetWidget()->window);
		gtk_widget_show_all (container);
	}
}

void
PluginInstance::UpdateSource ()
{
	if (!source)
		return;

	char *pos = strchr (source, '#');
	if (pos) {
		source_idle = g_idle_add (IdleUpdateSourceByReference, this);
	} else {
		StreamNotify *notify = new StreamNotify (StreamNotify::SOURCE, source);
		
		// FIXME: check for errors
		NPN_GetURLNotify (instance, source, NULL, notify);
	}
}

gboolean
PluginInstance::IdleUpdateSourceByReference (gpointer data)
{
	PluginInstance *instance = (PluginInstance*)data;

	char *pos = strchr (instance->source, '#');
	if (strlen (pos+1) > 0)
		instance->UpdateSourceByReference (pos+1);

	return FALSE;
}

void
PluginInstance::UpdateSourceByReference (const char *value)
{
	const char *xaml;

	xaml = html_get_element_text (this, value);
	if (!xaml)
		return;

	if (xaml_loader)
		delete xaml_loader;

	xaml_loader = PluginXamlLoader::FromStr (xaml, this, surface);
	TryLoad ();

	g_free ((gpointer) xaml);
}

bool
PluginInstance::JsRunOnload ()
{
	bool retval = false;
	NPObject *object = NULL;
	NPVariant result;
	const char *expression = onLoad;
	
	if (NPERR_NO_ERROR != NPN_GetValue(instance, NPNVWindowNPObject, &object)) {
		DEBUGMSG ("*** Failed to get window object");
		return false;
	}

	if (!strncmp (expression, "javascript:", strlen ("javascript:")))
		expression += strlen ("javascript:");

	NPVariant args[1];

	DependencyObject *toplevel = surface->GetToplevel ();
	DEBUGMSG ("In JsRunOnload, toplevel = %p", toplevel);

	MoonlightEventObjectObject *depobj = EventObjectCreateWrapper (instance, toplevel);
	OBJECT_TO_NPVARIANT ((NPObject*)depobj, args[0]);

	if (NPN_Invoke (instance, object, NPID (expression), args, 1, &result)) {
		DEBUGMSG ("NPN_Invoke succeeded");
		NPN_ReleaseVariantValue (&result);

		retval = true;
	} else {
		DEBUGMSG ("NPN_Invoke failed");
	}
	
	NPN_ReleaseVariantValue (&args [0]);
	NPN_ReleaseObject (object);

	return retval;
}

NPError
PluginInstance::NewStream (NPMIMEType type, NPStream *stream, NPBool seekable, uint16_t *stype)
{
  	//DEBUGMSG ("NewStream (%s) %s\n", this->source, stream->url);
	
	if (IS_NOTIFY_SOURCE (stream->notifyData)) {
		*stype = NP_ASFILEONLY;
		return NPERR_NO_ERROR;
	}
	
	if (IS_NOTIFY_DOWNLOADER (stream->notifyData)) {
		StreamNotify *notify = (StreamNotify *) stream->notifyData;
		
		downloader_set_stream_data ((Downloader *) notify->pdata, instance, stream);
		*stype = NP_ASFILE;
		
		return NPERR_NO_ERROR;
	}

	if (IS_NOTIFY_REQUEST (stream->notifyData)) {
		*stype = NP_ASFILEONLY;
		return NPERR_NO_ERROR;
	}

	*stype = NP_NORMAL;

	return NPERR_NO_ERROR;
}

NPError
PluginInstance::DestroyStream (NPStream *stream, NPError reason)
{
	PluginDownloader *pd = (PluginDownloader*) stream->pdata;
	if (pd != NULL)
		pd->StreamDestroyed ();
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

	//
	// Only try to load if there's no missing files.
	//

	if (vm_missing_file == NULL)
		vm_missing_file = g_strdup (xaml_loader->TryLoad (&error));

	//printf ("PluginInstance::TryLoad, vm_missing_file: %s, error: %i\n", vm_missing_file, error);
	
	if (vm_missing_file != NULL) {
		StreamNotify *notify = new StreamNotify (StreamNotify::REQUEST, vm_missing_file);
		
		// FIXME: check for errors
		NPN_GetURLNotify (instance, vm_missing_file, NULL, notify);
		return;
	}

	//
	// missing file was NULL, if error is set, display some message
	//
	if (!isLoaded && surface->GetToplevel ()) {
		isLoaded = true;
		if (onLoad)
			JsRunOnload ();
	}
}

/*
 * Prepares a string to be passed to Javascript, escapes the " and '
 * characters and maps the newline to \n sequence and cr to \r sequence
 */
static char*
string_to_js (char *s)
{
	char *res;
	GString *result;

	if (strchr (s, '\'') == NULL && strchr (s, '\n') == NULL)
		return g_strdup (s);

	result = g_string_new ("");

	for (char *p = s; *p != 0; *p++){
		if (*p == '"' || *p == '\''){
			g_string_append_c (result, '\\');
			g_string_append_c (result, *p);
		} else if (*p == '\n'){
			g_string_append_c (result, '\\');
			g_string_append_c (result, 'n');
		} else if (*p == '\r'){
			g_string_append_c (result, '\\');
			g_string_append_c (result, 'r');
		} else
			g_string_append_c (result, *p);
	}

	res = result->str;
	g_string_free (result, FALSE);

	return res;
}

void
PluginInstance::ReportException (char *msg, char *details, char **stack_trace, int num_frames)
{
	NPObject *object = NULL;
	NPVariant result;
	char *script, *row_js, *msg_escaped, *details_escaped;
	char **stack_trace_escaped;
	NPString str;
	int i;
	bool res;

	// Get a reference to our element
	if (NPERR_NO_ERROR != NPN_GetValue(instance, NPNVPluginElementNPObject, &object)) {
		DEBUGMSG ("*** Failed to get plugin element object");
		return;
	}

	// FIXME:
	// - make sure the variables do not become global

	// Remove ' from embedded strings
	msg_escaped = string_to_js (msg);
	details_escaped = string_to_js (details);
	stack_trace_escaped = g_new0 (char*, num_frames);
	for (i = 0; i < num_frames; ++i)
		stack_trace_escaped [i] = string_to_js (stack_trace [i]);

	// JS code to create our elements
	row_js = g_strdup (" ");
	for (i = 0; i < num_frames; ++i) {
		char *s;

		s = g_strdup_printf ("%s%s%s", row_js, (i == 0) ? "" : "\\n ", stack_trace_escaped [i]);
		g_free (row_js);
		row_js = s;
	}

	script = g_strdup_printf ("text1 = document.createTextNode ('%s'); text2 = document.createTextNode ('Exception Details: '); text3 = document.createTextNode ('%s'); text4 = document.createTextNode ('Stack Trace:'); parent = this.parentNode; a = document.createElement ('div'); a.appendChild (document.createElement ('hr')); msg = document.createElement ('font'); a.appendChild (msg); h2 = document.createElement ('h2'); i = document.createElement ('i'); b = document.createElement ('b'); msg.appendChild (h2); msg.appendChild (b); msg.appendChild (text3); msg.appendChild (document.createElement ('br')); msg.appendChild (document.createElement ('br')); b2 = document.createElement ('b'); b2.appendChild (text4); msg.appendChild (b2); b.appendChild (text2); h2.appendChild (i); i.appendChild (text1); msg.appendChild (document.createElement ('br')); msg.appendChild (document.createElement ('br')); a.appendChild (document.createElement ('hr')); table = document.createElement ('table'); msg.appendChild (table); table.width = '100%%'; table.bgColor = '#ffffcc'; tbody = document.createElement ('tbody'); table.appendChild (tbody); tr = document.createElement ('tr'); tbody.appendChild (tr); td = document.createElement ('td'); tr.appendChild (td); pre = document.createElement ('pre'); td.appendChild (pre); text = document.createTextNode ('%s'); pre.appendChild (text); previous = parent.firstChild; if (parent.firstChild.tagName == 'DIV') parent.removeChild (parent.firstChild); parent.insertBefore (a, this)", msg_escaped, details_escaped, row_js);

	g_free (msg_escaped);
	g_free (details_escaped);
	for (i = 0; i < num_frames; ++i)
		g_free (stack_trace_escaped [i]);
	g_free (stack_trace_escaped);
	g_free (row_js);

	str.utf8characters = script;
	str.utf8length = strlen (script);

	res = NPN_Evaluate (instance, object, &str, &result);
	if (res)
		NPN_ReleaseVariantValue (&result);
	NPN_ReleaseObject (object);
	g_free (script);
}

//
// Download URL synchronously using the browser and return its contents as a byte array.
//
void *
PluginInstance::LoadUrl (char *url, int32_t *length)
{
	NPObject *object = NULL;
	NPVariant result;
	char *script, *url_escaped;
	void *load_res = NULL;
	NPString str;
	int i;
	bool res;

	*length = 0;

	// Get a reference to our element
	if (NPERR_NO_ERROR != NPN_GetValue(instance, NPNVPluginElementNPObject, &object)) {
		DEBUGMSG ("*** Failed to get plugin element object");
		return NULL;
	}

	//
	// Since NPAPI doesn't contain the neccessary functionality, we use the JS XMLHttpRequest
	// object of AJAX fame. Some info on downloading binary data:
	// http://mgran.blogspot.com/2006/08/downloading-binary-streams-with.html
	// This can only load stuff below our base URL.
	// During the call, the UI will freeze, which is a problem, but this is the price we
	// pay for synchronous access.

	// FIXME:
	// - make sure the variables do not become global
	url_escaped = string_to_js (url);
	script = g_strdup_printf ("var req = new XMLHttpRequest(); req.open('GET', '%s', false); req.overrideMimeType('text/plain; charset=x-user-defined'); req.send (null); req.responseText;", url_escaped);

	str.utf8characters = script;
	str.utf8length = strlen (script);

	res = NPN_Evaluate (instance, object, &str, &result);
	if (res) {
		if (NPVARIANT_IS_STRING (result)) {
			char *s, *in, *arr;
			int arr_len, len;

			len = NPVARIANT_TO_STRING (result).utf8length;
			s = (char*)NPVARIANT_TO_STRING (result).utf8characters;

			// Convert the utf8 string into an ASCII string
			// See the blog entry above for why this is needed
			in = s;
			arr_len = 0;
			while (in - s < len) {
				in = g_utf8_next_char (in);
				arr_len ++;
			}

			arr = (char*)g_malloc (arr_len);

			in = s;
			i = 0;
			while (in - s < len) {
				arr [i] = g_utf8_get_char (in);

				in = g_utf8_next_char (in);
				i ++;
			}

			load_res = arr;
			*length = arr_len;
		}
		NPN_ReleaseVariantValue (&result);
	}
	NPN_ReleaseObject (object);
	g_free (script);
	g_free (url_escaped);

	return load_res;
}

void
PluginInstance::StreamAsFile (NPStream *stream, const char *fname)
{
  //	DEBUGMSG ("StreamAsFile: %s", fname);

#if DEBUG
	AddSource (stream->url, fname);
#endif

	if (IS_NOTIFY_SOURCE (stream->notifyData)) {
	  //		DEBUGMSG ("LoadFromXaml: %s", fname);
	  	if (xaml_loader)
	  		delete xaml_loader;
		xaml_loader = PluginXamlLoader::FromFilename (fname, this, surface);
		TryLoad ();
	}

	if (IS_NOTIFY_DOWNLOADER (stream->notifyData)){
		Downloader *dl = (Downloader *) ((StreamNotify *)stream->notifyData)->pdata;
		
		dl->NotifyFinished (fname);
	}

	if (IS_NOTIFY_REQUEST (stream->notifyData)) {
		bool reload = true;
		// printf ("PluginInstance::StreamAsFile: vm_missing_file: '%s', url: '%s', fname: '%s'.\n", vm_missing_file, stream->url, fname);

		if (!vm_missing_file)
			reload = false;

		if (reload && xaml_loader->GetMapping (vm_missing_file) != NULL) {
			// printf ("PluginInstance::StreamAsFile: the file '%s' has already been downloaded, won't try to reload xaml. Mapped to: '%s' (new url: '%s').", vm_missing_file, xaml_loader->GetMapping (vm_missing_file), stream->url);
			reload = false;
		}
		if (reload && xaml_loader->GetMapping (stream->url) != NULL) {
			// printf ("PluginInstance::StreamAsFile: the url '%s' has already been downloaded, won't try to reload xaml. Mapped to: '%s' (new url: '%s').", vm_missing_file, xaml_loader->GetMapping (stream->url), stream->url);
			reload = false;
		}

		if (vm_missing_file)
			xaml_loader->RemoveMissing (vm_missing_file);

		char *missing = vm_missing_file;
		vm_missing_file = NULL;

		if (reload) {
			// There may be more missing files.
			vm_missing_file = g_strdup (xaml_loader->GetMissing ());

			xaml_loader->InsertMapping (missing, fname);
			xaml_loader->InsertMapping (stream->url, fname);
			// printf ("PluginInstance::StreamAsFile: retry xaml loading, downloaded: %s to %s\n", missing, stream->url);

			// retry to load
			TryLoad ();
		}

		g_free (missing);
	}
}

int32_t
PluginInstance::WriteReady (NPStream *stream)
{
	StreamNotify *notify = STREAM_NOTIFY (stream->notifyData);
	
	//DEBUGMSG ("WriteReady (%d)", stream->end);
	
	if (notify && notify->pdata && IS_NOTIFY_DOWNLOADER (notify)) {
		Downloader *dl = (Downloader *) notify->pdata;
		
		dl->NotifySize (stream->end);
		
		return MAX_STREAM_SIZE;
	}
	
	NPN_DestroyStream (instance, stream, NPRES_DONE);
	
	return -1;
}

int32_t
PluginInstance::Write (NPStream *stream, int32_t offset, int32_t len, void *buffer)
{
	StreamNotify *notify = STREAM_NOTIFY (stream->notifyData);
	
	//DEBUGMSG ("Write size: %d offset: %d len: %d", stream->end, offset, len);
	
	if (notify && notify->pdata && IS_NOTIFY_DOWNLOADER (notify)) {
		Downloader *dl = (Downloader *) notify->pdata;
		
		dl->Write (buffer, offset, len);
	}
	
	return len;
}

void
PluginInstance::UrlNotify (const char *url, NPReason reason, void *notifyData)
{
	StreamNotify *notify = STREAM_NOTIFY (notifyData);
	
	//if (reason == NPRES_DONE) {
	//	DEBUGMSG ("URL %s downloaded successfully.", url);
	//} else {
	//	DEBUGMSG ("Download of URL %s failed: %i (%s)", url, reason,
	//		  reason == NPRES_USER_BREAK ? "user break" :
	//		  (reason == NPRES_NETWORK_ERR ? "network error" : "other error"));
	//}
	
	if (reason != NPRES_DONE) {
		if (notify && notify->pdata && IS_NOTIFY_DOWNLOADER (notify)) {
			Downloader *dl = (Downloader *) notify->pdata;
			
			switch (reason) {
			case NPRES_USER_BREAK:
				dl->NotifyFailed ("user break");
				break;
			case NPRES_NETWORK_ERR:
				dl->NotifyFailed ("network error");
				break;
			default:
				dl->NotifyFailed ("unknown error");
				break;
			}
		}
	}
	
	if (notify)
		delete notify;
}

void
PluginInstance::Print (NPPrint *platformPrint)
{
	// nothing to do.
}

int16_t
PluginInstance::EventHandle (void *event)
{
	XEvent *xev = (XEvent*)event;
	int16_t handled = 0;

	switch (xev->type) {
	case GraphicsExpose: {
		GdkDrawable *drawable = gdk_pixmap_foreign_new ((GdkNativeWindow)xev->xgraphicsexpose.drawable);
		if (!drawable)
		  drawable = gdk_window_foreign_new ((GdkNativeWindow)xev->xgraphicsexpose.drawable);

		if (drawable) {
			GdkVisual *visual = gdkx_visual_get (((NPSetWindowCallbackStruct*)window->ws_info)->visual->visualid);

			if (visual) {
				GdkEventExpose expose;

				expose.type = GDK_EXPOSE;
				expose.window = NULL;
				expose.send_event = FALSE;
				expose.area = Rect (xev->xgraphicsexpose.x,
						    xev->xgraphicsexpose.y,
						    xev->xgraphicsexpose.width,
						    xev->xgraphicsexpose.height).ToGdkRectangle ();
				/* XXX ugh */
				expose.region = gdk_region_rectangle (&expose.area);

				handled = surface->expose_to_drawable (drawable, visual, &expose, window->x, window->y);

				gdk_region_destroy (expose.region);
			}
			else {
				printf ("no gdk visual\n");
			}
			g_object_unref (drawable);
		}
		else {
			printf ("no gdk drawable\n");
		}
		break;
	}
	case MotionNotify: {
		GdkEventMotion motion;

		motion.type = GDK_MOTION_NOTIFY;
		motion.window = NULL;
		motion.send_event = xev->xmotion.send_event;
		motion.x = xev->xmotion.x;
		motion.y = xev->xmotion.y;
		motion.axes = NULL;
		motion.state = xev->xmotion.state;
		motion.is_hint = xev->xmotion.is_hint;
		motion.device = NULL; // XXX
		motion.x_root = xev->xmotion.x_root;
		motion.y_root = xev->xmotion.y_root;

		handled = Surface::motion_notify_callback (NULL, &motion, surface);
		break;
	}
	case ButtonPress:
	case ButtonRelease: {
		GdkEventButton button;

		button.type = xev->type == ButtonPress ? GDK_BUTTON_PRESS : GDK_BUTTON_RELEASE;
		button.window = NULL;
		button.send_event = xev->xbutton.send_event;
		button.time = xev->xbutton.time;
		button.x = xev->xbutton.x;
		button.y = xev->xbutton.y;
		button.x_root = xev->xbutton.x_root;
		button.y_root = xev->xbutton.y_root;
		button.state = xev->xbutton.state;
		button.button = xev->xbutton.button;
		button.axes = NULL;

		handled = plugin_event_callback (NULL, (GdkEvent*)&button, this);
		if (!handled) {
			if (xev->type == ButtonPress)
				handled = Surface::button_press_callback (NULL, &button, surface);
			else
				handled = Surface::button_release_callback (NULL, &button, surface);
		}
		break;
	}
	case KeyPress:
	case KeyRelease: {
		GdkEventKey key;

		key.type = xev->type == KeyPress ? GDK_KEY_PRESS : GDK_KEY_RELEASE;
		key.window = NULL;
		key.send_event = xev->xkey.send_event;
		key.time = xev->xkey.time;
		key.state = xev->xkey.state;
		key.hardware_keycode = xev->xkey.keycode;

		gint effective_group;

		gdk_keymap_translate_keyboard_state (gdk_keymap_get_default (),
						     xev->xkey.keycode,
						     (GdkModifierType)xev->xkey.state, // XXX
						     0, // XXX
						     &key.keyval,
						     &effective_group,
						     NULL,
						     NULL);

		key.group = (guint8)effective_group;

		if (xev->type == KeyPress)
			handled = Surface::key_press_callback (NULL, &key, surface);
		else
			handled = Surface::key_release_callback (NULL, &key, surface);

		break;
	}
	case EnterNotify:
	case LeaveNotify: {
		GdkEventCrossing crossing;

		crossing.type = xev->type == EnterNotify ? GDK_ENTER_NOTIFY : GDK_LEAVE_NOTIFY;
		crossing.window = crossing.subwindow = NULL;
		crossing.send_event = xev->xcrossing.send_event;
		crossing.time = xev->xcrossing.time;
		crossing.x = xev->xcrossing.x;
		crossing.y = xev->xcrossing.y;
		crossing.x_root = xev->xcrossing.x_root;
		crossing.y_root = xev->xcrossing.y_root;
		crossing.mode = (GdkCrossingMode)xev->xcrossing.mode; // XXX
		crossing.detail = (GdkNotifyType)xev->xcrossing.detail; // XXX
		crossing.focus = xev->xcrossing.focus;
		crossing.state = xev->xcrossing.state;

		Surface::crossing_notify_callback (NULL, &crossing, surface);
		break;
	}
	case FocusIn:
	case FocusOut: {
		// don't think there's anything to do for these.
		// maybe we shouldn't respond to keypresses at all
		// unless we're focused?  will mozilla even send them
		// to us?
		break;
	}
	default:
		printf ("Unhandled Xlib event %d\n", xev->type);
		break;
	}

	return handled;
}

void
PluginInstance::AddWrappedObject (EventObject *obj, NPObject *wrapper)
{
	g_hash_table_insert (wrapped_objects, obj, wrapper);
}

void
PluginInstance::RemoveWrappedObject (EventObject *obj)
{
	g_hash_table_remove (wrapped_objects, obj);
}

NPObject*
PluginInstance::LookupWrappedObject (EventObject *obj)
{
	return (NPObject*)g_hash_table_lookup (wrapped_objects, obj);
}

/*** Getters and Setters ******************************************************/

void
PluginInstance::setSource (const char *value)
{
	if (!value)
		return;

	if (source)
		g_free (source);

	if (source_idle)
		g_source_remove (source_idle);
	source_idle = 0;

	source = g_strdup (value);
	UpdateSource ();
}

char *
PluginInstance::getBackground ()
{
	return background;
}

bool
PluginInstance::setBackground (const char *value)
{
	g_free (background);
	background = g_strdup (value);
	
	printf ("new background color is %s\n", background);
	
	if (surface) {
		Color *c = color_from_str (background);
		
		if (c == NULL) {
			printf ("failed to parse color name\n");
			return false;
		}
		
		surface->SetBackgroundColor (c);
		delete c;
	}
	
	return true;
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
	return windowless;
}

int
PluginInstance::getMaxFrameRate ()
{
	return maxFrameRate;
}

void
PluginInstance::setMaxFrameRate (int value)
{
	maxFrameRate = value;
	
	surface->GetTimeManager()->SetMaximumRefreshRate (MAX (value, 64));
}

int32
PluginInstance::getActualHeight ()
{
	return surface->GetActualHeight ();
}

int32
PluginInstance::getActualWidth ()
{
	return surface->GetActualWidth ();
}

void
PluginInstance::getBrowserInformation (char **name, char **version,
				       char **platform, char **userAgent,
				       bool *cookieEnabled)
{
	*userAgent = (char*)NPN_UserAgent (instance);
	DEBUG_WARN_NOTIMPLEMENTED ("pluginInstance.getBrowserInformation");

	*name = (char *) "Foo!";
	*version = (char *) "Foo!";
	*platform = (char *) "Foo!";
	*cookieEnabled = true;
}

MoonlightScriptControlObject *
PluginInstance::getRootObject ()
{
	if (rootobject == NULL)
		rootobject = NPN_CreateObject (instance, MoonlightScriptControlClass);

	NPN_RetainObject (rootobject);
	return (MoonlightScriptControlObject*)rootobject;
}

NPP
PluginInstance::getInstance ()
{
	return instance;
}

Surface *
plugin_instance_get_surface (PluginInstance *instance)
{
	return instance->surface;
}

int32
plugin_instance_get_actual_width (PluginInstance *instance)
{
	return instance->getActualWidth ();
}

int32
plugin_instance_get_actual_height (PluginInstance *instance)
{
	return instance->getActualHeight ();
}

char*
plugin_instance_get_init_params  (PluginInstance *instance)
{
	return instance->getInitParams();
}

uint32_t
plugin_html_timer_timeout_add (PluginInstance *instance, int32_t interval, GSourceFunc callback, gpointer data)
{
	uint32_t id;

#if GLIB_CHECK_VERSION(2,14,0)
	if (interval > 1000 && ((interval % 1000) == 0))
		id = g_timeout_add_seconds (interval / 1000, callback, data);
	else
#endif
		id = g_timeout_add (interval, callback, data);

	instance->timers = g_slist_append (instance->timers, GINT_TO_POINTER ((int)id));

	return id;
}

void
plugin_html_timer_timeout_stop (PluginInstance *instance, uint32_t source_id)
{
	g_source_remove (source_id);
	instance->timers = g_slist_remove (instance->timers, GINT_TO_POINTER (source_id));
}


void
plugin_instance_get_browser_information (PluginInstance *instance,
					 char **name, char **version,
					 char **platform, char **userAgent,
					 bool *cookieEnabled)
{
	instance->getBrowserInformation (name, version, platform, userAgent, cookieEnabled);
}

void
plugin_instance_get_browser_runtime_settings (bool *debug, bool *html_access,
											  bool *httpnet_access, bool *script_access)
{
	*debug = *html_access = *httpnet_access = *script_access = false;
}

void
plugin_instance_report_exception (PluginInstance *instance, char *msg, char *details, char **stack_trace, int num_frames)
{
	instance->ReportException (msg, details, stack_trace, num_frames);
}

void*
plugin_instance_load_url (PluginInstance *instance, char *url, gint32 *length)
{
	return instance->LoadUrl (url, length);
}

/*
	XamlLoader
*/


bool
PluginXamlLoader::LoadVM ()
{
#if INCLUDE_MONO_RUNTIME
	if (!vm_is_loaded ())
		vm_init ();

	if (vm_is_loaded ())
		return InitializeLoader ();
#endif

	return FALSE;
}

//
// On error it sets the @error ref to 1
// Returns the filename that we are missing
//
const char *
PluginXamlLoader::TryLoad (int *error)
{
	DependencyObject *element;
	Type::Kind element_type;

	*error = 0;

	printf ("PluginXamlLoader::TryLoad, filename: %s, str: %s\n", GetFilename (), GetString ());

	if (GetFilename ()) {
		element = xaml_create_from_file (this, GetFilename (), true, &element_type);
	} else if (GetString ()) {
		element = xaml_create_from_str (this, GetString (), true, &element_type);
	} else {
		*error = 1;
		return NULL;
	}

	if (!element) {
		if (error_args && error_args->error_code != -1) {
			printf ("PluginXamlLoader::TryLoad: Could not load xaml %s: %s (error: %s attr=%s)\n",
				GetFilename () ? "file" : "string", GetFilename () ? GetFilename () : GetString (),
				error_args->xml_element, error_args->xml_attribute);
			GetSurface ()->Attach (NULL);
			GetSurface ()->EmitError (error_args);
			return NULL;
		} else {
			printf ("PluginXamlLoader::TryLoad: Could not load xaml %s: %s (missing_assembly: %s)\n",
				GetFilename () ? "file" : "string", GetFilename () ? GetFilename () : GetString (),
				GetMissing ());
			xaml_is_managed = true;
			return GetMissing ();
		}
	}

	Type *t = Type::Find(element_type);
	if (!t) {
		printf ("PluginXamlLoader::TryLoad: Return value is not a subclass of Canvas, it is an unregistered type\n");
		element->unref ();
		GetSurface ()->Attach (NULL);
		GetSurface ()->EmitError (new ErrorEventArgs (RuntimeError, 2101, "AG_E_INIT_ROOTVISUAL"));
		return NULL;
	}

	if (!t->IsSubclassOf(Type::CANVAS) && !t->IsSubclassOf(Type::CONTROL)) {
		printf ("PluginXamlLoader::TryLoad: Return value is not a subclass of Canvas, it is a %s\n", element->GetTypeName ());
		element->unref ();
		GetSurface ()->Attach (NULL);
		GetSurface ()->EmitError (new ErrorEventArgs (RuntimeError, 2101, "AG_E_INIT_ROOTVISUAL"));
		return NULL;
	}

	printf ("PluginXamlLoader::TryLoad () succeeded.\n");

	GetSurface ()->Attach ((Canvas*) element);

	// xaml_create_from_* passed us a ref which we don't need to
	// keep.
	element->unref ();

	return NULL;
}

bool
PluginXamlLoader::HookupEvent (void *target, const char *name, const char *value)
{
	if (!XamlLoader::HookupEvent (target, name, value))
		event_object_add_javascript_listener ((EventObject*) target, plugin, name, value);

	return true;
}

bool
PluginXamlLoader::InitializeLoader ()
{
	if (initialized)
		return TRUE;

#if INCLUDE_MONO_RUNTIME
	if (!vm_is_loaded ())
		return FALSE;

	if (managed_loader)
		return TRUE;

	if (GetFilename ()) {
		managed_loader = vm_xaml_file_loader_new (this, plugin, GetSurface (), GetFilename ());
	} else if (GetString ()) {
		managed_loader = vm_xaml_str_loader_new (this, plugin, GetSurface (), GetString ());
	} else {
		return FALSE;
	}

	initialized = managed_loader != NULL;
#else
	initialized = TRUE;
#endif

	return initialized;
}

PluginXamlLoader::PluginXamlLoader (const char *filename, const char *str, PluginInstance *plugin, Surface *surface) : XamlLoader (filename, str, surface)
{
	this->plugin = plugin;
	this->initialized = FALSE;
	this->xaml_is_managed = FALSE;
	this->error_args = NULL;
#if INCLUDE_MONO_RUNTIME
	this->managed_loader = NULL;
#endif
}

PluginXamlLoader::~PluginXamlLoader ()
{
#if INCLUDE_MONO_RUNTIME
	if (managed_loader)
		vm_loader_destroy (managed_loader);
#endif
}

PluginXamlLoader *
plugin_xaml_loader_from_str (const char *str, PluginInstance *plugin, Surface *surface)
{
	return PluginXamlLoader::FromStr (str, plugin, surface);
}
