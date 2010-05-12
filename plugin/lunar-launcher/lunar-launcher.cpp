/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * lunar-launcher.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

#include "runtime.h"
#include "deployment.h"
#include "downloader.h"

#include "gtk/window-gtk.h"
#include "gtk/pal-gtk.h"

#include "lunar-downloader.h"
#include "getopts.h"

static BrowserBridge *bridge = NULL;
static const char *geometry = NULL;

typedef struct {
	GdkPixbufLoader *loader;
	GtkWindow *window;
} IconLoader;

static void
icon_loader_notify_cb (NotifyType type, gint64 args, gpointer user_data)
{
	IconLoader *icon = (IconLoader *) user_data;
	GtkWindow *window = icon->window;
	GdkPixbuf *pixbuf;
	GList *icon_list;
	
	icon_list = gtk_window_get_icon_list (window);
	
	switch (type) {
	case NotifyCompleted:
		if (icon->loader) {
			if (gdk_pixbuf_loader_close (icon->loader, NULL)) {
				/* get the pixbuf and add it to our icon_list */
				pixbuf = gdk_pixbuf_loader_get_pixbuf (icon->loader);
				icon_list = g_list_prepend (icon_list, pixbuf);
				gtk_window_set_icon_list (window, icon_list);
				g_free (icon);
				break;
			}
			
			/* fall through as if we got a NotifyFailed */
		}
	case NotifyFailed:
		if (icon->loader)
			g_object_unref (icon->loader);
		
		g_free (icon);
		break;
	default:
		break;
	}
}

static void
icon_loader_write_cb (void *buffer, gint32 offset, gint32 n, gpointer user_data)
{
	IconLoader *icon = (IconLoader *) user_data;
	
	if (icon->loader && !gdk_pixbuf_loader_write (icon->loader, (const guchar *) buffer, n, NULL)) {
		/* loading failed, destroy the loader */
		g_object_unref (icon->loader);
		icon->loader = NULL;
	}
}

static void
load_window_icons (GtkWindow *window, Deployment *deployment, IconCollection *icons)
{
	Application *application = deployment->GetCurrentApplication ();
	IconLoader *loader;
	int count, i;
	
	/* load the icons */
	if (icons && (count = icons->GetCount ()) > 0) {
		for (i = 0; i < count; i++) {
			Value *value = icons->GetValueAt (i);
			Icon *icon = value->AsIcon ();
			Uri *uri = icon->GetSource ();
			
			loader = g_new (IconLoader, 1);
			loader->loader = gdk_pixbuf_loader_new ();
			loader->window = window;
			
			application->GetResource (NULL, uri, icon_loader_notify_cb, icon_loader_write_cb, MediaPolicy, NULL, loader);
		}
	}
}

static void
resize_surface (GtkWidget *widget, GtkAllocation *allocated, Surface *surface)
{
	surface->Resize (allocated->width, allocated->height);
}

static void
error_handler (EventObject *sender, EventArgs *args, gpointer user_data)
{
	ErrorEventArgs *err = (ErrorEventArgs *) args;
	
	fprintf (stderr, "LunarLauncher Error: %s\n", err->GetErrorMessage ());
	
	exit (EXIT_FAILURE);
}

typedef BrowserBridge * (* create_bridge_func) (void);

static bool
load_bridge (const char *plugin_dir)
{
	create_bridge_func create_browser_bridge;
	char *bridge_path;
	void *handle;
	
	bridge_path = g_build_filename (plugin_dir, "libmoonplugin-curlbridge.so", NULL);
	
	handle = dlopen (bridge_path, RTLD_LAZY);
	g_free (bridge_path);
	
	if (handle == NULL) {
		g_warning ("Could not load curl bridge: %s.", dlerror ());
		return false;
	}
	
	if (!(create_browser_bridge = (create_bridge_func) dlsym (handle, "CreateBrowserBridge"))) {
		g_warning ("Could not locate CreateBrowserBridge symbol: %s.", dlerror ());
		return false;
	}
	
	bridge = create_browser_bridge ();
	
	LunarDownloader::SetBridge (bridge);
	
	return true;
}

static bool
load_app (Deployment *deployment, const char *app_id)
{
	MoonInstallerService *installer;
	const char *base_dir;
	MoonAppRecord *app;
	Surface *surface;
	char *path, *uri;
	
	installer = runtime_get_installer_service ();
	if (!(app = installer->GetAppRecord (app_id))) {
		g_warning ("Could not find application: %s", app_id);
		return false;
	}
	
	/* use the App's origin URI as the surface's SourceLocation so that it
	 * can do proper web download checks */
	surface = deployment->GetSurface ();
	surface->SetSourceLocation (app->origin);
	delete app;
	
	/* get the install path/uri for the Xap */
	base_dir = installer->GetBaseInstallDir ();
	path = g_build_filename (base_dir, app_id, "Application.xap", NULL);
	uri = g_strdup_printf ("file://%s", path);
	
	deployment->SetXapFilename (path);
	deployment->SetXapLocation (uri);
	g_free (path);
	g_free (uri);
	
	/* load the xap */
	return deployment->InitializeManagedDeployment (NULL, NULL, NULL);
}

