/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * harness.h: Interface with our managed test harness
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */


#ifndef __HARNESS_H__
#define __HARNESS_H__

#include <glib.h>

#define Visual _XVisual
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#undef Visual

class Harness {
public:
	/* Sends a message to the harness, the response from the harness is stored in 'buffer' (which must be freed using g_free) */
	static bool SendMessage (const char *msg, guint8 **buffer, guint32 *output_length);

	static int StartProcess (const char *exePath, const char *argList, guint32 *id, bool isShortcut);
	static int WaitForProcessExit (guint32 processId, int timeoutSec);
	static int FindProcesses (const char *processName, guint32 **processes, guint32 *processCount);

	static char *GetRuntimePropertyValue (const char *propertyName);
	static void SetRuntimePropertyValue (const char *propertyName, const char *value);
	static void GetProcessTree (int pid, int *pids, int size, int *count);
	static int GetChromePid ();
};

#define MAX_WINDOW_TITLE 512
enum WindowState {
	Normal,
	Minimized,
	Maximized,
};

struct WindowInfo {
	guint32 windowLeft;
	guint32 windowTop;
	guint32 windowRight;
	guint32 windowBottom;
	guint32 clientLeft;
	guint32 clientTop;
	guint32 clientRight;
	guint32 clientBottom;
	guint32 windowWidth;
	guint32 windowHeight;
	guint32 clientWidth;
	guint32 clientHeight;
	char title [MAX_WINDOW_TITLE];
	guint32 isTopMost; // 32bit bool
	guint32 isAcive; // 32bit bool
	guint32 windowState; // WindowState
};

struct WindowInfoEx {
	WindowInfo wi;
	Window window;
};

G_BEGIN_DECLS

int ProcessHelper_GetCurrentProcessID (guint32 *id);
int ProcessHelper_StartProcess (const char *exePath, const char *argList, guint32 *id, bool isShortcut);
int ProcessHelper_WaitForProcessExit (guint32 processId, guint32 timeoutSec);
int ProcessHelper_BringToFront (guint32 processId);
int ProcessHelper_FindProcesses (const char *processName, guint32 **processes, guint32 *processCount);
int ProcessHelper_KillProcess (guint32 processId);
int ProcessHelper_CloseTestWindows ();

int WindowHelper_GetWindowInfo (guint32 pid, WindowInfo *wi);
int WindowHelper_GetWindowInfoInternal (guint32 pid, WindowInfo *wi, bool is_window_info_ex); // this is not used by the tests
int WindowHelper_GetNotificationWindowInfo (guint32 pid, WindowInfo *wi);
int WindowHelper_Maximize (guint32 pid);
int WindowHelper_Minimize (guint32 pid);
int WindowHelper_Restore (guint32 pid);
int WindowHelper_EnsureOOBWindowIsActive (guint32 active /* 32bit bool */);
int WindowHelper_SetWindowPosition (guint32 left, guint32 top);
void WindowHelper_GetPrimaryScreenSize (guint32 *width, guint32 *height);
int WindowHelper_GetOOBConsentDialogIconPosition (/* TODO */);

int ClipboardHelper_ClearClipboard ();
int ClipboardHelper_WriteCustomFormatTextToClipboard (const gunichar2 *customFormat, gunichar2 *textToWrite, gint32 textLen, bool makeUTF8Encoded);
int ClipboardHelper_WriteImageToClipboard (const char *pathToImage);
int ClipboardHelper_ReadCustomFormatTextFromClipboard (const char *customFormat, bool readAsUTF8Encoded, gunichar2 **textRead, gint32 *textLen);

int TestHost_CleanDRM ();
int TestHost_SetRegKey (const char *keyPath, const char *keyName, gint32 Value);
int TestHost_GetMachineName (char **name);
void TestHost_GetJTRURenderDataCapturer (void **ppJtruWrapper);

void NetworkHelper_DoNetworkRequestNative ();

void FileSystemHelper_GetShortcutInfo ();

int PlatformServices_RunAppleScript ();
int PlatformServices_GetEnvironmentVariable ();

int HardwareHelper_StartWebCamWriter (const char *filename, int width, int height, short framerate, int color_format);
int HardwareHelper_StopWebCamWriter ();
int HardwareHelper_StartMicrophoneWriter (const char *filename);
int HardwareHelper_EnsureDevice (int device_type, guint8 *has_device);

G_END_DECLS

#endif /* __HARNESS_H__ */

