/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include "pal-android.h"

#include "runtime.h"
#include "window-android.h"
#include "pixbuf-android.h"
#include "im-android.h"
#include "debug.h"

#ifdef USE_GALLIUM
#define __MOON_GALLIUM__
#include "context-gallium.h"
extern "C" {
#include "pipe/p_screen.h"
#include "util/u_debug.h"
#define template templat
#include "state_tracker/sw_winsys.h"
#include "sw/null/null_sw_winsys.h"
#include "softpipe/sp_public.h"
#ifdef USE_LLVM
#include "llvmpipe/lp_public.h"
#endif
};
#endif

#include <glib.h>

#include "android_native_app_glue.h"
#include "android/input.h"

#include <sys/stat.h>

#ifdef USE_GALLIUM
static struct pipe_screen *
swrast_screen_create (struct sw_winsys *ws)
{
	const char         *default_driver;
	const char         *driver;
	struct pipe_screen *screen = NULL;

#ifdef USE_LLVM
	default_driver = "llvmpipe";
#else
	default_driver = "softpipe";
#endif

	driver = debug_get_option ("GALLIUM_DRIVER", default_driver);

#ifdef USE_LLVM
	if (screen == NULL && strcmp (driver, "llvmpipe") == 0)
		screen = llvmpipe_create_screen (ws);
#endif

	if (screen == NULL)
		screen = softpipe_create_screen (ws);

	return screen;
}
#endif

using namespace Moonlight;

static Key
MapKeyCodeToKey (guint keycode)
{
	switch (keycode) {
	case AKEYCODE_TAB:		return KeyTAB;
	case AKEYCODE_ENTER:		return KeyENTER;
	case AKEYCODE_SHIFT_LEFT: case AKEYCODE_SHIFT_RIGHT:		return KeySHIFT;
	case AKEYCODE_ALT_LEFT: case AKEYCODE_ALT_RIGHT:			return KeyALT;
	case AKEYCODE_SPACE:		return KeySPACE;
	case AKEYCODE_PAGE_UP:		return KeyPAGEUP;
	case AKEYCODE_PAGE_DOWN:	return KeyPAGEDOWN;
	case AKEYCODE_DPAD_LEFT:		return KeyLEFT;
	case AKEYCODE_DPAD_UP:			return KeyUP;
	case AKEYCODE_DPAD_RIGHT:		return KeyRIGHT;
	case AKEYCODE_DPAD_DOWN:		return KeyDOWN;
	case AKEYCODE_DEL:		return KeyDELETE;
	case AKEYCODE_0:	return KeyDIGIT0;
	case AKEYCODE_1:	return KeyDIGIT1;
	case AKEYCODE_2:	return KeyDIGIT2;
	case AKEYCODE_3:	return KeyDIGIT3;
	case AKEYCODE_4:	return KeyDIGIT4;
	case AKEYCODE_5:	return KeyDIGIT5;
	case AKEYCODE_6:	return KeyDIGIT6;
	case AKEYCODE_7:	return KeyDIGIT7;
	case AKEYCODE_8:	return KeyDIGIT8;
	case AKEYCODE_9:	return KeyDIGIT9;
	case AKEYCODE_A:				return KeyA;
	case AKEYCODE_B:				return KeyB;
	case AKEYCODE_C:				return KeyC;
	case AKEYCODE_D:				return KeyD;
	case AKEYCODE_E:				return KeyE;
	case AKEYCODE_F:				return KeyF;
	case AKEYCODE_G:				return KeyG;
	case AKEYCODE_H:				return KeyH;
	case AKEYCODE_I:				return KeyI;
	case AKEYCODE_J:				return KeyJ;
	case AKEYCODE_K:				return KeyK;
	case AKEYCODE_L:				return KeyL;
	case AKEYCODE_M:				return KeyM;
	case AKEYCODE_N:				return KeyN;
	case AKEYCODE_O:				return KeyO;
	case AKEYCODE_P:				return KeyP;
	case AKEYCODE_Q:				return KeyQ;
	case AKEYCODE_R:				return KeyR;
	case AKEYCODE_S:				return KeyS;
	case AKEYCODE_T:				return KeyT;
	case AKEYCODE_U:				return KeyU;
	case AKEYCODE_V:				return KeyV;
	case AKEYCODE_W:				return KeyW;
	case AKEYCODE_X:				return KeyX;
	case AKEYCODE_Y:				return KeyY;
	case AKEYCODE_Z:				return KeyZ;
		
#if notyet
	case GDK_F1: case GDK_KP_F1:			return KeyF1;
	case GDK_F2: case GDK_KP_F2:			return KeyF2;
	case GDK_F3: case GDK_KP_F3:			return KeyF3;
	case GDK_F4: case GDK_KP_F4:			return KeyF4;
	case GDK_F5:					return KeyF5;
	case GDK_F6:					return KeyF6;
	case GDK_F7:					return KeyF7;
	case GDK_F8:					return KeyF8;
	case GDK_F9:					return KeyF9;
	case GDK_F10:					return KeyF10;
	case GDK_F11:					return KeyF11;
	case GDK_F12:					return KeyF12;
		
	case GDK_KP_0:					return KeyNUMPAD0;
	case GDK_KP_1:					return KeyNUMPAD1;
	case GDK_KP_2:					return KeyNUMPAD2;
	case GDK_KP_3:					return KeyNUMPAD3;
	case GDK_KP_4:					return KeyNUMPAD4;
	case GDK_KP_5:					return KeyNUMPAD5;
	case GDK_KP_6:					return KeyNUMPAD6;
	case GDK_KP_7:					return KeyNUMPAD7;
	case GDK_KP_8:					return KeyNUMPAD8;
	case GDK_KP_9:					return KeyNUMPAD9;
		
	case GDK_KP_Multiply:				return KeyMULTIPLY;
	case GDK_KP_Add:				return KeyADD;
	case GDK_KP_Subtract:				return KeySUBTRACT;
	case GDK_KP_Decimal:				return KeyDECIMAL;
	case GDK_KP_Divide:				return KeyDIVIDE;
#endif	
	default:
		return KeyUNKNOWN;
	}
}

