/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * harness.cpp: Interface with our managed test harness
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "debug.h"
#include "harness.h"
#include "plugin.h"

bool
send_all (int sockfd, const char *buffer, guint32 length)
{
	guint32 written = 0;
	ssize_t result;
	
	do {
		do {
			result = send (sockfd, buffer + written, length - written, MSG_NOSIGNAL);
		} while (result == -1 && errno == EINTR);
		
		if (result == -1) {
			printf ("[Shocker]: send failed, returned %i (%i %s)\n", (int) result, errno, strerror (errno));
			return false;
		}
		
		written += result;
	} while (written < length);
	
	return true;
}

bool
recv_all (int sockfd, guint8 *buffer, guint32 length)
{
	guint32 read = 0;
	ssize_t result;

	do {
		do {
			result = recv (sockfd, buffer + read, length - read, MSG_WAITALL);
		} while (result == -1 && errno == EINTR);

		if (result == -1) {
			printf ("[%i shocker]: receive failed, returned %i (%i %s)\n", getpid (), (int) result, errno, strerror (errno));
			return false;
		}

		read += result;
	} while (read < length);

	return true;
}

bool
Harness::SendMessage (const char *msg, guint8 **buffer, guint32 *output_length)
{
	char *tmp;
	int sockfd;
	int result;
	sockaddr_in addr;
	char *strport;
	int port;
	sockaddr_in peer;
	socklen_t peer_length = sizeof (peer);

	*output_length = 0;
	*buffer = NULL;

	// get and validate port
	strport = getenv ("MOONLIGHT_HARNESS_LISTENER_PORT");
	if (strport == NULL || strport [0] == 0) {
		printf ("[%i shocker]: MOONLIGHT_HARNESS_LISTENER_PORT is not set, assuming a default value of 1234\n", getpid ());
		port = 1234;
	} else {
		port = atoi (strport);
		if (port < 1024) {
			printf ("[%i shocker]: The port MOONLIGHT_HARNESS_LISTENER_PORT (%s) is probably invalid, it should be >= 1024.\n", getpid (), strport);
		}
	}

	// create the socket
	sockfd = socket (PF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		printf ("[%i shocker]: Failed to open socket: %i (%s)\n", getpid (), errno, strerror (errno));
		return false;
	}

	// connect
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	memset (addr.sin_zero, 0, sizeof (addr.sin_zero));
	result = inet_pton (AF_INET, "127.0.0.1", &addr.sin_addr);
	result = connect (sockfd, (struct sockaddr *) &addr, sizeof (addr));

	if (result == -1) {
		printf ("[%i shocker]: Could not connect to localhost:%i (%i %s)\n", getpid (), port, errno, strerror (errno));
		goto cleanup;
	} 

	result = getsockname (sockfd, (struct sockaddr *) &peer, &peer_length);
	LOG_HARNESS ("[%i shocker]: Connected to port %i\n", getpid (), ntohs (peer.sin_port));

	// send request
	tmp = g_strdup_printf ("v2|%s", msg);
	if (!send_all (sockfd, tmp, strlen (tmp))) {
		g_free (tmp);
		result = -1;
		goto cleanup;
	}
	g_free (tmp);

	// First 4 bytes is the size
	if (!recv_all (sockfd, (guint8 *) output_length, 4)) {
		result = -1;
		goto cleanup;
	}

	LOG_HARNESS ("[%i shocker]: receiving %i bytes...\n", getpid (), *output_length);

	// Then the response
	*buffer = (guint8 *) g_malloc0 (*output_length + 1 /* any missing null terminator */);
	if (!recv_all (sockfd, *buffer, *output_length)) {
		g_free (*buffer);
		*buffer = NULL;
		result = -1;
		goto cleanup;
	}

	LOG_HARNESS ("[%i shocker]: received %i bytes from port %i\n", getpid (), *output_length, ntohs (peer.sin_port));

cleanup:
	close (sockfd);

	return result != -1;
}

