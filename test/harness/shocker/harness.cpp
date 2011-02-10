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
#include "config.h"
 
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
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if PAL_V4L2_VIDEO_CAPTURE
#include <linux/videodev2.h>
#endif
#include <sys/ioctl.h>

#if 0
#if INCLUDE_PULSEAUDIO
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>
#endif
#endif

#include "debug.h"
#include "harness.h"
#include "shocker-plugin.h"
#include "clipboard.h"
#include "src/moonlightconfiguration.h"
#include "src/capture.h"

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

int
Harness::GetChromePid ()
{
	int chrome_pid = 0;
	gsize size;
	char *cmdline;
	char *arg;
	gboolean res;

	res = g_file_get_contents ("/proc/self/cmdline", &cmdline, &size, NULL);
	if (res) {
		/*/proc/self/cmdline should be an array of null-terminated strings
		 * but chrome doesn't do it that way */
		/*
		arg = cmdline;
		while (*arg != 0) {
			printf ("checking arg: %s\n", arg);
			if (strncmp (arg, "--channel=", 10)) {
				*strchr (arg, '.') = 0;
				chrome_pid = atoi (arg + 10);
				printf ("found pid: %i\n", chrome_pid);
				break;
			}
			arg += strlen (arg) + 1;
		}
		*/

		/* We need to find --channel=<pid>.<something else> */
		arg = strstr (cmdline, "--channel=");
		if (arg != NULL) {
			arg += 10;
			*strchr (arg, '.') = 0;
			chrome_pid = atoi (arg);
			LOG_SHUTDOWN ("[%i shocker] Harness::GetChromePid () found chrome pid: %i\n", getpid (), chrome_pid);
		}
	}
	g_free (cmdline);

	return chrome_pid;
}

void
Harness::GetProcessTree (int pid, int *pids, int size, int *count)
{
	char executable [1024];
	int cur_pid = pid;
	int ppid;
	char *filename;
	gchar *contents;
	gchar *ptr;
	gboolean res;
	bool is_chrome = true;

	*count = 0;

	do {
		LOG_SHUTDOWN ("[%i shocker] Harness::GetProcessTree (%i): adding pid %i to tree count: %i\n", getpid (), pid, cur_pid, *count);
		pids [*count] = cur_pid;
		*count += 1;

		// find the parent pid
		filename = g_strdup_printf ("/proc/%i/stat", cur_pid);
		res = g_file_get_contents (filename, &contents, NULL, NULL);
		g_free (filename);
		if (res == FALSE) {
			LOG_SHUTDOWN ("[%i shocker] Harness::GetProcessTree (%i): can't read /proc/%i/stat\n", getpid (), cur_pid, cur_pid);
			return;
		}
	
		ppid = -1;
		ptr = contents;
		while (*ptr && *ptr != ')') {
			ptr++;
		}
		if (*ptr) {
			ptr++;
			if (*ptr) {
				ptr++;
				while (*ptr && *ptr != ' ') {
					ptr++;
				}
				if (*ptr) {
					ptr++;
					if (*ptr) {
						ppid = atoi (ptr);
					}
				}
			}
		}
		g_free (contents);
		LOG_SHUTDOWN ("[%i shocker] Harness::GetProcessTree (%i): found ppid %i of pid %i\n", getpid (), pid, ppid, cur_pid);
	
		if (ppid == -1) {
			LOG_SHUTDOWN ("[%i shocker] Harness::GetProcessTree (%i): could not find ppid of pid %i\n", getpid (), pid, cur_pid);
			return;
		}
	
		/* Check if the specified ppid is a browser */
		memset (executable, 0, sizeof (executable));
		filename = g_strdup_printf ("/proc/%i/exe", ppid);
		readlink (filename, executable, sizeof (executable) - 1);
		g_free (filename);
		LOG_SHUTDOWN ("[%i shocker] Harness::GetProcessTree (%i): executable of %i: %s\n", getpid (), pid, ppid, executable);

		if (strstr (executable, "chrome")) {
			/* google-chrome */
			is_chrome = true;
		} else if (strstr (executable, "chromium")) {
			/* chromium */
			is_chrome = true;
		} else if (strstr (executable, "firefox")) {
			/* firefox */
		} else {
			LOG_SHUTDOWN ("[%i shocker] Harness::GetProcessTree (%i): parent of pid %i is not a browser.\n", getpid (), pid, cur_pid);
			if (is_chrome) {
				pids [*count] = GetChromePid ();
				*count += 1;
				LOG_SHUTDOWN ("[%i shocker] Harness::GetProcessTree (%i): added chrome pid %i.\n", getpid (), pid, pids [(*count) - 1]);
			}
			return;
		}

		cur_pid = ppid;
	} while (1);
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
	return WindowHelper_GetWindowInfoInternal (pid, wi, false);
}