static int
MapAndroidToVKey (int32_t keycode, int32_t scancode)
{
	if (keycode >= AKEYCODE_A && keycode <= AKEYCODE_Z)
		return keycode;

	switch (keycode) {
	case AKEYCODE_DEL:
		return 0x2e;

	case AKEYCODE_0:
		return 0x30;

	case AKEYCODE_1:
		return 0x31;

	case AKEYCODE_2:
	case AKEYCODE_AT:
		return 0x32;

	case AKEYCODE_3:
		return 0x33;

	case AKEYCODE_4:
		return 0x34;

	case AKEYCODE_5:
		return 0x35;

	case AKEYCODE_6:
		return 0x36;

	case AKEYCODE_7:
		return 0x37;

	case AKEYCODE_8:
		return 0x38;

	case AKEYCODE_9:
		return 0x39;

	case AKEYCODE_SEMICOLON:
		return 0xba;

	case AKEYCODE_EQUALS:
	case AKEYCODE_PLUS:
		return 0xbb;

	case AKEYCODE_COMMA:
		return 0xbc;

	case AKEYCODE_MINUS:
		return 0xbd;

	case AKEYCODE_PERIOD:
		return 0xbe;

	case AKEYCODE_SLASH:
		return 0xbf;

	case AKEYCODE_GRAVE:
		return 0xc0;

	case AKEYCODE_LEFT_BRACKET:
		return 0xdb;

	case AKEYCODE_BACKSLASH:
		return 0xdc;

	case AKEYCODE_RIGHT_BRACKET:
		return 0xdd;

	case AKEYCODE_APOSTROPHE:
		return 0xde;

	default:
		printf ("default case for keycode 0x%0x scancode %d\n", keycode, scancode);
		return scancode;
	}
}

class MoonKeyEventAndroid : public MoonKeyEvent {
public:
	MoonKeyEventAndroid (AInputEvent *event)
	{
		down = AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN;

		android_keycode = AKeyEvent_getKeyCode (event);
		android_scancode = AKeyEvent_getScanCode (event);
		android_metastate = AKeyEvent_getMetaState (event);

		key = MapKeyCodeToKey (android_keycode);
		keycode = (moonlight_flags & RUNTIME_INIT_EMULATE_KEYCODES) ? MapAndroidToVKey (android_keycode, android_scancode) : android_scancode;
	}

	MoonKeyEventAndroid (bool down, int32_t keycode, int32_t scancode, int32_t metastate)
	{
		this->down = down;
		android_keycode = keycode;
		android_scancode = scancode;
		android_metastate = metastate;

		key = MapKeyCodeToKey (android_keycode);
		this->keycode = (moonlight_flags & RUNTIME_INIT_EMULATE_KEYCODES) ? MapAndroidToVKey (android_keycode, android_scancode) : android_scancode;
	}

	virtual ~MoonKeyEventAndroid ()
	{
	}

	virtual MoonEvent* Clone ()
	{
		return new MoonKeyEventAndroid (down, android_keycode, android_scancode, android_metastate);
	}

	virtual gpointer GetPlatformEvent ()
	{
		// FIXME we can't cache the native event in this
		// wrapper class. this is used by gtk to deal with the
		// input method support.  is this necessary on
		// android?
		return NULL;
	}

	virtual Key GetSilverlightKey ()
	{
		return key;
	}

	virtual int GetPlatformKeycode ()
	{
		return android_scancode;
	}

	virtual int GetPlatformKeyval ()
	{
		return android_keycode;
	}

	virtual gunichar GetUnicode ()
	{
		// FIXME dunno what to do here..
		return (gunichar)-1;
	}

	virtual bool HasModifiers () { return true; }

	virtual MoonModifier GetModifiers ()
	{
		if (android_metastate == AMETA_NONE)
			return (MoonModifier)0;

		int mods = 0;

		if (android_metastate & AMETA_ALT_ON)
			mods |= MoonModifier_Meta; // why couldn't gtk just use *ALT*?  is this the right modifier? FIXME
		if (android_metastate & AMETA_SHIFT_ON)
			mods |= MoonModifier_Shift;
		if (android_metastate & AMETA_SYM_ON)
			mods |= MoonModifier_Super; // FIXME

		return (MoonModifier)mods;
	}
	