char *
Harness::GetRuntimePropertyValue (const char *propertyName)
{
	char *result = NULL;
	char *msg;
	guint8 *response;
	guint32 response_length;

	LOG_HARNESS ("[%i shocker] Harness::GetRuntimePropertyValue (%s)\n", getpid (), propertyName);

	msg = g_strdup_printf ("TestLogger.GetRuntimePropertyValue %s", propertyName);

	if (SendMessage (msg, &response, &response_length)) {
		result = (char *) response;
	} else {
		g_free (response);
		response = NULL;
	}
	g_free (msg);

	LOG_HARNESS ("[%i shocker] Harness::GetRuntimePropertyValue (%s) => %s\n", getpid (), propertyName, result);

	return result;
}

void
Harness::SetRuntimePropertyValue (const char *propertyName, const char *value)
{
	char *msg;
	guint8 *response;
	guint32 response_length;

	LOG_HARNESS ("[%i shocker] Harness::SetRuntimePropertyValue (%s, %s)\n", getpid (), propertyName, value);

	g_assert (strchr (propertyName, '|') == NULL);
	g_assert (strchr (value, '|') == NULL);

	msg = g_strdup_printf ("TestLogger.SetRuntimePropertyValue %s|%s", propertyName, value);

	if (SendMessage (msg, &response, &response_length)) {
		/* nothing to do here */
	}
	g_free (response);
	g_free (msg);
}

int
Harness::StartProcess (const char *exePath, const char *argList, guint32 *id, bool isShortcut)
{
	int result = -1; /* failure */
	char *msg;
	guint8 *response;
	guint32 response_length;

	g_assert (strchr (exePath, '|') == NULL);
	g_assert (strchr (argList, '|') == NULL);

	LOG_HARNESS ("[%i shocker] Harness::StartProcess (exePath: '%s', argList: '%s', isShortcut: %i)\n", getpid (), exePath, argList, isShortcut);

	msg = g_strdup_printf ("ProcessHelper.StartProcess %s|%s|%i", exePath, argList, isShortcut);

	if (SendMessage (msg, &response, &response_length)) {
		if (id != NULL) {
			if (response_length >= 4) {
				*id = *(guint32 *) response;
			}
			LOG_HARNESS ("[%i shocker] Harness::StartProcess (exePath: '%s', argList: '%s', isShortcut: %i): Started, id: %i\n", getpid (), exePath, argList, isShortcut, *id);
		} else {
			LOG_HARNESS ("[%i shocker] Harness::StartProcess (exePath: '%s', argList: '%s', isShortcut: %i): Started (no id returned)\n", getpid (), exePath, argList, isShortcut);
		}
		result = 0; /* success */
	} else {
		LOG_HARNESS ("[%i shocker] Harness::StartProcess (exePath: '%s', argList: '%s', isShortcut: %i): Failed.\n", getpid (), exePath, argList, isShortcut);
	}

	g_free (response);
	g_free (msg);

	
	return result;
}

int
Harness::WaitForProcessExit (guint32 processId, int timeoutSec)
{
	int result = -1; /* failure */
	char *msg;
	guint8 *response;
	guint32 response_length;

	LOG_HARNESS ("[%i shocker] Harness::WaitForProcessExit (%i, %i)\n", getpid (), processId, timeoutSec);

	msg = g_strdup_printf ("ProcessHelper.WaitForProcessExit %i|%i", processId, timeoutSec);

	if (SendMessage (msg, &response, &response_length)) {
		/* No response required, this method succeeds even if we hit the timeout */
		result = 0; /* success */
	}

	g_free (response);
	g_free (msg);

	LOG_HARNESS ("[%i shocker] Harness::WaitForProcessExit (%i, %i). Wait completed (or timeout was hit)\n", getpid (), processId, timeoutSec);

	return result;
}