int WindowHelper_GetWindowInfoInternal (guint32 pid, WindowInfo *wi, bool is_window_info_ex)
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
			if (is_window_info_ex) {
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
	printf ("[%i shocker] WindowHelper_SetWindowPosition (%d, %d) [NOT IMPLEMENTED]\n", getpid (), left, top);
	return 0;
}

void WindowHelper_GetPrimaryScreenSize (guint32 *width, guint32 *height)
{
	int x, y;
	unsigned int bwidth, depth;
	Display *display = XOpenDisplay (NULL);
	Window root = XDefaultRootWindow (display);

	XGetGeometry (display, root, &root, &x, &y, width, height, &bwidth, &depth);

	LOG_HARNESS ("[%i shocker] WindowHelper_GetPrimaryScreenSize () -> w: %d, h: %d\n", getpid (), *width, *height);

	XCloseDisplay (display);
}

int WindowHelper_GetOOBConsentDialogIconPosition (/* TODO */)
{
	Shocker_FailTestFast ("WindowHelper_GetOOBConsentDialogIconPosition (): Not implemented");
	return 0;
}

int ClipboardHelper_ClearClipboard ()
{
	return Clipboard::ClearClipboard ();
}

int ClipboardHelper_WriteCustomFormatTextToClipboard (const gunichar2 *customFormat, gunichar2 *textToWrite, gint32 textLen, bool makeUTF8Encoded)
{
	return Clipboard::WriteCustomFormatTextToClipboard (customFormat, textToWrite, textLen, makeUTF8Encoded);
}

int ClipboardHelper_ReadCustomFormatTextFromClipboard (const char *customFormat, bool readAsUTF8Encoded, gunichar2 **textRead, gint32 *textLen)
{
	return Clipboard::ReadCustomFormatTextFromClipboard (customFormat, readAsUTF8Encoded, textRead, textLen);
}

int ClipboardHelper_WriteImageToClipboard (const char *pathToImage)
{
	return Clipboard::WriteImageToClipboard (pathToImage);
}

int TestHost_CleanDRM ()
{
	Shocker_FailTestFast ("TestHost_CleanDRM (): not implemented");
	return 0;
}