	virtual bool IsModifier ()
	{
		switch (android_keycode) {
		case AKEYCODE_ALT_LEFT:
		case AKEYCODE_ALT_RIGHT:
		case AKEYCODE_SHIFT_LEFT:
		case AKEYCODE_SHIFT_RIGHT:
		case AKEYCODE_SYM:
			return true;
		default:
			return false;
		}
	}

	bool IsRelease ()
	{
		return !down;
	}

	virtual MoonEventStatus DispatchToWindow (MoonWindow *window)
	{
		if (!window || !window->GetSurface())
			return MoonEventNotHandled;

		return IsRelease () ? window->GetSurface()->HandleUIKeyRelease (this) : window->GetSurface()->HandleUIKeyPress (this);
	}

private:
	bool down;
	Key key;
	int32_t keycode;
	int32_t android_keycode;
	int32_t android_scancode;
	int32_t android_metastate;
};

class MoonButtonEventAndroid : public MoonButtonEvent {
public:
	MoonButtonEventAndroid (AInputEvent *event)
	{
		this->action = AMotionEvent_getAction (event);

		this->is_release = (this->action & AMOTION_EVENT_ACTION_MASK) == AMOTION_EVENT_ACTION_UP;

		this->metastate = AMotionEvent_getMetaState (event);

		this->pressure = AMotionEvent_getPressure (event, 0);

		this->x = AMotionEvent_getX (event, 0);
		this->y = AMotionEvent_getY (event, 0);
	}

	MoonButtonEventAndroid (int32_t action, int32_t metastate, float pressure, float x, float y)
	{
		this->action = action;
		this->is_release = (this->action & AMOTION_EVENT_ACTION_MASK) == AMOTION_EVENT_ACTION_UP;

		this->metastate = metastate;
		this->pressure = pressure;
		this->x = x;
		this->y = y;
	}

	virtual ~MoonButtonEventAndroid ()
	{
	}

	virtual MoonEvent* Clone ()
	{
		return new MoonButtonEventAndroid (action, metastate, pressure, x, y);
	}

	virtual gpointer GetPlatformEvent ()
	{
		return NULL;
	}
	
	virtual Point GetPosition ()
	{
		return Point (x, y);
	}

	virtual double GetPressure ()
	{
		return pressure;
	}

	virtual void GetStylusInfo (TabletDeviceType *type, bool *is_inverted)
	{
		// FIXME
	}

	virtual bool HasModifiers () { return true; }

	virtual MoonModifier GetModifiers ()
	{
		if (metastate == AMETA_NONE)
			return (MoonModifier)0;

		int mods = 0;

		if (metastate & AMETA_ALT_ON)
			mods |= MoonModifier_Meta; // why couldn't gtk just use *ALT*?  is this the right modifier? FIXME
		if (metastate & AMETA_SHIFT_ON)
			mods |= MoonModifier_Shift;
		if (metastate & AMETA_SYM_ON)
			mods |= MoonModifier_Super; // FIXME

		return (MoonModifier)mods;
	}

	bool IsRelease ()
	{
		return is_release;
	}

	int GetButton ()
	{
		return 1;
	}

	// the number of clicks.  gdk provides them as event->type ==
	// GDK_3BUTTON_PRESS/GDK_2BUTTON_PRESS/GDK_BUTTON_PRESS
	virtual int GetNumberOfClicks ()
	{
		return 1;
	}

	virtual MoonEventStatus DispatchToWindow (MoonWindow *window)
	{
		if (!window || !window->GetSurface())
			return MoonEventNotHandled;

		g_debug ("MoonButtonEvent (action= %s, pressure= %g, x= %g, y= %g)",
			 (this->action & AMOTION_EVENT_ACTION_MASK) == AMOTION_EVENT_ACTION_UP ? "AMOTION_EVENT_ACTION_UP" : "AMOTION_EVENT_ACTION_DOWN",
			 pressure,
			 x, y);

		return IsRelease () ? window->GetSurface()->HandleUIButtonRelease (this) : window->GetSurface()->HandleUIButtonPress (this);
	}

private:
	int32_t action;
	int32_t metastate;
	bool is_release;
	float pressure;
	float x;
	float y;
};

class MoonMotionEventAndroid : public MoonMotionEvent {
public:
	MoonMotionEventAndroid (AInputEvent *event)
	{
		this->metastate = AMotionEvent_getMetaState (event);

		this->pressure = AMotionEvent_getPressure (event, 0);

		this->x = AMotionEvent_getX (event, 0);
		this->y = AMotionEvent_getY (event, 0);
	}

	MoonMotionEventAndroid (int32_t metastate, float pressure, float x, float y)
	{
		this->metastate = metastate;
		this->pressure = pressure;
		this->x = x;
		this->y = y;
	}

	virtual ~MoonMotionEventAndroid ()
	{
	}