int
Harness::FindProcesses (const char *processName, guint32 **processes, guint32 *processCount)
{
	int result = -1; /* failure */
	char *msg;
	guint8 *response;
	guint32 *ptr;
	guint32 response_length;

	g_assert (strchr (processName, '|') == NULL);

	LOG_HARNESS ("[%i shocker] Harness::FindProcesses (%s)\n", getpid (), processName);

	msg = g_strdup_printf ("ProcessHelper.FindProcesses %s", processName);

	if (SendMessage (msg, &response, &response_length)) {
		/* first four bytes is the processCount */
		ptr = (guint32 *) response;
		*processCount = *ptr++;
		*processes = (guint32 *) g_malloc0 (sizeof (guint32) * (*processCount)); /* we leak this */
		for (unsigned int i = 0; i < *processCount; i++) {
			(*processes) [i] = *ptr++;
		}
		if (shocker_flags & SHOCKER_DEBUG_HARNESS) {
			printf ("[%i shocker] Harness::FindProcesses (%s): Found %i processes:", getpid (), processName, *processCount);
			for (unsigned int i = 0; i < *processCount; i++) {
				printf ("%s%i", i == 0 ? "" : ", ", (*processes) [i]);
			}
			printf ("\n");
		}

		result = 0; /* success */
	}

	g_free (response);
	g_free (msg);

	return result;
}

int ProcessHelper_GetCurrentProcessID (guint32 *id)
{
	*id = getpid ();
	LOG_HARNESS ("[%i shocker] ProcessHelper_GetCurrentProcessId (): %i\n", getpid (), *id);
	return 0;
}

int
ProcessHelper_StartProcess (const char *exePath, const char *argList, guint32 *id, bool isShortcut)
{
	return Harness::StartProcess (exePath, argList, id, isShortcut);
}

int ProcessHelper_WaitForProcessExit (guint32 processId, guint32 timeoutSec)
{
	return Harness::WaitForProcessExit (processId, timeoutSec);
}

int ProcessHelper_BringToFront (guint32 processId)
{
	LOG_HARNESS ("[%i shocker] ProcessHelper_BringToFront (%i) [NOT IMPLEMENTED, ASSUMING PROCESS IS ALREADY AT FRONT]\n", getpid (), processId);
	return 0;
}

int ProcessHelper_FindProcesses (const char *processName, guint32 **processes, guint32 *processCount)
{
	return Harness::FindProcesses (processName, processes, processCount);
}

int ProcessHelper_KillProcess (guint32 processId)
{
	Shocker_FailTestFast ("ProcessHelper_KillProcess (%i): Not implemented");
	return -1;
}

int ProcessHelper_CloseTestWindows ()
{
	Shocker_FailTestFast ("ProcessHelper_CloseTestWindows (): Not implemented");
	return -1;
}

/*
 * WindowHelper
 */