static GtkWidget *
create_window (Deployment *deployment, const char *geometry, const char *app_id)
{
	MoonWindowingSystem *winsys;
	GtkWidget *widget, *window;
	OutOfBrowserSettings *oob;
	WindowSettings *settings;
	MoonWindow *moon_window;
	int width, height;
	Surface *surface;
	
	winsys = runtime_get_windowing_system ();
	
	/* create the moonlight widget */
	moon_window = winsys->CreateWindow (false, 0, 0);
	surface = new Surface (moon_window);
	deployment->SetSurface (surface);
	moon_window->SetSurface (surface);
	bridge->SetSurface (surface);
	
	surface->AddXamlHandler (Surface::ErrorEvent, error_handler, NULL);
	
	if (!load_app (deployment, app_id))
		return NULL;
	
	widget = ((MoonWindowGtk *) moon_window)->GetWidget ();
	g_signal_connect (widget, "size-allocate", G_CALLBACK (resize_surface), surface);
	gtk_widget_show (widget);
	
	/* create the window */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_app_paintable (window, true);
	g_signal_connect_after (window, "delete-event", G_CALLBACK (gtk_main_quit), NULL);
	gtk_container_add ((GtkContainer *) window, widget);
	
	if ((oob = deployment->GetOutOfBrowserSettings ())) {
		load_window_icons ((GtkWindow *) window, deployment, oob->GetIcons ());
		settings = oob->GetWindowSettings ();
	} else
		settings = NULL;
	
	if (settings != NULL) {
		gtk_window_set_title ((GtkWindow *) window, settings->GetTitle ());
		
		if (geometry == NULL) {
			gtk_window_resize ((GtkWindow *) window, settings->GetWidth (), settings->GetHeight ());
			
			if (settings->GetWindowStartupLocation () == WindowStartupLocationManual)
				gtk_window_move ((GtkWindow *) window, settings->GetLeft (), settings->GetTop ());
		}
		
		switch (settings->GetWindowStyle ()) {
		case WindowStyleBorderlessRoundCornersWindow:
		case WindowStyleNone:
			gtk_window_set_decorated ((GtkWindow *) window, false);
			break;
		default:
			/* by default, gtk enables window decorations */
			break;
		}
	} else if (oob != NULL) {
		gtk_window_set_title ((GtkWindow *) window, oob->GetShortName ());
	} else {
		gtk_window_set_title ((GtkWindow *) window, "Moonlight");
	}
	
	if (geometry != NULL) {
		/* override WindowSettings' dimensions */
		gtk_window_parse_geometry ((GtkWindow *) window, geometry);
	}
	
	/* resize the moonlight widget */
	gtk_window_get_size ((GtkWindow *) window, &width, &height);
	moon_window->Resize (width, height);
	
	return window;
}



static int
display_help (GetOptsContext *ctx, GetOptsOption *opt, const char *arg, void *valuep)
{
	fputs ("Usage: lunar-launcher [OPTIONS...] [appid]\n\n", stdout);
	getopts_print_help (ctx);
	exit (EXIT_SUCCESS);
	
	return 0;
}

static int
display_version (GetOptsContext *ctx, GetOptsOption *opt, const char *arg, void *valuep)
{
	fputs ("lunar-launcher " VERSION "\n", stdout);
	exit (EXIT_SUCCESS);
	
	return 0;
}

#define HELP_DESC      "Display help and quit"
#define VERSION_DESC   "Display version and quit"
#define GEOMETRY_DESC  "Override window geometry"

static GetOptsOption options[] = {
	{ "help",      'h',  GETOPTS_ARG_CUSTOM,          HELP_DESC,      NULL,     'h', display_help,    NULL       },
	{ "version",   'v',  GETOPTS_ARG_CUSTOM,          VERSION_DESC,   NULL,     'v', display_version, NULL       },
	{ "geometry",   0,   GETOPTS_REQUIRED_STRING_ARG, GEOMETRY_DESC,  NULL,     'g', NULL,            &geometry  },
	GETOPTS_TABLE_END
};

int main (int argc, char **argv)
{
	Deployment *deployment;
	GetOptsContext *ctx;
	const char **args;
	GtkWidget *window;
	char *plugin_dir;
	int n;
	
	gtk_init (&argc, &argv);
	g_thread_init (NULL);
	gdk_threads_init ();
	
	ctx = getopts_context_new (argc, argv, options, GETOPTS_FLAG_DEFAULT);
	getopts_parse_args (ctx);
	args = getopts_get_args (ctx, &n);
	
	if (n != 1)
		display_help (ctx, NULL, NULL, NULL);
	
	/* expects to be run from the xpi plugin dir */
	plugin_dir = g_path_get_dirname (argv[0]);
	runtime_init_browser (plugin_dir);
	load_bridge (plugin_dir);
	g_free (plugin_dir);
	
	lunar_downloader_init ();
	
	deployment = new Deployment ();
	Deployment::SetCurrent (deployment);
	
	if (!deployment->InitializeAppDomain ()) {
		g_warning ("Could not initialize the AppDomain.");
		return EXIT_FAILURE;
	}
	
	if (!(window = create_window (deployment, geometry, args[0])))
		return EXIT_FAILURE;
	
	gtk_widget_show (window);
	
	gdk_threads_enter ();
	gtk_main ();
	gdk_threads_leave ();
	
	getopts_context_free (ctx, true);
	lunar_downloader_shutdown ();
	deployment->Shutdown ();
	runtime_shutdown ();
	
	/* Our shutdown is async, so keep processing events/timeouts until there is
	 * nothing more to do */	
	while (g_main_context_iteration (NULL, false)) {
		;
	}

	return EXIT_SUCCESS;
}
