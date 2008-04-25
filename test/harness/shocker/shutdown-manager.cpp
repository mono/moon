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
 *
 * Author:
 *   Jackson Harper (jackson@ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 *
 * shutdown-manager.cpp: When shutdown is signaled, the shutdown manager
 * 			 will wait until everything (image capture) is done
 *			 before it will actually shutdown the app.
 * 
 */


#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <prinit.h>
#include <gtk/gtk.h>

#ifdef DBUS_ENABLED
#include <dbus/dbus-glib.h>
#endif

#include "shutdown-manager.h"


#define DRT_AGSERVER_SERVICE    "mono.moonlight.agserver"
#define DRT_AGSERVER_PATH       "/mono/moonlight/agserver"
#define DRT_AGSERVER_INTERFACE  "mono.moonlight.agserver.IAgserver"


#define TIMEOUT_INTERVAL 10000

static GMutex* shutdown_mutex = NULL;
static GCond*  shutdown_cond = NULL;
static gint    wait_count = 0;

static void execute_shutdown (ShockerScriptableControlObject *shocker);
static gboolean attempt_clean_shutdown (gpointer data);

void
shutdown_manager_init ()
{
	wait_count = 0;
	shutdown_mutex = g_mutex_new ();
	shutdown_cond = g_cond_new ();
}

void
shutdown_manager_shutdown ()
{
	wait_count = 0;

	g_mutex_free (shutdown_mutex);
	g_cond_free (shutdown_cond);
}

void
shutdown_manager_wait_increment ()
{
	g_assert (shutdown_mutex);
	g_assert (shutdown_cond);

	g_mutex_lock (shutdown_mutex);


	wait_count++;
	g_mutex_unlock (shutdown_mutex);
}

void
shutdown_manager_wait_decrement ()
{
	g_assert (shutdown_mutex);
	g_assert (shutdown_cond);

	g_mutex_lock (shutdown_mutex);

	wait_count--;
	if (wait_count == 0)
		g_cond_signal (shutdown_cond);

	g_mutex_unlock (shutdown_mutex);
}

void
shutdown_manager_wait ()
{
	g_assert (shutdown_mutex);
	g_assert (shutdown_cond);

	while (wait_count > 0)
		g_cond_wait (shutdown_cond, shutdown_mutex);
}

static void
execute_shutdown (ShockerScriptableControlObject *shocker)
{
	char *dont_die = getenv ("MOONLIGHT_SHOCKER_DONT_DIE");
	if (dont_die != NULL && dont_die [0] != 0)
		return;

	g_type_init ();

	DBusGConnection* connection;
	GError* error = NULL;  

	error = NULL;
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (!connection) {
		g_warning ("Failed to open connection to bus: %s\n", error->message);
		g_error_free (error);
	}

	DBusGProxy* dbus_proxy = dbus_g_proxy_new_for_name (connection,
			DRT_AGSERVER_SERVICE,
			DRT_AGSERVER_PATH,
			DRT_AGSERVER_INTERFACE);
	

	dbus_g_proxy_call_no_reply (dbus_proxy, "SignalShutdown", G_TYPE_INVALID, G_TYPE_INVALID);

//	if (!dbus_g_proxy_call (dbus_proxy, "SignalShutdown", &error, G_TYPE_INVALID, G_TYPE_INVALID)) {
//		g_warning ("unable to make signal shutdown call:  %s\n", error->message);
//	}



	if (gtk_main_level ()) {
		// We are running inside the embedded agviewer, so we can use gtk to signal shutdown
//		gtk_main_quit ();
	} else {
		// This block never actually gets called, since firefox is also using gtk_main.
		PR_ProcessExit (0);
	}
}

static gboolean
attempt_clean_shutdown (gpointer data)
{
	ShockerScriptableControlObject *shocker = (ShockerScriptableControlObject *) data;
	char *dont_die = getenv ("MOONLIGHT_SHOCKER_DONT_DIE");
	if (dont_die != NULL && dont_die [0] != 0)
		return FALSE;

	g_assert (shutdown_mutex);
	g_assert (shutdown_cond);

	bool ready_for_shutdown = false;

	g_mutex_lock (shutdown_mutex);
	if (wait_count <= 0)
		ready_for_shutdown = true;
	g_mutex_unlock (shutdown_mutex);

	if (ready_for_shutdown) {
		execute_shutdown (shocker);
		return FALSE;
	}

	return TRUE;
}

void
shutdown_manager_queue_shutdown (ShockerScriptableControlObject* shocker)
{
	g_assert (shutdown_mutex);
	g_assert (shutdown_cond);

	if (!wait_count)
		return execute_shutdown (shocker);

	if (!g_timeout_add (TIMEOUT_INTERVAL, attempt_clean_shutdown, shocker)) {
		g_error ("Unable to create timeout for queued shutdown, executing immediate shutdown.");
		execute_shutdown (shocker);
	}
}