void
find_window_r (guint32 pid, Window window, Display *display, Atom atom_pid, guint32 level, GSList **result)
{
	Atom actual_type_return;
	int actual_format_return;
	unsigned long nitems_return;
	unsigned long bytes_after_return;
	unsigned char *prop_return;
	guint32 pid_of_window = 0;
	XWindowAttributes window_attributes;
	WindowInfoEx *wix = NULL;
	WindowInfo *wi = NULL;
	Window dummy;
	int x, y;

	if (Success != XGetWindowProperty (display, window, atom_pid, 0, 1, False, XA_CARDINAL, &actual_type_return, &actual_format_return, &nitems_return, &bytes_after_return, &prop_return))
		return;

	if (prop_return != NULL) {
		pid_of_window = *(guint32 *) prop_return;
		XFree (prop_return);
	}

	if (pid_of_window == pid) {
		Atom _NET_WM_NAME = XInternAtom (display, "_NET_WM_NAME", False);
		Atom UTF8_STRING = XInternAtom (display, "UTF8_STRING", False);

//		printf ("[%i shocker] %*sFound window %p for pid %i\n", getpid (), level, " ", (void *) window, pid);

		XGetWindowAttributes (display, window, &window_attributes);
		if (window_attributes.map_state != IsViewable || window_attributes.c_class != InputOutput) {
// 			printf ("[%i shocker] WindowHelper_GetWindowInfo (%i): Window %x isn't viewable and inputoutput.\n", getpid (), pid, (int) window);
			goto recurse;
		}

		XTranslateCoordinates (display, window, window_attributes.root, -window_attributes.border_width, -window_attributes.border_width, &x, &y, &dummy);

		wix = (WindowInfoEx *) g_malloc0 (sizeof (WindowInfoEx));
		wix->window = window;
		wi = &wix->wi;
		// This is not entirely correct: we're not taking into account the title bar of the window
		wi->windowLeft = x - window_attributes.border_width;
		wi->windowTop = y - window_attributes.border_width;
		wi->windowWidth = window_attributes.width;
		wi->windowHeight = window_attributes.height;
		wi->windowRight = wi->windowLeft + wi->windowWidth;
		wi->windowBottom = wi->windowTop + wi->windowBottom;
		
		wi->clientLeft = x;
		wi->clientTop = y;
		wi->clientWidth = window_attributes.width - window_attributes.border_width * 2;
		wi->clientHeight = window_attributes.height - window_attributes.border_width * 2;
		wi->clientRight = wi->clientLeft + wi->clientWidth;
		wi->clientBottom = wi->clientTop + wi->clientHeight;
		
		// Get window name
		XGetWindowProperty (display, window, _NET_WM_NAME, 0, MAX_WINDOW_TITLE / 4, False, UTF8_STRING, &actual_type_return, &actual_format_return, &nitems_return, &bytes_after_return, &prop_return);
		if (nitems_return > 0 && prop_return != NULL) {
			memcpy (wi->title, prop_return, MIN (nitems_return, MAX_WINDOW_TITLE - 1));
		}
		
		*result = g_slist_prepend (*result, wix);
		return;
	}

recurse:
// 	printf ("[%i shocker] %*sFound window %p is not in pid %i\n", getpid (), level, " ", (void *) window, pid);
	
	// Recurse into children
	Window root_return;
	Window parent_return;
	Window *children;
	unsigned int n_children;
	if (0 != (XQueryTree (display, window, &root_return, &parent_return, &children, &n_children))) {
		for (unsigned int i = 0; i < n_children; i++) {
			find_window_r (pid, children [i], display, atom_pid, level + 1, result);
		}
	}
}

void
find_window (guint32 pid, GSList **result)
{
	// Finds all top-level windows in the specified pid
//	printf ("[%i shocker] Finding windows for pid: %i\n", getpid (), pid);
	*result = NULL;

	Display *display = XOpenDisplay (NULL);
	Window root = XDefaultRootWindow (display);
	Atom atom_pid = XInternAtom (display, "_NET_WM_PID", True);
	find_window_r (pid, root, display, atom_pid, 0, result);
	XCloseDisplay (display);

	*result = g_slist_reverse (*result);
//	printf ("[%i shocker] Finding windows for pid: %i Found %i windows.\n", getpid (), pid, g_slist_length (*result));
}

int WindowHelper_GetWindowInfo (guint32 pid, WindowInfo *wi)
{
	WindowInfoEx *wix;
	GSList *windows = NULL;
	GSList *next;

	LOG_HARNESS ("[%i shocker] WindowHelper_GetWindowInfo (%i)\n", getpid (), pid);

	find_window (pid, &windows);

	if (windows != NULL) {
		bool found = false;

		next = windows;
		while (next != NULL) {
			wix = (WindowInfoEx *) next->data;
			if (!strncmp (wi->title, "IWANTEX", sizeof ("IWANTEX"))) {
				*(WindowInfoEx *) wi = *wix;
			} else {
				*wi = wix->wi;
			}

			LOG_HARNESS ("[%i shocker] WindowHelper_GetWindowInfo (%i): Found window: %p '%s' [%i,%i %i:%i] Client [%i,%i %i:%i]\n",
				getpid (), pid, (void *) wix->window, wi->title, wi->windowLeft, wi->windowTop, wi->windowWidth, wi->windowHeight,
				wi->clientLeft, wi->clientTop, wi->clientWidth, wi->clientHeight);

			if (found) {
				LOG_HARNESS ("[%i shocker] WindowHelper_GetWindowInfo (%i): Found multiple top-level windows! Last one will prevail\n", getpid (), pid);
			}
			found = true;
			next = next->next;
		}
	} else {
		LOG_HARNESS ("[%i shocker] WindowHelper_GetWindowInfo (%i): Found no top-level windows for the specified process!\n", getpid (), pid);
	}

	g_slist_free (windows);

	return 0;
}

