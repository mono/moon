/*
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Copyright (c) 2007-2008 Novell, Inc.
 *
 * Authors:
 *	Jackson Harper (jackson@ximian.com)
 *
 */


#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <dbus/dbus-glib.h>

#include "gtkembedmoz/gtkmozembed.h"


#include "nsXPCOMGlue.h"


#define DRT_LOGGER_SERVICE    "mono.moonlight.tests"
#define DRT_LOGGER_PATH       "/mono/moonlight/tests/logger"
#define DRT_LOGGER_INTERFACE  "mono.moonlight.tests.logger.ITestLogger"



typedef struct _AgViewer {
	GtkWindow  *top_level_window;
	GtkWidget  *moz_embed;
} AgViewer;

static int escaped_pressed_count = 0;
static AgViewer *new_gtk_browser    ();
static gboolean key_press_cb (GtkWidget* widget, GdkEventKey* event, GtkWindow* window);
static void request_test_runner_shutdown ();

int
main(int argc, char **argv)
{
	char* test_path = argv [argc - 1];

	int frame_width = 800;
	int frame_height = 800;

	gtk_init (&argc, &argv);

	if (argc < 2) {
		g_print ("You must specifiy a test file for agviewer to load\n");
		exit (-1);
	}

	for (int i = 1; i < argc - 2; i++) {
		if (!g_strcasecmp ("-framewidth", argv [i]))
			frame_width = strtol (argv [++i], NULL, 10);
		if (!g_strcasecmp ("-frameheight", argv [i]))
			frame_height = strtol (argv [++i], NULL, 10);
		if (!g_strcasecmp ("-working-dir", argv [i])) {
			
			if (chdir (argv [++i]) != 0) {
				g_warning ("Unable to set working directory.\n");
				exit (-1);
			}
			
		}
	}

	AgViewer* browser = new_gtk_browser ();

	gtk_widget_set_usize (browser->moz_embed, frame_width, frame_height);

	gtk_widget_show_all (browser->moz_embed);
	gtk_widget_show_all (GTK_WIDGET (browser->top_level_window));
	gtk_window_fullscreen (browser->top_level_window);

	gtk_moz_embed_load_url (GTK_MOZ_EMBED (browser->moz_embed), test_path);

	gtk_main();

	gtk_widget_destroy (GTK_WIDGET (browser->top_level_window));
	
	return 0;
}

static AgViewer *
new_gtk_browser ()
{
	AgViewer *browser = 0;

	browser = g_new0 (AgViewer, 1);
	browser->top_level_window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

	static const GREVersionRange gre_version = {
		"1.8", PR_TRUE,
		"9.9", PR_TRUE
	};

	char xpcom_lib_path [PATH_MAX];
	char* xpcom_dir_path;

	nsresult rv = GRE_GetGREPathWithProperties (&gre_version, 1, nsnull, 0,	xpcom_lib_path, sizeof (xpcom_lib_path));
	xpcom_dir_path = g_path_get_dirname (xpcom_lib_path);
	gtk_moz_embed_set_comp_path (xpcom_dir_path);

	g_free (xpcom_dir_path);

	browser->moz_embed = gtk_moz_embed_new();
	gtk_container_add (GTK_CONTAINER (browser->top_level_window),
			browser->moz_embed);

	g_signal_connect (G_OBJECT (browser->top_level_window), "key_press_event", G_CALLBACK (key_press_cb), browser->top_level_window);

	return browser;
}

static gboolean
key_press_cb (GtkWidget* widget, GdkEventKey* event, GtkWindow* window)
{
	switch (event->keyval) {
	case GDK_Escape:
		if (escaped_pressed_count == 0)
			request_test_runner_shutdown ();
		else if (escaped_pressed_count > 0)
			exit (1);
		escaped_pressed_count++;
		return TRUE;
	}

	return FALSE;
}

static void
request_test_runner_shutdown ()
{
	g_type_init ();

	DBusGProxy* dbus_proxy;
	DBusGConnection* connection;
	GError* error = NULL;  

	error = NULL;
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (!connection) {
		g_warning ("Failed to open connection to bus: %s\n", error->message);
		g_error_free (error);
	}

	dbus_proxy = dbus_g_proxy_new_for_name (connection,
			DRT_LOGGER_SERVICE,
			DRT_LOGGER_PATH,
			DRT_LOGGER_INTERFACE);

	dbus_g_proxy_call_no_reply (dbus_proxy, "RequestShutdown", G_TYPE_INVALID);
}