int TestHost_SetRegKey (const char *keyPath, const char *keyName, gint32 Value)
{
	LOG_HARNESS ("TestHost_SetRegKey ('%s', '%s', %i)\n", keyPath, keyName, Value);

	if (strcmp (keyName, "Clipboard") == 0) {
		if (strncmp (keyPath, "Settings\\Permissions\\", 21) == 0)
			keyPath += 21;

		char key [strlen (keyPath) + 16];
		int key_len = strlen (keyPath);

		memcpy (key, keyPath, key_len);

			// strip off any trailing backslashes
		if (key [key_len - 1] == '\\') {
			key [key_len - 1] = 0;
			key_len--;
		}

		memcpy (key + key_len, "-clipboard", 10);

		Moonlight::MoonlightConfiguration configuration;
		if (Value == 17) { // Allow
			LOG_HARNESS ("TestHost_SetRegKey ('%s', '%s', %i): Allowing clipboard for key '%s'\n", keyPath, keyName, Value, key);
			configuration.SetBooleanValue ("Permissions", key, true);
		} else if (Value == 4) { // Deny
			LOG_HARNESS ("TestHost_SetRegKey ('%s', '%s', %i): Denying clipboard for key '%s'\n", keyPath, keyName, Value, key);
			configuration.SetBooleanValue ("Permissions", key, false);
		} else if (Value == 0) { // Remove
			LOG_HARNESS ("TestHost_SetRegKey ('%s', '%s', %i): Removing clipboard key '%s'\n", keyPath, keyName, Value, key);
			configuration.RemoveKey ("Permissions", key);
		} else {
			Shocker_FailTestFast ("TestHost_SetRegKey (): Unknown clipboard access value");
		}
		configuration.Save ();
	} else if (strcmp (keyName, "WebcamAndMicrophone") == 0) {
		if (strncmp (keyPath, "Settings\\Permissions\\", 21) == 0)
			keyPath += 21;

		char key [strlen (keyPath) + 16];
		int key_len = strlen (keyPath);

		memcpy (key, keyPath, key_len);

			// strip off any trailing backslashes
		if (key [key_len - 1] == '\\') {
			key [key_len - 1] = 0;
			key_len--;
		}

		memcpy (key + key_len, "-capture", 9);

		Moonlight::MoonlightConfiguration configuration;
		if (Value == 17) { // Allow
			LOG_HARNESS ("TestHost_SetRegKey ('%s', '%s', %i): Allowing capture for key '%s'\n", keyPath, keyName, Value, key);
			configuration.SetBooleanValue ("Permissions", key, true);
		} else if (Value == 4) { // Deny
			LOG_HARNESS ("TestHost_SetRegKey ('%s', '%s', %i): Denying capture for key '%s'\n", keyPath, keyName, Value, key);
			configuration.SetBooleanValue ("Permissions", key, false);
		} else if (Value == 0) { // Remove
			LOG_HARNESS ("TestHost_SetRegKey ('%s', '%s', %i): Removing capture key '%s'\n", keyPath, keyName, Value, key);
			configuration.RemoveKey ("Permissions", key);
		} else {
			Shocker_FailTestFast ("TestHost_SetRegKey (): Unknown clipboard access value");
		}
		configuration.Save ();
	} else {
		Shocker_FailTestFast ("TestHost_SetRegKey (): not implemented");
	}

	return 0;
}

int TestHost_GetMachineName (char **name)
{
	Shocker_FailTestFast ("TestHost_GetMachineName (): not implemented");
	return 0;
}

void TestHost_GetJTRURenderDataCapturer (void **ppJtruWrapper)
{
	Shocker_FailTestFast ("TestHost_GetJTRURenderDataCapturer (): not implemented");
}

void NetworkHelper_DoNetworkRequestNative (/* TODO */)
{
	Shocker_FailTestFast ("NetworkHelper_DoNetworkRequestNative (): not implemented");
}

void FileSystemHelper_GetShortcutInfo (/* TODO */)
{
	Shocker_FailTestFast ("FileSystemHelper_GetShortcutInfo (): not implemented");
}

int PlatformServices_RunAppleScript ()
{
	Shocker_FailTestFast ("PlatformServices_RunAppleScript (): test has likely entered a mac-specific branch, which is probably not correct");
	return 0;
}

int PlatformServices_GetEnvironmentVariable ()
{
	Shocker_FailTestFast ("PlatformServices_GetEnvironmentVariable (): not implemented");
	return 0;
}

enum HWHelper_PixelFormat
{
    I420 = 0, //IYUV
    RGB24 = 1,
    UYVY = 2,
    NV12 = 3,
    YV12 = 4,
    YVYU = 5,
    YUY2 = 6,
    YVU9 = 7,
    MJPG = 8,
};