int WindowHelper_GetNotificationWindowInfo (guint32 pid, WindowInfo *wi)
{
	Shocker_FailTestFast ("WindowHelper_GetNotificationWindowInfo (%i): Not implemented");
	return 0;
}

int WindowHelper_Maximize (guint32 pid)
{
	GSList *windows = NULL;

	LOG_HARNESS ("[%i shocker] WindowHelper_Maximize (%i)\n", getpid (), pid);

	find_window (pid, &windows);

	if (windows != NULL) {
		Window window;
		XEvent xev;

		memset (&xev, 0, sizeof (XEvent));

		Display *display = XOpenDisplay (NULL);
		Atom _NET_WM_STATE = XInternAtom (display, "_NET_WM_STATE", True);
		Atom _NET_WM_STATE_MAXIMIZED_HORZ = XInternAtom (display, "_NET_WM_STATE_MAXIMIZED_HORZ", True);
		Atom _NET_WM_STATE_MAXIMIZED_VERT = XInternAtom (display, "_NET_WM_STATE_MAXIMIZED_VERT", True);
		
		GSList *next = windows;
		while (next != NULL) {
			WindowInfoEx *wix = (WindowInfoEx *) next->data;
			window = wix->window;

			LOG_HARNESS ("[%i shocker] WindowHelper_Maximize (%i): Sending maximize request to: %p\n", getpid (), pid, (void *) window);
			xev.xclient.display = display;
			xev.xclient.type = ClientMessage;
			xev.xclient.send_event = True;
			xev.xclient.window = window;
			xev.xclient.message_type = _NET_WM_STATE;
			xev.xclient.format = 32;
			xev.xclient.data.l [0] = 2;
			xev.xclient.data.l [1] = _NET_WM_STATE_MAXIMIZED_HORZ;
			xev.xclient.data.l [2] = _NET_WM_STATE_MAXIMIZED_VERT;
			xev.xclient.data.l [3] = 0;
			XSendEvent (display, XDefaultRootWindow (display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
			next = next->next;
		}

		XCloseDisplay (display);
	} else {
		LOG_HARNESS ("[%i shocker] WindowHelper_Maximize (%i): No toplevel windows found for the specified process.\n", getpid (), pid);
	}

	g_slist_free (windows);	

	return 0;
}

int WindowHelper_Minimize (guint32 pid)
{
	printf ("[%i shocker] WindowHelper_Minimize (%i) [NOT IMPLEMENTED]\n", getpid (), pid);
	printf ("[%i shocker] ASSUMING WINDOW IS ALREADY MINIMIZED\n", getpid ());
	return 0;
}

int WindowHelper_Restore (guint32 pid)
{
	Shocker_FailTestFast ("WindowHelper_Restore: Not implemented");
	return 0;
}

int WindowHelper_EnsureOOBWindowIsActive (guint32 active /* 32bit bool */)
{
	Shocker_FailTestFast ("WindowHelper_EnsureOOBWindowIsActive (): Not implemented");
	return 0;
}

int WindowHelper_SetWindowPosition (guint32 left, guint32 top)
{
	Shocker_FailTestFast ("WindowHelper_SetWindowPosition (): Not implemented");
	return 0;
}

void WindowHelper_GetPrimaryScreenSize (guint32 *width, guint32 *height)
{
	LOG_HARNESS ("[%i shocker] WindowHelper_GetPrimaryScreenSize ()\n", getpid ());
	*width = 0;
	*height = 0;
	Shocker_FailTestFast ("WindowHelper_GetPrimaryScreenSize: Not implemented");
}

