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

#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#include <signal.h>

#include "runtime.h"
#include "deployment.h"
#include "downloader.h"
#include "uri.h"

#include "gtk/window-gtk.h"
#include "gtk/pal-gtk.h"

#include "getopts.h"
#include "debug.h"

#include <mono/metadata/mono-config.h>

using namespace Moonlight;

extern guint32 moonlight_flags;
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
icon_loader_write_cb (EventObject *obj, EventArgs *ea, gpointer user_data)
{
	HttpRequestWriteEventArgs *args = (HttpRequestWriteEventArgs *) ea;
	IconLoader *icon = (IconLoader *) user_data;
	
	if (icon->loader && !gdk_pixbuf_loader_write (icon->loader, (const guchar *) args->GetData (), args->GetCount (), NULL)) {
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

			application->GetResource (NULL, uri, icon_loader_notify_cb, icon_loader_write_cb, MediaPolicy, HttpRequest::DisableFileStorage, NULL, loader);
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

static bool
add_mono_config (const char *plugin_dir)
{
	struct stat st;
	char *plugin_path = g_build_filename (plugin_dir, "libmoonpluginxpi.so", NULL);
	if (g_stat (plugin_path, &st) == -1) {
		g_free (plugin_path);
		plugin_path = g_build_filename (plugin_dir, "libmoonplugin.so", NULL);
		if (g_stat (plugin_path, &st) == -1) {
			g_free (plugin_path);
			g_warning ("unable to locate libmoonplugin{xpi}.so");
			return false;
		}
	}

	// Must dllmap moon and moonplugin, otherwise it doesn't know where to get it
	char* plugin_config = g_strdup_printf(
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
"<configuration>"
	"<dllmap dll=\"moonplugin\" target=\"%s\" />"
	"<dllmap dll=\"moon\" target=\"%s\" />"
"</configuration>", plugin_path, plugin_path);
	mono_config_parse_memory(plugin_config);
	g_free (plugin_config);

	g_free (plugin_path);

	return true;
}

static bool
load_app (Deployment *deployment, const char *base_dir, MoonAppRecord *app)
{
	Surface *surface;
	char *path, *uri;
	
	/* use the App's origin URI as the surface's SourceLocation so that it
	 * can do proper web download checks */
	surface = deployment->GetSurface ();
	surface->SetSourceLocation (app->origin);
	
	/* get the install path/uri for the Xap */
	path = g_build_filename (base_dir, app->uid, "Application.xap", NULL);
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
	MoonInstallerService *installer;
	MoonAppRecord *app;
	GtkWidget *widget, *window;
	OutOfBrowserSettings *oob;
	WindowSettings *settings;
	MoonWindow *moon_window;
	int width, height;
	Surface *surface;

	/* get the pal services we need */
	winsys = runtime_get_windowing_system ();
	installer = runtime_get_installer_service ();

	/* fetch the app record */
	if (!(app = installer->GetAppRecord (app_id))) {
		g_warning ("Could not find application: %s", app_id);
		return NULL;
	}

	/* create the moonlight widget */
	moon_window = winsys->CreateWindow (false, 0, 0);
	surface = new Surface (moon_window);
	deployment->SetSurface (surface);
	moon_window->SetSurface (surface);

	if (!load_app (deployment, installer->GetBaseInstallDir (), app))
		return NULL;
	
	surface->AddXamlHandler (Surface::ErrorEvent, error_handler, NULL);
	
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
		Uri uri;
		const char *hostname = NULL;

		if (uri.Parse (app->origin))
			hostname = uri.GetHost();

		if (!hostname || !*hostname)
			hostname = "localhost";

		char *window_title = g_strdup_printf ("%s - %s",
						      settings->GetTitle(),
						      hostname);

		gtk_window_set_title ((GtkWindow *) window, window_title);

		g_free (window_title);

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

	delete app;

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

#if DEBUG
void
static lunar_handle_native_sigsegv (int signal, siginfo_t *info, void *ptr)
{
	const char *signal_str;

	switch (signal) {
	case SIGSEGV: signal_str = "SIGSEGV"; break;
	case SIGFPE: signal_str = "SIGFPE"; break;
	case SIGABRT: signal_str = "SIGABRT"; break;
	case SIGQUIT: signal_str = "SIGQUIT"; break;
	case SIGCHLD: signal_str = "SIGCHLD"; break;
	case SIGHUP: signal_str = "SIGHUP"; break;
	default: signal_str = "UNKNOWN"; break;
	}
	LOG_OOB ("[%i lunar-launcher] %s (ignored)\n", getpid (), signal_str);
}
#endif

int main (int argc, char **argv)
{
#if DEBUG
	/* stdout defaults to block buffering if it's not writing to a terminal, which happens with our test harness:
	 * we redirect stdout to capture it. Force line buffering in all cases. */
	setlinebuf (stdout);

	/* We also need to disable SIGHUP - we have another workaround for the above problem for chrome, where we create
	 * a pseudo terminal to capture stdout/stderr. Problem is that when the browser closes after executing
	 * lunar-launcher, a SIGHUP is sent to all its descendant processes (since the browser is the foreground process
	 * of the controlling terminal), which ends up killing lunar-launcher. */
	struct sigaction sa;

	sa.sa_sigaction = lunar_handle_native_sigsegv;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;

	g_assert (sigaction (SIGHUP, &sa, NULL) != -1);
#endif

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

	add_mono_config (plugin_dir);

	runtime_init_browser (plugin_dir);
	g_free (plugin_dir);
	
	LOG_OOB ("[%i lunar-launcher]: Starting\n", getpid ());

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
	deployment->Shutdown ();
	runtime_shutdown ();
	
	/* Our shutdown is async, so keep processing events/timeouts until there is
	 * nothing more to do */	
	while (g_main_context_iteration (NULL, false)) {
		;
	}

	LOG_OOB ("[%i lunar-launcher]: Exiting\n", getpid ());

	return EXIT_SUCCESS;
}