static void
write_to_vivimoon (void *data, int size)
{
	GDir *dir;
	GError *err = NULL;
	char name [PATH_MAX];
	const char *entry_name;
	int fd;
	bool sent_command = false;
	v4l2_capability cap;

	// find the vivimoon driver
	dir = g_dir_open ("/dev", 0, &err);
	if (!dir) { 
		// should hopefully never happen.  what is this, !linux?
		Shocker_FailTestFast (g_strdup_printf ("write_to_vivimoon (): Could not open /dev: %s\n",err->message));
		g_error_free (err);
		return;
	}

	while ((entry_name = g_dir_read_name (dir))) {
		if (strncmp (entry_name, "video", 5))
			continue;

		if (g_snprintf (name, PATH_MAX, "/dev/%s", entry_name) > PATH_MAX)
			continue;

		// found a video device, make sure it works.
		fd = open (name, O_RDWR);
		if (fd == -1) {
			printf ("[libshocker] write_to_vivimoon (): Could not open %s: %s\n", name, strerror (errno));
			continue;
		}

		// find the friendly name
		memset (&cap, 0, sizeof (cap));
		if (-1 == ioctl (fd, VIDIOC_QUERYCAP, &cap)) {
			printf ("[libshocker] write_to_vivimoon (): Could not query device capabilities for %s: %s\n", name, strerror (errno));
			close (fd);
			continue;
		}

		LOG_HARNESS ("[libshocker] write_to_vivimoon (): Found driver: %s: %s\n",name, cap.driver);

		if (!strcmp ((const char *) cap.driver, "vivimoon")) {
			sent_command = true;
			write (fd, data, size); // send the command to the driver
			close (fd);
			break;
		}

		close (fd);
	}

	g_dir_close (dir);

	if (!sent_command) {
		Shocker_FailTestFast (g_strdup_printf ("write_to_vivimoon (): You must install the vivimoon virtual webcam driver\n"));
	}
}

int HardwareHelper_StartWebCamWriter (const char *filename, int width, int height, short framerate, int color_format)
{
#if PAL_V4L2_VIDEO_CAPTURE
	int input_idx = -1;
	unsigned char cmd [10];

	const char *inputs [] = { "Black.avi", "Red.avi", "Green.avi", "Yellow.avi", "Blue.avi", "Pink.avi", "Aqua.avi", "White.avi", "CaptureTestRGB.avi", "CaptureTest640_480.YUY.avi", "CaptureTest1280_720.UYVY.avi", NULL };
	int inputi [] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 8, 0 };
	const char *colors [] = { "I420", "RGB24", "UYVY", "NV12", "YV12", "YVYU", "YUY2", "YVU9", "MJPG", NULL };
	const char *fourcc [] = { "I420", "RGB3", "UYVY", "NV12", "YV12", "YVYU", "YUYV", "YVU9", "MJPG", NULL };

	for (int i = 0; inputs [i] != NULL; i++) {
		if (!strcmp (filename, inputs [i])) {
			input_idx = inputi [i];
			break;
		}
	}

	if (input_idx == -1) {
		Shocker_FailTestFast (g_strdup_printf ("HardwareHelper_StartWebCamWriter (%s, %i, %i, %i, %i): Unknown input file", filename, width, height, framerate, color_format));
	}

	cmd [0] = input_idx;
	cmd [1] = width >> 8;
	cmd [2] = width & 0xFF;
	cmd [3] = height >> 8;
	cmd [4] = height & 0xFF;
	cmd [5] = framerate;
	cmd [6] = fourcc [color_format] [3];
	cmd [7] = fourcc [color_format] [2];
	cmd [8] = fourcc [color_format] [1];
	cmd [9] = fourcc [color_format] [0];

	LOG_HARNESS ("[libshocker] HardwareHelper_StartWebCamWriter (%s = %i, %i, %i, %i, %i = %s = %s) writing 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", filename
		, input_idx, width, height, framerate, color_format, colors [color_format], fourcc [color_format], cmd [0], cmd [1], cmd [2], cmd [3], cmd [4], cmd [5], cmd [6], cmd [7], cmd [8], cmd [9]);

	write_to_vivimoon (cmd, sizeof (cmd));