	virtual MoonEvent* Clone ()
	{
		return new MoonMotionEventAndroid (metastate, pressure, x, y);
	}

	virtual gpointer GetPlatformEvent ()
	{
		return NULL;
	}

	virtual Point GetPosition ()
	{
		return Point (x, y);
	}

	virtual double GetPressure ()
	{
		return pressure;
	}

	virtual void GetStylusInfo (TabletDeviceType *type, bool *is_inverted)
	{
		// FIXME
	}

	virtual bool HasModifiers () { return true; }

	virtual MoonModifier GetModifiers ()
	{
		if (metastate == AMETA_NONE)
			return (MoonModifier)0;

		int mods = 0;

		if (metastate & AMETA_ALT_ON)
			mods |= MoonModifier_Meta; // why couldn't gtk just use *ALT*?  is this the right modifier? FIXME
		if (metastate & AMETA_SHIFT_ON)
			mods |= MoonModifier_Shift;
		if (metastate & AMETA_SYM_ON)
			mods |= MoonModifier_Super; // FIXME

		return (MoonModifier)mods;
	}

	virtual MoonEventStatus DispatchToWindow (MoonWindow *window)
	{
		if (!window || !window->GetSurface())
			return MoonEventNotHandled;

		g_debug ("MoonMotionEvent (pressure= %g, x= %g, y= %g)",
			 pressure,
			 x, y);

		return window->GetSurface()->HandleUIMotion (this);
	}

private:
	int32_t metastate;
	float pressure;
	float x;
	float y;
};

/// our windowing system

MoonWindowingSystemAndroid::MoonWindowingSystemAndroid (bool out_of_browser)
	: sourceMutex(false)
{
	if (out_of_browser) {
		g_thread_init (NULL);
	}

	LoadSystemColors ();

#ifdef USE_GALLIUM
	gscreen = swrast_screen_create (null_sw_create ());
#endif

	source_id = 0;
	sources = NULL;
}

MoonWindowingSystemAndroid::~MoonWindowingSystemAndroid ()
{

#ifdef USE_GALLIUM
	gscreen->destroy (gscreen);
#endif

	for (int i = 0; i < (int) NumSystemColors; i++)
		delete system_colors[i];
}

void
MoonWindowingSystemAndroid::ShowCodecsUnavailableMessage ()
{
	// FIXME
}

cairo_surface_t *
MoonWindowingSystemAndroid::CreateSurface ()
{
	// FIXME...
	g_assert_not_reached ();
}

void
MoonWindowingSystemAndroid::ExitApplication ()
{
	// FIXME
}

MoonWindow *
MoonWindowingSystemAndroid::CreateWindow (MoonWindowType windowType, int width, int height, MoonWindow *parentWindow, Surface *surface)
{
	MoonWindowAndroid *window = new MoonWindowAndroid (windowType, width, height, parentWindow, surface);
	RegisterWindow (window);
#ifdef USE_GALLIUM
	gtwindow->SetGalliumScreen (gscreen);
#endif
	return window;
}

MoonWindow *
MoonWindowingSystemAndroid::CreateWindowless (int width, int height, PluginInstance *forPlugin)
{
	g_warning ("no windowless support on android");
	return NULL;
}

MoonMessageBoxResult
MoonWindowingSystemAndroid::ShowMessageBox (MoonMessageBoxType message_type, const char *caption, const char *text, MoonMessageBoxButton button)
{
	// FIXME
	return MessageBoxResultNone;
}

char**
MoonWindowingSystemAndroid::ShowOpenFileDialog (const char *title, bool multsel, const char *filter, int idx)
{
	// FIXME
	return NULL;
}

char*
MoonWindowingSystemAndroid::ShowSaveFileDialog (const char *title, const char *filter, int idx)
{
	// FIXME
	return NULL;
}


bool
MoonWindowingSystemAndroid::ShowConsentDialog (const char *question, const char *detail, const char *website, bool *remember)
{
	// FIXME
	return false;
}

void
MoonWindowingSystemAndroid::RegisterWindow (MoonWindow *window)
{
}

void
MoonWindowingSystemAndroid::UnregisterWindow (MoonWindow *window)
{
}

