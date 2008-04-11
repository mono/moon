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
#include "shutdown-manager.h"

#define TIMEOUT_INTERVAL 10000

static GMutex* shutdown_mutex = NULL;
static GCond*  shutdown_cond = NULL;
static gint    wait_count = 0;

static void execute_shutdown ();
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
execute_shutdown ()
{
	char *dont_die = getenv ("MOONLIGHT_SHOCKER_DONT_DIE");
	if (dont_die != NULL && dont_die [0] != 0)
		return;

	if (gtk_main_level ()) {
		// We are running inside the embedded agviewer, so we can use gtk to signal shutdown
		gtk_main_quit ();
	} else {
		// This block never actually gets called, since firefox is also using gtk_main.
		PR_ProcessExit (0);
	}
}

static gboolean
attempt_clean_shutdown (gpointer data)
{
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
		execute_shutdown ();
		return FALSE;
	}

	return TRUE;
}

void
shutdown_manager_queue_shutdown ()
{
	g_assert (shutdown_mutex);
	g_assert (shutdown_cond);

	if (!wait_count)
		return execute_shutdown ();

	if (!g_timeout_add (TIMEOUT_INTERVAL, attempt_clean_shutdown, NULL)) {
		g_error ("Unable to create timeout for queued shutdown, executing immediate shutdown.");
		execute_shutdown ();
	}
}

