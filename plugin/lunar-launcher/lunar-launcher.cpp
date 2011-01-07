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

#include <glib.h>
#include <glib/gstdio.h>

#include <signal.h>

#include "runtime.h"
#include "deployment.h"
#include "downloader.h"
#include "uri.h"
#include "pal.h"
#include "window.h"

#include "debug.h"

#include <mono/metadata/mono-config.h>

using namespace Moonlight;

static MoonWindowingSystem *winsys;
static MoonInstallerService *installer;

typedef struct {
	MoonPixbufLoader *loader;
	MoonWindow *window;
	MoonError *moon_error;
} IconLoader;

static void
icon_loader_notify_cb (NotifyType type, gint64 args, gpointer user_data)
{
	IconLoader *icon = (IconLoader *) user_data;
	MoonWindow *window = icon->window;
	MoonPixbuf *pixbuf;
	
	switch (type) {
	case NotifyCompleted:
		if (icon->loader) {
			MoonError *error = NULL;
			icon->loader->Close (&error);
			if (error == NULL) {
				/* get the pixbuf and add it to our icon_list */
				pixbuf = icon->loader->GetPixbuf ();
				window->SetIconFromPixbuf (pixbuf);

				g_free (icon);
				break;
			}
			
			/* fall through as if we got a NotifyFailed */
		}
	case NotifyFailed:
		delete icon->loader;
		g_free (icon);
		break;
	default:
		break;
	}
}

static void
CreateLoader (IconLoader *icon, const guchar *buffer)
{
	if (!(moonlight_flags & RUNTIME_INIT_ALL_IMAGE_FORMATS)) {
		// 89 50 4E 47 == png magic
		if (buffer[0] == 0x89)
			icon->loader = winsys->CreatePixbufLoader ("png");
		// ff d8 ff e0 == jfif magic
		else if (buffer[0] == 0xff)
			icon->loader = winsys->CreatePixbufLoader ("jpeg");

		else
			icon->moon_error = new MoonError (MoonError::EXCEPTION, 4001, "unsupported image type");
	} else {
		icon->loader = winsys->CreatePixbufLoader (NULL);
	}
}

static void
icon_loader_write_cb (EventObject *obj, EventArgs *ea, gpointer user_data)
{
	HttpRequestWriteEventArgs *args = (HttpRequestWriteEventArgs *) ea;
	IconLoader *icon = (IconLoader *) user_data;

	const guchar *buffer = (const guchar*)args->GetData ();
	gint32 offset = args->GetOffset ();
	gint32 n = args->GetCount ();

	if (icon->loader == NULL && offset == 0)
		CreateLoader (icon, buffer);

	if (icon->loader && icon->moon_error == NULL)
		icon->loader->Write (buffer, n, &icon->moon_error);
}

static void
load_window_icons (MoonWindow *window, Deployment *deployment, IconCollection *icons)
{
	Application *application = deployment->GetCurrentApplication ();
	IconLoader *loader;
	int count, i;
	
	/* load the icons */
	if (icons && (count = icons->GetCount ()) > 0) {
		for (i = 0; i < count; i++) {
			Value *value = icons->GetValueAt (i);
			Icon *icon = value->AsIcon ();
			const Uri *uri = icon->GetSource ();
			
			loader = g_new0 (IconLoader, 1);
			loader->window = window;

			application->GetResource (NULL, uri, icon_loader_notify_cb, icon_loader_write_cb, MediaPolicy, HttpRequest::DisableFileStorage, NULL, loader);
		}
	}
}