void
MoonWindowingSystemAndroid::LoadSystemColors ()
{
	// FIXME
#if GTK_PAL_CODE_VERSION
	AndroidSettings *settings = gtk_settings_get_default ();
	GtkWidget *widget;
	GtkStyle *style;
	
	widget = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_ensure_style (widget);
	style = gtk_widget_get_style (widget);
	
	// AppWorkspace colors (FIXME: wtf is an Application Workspace?)
	system_colors[AppWorkspaceColor] = color_from_gdk (style->bg[GTK_STATE_ACTIVE]);
	
	// Border colors (the Window's border - FIXME: get this from the WM?)
	system_colors[ActiveBorderColor] = color_from_gdk (style->bg[GTK_STATE_ACTIVE]);
	system_colors[InactiveBorderColor] = color_from_gdk (style->bg[GTK_STATE_INSENSITIVE]);
	
	// Caption colors (the Window's title bar - FIXME: get this from the WM?)
	system_colors[ActiveCaptionColor] = color_from_gdk (style->bg[GTK_STATE_ACTIVE]);
	system_colors[ActiveCaptionTextColor] = color_from_gdk (style->fg[GTK_STATE_ACTIVE]);
	system_colors[InactiveCaptionColor] = color_from_gdk (style->bg[GTK_STATE_INSENSITIVE]);
	system_colors[InactiveCaptionTextColor] = color_from_gdk (style->fg[GTK_STATE_INSENSITIVE]);
	
	// Desktop colors (FIXME: get this from gconf?)
	system_colors[DesktopColor] = color_from_gdk (style->bg[GTK_STATE_ACTIVE]);
	
	// Window colors (GtkWindow)
	system_colors[WindowColor] = color_from_gdk (style->bg[GTK_STATE_NORMAL]);
	system_colors[WindowFrameColor] = color_from_gdk (style->bg[GTK_STATE_NORMAL]);
	system_colors[WindowTextColor] = color_from_gdk (style->fg[GTK_STATE_NORMAL]);
	
	gtk_widget_destroy (widget);
	
	// Control colors (FIXME: what widget should we use? Does it matter?)
	widget = gtk_button_new ();
	gtk_widget_ensure_style (widget);
	style = gtk_widget_get_style (widget);
	
	system_colors[ControlColor] = color_from_gdk (style->bg[GTK_STATE_ACTIVE]);
	system_colors[ControlTextColor] = color_from_gdk (style->fg[GTK_STATE_ACTIVE]);
	
	system_colors[ControlDarkColor] = color_from_gdk (style->dark[GTK_STATE_ACTIVE]);
	system_colors[ControlDarkDarkColor] = color_from_gdk (style->dark[GTK_STATE_ACTIVE]);
	system_colors[ControlDarkDarkColor]->Darken ();
	
	system_colors[ControlLightColor] = color_from_gdk (style->light[GTK_STATE_ACTIVE]);
	system_colors[ControlLightLightColor] = color_from_gdk (style->light[GTK_STATE_ACTIVE]);
	system_colors[ControlLightLightColor]->Lighten ();
	
	// Gray Text colors (disabled text)
	system_colors[GrayTextColor] = color_from_gdk (style->fg[GTK_STATE_INSENSITIVE]);
	
	gtk_widget_destroy (widget);
	
	// Highlight colors (selected items - FIXME: what widget should we use? Does it matter?)
	widget = gtk_entry_new ();
	gtk_widget_ensure_style (widget);
	style = gtk_widget_get_style (widget);
	system_colors[HighlightColor] = color_from_gdk (style->bg[GTK_STATE_SELECTED]);
	system_colors[HighlightTextColor] = color_from_gdk (style->fg[GTK_STATE_SELECTED]);
	gtk_widget_destroy (widget);
	
	// Info colors (GtkTooltip)
	if (!(style = gtk_rc_get_style_by_paths (settings, "gtk-tooltip", "GtkWindow", GTK_TYPE_WINDOW))) {
		widget = gtk_window_new (GTK_WINDOW_POPUP);
		gtk_widget_ensure_style (widget);
		style = gtk_widget_get_style (widget);
	} else {
		widget = NULL;
	}
	system_colors[InfoColor] = color_from_gdk (style->bg[GTK_STATE_NORMAL]);
	system_colors[InfoTextColor] = color_from_gdk (style->fg[GTK_STATE_NORMAL]);
	if (widget)
		gtk_widget_destroy (widget);
	
	// Menu colors (GtkMenu)
	widget = gtk_menu_new ();
	gtk_widget_ensure_style (widget);
	style = gtk_widget_get_style (widget);
	system_colors[MenuColor] = color_from_gdk (style->bg[GTK_STATE_NORMAL]);
	system_colors[MenuTextColor] = color_from_gdk (style->fg[GTK_STATE_NORMAL]);
	gtk_widget_destroy (widget);
	
	// ScrollBar colors (GtkScrollbar)
	widget = gtk_vscrollbar_new (NULL);
	gtk_widget_ensure_style (widget);
	style = gtk_widget_get_style (widget);
	system_colors[ScrollBarColor] = color_from_gdk (style->bg[GTK_STATE_NORMAL]);
	gtk_widget_destroy (widget);
#endif
}

Color *
MoonWindowingSystemAndroid::GetSystemColor (SystemColor id)
{
	if (id < 0 || id >= (int) NumSystemColors)
		return NULL;
	
	return system_colors[id];
}

class AndroidSource {
public:
	AndroidSource (int source_id, int priority, int interval, MoonSourceFunc source_func, gpointer data)
	{
		this->source_id = source_id;
		this->priority = priority;
		this->interval = interval;
		this->source_func = source_func;
		this->data = data;
		time_remaining = interval;
		pending_destroy = false;
	}

	bool InvokeSourceFunc ()
	{
		return source_func (data);
	}

