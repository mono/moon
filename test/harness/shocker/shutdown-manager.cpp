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
#include "shutdown-manager.h"

static GMutex* shutdown_mutex = NULL;
static GCond*  shutdown_cond = NULL;
static gint    wait_for_threads = 0;

void
shutdown_manager_init ()
{
	wait_for_threads = 0;
	shutdown_mutex = g_mutex_new ();
	shutdown_cond = g_cond_new ();
}

void
shutdown_manager_shutdown ()
{
	wait_for_threads = 0;

	g_mutex_free (shutdown_mutex);
	g_cond_free (shutdown_cond);
}

void
shutdown_manager_wait_threads_increment ()
{
	g_assert (shutdown_mutex);
	g_assert (shutdown_cond);

	g_mutex_lock (shutdown_mutex);


	wait_for_threads++;
	g_mutex_unlock (shutdown_mutex);
}

void
shutdown_manager_wait_threads_decrement ()
{
	g_assert (shutdown_mutex);
	g_assert (shutdown_cond);

	g_mutex_lock (shutdown_mutex);

	wait_for_threads--;
	if (wait_for_threads == 0)
		g_cond_signal (shutdown_cond);

	g_mutex_unlock (shutdown_mutex);
}

void
shutdown_manager_wait_for_threads ()
{
	g_assert (shutdown_mutex);
	g_assert (shutdown_cond);

	while (wait_for_threads > 0)
		g_cond_wait (shutdown_cond, shutdown_mutex);
}

static void *
wait_for_shutdown (void* data)
{
	shutdown_manager_wait_for_threads ();

	PR_ProcessExit (0);

	g_thread_exit (NULL);
	return NULL;
}

void
shutdown_manager_queue_shutdown ()
{
	g_assert (shutdown_mutex);
	g_assert (shutdown_cond);

	if (!wait_for_threads) {
		exit (0);
		return;
	}

	GError *error;
	GThread *worker;

	worker = g_thread_create ((GThreadFunc) wait_for_shutdown, NULL, FALSE, &error);
	if (!worker) {
		g_warning ("Unable to create thread for queued shutdown thread, things aren't going to go so well for you.", error->message);
		g_error_free (error);
	}	
}