static void
error_handler (EventObject *sender, EventArgs *args, gpointer user_data)
{
	ErrorEventArgs *err = (ErrorEventArgs *) args;
	
	fprintf (stderr, "LunarLauncher Error: %s\n", err->GetErrorMessage ());
	
	exit (EXIT_SUCCESS);
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

static void
setup_app (Deployment *deployment, const char *base_dir, MoonAppRecord *app)
{
	Surface *surface;
	char *path, *uri;
	Uri *origin, *url;
	
	/* use the App's origin URI as the surface's SourceLocation so that it
	 * can do proper web download checks */
	surface = deployment->GetSurface ();

	origin = Uri::Create (app->origin);
	surface->SetSourceLocation (origin);
	delete origin;
	
	/* get the install path/uri for the Xap */
	path = g_build_filename (base_dir, app->uid, "Application.xap", NULL);
	uri = g_strdup_printf ("file://%s", path);
	url = Uri::Create (uri);
	
	deployment->SetXapFilename (path);
	deployment->SetXapLocation (url);
	g_free (path);
	g_free (uri);
	delete url;
}

static MoonWindow*
create_window (Deployment *deployment, const char *app_id)
{
	MoonAppRecord *app;
	OutOfBrowserSettings *oob;
	WindowSettings *settings;
	MoonWindow *moon_window;
	Surface *surface;

	/* fetch the app record */
	if (!(app = installer->GetAppRecord (app_id))) {
		g_warning ("Could not find application: %s", app_id);
		return NULL;
	}

	/* create the moonlight widget */
	moon_window = winsys->CreateWindow (MoonWindowType_Desktop, 0, 0);
	surface = new Surface (moon_window);
	surface->EnsureManagedPeer ();
	deployment->SetSurface (surface);
	moon_window->SetSurface (surface);

	setup_app (deployment, installer->GetBaseInstallDir (), app);
	
	surface->AddXamlHandler (Surface::ErrorEvent, error_handler, NULL);
	surface->unref ();
	
	/* load the xap and the AppManifest */
	if (!deployment->InitializeManagedDeployment (NULL, NULL, NULL)) {
		surface->unref ();
		delete app;
		return NULL;
	}
	
	if ((oob = deployment->GetOutOfBrowserSettings ())) {
		load_window_icons (moon_window, deployment, oob->GetIcons ());
		settings = oob->GetWindowSettings ();
	} else
		settings = NULL;
	
	if (settings != NULL) {
		Uri *uri;
		const char *hostname = NULL;

		uri = Uri::Create (app->origin);
		if (uri != NULL)
			hostname = uri->GetHost ();

		if (!hostname || !*hostname)
			hostname = "localhost";

		char *window_title = g_strdup_printf ("%s - %s",
						      settings->GetTitle(),
						      hostname);

		delete uri;

		moon_window->SetTitle (window_title);

		g_free (window_title);

		moon_window->SetStyle (settings->GetWindowStyle ());
	} else if (oob != NULL) {
		moon_window->SetTitle (oob->GetShortName ());
	} else {
		moon_window->SetTitle ("Moonlight");
	}
	
	delete app;
	
	return moon_window;
}



static int
display_help (void)
{
	fputs ("Usage: lunar-launcher [TOOLKIT OPTIONS...] [appid]\n\n", stdout);
	exit (EXIT_SUCCESS);
	
	return 0;
}

#if DEBUG
static void
lunar_handle_native_sigsegv (int signal, siginfo_t *info, void *ptr)
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
	fputs ("lunar-launcher " VERSION "\n", stdout);


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
	MoonWindow *window;
	char *plugin_dir;

	if (argc != 2)
		display_help ();

	/* expects to be run from the xpi plugin dir */
	plugin_dir = g_path_get_dirname (argv[0]);

	add_mono_config (plugin_dir);

	runtime_init_browser (plugin_dir, true);
	g_free (plugin_dir);

	/* get the pal services we need */
	winsys = runtime_get_windowing_system ();
	installer = runtime_get_installer_service ();

	LOG_OOB ("[%i lunar-launcher]: Starting\n", getpid ());

	deployment = new Deployment ();
	Deployment::SetCurrent (deployment);
	
	if (!deployment->InitializeAppDomain ()) {
		g_warning ("Could not initialize the AppDomain.");
		return EXIT_FAILURE;
	}
	
	if (!(window = create_window (deployment, argv[1])))
		return EXIT_FAILURE;

	winsys->RunMainLoop (window);

	deployment->Shutdown ();
	
	/* Our shutdown is async, so keep processing events/timeouts until there is
	 * nothing more to do */	
	while (g_main_context_iteration (NULL, false)) {
		;
	}

	runtime_shutdown ();

	LOG_OOB ("[%i lunar-launcher]: Exiting\n", getpid ());

	return EXIT_SUCCESS;
}