	static gint Compare (gconstpointer p1, gconstpointer p2)
	{
		const AndroidSource *source1 = (const AndroidSource*)p1;
		const AndroidSource *source2 = (const AndroidSource*)p2;

		gint result = source1->time_remaining - source2->time_remaining;
		if (result != 0)
			return result;

		// reverse source1 and source2 here from above, since lower
		// priority values represent higher priorities
		return source2->priority - source1->priority;
	}

	// this one must be signed
	gint32 time_remaining;

	bool pending_destroy;
	guint source_id;
	int priority;
	gint32 interval;
	MoonSourceFunc source_func;
	gpointer data;
};


guint
MoonWindowingSystemAndroid::AddTimeout (gint priority, gint ms, MoonSourceFunc timeout, gpointer data)
{
	sourceMutex.Lock ();

	int new_source_id = source_id;

	AndroidSource *new_source = new AndroidSource (new_source_id, priority, ms, timeout, data);

	sources = g_list_insert_sorted (sources, new_source, AndroidSource::Compare);

	source_id ++;

	sourceMutex.Unlock ();

	return new_source_id;
}

void
MoonWindowingSystemAndroid::RemoveSource (guint sourceId)
{
	sourceMutex.Lock ();

	for (GList *l = sources; l; l = l->next) {
		AndroidSource *s = (AndroidSource*)l->data;
		if (s->source_id == sourceId) {
			if (emitting_sources) {
				s->pending_destroy = true;
			}
			else {
				sources = g_list_delete_link (sources, l);
				delete s;
			}
			break;
		}
	}

	sourceMutex.Unlock ();
}

void
MoonWindowingSystemAndroid::RemoveTimeout (guint timeoutId)
{
	RemoveSource (timeoutId);
}

guint
MoonWindowingSystemAndroid::AddIdle (MoonSourceFunc idle, gpointer data)
{
	sourceMutex.Lock ();

	int new_source_id = source_id;

	AndroidSource *new_source = new AndroidSource (new_source_id, MOON_PRIORITY_DEFAULT_IDLE, 0, idle, data);
	sources = g_list_insert_sorted (sources, new_source, AndroidSource::Compare);
	source_id ++;

	sourceMutex.Unlock ();
	return new_source_id;
}

void
MoonWindowingSystemAndroid::RemoveIdle (guint idleId)
{
	RemoveSource (idleId);
}

MoonIMContext*
MoonWindowingSystemAndroid::CreateIMContext ()
{
	return new MoonIMContextAndroid ();
}

MoonEvent*
MoonWindowingSystemAndroid::CreateEventFromPlatformEvent (gpointer platformEvent)
{
	AInputEvent *aevent = (AInputEvent*)platformEvent;

	switch (AInputEvent_getType(aevent)) {
	case AINPUT_EVENT_TYPE_KEY: {
		switch (AKeyEvent_getAction(aevent)) {
		case AKEY_EVENT_ACTION_DOWN:
		case AKEY_EVENT_ACTION_UP:
			return new MoonKeyEventAndroid (aevent);
		case AKEY_EVENT_ACTION_MULTIPLE:
			g_warning ("unsupported AKEY_EVENT_ACTION_MULTIPLE");
			return NULL;
		}
	}
	case AINPUT_EVENT_TYPE_MOTION: {
		switch (AMotionEvent_getAction(aevent) & AMOTION_EVENT_ACTION_MASK) {
		case AMOTION_EVENT_ACTION_DOWN:
		case AMOTION_EVENT_ACTION_UP:
			return new MoonButtonEventAndroid (aevent);
		case AMOTION_EVENT_ACTION_MOVE:
			return new MoonMotionEventAndroid (aevent);
		case AMOTION_EVENT_ACTION_POINTER_DOWN:
			g_warning ("unsupported AMOTION_EVENT_ACTION_POINTER_DOWN");
			return NULL;
		case AMOTION_EVENT_ACTION_POINTER_UP:
			g_warning ("unsupported AMOTION_EVENT_ACTION_POINTER_UP");
			return NULL;
		case AMOTION_EVENT_ACTION_CANCEL:
			g_warning ("unsupported AMOTION_EVENT_ACTION_CANCEL");
			return NULL;
		case AMOTION_EVENT_ACTION_OUTSIDE:
			g_warning ("unsupported AMOTION_EVENT_ACTION_OUTSIDE");
			return NULL;
			break;
		}
	}
	default:
		g_warning ("unknown android event");
		break;
	}

	// FIXME
	return NULL;
}

guint
MoonWindowingSystemAndroid::GetCursorBlinkTimeout (MoonWindow *moon_window)
{
	// FIXME
	return CURSOR_BLINK_TIMEOUT_DEFAULT;
}


MoonPixbufLoader*
MoonWindowingSystemAndroid::CreatePixbufLoader (const char *imageType)
{
	return new MoonPixbufLoaderAndroid (imageType);
}