#else
	Shocker_FailTestFast (g_strdup_printf ("HardwareHelper_StartWebCamWriter (%s, %i, %i, %i, %i): You must install the v4l2 devel package\n", filename, width, height, framerate, color_format));
#endif
	return 0;
}

int HardwareHelper_StopWebCamWriter ()
{
	unsigned char cmd [1];

	LOG_HARNESS ("[libshocker] HardwareHelper_StopWebCamWriter ().\n");

	cmd [0] = 0;
	//write_to_vivimoon (cmd, 1);

	return 0;
}

#if 0
// I'm leaving this audio writing code in for now, who knows if it'll become useful
// some time in the future. Right now I can't get it to play/record click-free, so
// the tests are obviously not even close to passing.
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
static char *mic_files [] = { NULL, NULL, NULL, NULL, NULL, NULL };
static bool running = false;

void *
microphone_writer_thread (void *)
{
#if INCLUDE_PULSEAUDIO
	/* The Sample format to use */
	const int BUFSIZE = 10240;
	pa_sample_spec ss;
	pa_buffer_attr buff;
	ss.format = PA_SAMPLE_S16LE;
	ss.rate = 48000;
	ss.channels = 1;

	pa_simple *s = NULL;
	int ret = 1;
	int error;

	/* Create a new playback stream */
	memset (&buff, 0, sizeof (buff));
	buff.maxlength = (uint32_t) -1;
	buff.tlength = (uint32_t) -1;
	buff.minreq = (uint32_t) -1;
	buff.prebuf = (uint32_t) -1;
	buff.tlength = (uint32_t) -1;
	if (!(s = pa_simple_new (NULL, "Moonlight Microphone Writer", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, &buff, &error))) {
		Shocker_FailTestFast (g_strdup_printf ("microphone_writer_thread (): could not create pa stream: %s\n", pa_strerror (error)));
		goto finish;
	}

	while (1) {
		char *file = NULL;
		pthread_mutex_lock (&file_mutex);
		for (int i = 0; i < 6; i++) {
			if (mic_files [i] != NULL) {
				file = mic_files [i];
				for (int j = i + 1; j < 6; j++)
					mic_files [j - 1] = mic_files [j];
				mic_files [5] = NULL;
				break;
			}
		}
		if (file == NULL)
			running = false;
		pthread_mutex_unlock (&file_mutex);
		if (file == NULL) {
			printf ("microphone_writer_thread (): nothing more to write\n");
			break;
		}
		printf ("microphone_writer_thread (): reading %s\n", file);
	
		guint16 header [19];
	
		FILE *fd = fopen (file, "r");
		if (fd == NULL) {
			Shocker_FailTestFast (g_strdup_printf ("microphone_writer_thread (): could not open file %s: %s\n", file, strerror (errno)));
		}

		if (fread (header, 1, 38, fd) != 38) {
			Shocker_FailTestFast (g_strdup_printf ("microphone_writer_thread (): could not read file header of %s: %s\n", file, strerror (errno)));
		}

		int header_size = 20 + header [8] + (header [9] << 16) + 8;
		printf ("microphone_writer_thread (): skipping header to %s %i, wFormatTag: %i channels: %i samplesPerSec: %ihz BytesPerSec: %i BlockAlign: %i BitsPerSample: %i\n",
			file, header_size, header [10], header [11], (header [13] << 16) + header [12], (header [15] << 16) + header [14], header [16], header [17]);

		if (fseek (fd, header_size, SEEK_SET) != 0) {
			Shocker_FailTestFast (g_strdup_printf ("microphone_writer_thread (): could not seek beyond file header of %s at %i: %s\n", file, header_size, strerror (errno)));
		}
	
		while (1) {
			uint8_t buf[BUFSIZE];
			ssize_t r;

			r = fread (buf, 1, BUFSIZE, fd);
			if (r == 0) {
				printf ("microphone_writer_thread () eof: %s\n", file);
				break;
			}
	
			if (r < 0) {
				Shocker_FailTestFast (g_strdup_printf ("microphone_writer_thread (): read failed in %s: %s\n", file, strerror (errno)));
				break;
			}
	
			/* ... and play it */
			printf ("microphone_writer_thread () %s about to write %i bytes\n", file, (int) r);
			int pa_r = pa_simple_write (s, buf, (size_t) r, &error);
			if (r < 0) {
				Shocker_FailTestFast (g_strdup_printf ("microphone_writer_thread (): write failed: %s\n", pa_strerror (error)));
				break;
			} else {
				printf ("microphone_writer_thread () %s pa result: %i, wrote %i bytes\n", file, pa_r, (int) r);
			}
		}

		fclose (fd);
		g_free (file);
	}

	/* Make sure that every single sample was played */
	if (pa_simple_drain(s, &error) < 0) {
		Shocker_FailTestFast (g_strdup_printf ("microphone_writer_thread (): drain failed: %s\n", pa_strerror (error)));
		goto finish;
	}

	ret = 0;

finish:
	if (s)
		pa_simple_free(s);

#else
	Shocker_FailTestFast (g_strdup_printf ("microphone_writer_thread (): you need to build with pulseaudio support\n", filename, err->message));
#endif

	printf ("microphone_writer_thread (): done\n");
	return NULL;
}

