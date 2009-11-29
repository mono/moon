/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_PAL_H
#define MOON_PAL_H

#include <glib.h>

#include "enums.h"
#include "cairo.h"
#include "color.h"
#include "point.h"
#include "rect.h"
#include "error.h"

// I hate X11
#ifdef FocusIn
#undef FocusIn
#endif
#ifdef FocusOut
#undef FocusOut
#endif

// the default for MoonWindowingSystem::GetCursorBlinkTimeout
#define CURSOR_BLINK_TIMEOUT_DEFAULT  900

class Surface;
class UIElement;
class PluginInstance;

class MoonEvent;
class MoonWindow;
class MoonClipboard;

enum MoonModifier {
	MoonModifier_Shift    = 1 << 0,
	MoonModifier_Lock     = 1 << 1,
	MoonModifier_Control  = 1 << 2,

	MoonModifier_Mod1	    = 1 << 3,
	MoonModifier_Mod2	    = 1 << 4,
	MoonModifier_Mod3	    = 1 << 5,
	MoonModifier_Mod4	    = 1 << 6,
	MoonModifier_Mod5	    = 1 << 7,

	MoonModifier_Super    = 1 << 26,
	MoonModifier_Hyper    = 1 << 27,
	MoonModifier_Meta     = 1 << 28,
};

// useful abstractions for porting moonlight to other platforms.

// returns true if the timeout/idle should be removed
typedef bool (*MoonSourceFunc) (gpointer data);

typedef bool (*MoonCallback) (gpointer sender, gpointer data);

typedef void (*MoonClipboardGetTextCallback) (MoonClipboard *clipboard, const char *text, gpointer data);

class MoonEvent {
public:
	virtual MoonEvent *Clone () = 0;

	// returns a platform event so that other
	// platform interfaces which consume events can get at the actual data.
	virtual gpointer GetPlatformEvent() = 0;
};

class MoonKeyEvent : public MoonEvent {
public:
	virtual Key GetSilverlightKey () = 0; // returns the enum value.  this requires platform specific mapping

	virtual int GetPlatformKeycode () = 0; // FIXME: do we really need both of these?
	virtual int GetPlatformKeyval () = 0;

	virtual gunichar GetUnicode () = 0;

	virtual MoonModifier GetModifiers () = 0; // FIXME: should this be separate bool getters instead (like IsShiftDown, IsCtrlDown, IsAltDown)?

	virtual bool IsModifier () = 0;
};

class MoonMouseEvent : public MoonEvent {
public:
	virtual Point GetPosition () = 0;

	virtual double GetPressure () = 0;

	virtual void GetStylusInfo (TabletDeviceType *type, bool *is_inverted) = 0;

	virtual MoonModifier GetModifiers () = 0; // FIXME: should this be separate bool getters instead (like IsShiftDown, IsCtrlDown, IsAltDown)?
};

class MoonButtonEvent : public MoonMouseEvent {
public:
	virtual bool IsRelease () = 0;

	virtual int GetButton () = 0;

	// the number of clicks.  gdk provides them as event->type ==
	// GDK_3BUTTON_PRESS/GDK_2BUTTON_PRESS/GDK_BUTTON_PRESS
	virtual int GetNumberOfClicks () = 0; // FIXME: will this api work?
};

class MoonMotionEvent : public MoonMouseEvent {
};

class MoonCrossingEvent : public MoonMouseEvent {
public:
	virtual bool IsEnter () = 0;
};

class MoonScrollWheelEvent : public MoonMouseEvent {
public:
	virtual int GetWheelDelta () = 0;
};

class MoonFocusEvent : public MoonEvent {
public:
	virtual bool IsIn() = 0;
};


class MoonIMContext {
public:
	virtual void SetUsePreedit (bool flag) = 0;
	virtual void SetClientWindow (MoonWindow*  window) = 0;
	virtual void SetSurroundingText (const char *text, int offset, int length) = 0;
	virtual void Reset () = 0;

	virtual void FocusIn () = 0;
	virtual void FocusOut () = 0;

	virtual void SetCursorLocation (Rect r) = 0;

	virtual bool FilterKeyPress (MoonKeyEvent* event) = 0;

	virtual void SetRetrieveSurroundingCallback (MoonCallback cb, gpointer data) = 0;
	virtual void SetDeleteSurroundingCallback (MoonCallback cb, gpointer data) = 0;
	virtual void SetCommitCallback (MoonCallback cb, gpointer data) = 0;

	virtual gpointer GetPlatformIMContext () = 0;
};

enum MoonClipboardType {
	MoonClipboard_Clipboard,
	MoonClipboard_Primary
};

class MoonClipboard {
public:
	virtual void SetSelection (const char *text, int length) = 0;
	virtual void SetText (const char *text, int length) = 0;
	virtual void AsyncGetText (MoonClipboardGetTextCallback cb, gpointer data) = 0;
	virtual char* GetText () = 0;
};

class MoonPixbuf {
public:
	virtual gint GetWidth () = 0;
	virtual gint GetHeight () = 0;
	virtual gint GetRowStride () = 0;
	virtual gint GetNumChannels () = 0;
	virtual guchar *GetPixels () = 0;
};

class MoonPixbufLoader {
public:
	virtual void Write (const guchar *buffer, int buflen, MoonError **error = NULL) = 0;
	virtual void Close (MoonError **error = NULL) = 0;
	virtual MoonPixbuf *GetPixbuf () = 0;
};

// much match values from System.Windows.MessageBoxButtons
#define MESSAGE_BOX_BUTTON_OK		0
#define MESSAGE_BOX_BUTTON_OK_CANCEL	1

// much match values from System.Windows.MessageBoxResult
#define MESSAGE_BOX_RESULT_NONE		0
#define MESSAGE_BOX_RESULT_OK		1
#define MESSAGE_BOX_RESULT_CANCEL	2
#define MESSAGE_BOX_RESULT_YES		6
#define MESSAGE_BOX_RESULT_NO		7

/* @Version=2 */
class MoonWindowingSystem {
public:
	// creates a platform/windowing system specific surface
	virtual cairo_surface_t *CreateSurface () = 0;

	/* @GenerateCBinding,GeneratePInvoke */
	virtual MoonWindow *CreateWindow (bool fullscreen, int width = -1, int height = -1, MoonWindow *parentWindow = NULL, Surface* surface = NULL) = 0;
	virtual MoonWindow *CreateWindowless (int width, int height, PluginInstance *forPlugin) = 0;

	/* @GenerateCBinding,GeneratePInvoke */
	virtual int ShowMessageBox (const char *caption, const char *text, int buttons) = 0;

	virtual guint AddTimeout (gint priority, gint ms, MoonSourceFunc timeout, gpointer data) = 0;
	virtual void RemoveTimeout (guint timeoutId) = 0;

	virtual MoonIMContext* CreateIMContext () = 0;

	virtual MoonEvent* CreateEventFromPlatformEvent (gpointer platformEvent) = 0;

	virtual guint GetCursorBlinkTimeout (MoonWindow *window) = 0;

	virtual MoonPixbufLoader* CreatePixbufLoader (const char *imageType) = 0;
};

// XXX we need to think about multitouch events/tablets/accelerometers/gtk extension events, etc.

#endif /* MOON_PAL_H */