void
MoonWindowingSystemAndroid::OnAppCommand (android_app* app, int32_t cmd)
{
	MoonWindow *window = (MoonWindow*)app->userData;

	g_warning("MoonWindowingSystemAndroid::OnAppCommand");

	switch (cmd) {
	case APP_CMD_INPUT_CHANGED:
		g_warning (" APP_CMD_INPUT_CHANGED");
		break;
	case APP_CMD_INIT_WINDOW:
		g_warning (" APP_CMD_INIT_WINDOW");
		ANativeWindow_setBuffersGeometry(app->window, 0, 0, WINDOW_FORMAT_RGBA_8888);
		window->Resize (ANativeWindow_getWidth (app->window), ANativeWindow_getHeight (app->window));
		break;
	case APP_CMD_TERM_WINDOW:
		g_warning (" APP_CMD_TERM_WINDOW");
		((MoonWindowAndroid*)window)->ClearPlatformContext ();
		break;
	case APP_CMD_WINDOW_RESIZED:
		g_warning (" APP_CMD_WINDOW_RESIZED");
		window->Resize (ANativeWindow_getWidth (app->window), ANativeWindow_getHeight (app->window));
		break;
	case APP_CMD_WINDOW_REDRAW_NEEDED:
		g_warning (" APP_CMD_WINDOW_REDRAW_NEEDED");
		break;
	case APP_CMD_CONTENT_RECT_CHANGED:
		g_warning (" APP_CMD_CONTENT_RECT_CHANGED");
		break;
	case APP_CMD_GAINED_FOCUS:
		g_warning (" APP_CMD_GAINED_FOCUS");
		break;
	case APP_CMD_LOST_FOCUS:
		g_warning (" APP_CMD_LOST_FOCUS");
		break;
	case APP_CMD_CONFIG_CHANGED:
		g_warning (" APP_CMD_CONFIG_CHANGED");
		window->Resize (ANativeWindow_getWidth (app->window), ANativeWindow_getHeight (app->window));	
		break;
	case APP_CMD_LOW_MEMORY:
		g_warning (" APP_CMD_LOW_MEMORY");
		break;
	case APP_CMD_START:
		g_warning (" APP_CMD_START");
		break;
	case APP_CMD_RESUME:
		g_warning (" APP_CMD_RESUME");
		break;
	case APP_CMD_SAVE_STATE:
		g_warning (" APP_CMD_SAVE_STATE");
		break;
	case APP_CMD_PAUSE:
		g_warning (" APP_CMD_PAUSE");
		break;
	case APP_CMD_STOP:
		g_warning (" APP_CMD_STOP");
		break;
	case APP_CMD_DESTROY:
		g_warning (" APP_CMD_DESTROY");
		break;
	}
}

int32_t
MoonWindowingSystemAndroid::OnInputEvent (android_app* app, AInputEvent* aevent)
{
	// handle the back button here
	if (AInputEvent_getType(aevent) == AINPUT_EVENT_TYPE_KEY
	    && AKeyEvent_getAction(aevent) == AKEY_EVENT_ACTION_UP
	    && AKeyEvent_getKeyCode(aevent) == 	AKEYCODE_BACK) {

		ANativeActivity_finish (app->activity);
		exit (1);
	}

	g_warning ("MoonWindowingSystemAndroid::OnInputEvent");
	MoonEvent *event = Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (aevent);
	if (!event)
		return 0;

	MoonWindow *window = (MoonWindow*)app->userData;

	g_warning ("  Dispatching window to event");
	event->DispatchToWindow (window);

	// return 1 unconditionally for now.
	return 1;
}

gint32
get_now_in_millis (void)
{
        struct timeval tv;
#ifdef CLOCK_MONOTONIC
	struct timespec tspec;
	if (clock_gettime (CLOCK_MONOTONIC, &tspec) == 0) {
		return tspec.tv_sec * 1000 + tspec.tv_nsec / 1000000;
	}
#endif

        if (gettimeofday (&tv, NULL) == 0) {
                return tv.tv_sec * 1000  + tv.tv_usec / 1000;
        }

	// XXX error
	return 0;
}

