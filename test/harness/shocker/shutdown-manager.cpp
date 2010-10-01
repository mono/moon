/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * shutdown-manager.cpp: When shutdown is signaled, the shutdown manager
 * 			 will wait until everything (image capture) is done
 *			 before it will actually shutdown the app.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <glib.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "debug.h"
#include "shutdown-manager.h"
#include "input.h"
#include "shocker-plugin.h"
#include "harness.h"

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
	if (getenv ("MOONLIGHT_SHOCKER_DONT_EXIT") == NULL) {
		printf ("[%i shocker] Could not shutdown nicely, exiting the process. Set MOONLIGHT_SHOCKER_DONT_EXIT to disable.\n", getpid ());
		exit (0);
	} else {
		printf ("[%i shocker] Could not shutdown nicely, but won't exit process since MOONLIGHT_SHOCKER_DONT_EXIT is set.\n", getpid ());
	}
	return false;
}

static gboolean
send_alt_f4 (gpointer dummy)
{
	return false;
	printf ("[%i shocker] sending Alt+F4 to browser...\n", getpid ());
	InputProvider *input = InputProvider::GetInstance ();
	// send alt-f4
	input->SendKeyInput (VK_MENU, true, false, false);
	input->SendKeyInput (VK_F4, true, false, false);
	input->SendKeyInput (VK_F4, false, false, false);
	input->SendKeyInput (VK_MENU, false, false, false);
	return false;
}

static gboolean
send_ctrl_w (gpointer dummy)
{
	printf ("[%i shocker] sending Ctrl-W to browser...\n", getpid ());
	// This doesn't work with oob - there is no menu so nothing handles ctrl-q
	InputProvider *input = InputProvider::GetInstance ();
	// send ctrl-q
	input->SendKeyInput (VK_CONTROL, true, false, false);
	input->SendKeyInput (VK_W, true, false, false);
	input->SendKeyInput (VK_W, false, false, false);
	input->SendKeyInput (VK_CONTROL, false, false, false);
	return false;
}

static void send_wm_delete (Window window)
{
	printf ("[%i shocker] sending WM_DELETE_WINDOW event to %p\n", getpid (), (void*) window);

	Display *display = XOpenDisplay (NULL);
	Atom WM_PROTOCOLS = XInternAtom (display, "WM_PROTOCOLS", False);
	Atom WM_DELETE_WINDOW = XInternAtom (display, "WM_DELETE_WINDOW", False);
	XClientMessageEvent ev;
	
	ev.type = ClientMessage;
	ev.window = window;
	ev.message_type = WM_PROTOCOLS;
	ev.format = 32;
	ev.data.l [0] = WM_DELETE_WINDOW;
	ev.data.l [1] = 0;

	XSendEvent (display, ev.window, False, 0, (XEvent*) &ev);
	XCloseDisplay (display);
}

static void
execute_shutdown ()
{
	char executable [1024];

	LOG_SHUTDOWN ("[%i shocker] execute_shutdown\n", getpid ())

	char *dont_die = getenv ("MOONLIGHT_SHOCKER_DONT_DIE");
	if (dont_die != NULL && dont_die [0] != 0)
		return;

	g_type_init ();

	if (PluginObject::browser_app_context != 0) {
		printf ("[%i shocker] shutting down browser...\n", getpid ());

		send_wm_delete (PluginObject::browser_app_context);
		PluginObject::browser_app_context = 0;

		memset (executable, 0, sizeof (executable));
		readlink ("/proc/self/exe", executable, sizeof (executable) - 1);
		printf ("[%i shocker] executable: %s\n", getpid (), executable);

		if (executable [0] == 0 || strstr (executable, "chrome") != NULL) {
			/* This happens on OS 11.2 causing readlink to fail: /proc/self/exe: /lib64/libz.so.1: no version information available (required by /proc/self/exe)
			 * readlink does not fail for firefox. */
			LOG_SHUTDOWN ("[%i shocker] we're executing inside chrome ('%s' to be exact), sending ctrl+w\n", getpid (), executable);
			send_ctrl_w (NULL);
		} else {
			/* Some other browser (firefox), don't send anything since it messes with oob tests
			 * because it is racy: the current process might not be the one with the focus. */
		}
	} else {
		WindowInfoEx ex;
		memcpy (ex.wi.title, "IWANTEX", sizeof ("IWANTEX"));
		if (WindowHelper_GetWindowInfo (getpid (), (WindowInfo *) &ex) == 0) {
			send_wm_delete (ex.window);
		}

		// have a backup
		// note that this is racy: the current process might not be the one with the focus.
		g_timeout_add (5000, send_ctrl_w, NULL); // this doesn't work for oob apps
		g_timeout_add (5000, send_alt_f4, NULL); // this doesn't work if there is no window manager (inside xvfb while running tests for instance)
	}
	
	// Have a backup in case the above fails.
	g_timeout_add (25000, force_shutdown, NULL);
}

static gboolean
attempt_clean_shutdown (gpointer data)
{
	LOG_SHUTDOWN ("[%i shocker] attempt_clean_shutdown\n", getpid ())

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
	LOG_SHUTDOWN ("[%i shocker] shutdown_manager_queue_shutdown\n", getpid ())

	if (g_atomic_int_get (&wait_count) == 0)
		return execute_shutdown ();

	printf ("[%i shocker] Unable to execute shutdown immediately (pending screenshots), attempting a clean shutdown.\n", getpid ());
	
	if (!g_timeout_add (100, attempt_clean_shutdown, NULL)) {
		printf ("[%i shocker] Unable to create timeout for queued shutdown, executing immediate shutdown.\n", getpid ());
		execute_shutdown ();
	}
}

void
SignalShutdown (const char *window_name)
{
	LOG_SHUTDOWN ("[%i shocker] SignalShutdown\n", getpid ())
	shutdown_manager_queue_shutdown ();
}

void
TestHost_SignalShutdown (const char *window_name)
{
	LOG_SHUTDOWN ("[%i shocker] TestHost_SignalShutdown\n", getpid ())
	SignalShutdown (window_name);
}
