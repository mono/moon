/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
 * Authors:
 *   Jackson Harper (jackson@ximian.com)
 *   Rolf Bjarne Kvinge (RKvinge@novell.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 *
 * shutdown-manager.cpp: When shutdown is signaled, the shutdown manager
 * 			 will wait until everything (image capture) is done
 *			 before it will actually shutdown the app.
 * 
 */


#include <glib.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "shutdown-manager.h"
#include "input.h"
#include "plugin.h"

static gint wait_count = 0;

static void execute_shutdown ();
static gboolean attempt_clean_shutdown (gpointer data);

void
shutdown_manager_wait_increment ()
{
	g_atomic_int_inc (&wait_count);
}

void
shutdown_manager_wait_decrement ()
{
	g_atomic_int_dec_and_test (&wait_count);
}

static gboolean
force_shutdown (gpointer data)
{
	printf ("[shocker] Could not shutdown nicely, exiting the process.\n");
	exit (0);
	
	return false;
}

static void
execute_shutdown ()
{
	char *dont_die = getenv ("MOONLIGHT_SHOCKER_DONT_DIE");
	if (dont_die != NULL && dont_die [0] != 0)
		return;

	g_type_init ();

	if (PluginObject::browser_app_context != 0) {
		printf ("[shocker] shutting down firefox...\n");

		Display *display = XOpenDisplay (NULL);
		Atom WM_PROTOCOLS = XInternAtom (display, "WM_PROTOCOLS", False);
		Atom WM_DELETE_WINDOW = XInternAtom (display, "WM_DELETE_WINDOW", False);
		XClientMessageEvent ev;
		
		ev.type = ClientMessage;
		ev.window = PluginObject::browser_app_context;
		ev.message_type = WM_PROTOCOLS;
		ev.format = 32;
		ev.data.l [0] = WM_DELETE_WINDOW;
		ev.data.l [1] = 0;
		
		XSendEvent (display, ev.window, False, 0, (XEvent*) &ev);
		XCloseDisplay (display);
		
		PluginObject::browser_app_context = 0;
	} else {
		printf ("[shocker] sending Ctrl-Q to firefox...\n");
		InputProvider input;
		// send ctrl-q
		input.SendKeyInput (VK_CONTROL, true);
		input.SendKeyInput (VK_Q, true);
		input.SendKeyInput (VK_Q, false);
		input.SendKeyInput (VK_CONTROL, false);
	}
	
	// Have a backup in case the above fails.
	g_timeout_add (25000, force_shutdown, NULL);
}

static gboolean
attempt_clean_shutdown (gpointer data)
{
	bool ready_for_shutdown = false;

	ready_for_shutdown = g_atomic_int_get (&wait_count) <= 0;

	if (ready_for_shutdown) {
		execute_shutdown ();
		return FALSE;
	}

	return TRUE;
}

void
shutdown_manager_queue_shutdown ()
{
	if (g_atomic_int_get (&wait_count) == 0)
		return execute_shutdown ();

	printf ("[shocker] Unable to execute shutdown immediately (pending screenshots), attempting a clean shutdown.\n");
	
	if (!g_timeout_add (100, attempt_clean_shutdown, NULL)) {
		printf ("[shocker] Unable to create timeout for queued shutdown, executing immediate shutdown.\n");
		execute_shutdown ();
	}
}

void
SignalShutdown (const char *window_name)
{
	shutdown_manager_queue_shutdown ();
}