void
MoonWindowingSystemAndroid::RunMainLoop (MoonWindow *window, bool quit_on_window_close)
{
	android_app* state = (android_app*) system_data;

	state->userData = window;
	state->onAppCmd = MoonWindowingSystemAndroid::OnAppCommand;
	state->onInputEvent = MoonWindowingSystemAndroid::OnInputEvent;

	g_warning("showing window");

	window->Show ();
	// FIXME do something with quit_on_window_close

	g_warning("starting mainloop");

	// HACK HACK HACK
	window->GetSurface ()->HandleUIWindowAvailable ();
	window->GetSurface ()->HandleUIWindowAllocation (true);

#define DEBUG_MAIN_LOOP 1

	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;
		int timeout = -1;

		sourceMutex.Lock ();
		if (sources != NULL) {
			AndroidSource *s = (AndroidSource*)sources->data;
			timeout = s->time_remaining;
			if (timeout < 0)
				timeout = 0;
		}
		sourceMutex.Unlock ();

		gint32 before_poll = get_now_in_millis ();

#if DEBUG_MAINLOOP
		g_warning ("sleeping for at most %d milliseconds (until next timeout)", timeout);
#endif

		while ((ident = ALooper_pollAll (timeout,
						 NULL, &events,
						 (void**)&source))) {

			// Process this event if there was one
			if (source != NULL)
				source->process(state, source);

			// and exit early if we need to
			if (state->destroyRequested != 0) {
				g_warning ("state->destroyRequested set, leaving mainloop");
				return;
			}

			gint32 after_poll = get_now_in_millis ();

			sourceMutex.Lock();
			emitting_sources = true;

			GList *sources_to_dispatch = NULL;

			if (sources) {
				int max_priority = G_MAXINT;
				gint32 delta = before_poll - after_poll;
				GList *l = sources;

				while (l) {
					AndroidSource *s = (AndroidSource*)l->data;

					if (s->time_remaining + delta < 0) {
						if (max_priority == G_MAXINT) {
							// first time through here, so we do what glib does, and limit the sources we
							// dispatch on to those at or above this priority.
							max_priority = s->priority;
							sources_to_dispatch = g_list_prepend (sources_to_dispatch, s);
						}
						else {
							s->time_remaining += delta;
							if (s->priority >= max_priority && s->time_remaining < 0)
								sources_to_dispatch = g_list_prepend (sources_to_dispatch, s);
						}
					}
					else {
						s->time_remaining += delta;
					}

					l = l->next;
				}

				sources_to_dispatch = g_list_reverse (sources_to_dispatch);
			}

			sourceMutex.Unlock ();

			for (GList *l = sources_to_dispatch; l; l = l->next) {
				AndroidSource *s = (AndroidSource*)l->data;
				if (!s->pending_destroy)
					s->pending_destroy = !s->InvokeSourceFunc ();
			}

			g_list_free (sources_to_dispatch);

			sourceMutex.Lock ();
			for (GList *l = sources; l;) {
				AndroidSource *s = (AndroidSource*)l->data;
				if (s->pending_destroy) {
					GList *next = l->next;
					sources = g_list_delete_link (sources, l);
					delete s;
					l = next;
				}
				else {
					l = l->next;
				}
			}

			timeout = -1;
			if (sources != NULL) {
				AndroidSource *s = (AndroidSource*)sources->data;
				timeout = s->time_remaining;
				if (timeout < 0)
					timeout = 0;
			}

			emitting_sources = false;
			sourceMutex.Unlock();
		}
	}
}

guint32
MoonWindowingSystemAndroid::GetScreenHeight (MoonWindow *moon_window)
{
	// FIXME
	return 100;
}

guint32
MoonWindowingSystemAndroid::GetScreenWidth (MoonWindow *moon_window)
{
	// FIXME
	return 100;
}

gchar *
MoonWindowingSystemAndroid::GetTemporaryFolder ()
{
	android_app *app = (android_app *) system_data;

	const gchar* path = app->activity->internalDataPath;
	if (!path) // 2.3 has a bug, la de da
		path = "/sdcard";
	return (gchar*)path;
}

bool
MoonWindowingSystemAndroid::ConvertJPEGToBGRA (void *jpeg, guint32 jpeg_size, guint8 *buffer, guint32 buffer_stride, guint32 buffer_height)
{
	// FIXME
	return false;
#if GTK_PAL_CODE_VERSION
	bool result = false;
	GError *err = NULL;
	GdkPixbufLoader *loader;
	GdkPixbuf *pixbuf;
	guint32 gdk_stride;
	guint32 gdk_height;
	guint32 gdk_width;
	guint8 *gdk_pixels;
	guint8 *in;
	guint8 *out;

	if ((loader = gdk_pixbuf_loader_new_with_type ("jpeg", &err)) == NULL) {
		goto cleanup;
	}

	if (!gdk_pixbuf_loader_write (loader, (const guchar *) jpeg, jpeg_size, &err)) {
		goto cleanup;
	}

	if (!gdk_pixbuf_loader_close (loader, &err)) {
		goto cleanup;
	}

	if ((pixbuf = gdk_pixbuf_loader_get_pixbuf (loader)) == NULL) {
		fprintf (stderr, "Moonlight: Could not convert JPEG to BGRA: pixbufloader didn't create a pixbuf.\n");
		goto cleanup;
	}

	gdk_pixels = gdk_pixbuf_get_pixels (pixbuf);
	gdk_stride = gdk_pixbuf_get_rowstride (pixbuf);
	gdk_height = gdk_pixbuf_get_height (pixbuf);
	gdk_width = gdk_pixbuf_get_width (pixbuf);

	for (guint32 y = 0; y < MIN (gdk_height, buffer_height); y++) {
		out = buffer + buffer_stride * y;
		in = gdk_pixels + gdk_stride * y;
		for (guint32 x = 0; x < MIN (gdk_width, buffer_stride); x++) {
			out [0] = in [2];
			out [1] = in [1];
			out [2] = in [0];
			out [3] = 0xFF;;
			out += 4;
			in += 3;
		}
	}

	result = true;

cleanup:
	if (err) {
		fprintf (stderr, "Moonlight: could not convert jpeg to bgra: %s\n", err->message);
		g_error_free (err);
	}

	if (loader)
		g_object_unref (loader);

	return result;
#endif
}
