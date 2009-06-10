/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * moon-plugin.cpp: MoonLight browser plugin.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "plugin.h"
#include "plugin-spinner.h"
#include "plugin-class.h"
#include "plugin-debug.h"
#include "browser-bridge.h"
#include "downloader.h"
#include "pipeline-ui.h"
#include "plugin-downloader.h"
#include "npstream-request.h"
#include "xap.h"
#include "windowless.h"
#include "window-gtk.h"
#include "unzip.h"
#include "deployment.h"
#include "uri.h"
#include "timemanager.h"

#define Visual _XxVisual
#define Region _XxRegion
#include "gdk/gdkx.h"
#undef Visual
#undef Region

#ifdef DEBUG
#define d(x) x
#else
#define d(x)
#endif

#define w(x) x
// Debug NPStreams
#define nps(x)//x

extern guint32 moonlight_flags;

/* gleaned from svn log of the moon module, as well as olive/class/{agclr,agmono,System.Silverlight} */
static const char *moonlight_authors[] = {
	"Aaron Bockover <abockover@novell.com>",
	"Alan McGovern <amcgovern@novell.com>",
	"Alp Toker <alp@nuanti.com>",
	"Andreia Gaita <avidigal@novell.com>",
	"Andrew Jorgensen <ajorgensen@novell.com>",
	"Argiris Kirtzidis <akyrtzi@gmail.com>",
	"Atsushi Enomoto <atsushi@ximian.com>",
	"Chris Toshok <toshok@ximian.com>",
	"Dick Porter <dick@ximian.com>",
	"Everaldo Canuto <ecanuto@novell.com>",
	"Fernando Herrera <fherrera@novell.com>",
	"Geoff Norton <gnorton@novell.com>",
	"Jackson Harper <jackson@ximian.com>",
	"Jb Evain <jbevain@novell.com>",
	"Jeffrey Stedfast <fejj@novell.com>",
	"Larry Ewing <lewing@novell.com>",
	"Manuel Ceron <ceronman@unicauca.edu.co>",
	"Marek Habersack <mhabersack@novell.com>",
	"Michael Dominic K. <mdk@mdk.am>",
	"Michael Hutchinson <mhutchinson@novell.com>",
	"Miguel de Icaza <miguel@novell.com>",
	"Paolo Molaro <lupus@ximian.com>",
	"Raja R Harinath <harinath@hurrynot.org>",
	"Rodrigo Kumpera <rkumpera@novell.com>",
	"Rolf Bjarne Kvinge <RKvinge@novell.com>",
	"Rusty Howell <rhowell@novell.com>",
	"Sebastien Pouliot <sebastien@ximian.com>",
	"Stephane Delcroix <sdelcroix@novell.com>",
	"Zoltan Varga <vargaz@gmail.com>",
	NULL
};

void
plugin_menu_about (PluginInstance *plugin)
{
	GtkAboutDialog *about = GTK_ABOUT_DIALOG (gtk_about_dialog_new ());

	gtk_about_dialog_set_name (about, PLUGIN_OURNAME);
	gtk_about_dialog_set_version (about, VERSION);

	gtk_about_dialog_set_copyright (about, "Copyright 2007-2009 Novell, Inc. (http://www.novell.com/)");
#if FINAL_RELEASE
	gtk_about_dialog_set_website (about, "http://moonlight-project.com/");
#else
	gtk_about_dialog_set_website (about, "http://moonlight-project.com/Preview");
#endif

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
plugin_media_pack (PluginInstance *plugin)
{
	CodecDownloader::ShowUI (plugin->GetSurface ());
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

	if (!Media::IsMSCodecsInstalled ()) {
		menu_item = gtk_menu_item_new_with_label ("Install Microsoft Media Pack");
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
		g_signal_connect_swapped (G_OBJECT(menu_item), "activate", G_CALLBACK (plugin_media_pack), plugin);
	}
	
#ifdef DEBUG
	menu_item = gtk_menu_item_new_with_label ("Show XAML Hierarchy");
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
PluginInstance::plugin_button_press_callback (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	PluginInstance *plugin = (PluginInstance *) user_data;

	if (event->button == 3) {
		plugin_show_menu (plugin);
		return TRUE;
	}

	return FALSE;
}

char *
NPN_strdup (const char *tocopy)
{
	int len = strlen(tocopy);
	char *ptr = (char *)NPN_MemAlloc (len+1);
	if (ptr != NULL) {
		strcpy (ptr, tocopy);
		// WebKit should calloc so we dont have to do this
		ptr[len] = 0;
	}

	return ptr;
}

/*** PluginInstance:: *********************************************************/

GSList *plugin_instances = NULL;

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
textboxes (GtkToggleButton *checkbox, gpointer user_data)
{
 	if (gtk_toggle_button_get_active (checkbox))
 		moonlight_flags |= RUNTIME_INIT_SHOW_TEXTBOXES;
 	else
 		moonlight_flags &= ~RUNTIME_INIT_SHOW_TEXTBOXES;
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
	table_add (table, "Codecs:", 0, row++);
	
	row = 0;
	table_add (table, source, 1, row++);
	snprintf (buffer, sizeof (buffer), "%dpx", GetActualWidth ());
	table_add (table, buffer, 1, row++);
	snprintf (buffer, sizeof (buffer), "%dpx", GetActualHeight ());
	table_add (table, buffer, 1, row++);
	table_add (table, background, 1, row++);
	table_add (table, xaml_loader == NULL ? "(Unknown)" : (xaml_loader->IsManaged () ? "1.1 (XAML + Managed Code)" : "1.0 (Pure XAML)"), 1, row++);
	table_add (table, windowless ? "yes" : "no", 1, row++);
	snprintf (buffer, sizeof (buffer), "%i", maxFrameRate);
	table_add (table, buffer, 1, row++);
#if INCLUDE_FFMPEG
	table_add (table, Media::IsMSCodecsInstalled () ? "ms-codecs" : "ffmpeg", 1, row++);
#else
	table_add (table, Media::IsMSCodecsInstalled () ? "ms-codecs" : "none", 1, row++);
#endif
	
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
	
	checkbox = gtk_check_button_new_with_label ("Show text boxes");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), moonlight_flags & RUNTIME_INIT_SHOW_TEXTBOXES);
	g_signal_connect (checkbox, "toggled", G_CALLBACK (textboxes), NULL);
	gtk_box_pack_start (vbox, checkbox, FALSE, FALSE, 0);
	
	checkbox = gtk_check_button_new_with_label ("Show Frames Per Second");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), moonlight_flags & RUNTIME_INIT_SHOW_FPS);
	g_signal_connect (checkbox, "toggled", G_CALLBACK (show_fps), NULL);
	gtk_box_pack_start (vbox, checkbox, FALSE, FALSE, 0);

	g_signal_connect (dialog, "response", G_CALLBACK (properties_dialog_response), this);
	gtk_widget_show_all (dialog);
}