pthread_t mic_thread;

int HardwareHelper_StartMicrophoneWriter (const char *filename)
{
	printf ("HardwareHelper_StartMicrophoneWriter (%s)\n", filename);

	pthread_mutex_lock (&file_mutex);
	for (int i = 0; i < 6; i++) {
		if (mic_files [i] != NULL)
			continue;
		//mic_files [i] = g_strdup_printf ("/home/rolf/test/rec/%s", filename);
		mic_files [i] = g_strdup ("/home/rolf/test/rec/012.wav");
		break;
	}
	if (!running) {
		running = true;
		if (pthread_create (&mic_thread, NULL, microphone_writer_thread, NULL) != 0) {
			Shocker_FailTestFast (g_strdup_printf ("HardwareHelper_StartMicrophoneWriter (%s): could not create mic thread: %s", filename, strerror (errno)));
		} else {
			printf ("HardwareHelper_StartMicrophoneWriter (%s): started mic thread!\n", filename);
		}
		pthread_detach (mic_thread);
	}
	pthread_mutex_unlock (&file_mutex);

	return 0;
}
#endif

int HardwareHelper_EnsureDevice (int device_type, guint8 *has_device)
{
	Moonlight::CaptureDevice *dev;

	LOG_HARNESS ("HardwareHelper_EnsureDevice (%i = %s)", device_type, device_type == 0 ? "webcam" : (device_type == 1 ? "microphone" : (device_type == 2 ? "digitizer" : "?")));

	*has_device = 0;

	switch (device_type) {
	case 0: // webcam
		dev = Moonlight::CaptureDeviceConfiguration::GetDefaultVideoCaptureDevice ();
		if (dev != NULL) {
			*has_device = 1;
			dev->unref ();
		}
		break;
	case 1: // microphone
		dev = Moonlight::CaptureDeviceConfiguration::GetDefaultAudioCaptureDevice ();
		if (dev != NULL) {
			*has_device = 1;
			dev->unref ();
		}
		break;
		break;
	case 2: // digitizer
	default:
		return 0;
	}
	return 0;
}