PluginInstance::PluginInstance (NPMIMEType pluginType, NPP instance, guint16 mode)
{
	this->instance = instance;
	this->mode = mode;
	window = NULL;
	
	properties_fps_label = NULL;
	properties_cache_label = NULL;
	
	rootobject = NULL;
	
	container = NULL;
	surface = NULL;
	moon_window = NULL;
	
	// Property fields
	source_location = NULL;
	initParams = NULL;
	source = NULL;
	source_idle = 0;
	onLoad = NULL;
	onError = NULL;
	onResize = NULL;
	onSourceDownloadProgressChanged = NULL;
	onSourceDownloadCompleted = NULL;
	splashscreensource = NULL;
	background = NULL;
	id = NULL;

	windowless = false;
	// XXX default is true for same-domain applications, false for cross-domain applications XXX
	enable_html_access = true;
	allow_html_popup_window = false;
	xembed_supported = FALSE;

	bridge = NULL;

	// MSDN says the default is 24: http://msdn2.microsoft.com/en-us/library/bb979688.aspx
	// blog says the default is 60: http://blogs.msdn.com/seema/archive/2007/10/07/perf-debugging-tips-enableredrawregions-a-performance-bug-in-videobrush.aspx
	// testing seems to confirm that the default is 60.
	maxFrameRate = 60;
	enable_framerate_counter = false;
	
	vm_missing_file = NULL;
	xaml_loader = NULL;
#if PLUGIN_SL_2_0
	system_windows_assembly = NULL;

	moon_load_xaml =
		moon_initialize_deployment_xap =
		moon_initialize_deployment_xaml =
		moon_destroy_application = NULL;

#endif
	timers = NULL;
	
	wrapped_objects = g_hash_table_new (g_direct_hash, g_direct_equal);
	
	cleanup_pointers = NULL;

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

	Deployment::SetCurrent (deployment);

	for (p = timers; p != NULL; p = p->next){
		guint32 source_id = GPOINTER_TO_INT (p->data);

		g_source_remove (source_id);
	}
	g_slist_free (p);

	g_hash_table_destroy (wrapped_objects);

	// Remove us from the list.
	plugin_instances = g_slist_remove (plugin_instances, instance);

	for (GSList *l = cleanup_pointers; l; l = l->next) {
		gpointer* p = (gpointer*)l->data;
		*p = NULL;
	}
	g_slist_free (cleanup_pointers);

	if (rootobject)
		NPN_ReleaseObject ((NPObject*)rootobject);

	g_free (background);
	g_free (id);
	g_free (initParams);
	delete xaml_loader;

#if PLUGIN_SL_2_0
	// Destroy the XAP application
	DestroyApplication ();
#endif

	g_free (source);

	if (source_idle)
		g_source_remove (source_idle);
	
	//
	// The code below was an attempt at fixing this, but we are still getting spurious errors
	// we might have another source of problems
	//
	//fprintf (stderr, "Destroying the surface: %p, plugin: %p\n", surface, this);
	if (surface != NULL) {
		//gdk_error_trap_push ();
		surface->Zombify();
		surface->Dispose ();
		surface->unref_delayed();
		//gdk_display_sync (display);
		//gdk_error_trap_pop ();
	}

	if (bridge)
		delete bridge;
	bridge = NULL;

	deployment->Dispose ();
	deployment->unref_delayed();
#if DEBUG
	delete moon_sources;
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
PluginInstance::Initialize (int argc, char* const argn[], char* const argv[])
{
	for (int i = 0; i < argc; i++) {
		if (argn[i] == NULL) {
			//g_warning ("PluginInstance::Initialize, arg %d == NULL", i);
			continue;
		}
		else if (!g_ascii_strcasecmp (argn[i], "initParams")) {
			initParams = g_strdup (argv[i]);
		}
		else if (!g_ascii_strcasecmp (argn[i], "onLoad")) {
			onLoad = argv[i];
		}
		else if (!g_ascii_strcasecmp (argn[i], "onError")) {
			onError = argv[i];
		}
		else if (!g_ascii_strcasecmp (argn[i], "onResize")) {
			onResize = argv[i];
		}
		else if (!g_ascii_strcasecmp (argn[i], "src") || !g_ascii_strcasecmp (argn[i], "source")) {
			/* There is a new design pattern that creates a silverlight object with data="data:application/x-silverlight,"
			 * firefox is passing this to us as the src element.  We need to ensure we dont set source to this value
			 * as this design pattern sets a xap up after the fact, but checks to ensure Source hasn't been set yet.
			 * 
			 * eg: http://theamazingalbumcoveratlas.org/
			 *
			 * TODO: Find a site that has data:application/x-silverlight,SOMEDATA and figure out what we do with it
			 */
			if (g_ascii_strncasecmp (argv[i], "data:application/x-silverlight", 30) != 0 && argv[i][strlen(argv[i])-1] != ',')
				source = g_strdup (argv[i]);
		}
		else if (!g_ascii_strcasecmp (argn[i], "background")) {
			background = g_strdup (argv[i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "windowless")) {
			windowless = !g_ascii_strcasecmp (argv [i], "true");
		}
		else if (!g_ascii_strcasecmp (argn [i], "maxFramerate")) {
			maxFrameRate = atoi (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "id")) {
			id = g_strdup (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "enablehtmlaccess")) {
			enable_html_access = !g_ascii_strcasecmp (argv [i], "true");
		}
		else if (!g_ascii_strcasecmp (argn [i], "allowhtmlpopupwindow")) {
			allow_html_popup_window = !g_ascii_strcasecmp (argv [i], "true");
		}
		else if (!g_ascii_strcasecmp (argn [i], "splashscreensource")) {
			splashscreensource = g_strdup (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "onSourceDownloadProgressChanged")) {
			onSourceDownloadProgressChanged = g_strdup (argv [i]);
		}
		else if (!g_ascii_strcasecmp (argn [i], "onSourceDownloadCompleted")) {
			onSourceDownloadCompleted = g_strdup (argv [i]);
		}
		else {
		  //fprintf (stderr, "unhandled attribute %s='%s' in PluginInstance::Initialize\n", argn[i], argv[i]);
		}
	}

	guint32 supportsWindowless = FALSE; // NPBool + padding

	int plugin_major, plugin_minor;
	int netscape_major, netscape_minor;
	bool try_opera_quirks = FALSE;

	/* Find the version numbers. */
	NPN_Version(&plugin_major, &plugin_minor,
		    &netscape_major, &netscape_minor);

	//d(printf ("Plugin NPAPI version = %d.%d\n", plugin_major, netscape_minor));
	//d(printf ("Browser NPAPI version = %d.%d\n", netscape_major, netscape_minor));

	NPError error;
	error = NPN_GetValue (instance, NPNVSupportsXEmbedBool, &xembed_supported);
	if (error || !xembed_supported) {
		// This should be an error but we'll use it to detect
		// that we are running in opera
		//return NPERR_INCOMPATIBLE_VERSION_ERROR;
		if (!windowless)
			d(printf ("*** XEmbed not supported\n"));

		try_opera_quirks = true;
	}

	error = NPN_GetValue (instance, NPNVSupportsWindowless, &supportsWindowless);
	supportsWindowless = (error == NPERR_NO_ERROR) && supportsWindowless;

#ifdef DEBUG
	if ((moonlight_flags & RUNTIME_INIT_ALLOW_WINDOWLESS) == 0) {
		printf ("plugin wants to be windowless, but we're not going to let it\n");
		windowless = false;
	}
#endif
	if (windowless) {
		if (supportsWindowless) {
			NPN_SetValue (instance, NPPVpluginWindowBool, (void *) FALSE);
			NPN_SetValue (instance, NPPVpluginTransparentBool, (void *) TRUE);
			d(printf ("windowless mode\n"));
		} else {
			d(printf ("browser doesn't support windowless mode.\n"));
			windowless = false;
		}
	}

        // grovel around in the useragent and try to figure out which
        // browser bridge we should use.
        const char *useragent = NPN_UserAgent (instance);

	if (strstr (useragent, "Opera")) {
		// opera based
		TryLoadBridge ("opera");
	}
	else if (strstr (useragent, "AppleWebKit")) {
		// webkit based
		TryLoadBridge ("webkit");
	}
        else if (strstr (useragent, "Gecko")) {
		// gecko based, let's look for 'rv:1.8' vs 'rv:1.9'
		if (strstr (useragent, "rv:1.8")) {
			TryLoadBridge ("ff2");
		}
		else if (strstr (useragent, "rv:1.9")) {
			TryLoadBridge ("ff3");
		}
        }

	// XXX Opera currently claims to be mozilla when we query it
	if (!bridge && try_opera_quirks)
		TryLoadBridge ("opera");

        if (!bridge) {
		g_warning ("probing for browser type failed, user agent = `%s'",
			   useragent);
	}

	if (!CreatePluginDeployment ()) { 
		g_warning ("Couldn't initialize Mono or create the plugin Deployment");
	}
}

typedef BrowserBridge* (*create_bridge_func)();

const char*
get_plugin_dir (void)
{
	static char *plugin_dir = NULL;

	if (!plugin_dir) {
		Dl_info dlinfo;
		if (dladdr((void *) &plugin_show_menu, &dlinfo) == 0) {
			fprintf (stderr, "Unable to find the location of libmoonplugin.so: %s\n", dlerror ());
			return NULL;
		}
		plugin_dir = g_path_get_dirname (dlinfo.dli_fname);
	}
	return plugin_dir;
}

void
PluginInstance::TryLoadBridge (const char *prefix)
{
	char *bridge_name = g_strdup_printf ("libmoonplugin-%sbridge.so", prefix);
	char *bridge_path;

	bridge_path = g_build_filename (get_plugin_dir (), bridge_name, NULL);

	void* bridge_handle = dlopen (bridge_path, RTLD_LAZY);

	g_free (bridge_name);
	g_free (bridge_path);

	if (bridge_handle == NULL) {
		g_warning ("failed to load browser bridge: %s", dlerror());
		return;
	}

	create_bridge_func bridge_ctor = (create_bridge_func)dlsym (bridge_handle, "CreateBrowserBridge");
	if (bridge_ctor == NULL) {
		g_warning ("failed to locate CreateBrowserBridge symbol: %s", dlerror());
		return;
	}

	bridge = bridge_ctor ();
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
		*((NPBool *)result) = !windowless;
		break;
	case NPPVpluginScriptableNPObject:
		*((NPObject**) result) = GetRootObject ();
		break;
	default:
		err = NPERR_INVALID_PARAM;
		break;
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
	Deployment::SetCurrent (deployment);

 	if (moon_window) {
		// XXX opera Window lifetime hack needs this
		this->window = window;

		if (!surface)
			return NPERR_GENERIC_ERROR;
		
		moon_window->Resize (window->width, window->height);
		return NPERR_NO_ERROR;
	}

	this->window = window;
	CreateWindow ();
	
	return NPERR_NO_ERROR;
}

NPObject*
PluginInstance::GetHost()
{
	NPObject *object = NULL;
	if (NPERR_NO_ERROR != NPN_GetValue(instance, NPNVPluginElementNPObject, &object)) {
		d(printf ("Failed to get plugin host object\n"));
	}
	return object;
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
	if (windowless) {
		moon_window = new MoonWindowless (window->width, window->height, this);
		moon_window->SetTransparent (true);
	}
	else {
		moon_window = new MoonWindowGtk (false, window->width, window->height);
	}

	surface = new Surface (moon_window);
	deployment->SetSurface (surface);

	if (splashscreensource != NULL) {
		StreamNotify *notify = new StreamNotify (StreamNotify::SPLASHSOURCE, splashscreensource);
		
		NPN_GetURLNotify (instance, splashscreensource, NULL, notify);
	} else {
		xaml_loader = PluginXamlLoader::FromStr (PLUGIN_SPINNER, this, surface);
		LoadXAML ();
	} 
	if (onSourceDownloadProgressChanged != NULL) {
		char *retval = NPN_strdup (onSourceDownloadProgressChanged);
		NPVariant npvalue;

		STRINGZ_TO_NPVARIANT (retval, npvalue);
		NPIdentifier identifier = NPN_GetStringIdentifier ("onSourceDownloadProgressChanged");
		NPN_SetProperty (instance, GetRootObject (), 
				 identifier, &npvalue);
		NPN_MemFree (retval);
	}
	if (onSourceDownloadCompleted != NULL) {
		char *retval = NPN_strdup (onSourceDownloadCompleted);
		NPVariant npvalue;

		STRINGZ_TO_NPVARIANT (retval, npvalue);
		NPIdentifier identifier = NPN_GetStringIdentifier ("onSourceDownloadCompleted");
		NPN_SetProperty (instance, GetRootObject (), 
				 identifier, &npvalue);
		NPN_MemFree (retval);
	}
	if (onError != NULL) {
		char *retval = NPN_strdup (onError);
		NPVariant npvalue;

		STRINGZ_TO_NPVARIANT (retval, npvalue);
		NPIdentifier identifier = NPN_GetStringIdentifier ("onError");
		NPN_SetProperty (instance, GetRootObject (), 
				 identifier, &npvalue);
		NPN_MemFree (retval);
	}

	if (onResize != NULL) {
		char *retval = NPN_strdup (onResize);
		NPVariant npvalue;

		STRINGZ_TO_NPVARIANT (retval, npvalue);
		NPIdentifier identifier = NPN_GetStringIdentifier ("onResize");
		NPN_SetProperty (instance, GetRootObject ()->content,
				 identifier, &npvalue);
		NPN_MemFree (retval);
	}

	if (onLoad != NULL) {
		char *retval = NPN_strdup (onLoad);
		NPVariant npvalue;

		STRINGZ_TO_NPVARIANT (retval, npvalue);
		NPIdentifier identifier = NPN_GetStringIdentifier ("onLoad");
		NPN_SetProperty (instance, GetRootObject (), 
				 identifier, &npvalue);
		NPN_MemFree (retval);
	}

	surface->SetFPSReportFunc (ReportFPS, this);
	surface->SetCacheReportFunc (ReportCache, this);
	surface->SetDownloaderContext (this);
	
	//SetPageURL ();
	/* If we have a splash screen we dont start getting the source until we've resolved it */
	if (splashscreensource == NULL)
		UpdateSource ();
	
	surface->GetTimeManager()->SetMaximumRefreshRate (maxFrameRate);
	
	if (background) {
		Color *c = color_from_str (background);
		
		if (c == NULL) {
			d(printf ("error setting background color\n"));
			c = new Color (0x00FFFFFF);
		}
		
		surface->SetBackgroundColor (c);
		delete c;
	}
	
	if (!windowless) {
		//  GtkPlug container and surface inside
		container = gtk_plug_new ((GdkNativeWindow) window->window);

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

		g_signal_connect (G_OBJECT(container), "button-press-event", G_CALLBACK (PluginInstance::plugin_button_press_callback), this);

		gtk_container_add (GTK_CONTAINER (container), ((MoonWindowGtk*)moon_window)->GetWidget());
		//display = gdk_drawable_get_display (surface->GetWidget()->window);
		gtk_widget_show_all (container);
	}
}

void
PluginInstance::UpdateSource ()
{
	if (source_idle) {
		g_source_remove (source_idle);
		source_idle = 0;
	}

	if (surface != NULL)
		surface->DetachDownloaders ();

	if (!source)
		return;

	char *pos = strchr (source, '#');
	if (pos) {
		source_idle = g_idle_add (IdleUpdateSourceByReference, this);
		SetPageURL ();
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
	char *pos = NULL;
	
	instance->source_idle = 0;

	if (instance->source)
		pos = strchr (instance->source, '#');

	if (pos && strlen (pos+1) > 0)
		instance->UpdateSourceByReference (pos+1);

	return FALSE;
}

void
PluginInstance::UpdateSourceByReference (const char *value)
{
	// basically do the equivalent of document.getElementById('@value').textContent
	// all using NPAPI.
	//
	NPVariant _document;
	NPVariant _element;
	NPVariant _elementName;
	NPVariant _textContent;

	NPIdentifier id_ownerDocument = NPN_GetStringIdentifier ("ownerDocument");
	NPIdentifier id_getElementById = NPN_GetStringIdentifier ("getElementById");
	NPIdentifier id_textContent = NPN_GetStringIdentifier ("textContent");

	NPObject *host = GetHost();
	if (!host) {
//		printf ("no host\n");
		return;
	}

	// get host.ownerDocument
	bool nperr;
	if (!(nperr = NPN_GetProperty (instance, host, id_ownerDocument, &_document))
	    || !NPVARIANT_IS_OBJECT (_document)) {
//		printf ("no document (type == %d, nperr = %d)\n", _document.type, nperr);
		return;
	}

	// _element = document.getElementById ('@value')
	string_to_npvariant (value, &_elementName);
	if (!(nperr = NPN_Invoke (instance, NPVARIANT_TO_OBJECT (_document), id_getElementById,
				  &_elementName, 1, &_element))
	    || !NPVARIANT_IS_OBJECT (_element)) {
//		printf ("no valid element named #%s (type = %d, nperr = %d)\n", value, _element.type, nperr);
		NPN_ReleaseVariantValue (&_document);
	}

	// _textContent = _element.textContent
	if (!(nperr = NPN_GetProperty (instance, NPVARIANT_TO_OBJECT (_element), id_textContent, &_textContent))
	    || !NPVARIANT_IS_STRING (_textContent)) {
//		printf ("no text content for element named #%s (type = %d, nperr = %d)\n", value, _textContent.type, nperr);
		NPN_ReleaseVariantValue (&_document);
		NPN_ReleaseVariantValue (&_element);
		return;
	}

	char *xaml = g_strndup ((char *) NPVARIANT_TO_STRING (_textContent).utf8characters, NPVARIANT_TO_STRING (_textContent).utf8length);

//	printf ("yay, xaml = %s\n", xaml);

	if (xaml_loader)
		delete xaml_loader;

	xaml_loader = PluginXamlLoader::FromStr (xaml, this, surface);
	LoadXAML ();

	g_free (xaml);

	NPN_ReleaseVariantValue (&_document);
	NPN_ReleaseVariantValue (&_element);
	NPN_ReleaseVariantValue (&_textContent);
}


Downloader *
PluginInstance::CreateDownloader (PluginInstance *instance)
{
	if (instance) {
		return instance->surface->CreateDownloader ();
	} else {
		printf ("PluginInstance::CreateDownloader (%p): Unable to create contextual downloader.\n", instance);
		return new Downloader ();
	}
}

void
PluginInstance::SetInitParams (const char *value)
{
	g_free (initParams);
	initParams = g_strdup (value);
}

void
PluginInstance::SetPageURL ()
{
	if (source_location != NULL)
		return;
	
	NPIdentifier str_location = NPN_GetStringIdentifier ("location");
	NPIdentifier str_href = NPN_GetStringIdentifier ("href");
	NPVariant location_property;
	NPVariant location_object;
	NPObject *window;
	
	if (NPERR_NO_ERROR == NPN_GetValue (instance, NPNVWindowNPObject, &window)) {
		// Get the location property from the window object (which is another object).
		if (NPN_GetProperty (instance, window, str_location, &location_property)) {
			// Get the location property from the location object.
			if (NPN_GetProperty (instance, location_property.value.objectValue, str_href, &location_object )) {
				this->source_location = g_strndup (NPVARIANT_TO_STRING (location_object).utf8characters, NPVARIANT_TO_STRING (location_object).utf8length);
				if (surface)
					surface->SetSourceLocation (this->source_location);
				NPN_ReleaseVariantValue (&location_object);
			}
			NPN_ReleaseVariantValue (&location_property);
		}
	}
	NPN_ReleaseObject (window);
}


NPError
PluginInstance::NewStream (NPMIMEType type, NPStream *stream, NPBool seekable, guint16 *stype)
{
	nps (printf ("PluginInstance::NewStream (%p, %p, %i, %p)\n", type, stream, seekable, stype));

	if (IS_NOTIFY_SPLASHSOURCE (stream->notifyData)) {
		SetPageURL ();

		*stype = NP_ASFILEONLY;
		return NPERR_NO_ERROR;
	}
	if (IS_NOTIFY_SOURCE (stream->notifyData)) {
		// See http://developer.mozilla.org/En/Getting_the_page_URL_in_NPAPI_plugin
		//
		// but don't call GetProperty inside SetWindow because it breaks opera by
		// causing it to reenter
		//
		// this->source_location = g_strdup (stream->url);
		SetPageURL ();

		*stype = NP_ASFILE;
		return NPERR_NO_ERROR;
	}

	if (IS_NOTIFY_DOWNLOADER (stream->notifyData)) {
		StreamNotify *notify = (StreamNotify *) stream->notifyData;
		
		npstream_request_set_stream_data ((Downloader *) notify->pdata, instance, stream);
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
	nps (printf ("PluginInstance::DestroyStream (%p, %i)\n", stream, reason));
	
	PluginDownloader *pd = (PluginDownloader*) stream->pdata;
	if (pd != NULL) {
		NPStreamRequest *req = (NPStreamRequest *) pd->getRequest ();
		if (req != NULL) 
			req->StreamDestroyed ();
	}

	return NPERR_NO_ERROR;
}

//
// Tries to load the XAML file, the parsing might fail because a
// required dependency is not available, so we need to queue the
// request to fetch the data.
//
void
PluginInstance::LoadXAML ()
{
	int error = 0;

	if (!InitializePluginAppDomain ()) {
		g_warning ("Couldn't initialize the plugin AppDomain");
		return;
	}

	//
	// Only try to load if there's no missing files.
	//
	Surface *our_surface = surface;
	AddCleanupPointer (&our_surface);

	ManagedInitializeDeployment (NULL);
	xaml_loader->LoadVM ();

	const char *missing = xaml_loader->TryLoad (&error);

	if (!our_surface)
		return;

	RemoveCleanupPointer (&our_surface);

	if (vm_missing_file == NULL)
		vm_missing_file = g_strdup (missing);
	
	if (vm_missing_file != NULL) {
		StreamNotify *notify = new StreamNotify (StreamNotify::REQUEST, vm_missing_file);
		
		// FIXME: check for errors
		NPN_GetURLNotify (instance, vm_missing_file, NULL, notify);
		return;
	}
}

#if PLUGIN_SL_2_0
//
// Loads a XAP file
//
void
PluginInstance::LoadXAP (const char *url, const char *fname)
{
	if (!InitializePluginAppDomain ()) {
		g_warning ("Couldn't initialize the plugin AppDomain");
		return;
	}

	if (source_location)
		g_free (source_location);
	source_location = g_strdup (url);

	Deployment::GetCurrent ()->Reinitialize ();
	ManagedInitializeDeployment (fname);
	GetDeployment()->SetXapLocation (url);
}

void
PluginInstance::DestroyApplication ()
{
	ManagedDestroyApplication ();
}
#endif

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
	NPObject *object;
	NPVariant result;
	char *script, *row_js, *msg_escaped, *details_escaped;
	char **stack_trace_escaped;
	NPString str;
	int i;
	bool res;

	// Get a reference to our element
	object = GetHost();
	if (!object)
		return;

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

void *
PluginInstance::Evaluate (const char *code)
{
	NPObject *object = GetHost ();
	NPString string;
	NPVariant npresult;

	if (object == NULL)
		return NULL;
		
	
	string.utf8characters = code;
	string.utf8length = strlen (code);
		
	bool ret = NPN_Evaluate (instance, object, &string, &npresult);
	
	Value *res = NULL;
	bool keep_ref = false;
	if (ret) {
		if (!NPVARIANT_IS_VOID (npresult) && !NPVARIANT_IS_NULL (npresult)) {
			variant_to_value (&npresult, &res);
			if (npresult.type == NPVariantType_Object)
				keep_ref = true;
		}
	}

	if (!keep_ref)
		NPN_ReleaseVariantValue (&npresult);

	return (void*)res;
}

static bool
is_xap (const char *path)
{
	size_t n = strlen (path);
	
	return n > 4 && !g_ascii_strcasecmp (path + n - 4, ".xap");
}

void
PluginInstance::StreamAsFile (NPStream *stream, const char *fname)
{
	nps (printf ("PluginInstance::StreamAsFile (%p, %s)\n", stream, fname));
	
	Deployment::SetCurrent (deployment);
#if DEBUG
	AddSource (stream->url, fname);
#endif
	if (IS_NOTIFY_SPLASHSOURCE (stream->notifyData)) {
		xaml_loader = PluginXamlLoader::FromFilename (fname, this, surface);
		LoadXAML ();
	}
	if (IS_NOTIFY_SOURCE (stream->notifyData)) {
		delete xaml_loader;
		xaml_loader = NULL;
		
		Uri *uri = new Uri ();
		
		GetSurface ()->EmitSourceDownloadCompleted ();

		if (uri->Parse (stream->url, false) && is_xap (uri->GetPath())) {
			LoadXAP (stream->url, fname);
		} else {
			xaml_loader = PluginXamlLoader::FromFilename (fname, this, surface);
			LoadXAML ();
		}
		
		delete uri;
	} else if (IS_NOTIFY_DOWNLOADER (stream->notifyData)){
		Downloader *dl = (Downloader *) ((StreamNotify *)stream->notifyData)->pdata;
		
		dl->SetFilename (fname);
	} else if (IS_NOTIFY_REQUEST (stream->notifyData)) {
/*
  ///
  ///  Commented out for now, I don't think we should need this code at all anymore since we never request assemblies
  ///  to be downloaded anymore.
  ///
		bool reload = true;

		if (!vm_missing_file)
			reload = false;

		if (reload && xaml_loader->GetMapping (vm_missing_file) != NULL)
			reload = false;
		
		if (reload && xaml_loader->GetMapping (stream->url) != NULL)
			reload = false;
		
		if (vm_missing_file)
			xaml_loader->RemoveMissing (vm_missing_file);

		char *missing = vm_missing_file;
		vm_missing_file = NULL;

		if (reload) {
			// There may be more missing files.
			vm_missing_file = g_strdup (xaml_loader->GetMissing ());

			xaml_loader->InsertMapping (missing, fname);
			xaml_loader->InsertMapping (stream->url, fname);
			
			// retry to load
			LoadXAML ();
		}

		g_free (missing);
*/
	}
}

gint32
PluginInstance::WriteReady (NPStream *stream)
{
	nps (printf ("PluginInstance::WriteReady (%p)\n", stream));
	
	Deployment::SetCurrent (deployment);

	StreamNotify *notify = STREAM_NOTIFY (stream->notifyData);
	
	if (notify && notify->pdata) {
		if (IS_NOTIFY_DOWNLOADER (notify)) {
			Downloader *dl = (Downloader *) notify->pdata;
		
			dl->NotifySize (stream->end);
		
			return MAX_STREAM_SIZE;
		}
		if (IS_NOTIFY_SOURCE (notify)) {
			source_size = stream->end;

			return MAX_STREAM_SIZE;
		}
	}
	
	NPN_DestroyStream (instance, stream, NPRES_DONE);
	
	return -1;
}

gint32
PluginInstance::Write (NPStream *stream, gint32 offset, gint32 len, void *buffer)
{
	nps (printf ("PluginInstance::Write (%p, %i, %i, %p)\n", stream, offset, len, buffer));
	
	Deployment::SetCurrent (deployment);

	StreamNotify *notify = STREAM_NOTIFY (stream->notifyData);
	
	if (notify && notify->pdata) {
		if (IS_NOTIFY_DOWNLOADER (notify)) {
			Downloader *dl = (Downloader *) notify->pdata;
		
			dl->Write (buffer, offset, len);
		}
		if (IS_NOTIFY_SOURCE (notify)) {
			if (source_size > 0) {
				float progress = (offset+len)/(float)source_size;
				if (GetSurface ()->GetToplevel () != NULL) {
					GetSurface ()->EmitSourceDownloadProgressChanged (new DownloadProgressEventArgs (progress));
				}
			}
		}
	}
	
	return len;
}

class PluginClosure : public EventObject {
public:
	PluginClosure (PluginInstance *plugin)
		: plugin (plugin)
	{
	}

	virtual ~PluginClosure ()
	{
	}

	PluginInstance *plugin;
};

void
PluginInstance::network_error_tickcall (EventObject *data)
{
	PluginClosure *closure = (PluginClosure*)data;
	Surface *s = closure->plugin->GetSurface();

	s->EmitError (new ErrorEventArgs (RuntimeError, 2104, "Failed to download silverlight application."));
}

void
PluginInstance::splashscreen_error_tickcall (EventObject *data)
{
	PluginClosure *closure = (PluginClosure*)data;
	Surface *s = closure->plugin->GetSurface();

	s->EmitError (new ErrorEventArgs (RuntimeError, 2108, "Failed to download the splash screen"));

	// we need this check beccause the plugin might have been
	// dtor'ed (and the surface zombified) in the amove EmitError.
	if (!s->IsZombie())
		closure->plugin->UpdateSource ();

	closure->unref();
}

void
PluginInstance::UrlNotify (const char *url, NPReason reason, void *notifyData)
{
	nps (printf ("PluginInstance::UrlNotify (%s, %i, %p)\n", url, reason, notifyData));
	
	StreamNotify *notify = STREAM_NOTIFY (notifyData);
	
	Deployment::SetCurrent (deployment);
	
	if (reason == NPRES_DONE) {
		d(printf ("URL %s downloaded successfully.\n", url));
	} else {
		d(printf ("Download of URL %s failed: %i (%s)\n", url, reason,
			  reason == NPRES_USER_BREAK ? "user break" :
			  (reason == NPRES_NETWORK_ERR ? "network error" : "other error")));
		if (IS_NOTIFY_SOURCE (notify))
			GetSurface()->GetTimeManager()->AddTickCall (network_error_tickcall,
								     new PluginClosure (this));
	}
	
	if (notify && notify->pdata && IS_NOTIFY_DOWNLOADER (notify)) {
		Downloader *dl = (Downloader *) notify->pdata;

		if (reason != NPRES_DONE) {
			
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
		} else {
			dl->NotifyFinished (url);
		}
	}

	if (notify && notify->pdata && IS_NOTIFY_SPLASHSOURCE (notify)) {
		if (reason == NPRES_NETWORK_ERR)
			GetSurface()->GetTimeManager()->AddTickCall (splashscreen_error_tickcall,
								     new PluginClosure (this));
		else
			UpdateSource ();
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
	if (!surface) {
		g_warning ("EventHandle called before SetWindow, discarding event.");
		return 0;
	}

	if (!windowless) {
		g_warning ("EventHandle called for windowed plugin, discarding event.");
		return 0;
	}
		

	return ((MoonWindowless*)moon_window)->HandleEvent ((XEvent*)event);
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

void
PluginInstance::AddCleanupPointer (gpointer p)
{
	cleanup_pointers = g_slist_prepend (cleanup_pointers, p);
}

void
PluginInstance::RemoveCleanupPointer (gpointer p)
{
	cleanup_pointers = g_slist_remove (cleanup_pointers, p);
}

/*** Getters and Setters ******************************************************/

void
PluginInstance::SetSource (const char *value)
{
	if (source) {
		g_free (source);
		source = NULL;
	}

	source = g_strdup (value);

	UpdateSource ();
}

char *
PluginInstance::GetBackground ()
{
	return background;
}

bool
PluginInstance::SetBackground (const char *value)
{
	g_free (background);
	background = g_strdup (value);
	
	if (surface) {
		Color *c = color_from_str (background);
		
		if (c == NULL)
			return false;
		
		surface->SetBackgroundColor (c);
		delete c;
	}
	
	return true;
}

bool
PluginInstance::GetEnableFramerateCounter ()
{
	return enable_framerate_counter;
}

void
PluginInstance::SetEnableFramerateCounter (bool value)
{
	enable_framerate_counter = value;
}

bool
PluginInstance::GetEnableRedrawRegions ()
{
	return moonlight_flags & RUNTIME_INIT_SHOW_EXPOSE;
}

void
PluginInstance::SetEnableRedrawRegions (bool value)
{
	if (value)
		moonlight_flags |= RUNTIME_INIT_SHOW_EXPOSE;
	else
		moonlight_flags &= ~RUNTIME_INIT_SHOW_EXPOSE;
}

bool
PluginInstance::GetEnableHtmlAccess ()
{
	return enable_html_access;
}

bool
PluginInstance::GetAllowHtmlPopupWindow ()
{
	return allow_html_popup_window;
}

bool
PluginInstance::GetWindowless ()
{
	return windowless;
}

int
PluginInstance::GetMaxFrameRate ()
{
	return maxFrameRate;
}

Deployment*
PluginInstance::GetDeployment ()
{
	return deployment;
}

void
PluginInstance::SetMaxFrameRate (int value)
{
	maxFrameRate = value;
	
	surface->GetTimeManager()->SetMaximumRefreshRate (MAX (value, 64));
}

gint32
PluginInstance::GetActualHeight ()
{
	return surface ? surface->GetWindow()->GetHeight() : 0;
}

gint32
PluginInstance::GetActualWidth ()
{
	return surface ? surface->GetWindow()->GetWidth() : 0;
}

MoonlightScriptControlObject *
PluginInstance::GetRootObject ()
{
	if (rootobject == NULL)
		rootobject = NPN_CreateObject (instance, MoonlightScriptControlClass);

	NPN_RetainObject (rootobject);
	return (MoonlightScriptControlObject*)rootobject;
}

NPP
PluginInstance::GetInstance ()
{
	return instance;
}

NPWindow*
PluginInstance::GetWindow ()
{
	return window;
}

// [Obselete (this is obsolete in SL b2.)]
guint32
PluginInstance::TimeoutAdd (gint32 interval, GSourceFunc callback, gpointer data)
{
	guint32 id;

#if GLIB_CHECK_VERSION(2,14,0)
	if (glib_check_version (2,14,0) && interval > 1000 && ((interval % 1000) == 0))
		id = g_timeout_add_seconds (interval / 1000, callback, data);
	else
#endif
		id = g_timeout_add (interval, callback, data);

	timers = g_slist_append (timers, GINT_TO_POINTER ((int)id));

	return id;
}

// [Obselete (this is obsolete in SL b2.)]
void
PluginInstance::TimeoutStop (guint32 source_id)
{
	g_source_remove (source_id);
	timers = g_slist_remove (timers, GINT_TO_POINTER (source_id));
}

char*
plugin_instance_get_id (PluginInstance *instance)
{
	return instance->GetId ();
}

void
plugin_instance_get_browser_runtime_settings (bool *debug, bool *html_access,
					      bool *httpnet_access, bool *script_access)
{
	*debug = *html_access = *httpnet_access = *script_access = false;
}

/*
	XamlLoader
*/

bool
PluginXamlLoader::LoadVM ()
{
#if PLUGIN_SL_2_0
	return InitializeLoader ();
#endif
	return false;
}

bool
PluginXamlLoader::InitializeLoader ()
{
	if (initialized)
		return true;

#if PLUGIN_SL_2_0
	if (managed_loader)
		return true;

	if (GetFilename ()) {
		managed_loader = plugin->ManagedCreateXamlLoaderForFile (this, GetFilename ());
	} else if (GetString ()) {
		managed_loader = plugin->ManagedCreateXamlLoaderForString (this, GetString ());
	} else {
		return false;
	}

	initialized = managed_loader != NULL;
#else
	initialized = true;
#endif
	return initialized;
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
	
	//d(printf ("PluginXamlLoader::TryLoad, filename: %s, str: %s\n", GetFilename (), GetString ()));
	
	GetSurface ()->Attach (NULL);
	
	if (GetFilename ()) {
		element = CreateDependencyObjectFromFile (GetFilename (), true, &element_type);
	} else if (GetString ()) {
		element = CreateDependencyObjectFromString (GetString (), true, &element_type);
	} else {
		*error = 1;
		return NULL;
	}
	
	if (!element) {
		if (error_args && error_args->error_code != -1) {
			d(printf ("PluginXamlLoader::TryLoad: Could not load xaml %s: %s (error: %s attr=%s)\n",
				  GetFilename () ? "file" : "string", GetFilename () ? GetFilename () : GetString (),
				  error_args->xml_element, error_args->xml_attribute));
			error_args->ref ();
			GetSurface ()->EmitError (error_args);
			return NULL;
		} else {
			/*
			d(printf ("PluginXamlLoader::TryLoad: Could not load xaml %s: %s (missing_assembly: %s)\n",
				  GetFilename () ? "file" : "string", GetFilename () ? GetFilename () : GetString (),
				  GetMissing ()));

			xaml_is_managed = true;
			return GetMissing ();
			*/
			return NULL;
		}
	}
	
	Type *t = Type::Find(element_type);
	if (!t) {
		d(printf ("PluginXamlLoader::TryLoad: Return value does not subclass Canvas, it is an unregistered type\n"));
		element->unref ();
		GetSurface ()->EmitError (new ErrorEventArgs (RuntimeError, 2101, "Failed to initialize the application's root visual"));
		return NULL;
	}

	if (!t->IsSubclassOf(Type::PANEL)) {
		d(printf ("PluginXamlLoader::TryLoad: Return value does not subclass of Panel, it is a %s\n",
			  element->GetTypeName ()));
		element->unref ();
		GetSurface ()->EmitError (new ErrorEventArgs (RuntimeError, 2101, "Failed to initialize the application's root visual"));
		return NULL;
	}
	
	//d(printf ("PluginXamlLoader::TryLoad () succeeded.\n"));
	
	GetSurface ()->Attach ((Panel*) element);

	// xaml_create_from_* passed us a ref which we don't need to
	// keep.
	element->unref ();

	return NULL;
}

bool
PluginXamlLoader::SetProperty (void *parser, Value *top_level, const char *xmlns, Value* target, void* target_data, Value *target_parent, const char *prop_xmlns, const char *name, Value* value, void* value_data)
{
	if (XamlLoader::SetProperty (parser, top_level, xmlns, target, target_data, target_parent, prop_xmlns, name, value, value_data))
		return true;

	if (value->GetKind () != Type::STRING)
		return false;

	if (!xaml_is_valid_event_name (name))
		return false;

	const char* function_name = value->AsString ();

	if (!strncmp (function_name, "javascript:", strlen ("javascript:")))
		return false;

	event_object_add_xaml_listener ((EventObject *) target->AsDependencyObject (), plugin, name, function_name);
	
	return true;
}

PluginXamlLoader::PluginXamlLoader (const char *filename, const char *str, PluginInstance *plugin, Surface *surface)
	: XamlLoader (filename, str, surface)
{
	this->plugin = plugin;
	xaml_is_managed = false;
	initialized = false;
	error_args = NULL;
#if PLUGIN_SL_2_0
	xap = NULL;

	managed_loader = NULL;
#endif
}

PluginXamlLoader::~PluginXamlLoader ()
{
#if PLUGIN_SL_2_0
	if (xap)
		delete xap;
	
	if (managed_loader)
		plugin->ManagedLoaderDestroy (managed_loader);
#endif
}

PluginXamlLoader *
plugin_xaml_loader_from_str (const char *str, PluginInstance *plugin, Surface *surface)
{
	return PluginXamlLoader::FromStr (str, plugin, surface);
}


// Our Mono embedding bits are here.  By storing the mono_domain in
// the PluginInstance instead of in a global variable, we don't need
// the code in moonlight.cs for managing app domains.
#if PLUGIN_SL_2_0
MonoMethod *
PluginInstance::MonoGetMethodFromName (MonoClass *klass, const char *name, int narg)
{
	MonoMethod *method;
	method = mono_class_get_method_from_name (klass, name, narg);

	if (!method)
		printf ("Warning could not find method %s\n", name);

	return method;
}

MonoProperty *
PluginInstance::MonoGetPropertyFromName (MonoClass *klass, const char *name)
{
	MonoProperty *property;
	property = mono_class_get_property_from_name (klass, name);

	if (!property)
		printf ("Warning could not find property %s\n", name);

	return property;
}

bool PluginInstance::mono_is_loaded = false;

bool
PluginInstance::MonoIsLoaded ()
{
	return mono_is_loaded;
}

extern "C" {
extern gboolean mono_jit_set_trace_options (const char *options);
};

bool
PluginInstance::DeploymentInit ()
{
	return mono_is_loaded = true; // We load mono in runtime_init.
}


bool
PluginInstance::CreatePluginDeployment ()
{
	deployment = new Deployment ();
	Deployment::SetCurrent (deployment);

	return true;
}

bool
PluginInstance::InitializePluginAppDomain ()
{
	bool result = false;

	system_windows_assembly = mono_assembly_load_with_partial_name ("System.Windows, Version=2.0.5.0, Culture=neutral, PublicKeyToken=7cec85d7bea7798e", NULL);
	
	if (system_windows_assembly) {
		MonoImage *image;
		MonoClass *app_launcher;

		result = true;

		image = mono_assembly_get_image (system_windows_assembly);
		
		d (printf ("Assembly: %s\n", mono_image_get_filename (image)));
		
		app_launcher = mono_class_from_name (image, "Mono", "ApplicationLauncher");
		if (!app_launcher) {
			g_warning ("could not find ApplicationLauncher type");
			return false;
		}

		moon_exception = mono_class_from_name (image, "Mono", "MoonException");
		if (!moon_exception) {
			g_warning ("could not find MoonException type");
			return false;
		}
		
		moon_load_xaml  = MonoGetMethodFromName (app_launcher, "CreateXamlLoader", -1);
		moon_initialize_deployment_xap   = MonoGetMethodFromName (app_launcher, "InitializeDeployment", 2);
		moon_initialize_deployment_xaml   = MonoGetMethodFromName (app_launcher, "InitializeDeployment", 0);
		moon_destroy_application = MonoGetMethodFromName (app_launcher, "DestroyApplication", -1);

		if (moon_load_xaml == NULL || moon_initialize_deployment_xap == NULL || moon_initialize_deployment_xaml == NULL || moon_destroy_application == NULL) {
			g_warning ("lookup for ApplicationLauncher methods failed");
			result = false;
		}

		moon_exception_message = MonoGetPropertyFromName (mono_get_exception_class(), "Message");
		moon_exception_error_code = MonoGetPropertyFromName (moon_exception, "ErrorCode");

		if (moon_exception_message == NULL || moon_exception_error_code == NULL) {
			g_warning ("lookup for MoonException properties failed");
			result = false;
		}
	} else {
		printf ("Plugin AppDomain Creation: could not find System.Windows.dll.\n");
	}

	printf ("Plugin AppDomain Creation: %s\n", result ? "OK" : "Failed");

	return result;
}

ErrorEventArgs *
PluginInstance::ManagedExceptionToErrorEventArgs (MonoObject *exc)
{
	int errorCode = -1;
	char* message = NULL;

	if (mono_object_isinst (exc, mono_get_exception_class())) {
		MonoObject *ret = mono_property_get_value (moon_exception_message, exc, NULL, NULL);

		message = mono_string_to_utf8 ((MonoString*)ret);
	}
	if (mono_object_isinst (exc, moon_exception)) {
		MonoObject *ret = mono_property_get_value (moon_exception_error_code, exc, NULL, NULL);

		errorCode = *(int*) mono_object_unbox (ret);
	}
	
	return new ErrorEventArgs (RuntimeError, errorCode, message);
}

gpointer
PluginInstance::ManagedCreateXamlLoader (XamlLoader* native_loader, const char *file, const char *str)
{
	MonoObject *loader;
	MonoObject *exc = NULL;
	if (moon_load_xaml == NULL)
		return NULL;

	PluginInstance *this_obj = this;
	void *params [5];

	Deployment::SetCurrent (deployment);

	params [0] = &native_loader;
	params [1] = &this_obj;
	params [2] = &surface;
	params [3] = file ? mono_string_new (mono_domain_get (), file) : NULL;
	params [4] = str ? mono_string_new (mono_domain_get (), str) : NULL;
	loader = mono_runtime_invoke (moon_load_xaml, NULL, params, &exc);

	if (exc) {
		deployment->GetSurface()->EmitError (ManagedExceptionToErrorEventArgs (exc));
		return NULL;
	}

	return GUINT_TO_POINTER (mono_gchandle_new (loader, false));
}

gpointer
PluginInstance::ManagedCreateXamlLoaderForFile (XamlLoader *native_loader, const char *file)
{
	return ManagedCreateXamlLoader (native_loader, file, NULL);
}

gpointer
PluginInstance::ManagedCreateXamlLoaderForString (XamlLoader* native_loader, const char *str)
{
	return ManagedCreateXamlLoader (native_loader, NULL, str);
}

void
PluginInstance::ManagedLoaderDestroy (gpointer loader_object)
{
	guint32 loader = GPOINTER_TO_UINT (loader_object);
	if (loader)
		mono_gchandle_free (loader);
}

bool
PluginInstance::ManagedInitializeDeployment (const char *file)
{
	if (moon_initialize_deployment_xap == NULL && moon_initialize_deployment_xaml)
		return NULL;

	PluginInstance *this_obj = this;
	void *params [2];
	MonoObject *ret;
	MonoObject *exc = NULL;

	Deployment::SetCurrent (deployment);

	params [0] = &this_obj;
	if (file != NULL) {
		params [1] = mono_string_new (mono_domain_get (), file);
		ret = mono_runtime_invoke (moon_initialize_deployment_xap, NULL, params, &exc);
	} else {
		ret = mono_runtime_invoke (moon_initialize_deployment_xaml, NULL, params, &exc);
	}
	
	if (exc) {
		deployment->GetSurface()->EmitError (ManagedExceptionToErrorEventArgs (exc));
		return false;
	}

	return (bool) (*(MonoBoolean *) mono_object_unbox(ret));
}

void
PluginInstance::ManagedDestroyApplication ()
{
	if (moon_destroy_application == NULL)
		return;

	PluginInstance *this_obj = this;
	MonoObject *exc = NULL;
	void *params [1];
	params [0] = &this_obj;

	Deployment::SetCurrent (deployment);

	mono_runtime_invoke (moon_destroy_application, NULL, params, &exc);

	if (exc)
		deployment->GetSurface()->EmitError (ManagedExceptionToErrorEventArgs (exc));
}

#endif
