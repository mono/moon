/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include "pal-cocoa.h"

#include "runtime.h"
#include "window-cocoa.h"
#include "pixbuf-cocoa.h"
#include "im-cocoa.h"
#include "debug.h"

#include <glib.h>
#include <glib/gstdio.h>

#include <cairo.h>

#include "MLEvent.h"
#include "MLTimer.h"

#include <AppKit/AppKit.h>

using namespace Moonlight;

class MoonKeyEventCocoa : public MoonKeyEvent {
public:
	MoonKeyEventCocoa (MLEvent *event)
	{
		this->event = [event retain];
	}

	virtual ~MoonKeyEventCocoa ()
	{
		[this->event release];
	}

	virtual MoonEvent* Clone ()
	{
		return new MoonKeyEventCocoa ((MLEvent*) event);
	}

	virtual gpointer GetPlatformEvent ()
	{
		return event;
	}

	virtual Key GetSilverlightKey ()
	{
		g_assert_not_reached ();
	}

	virtual int GetPlatformKeycode ()
	{
		return [event.event keyCode];
	}

	virtual int GetPlatformKeyval ()
	{
		return [event.event keyCode];
	}

	virtual gunichar GetUnicode ()
	{
		return [[event.event characters] characterAtIndex: 0];
	}

	virtual bool HasModifiers () { return true; }

	virtual MoonModifier GetModifiers ()
	{
		// FIXME map modifiers
		return (MoonModifier) [event.event modifierFlags];
	}


	virtual bool IsModifier ()
	{
		// FIXME 
		return NO;
	}

private:
	MLEvent *event;
};

class MoonCrossingEventEnteredCocoa : public MoonCrossingEvent {
public:
	MoonCrossingEventEnteredCocoa (MLEvent *event)
	{
		this->event = [event retain];
	}

	virtual ~MoonCrossingEventEnteredCocoa ()
	{
		[event release];
	}

	virtual MoonEvent* Clone ()
	{
		return new MoonCrossingEventEnteredCocoa (event);
	}

	virtual gpointer GetPlatformEvent ()
	{
		return event;
	}

	virtual Moonlight::Point GetPosition ()
	{
		NSPoint loc = [event.view convertPoint: [event.event locationInWindow] fromView: nil];
		loc.y = event.view.frame.size.height - loc.y;
		return Moonlight::Point (loc.x, loc.y);
	}

	virtual double GetPressure ()
	{
		return 0.0;
	}

	virtual void GetStylusInfo (TabletDeviceType *type, bool *is_inverted)
	{
	}

	// FIXME: this needs to return true...
	virtual bool HasModifiers () { return false; }

	virtual MoonModifier GetModifiers ()
	{
		g_assert_not_reached ();
	}

	virtual bool IsEnter ()
	{
		return YES;
	}

private:
	MLEvent *event;
};

class MoonCrossingEventExitedCocoa : public MoonCrossingEvent {
public:
	MoonCrossingEventExitedCocoa (MLEvent *event)
	{
		this->event = [event retain];
	}

	virtual ~MoonCrossingEventExitedCocoa ()
	{
		[event release];
	}

	virtual MoonEvent* Clone ()
	{
		return new MoonCrossingEventExitedCocoa (event);
	}

	virtual gpointer GetPlatformEvent ()
	{
		return event;
	}

	virtual Moonlight::Point GetPosition ()
	{
		NSPoint loc = [event.view convertPoint: [event.event locationInWindow] fromView: nil];
		loc.y = event.view.frame.size.height - loc.y;
		return Moonlight::Point (loc.x, loc.y);
	}

	virtual double GetPressure ()
	{
		return 0.0;
	}

	virtual void GetStylusInfo (TabletDeviceType *type, bool *is_inverted)
	{
	}

	// FIXME: this needs to return true...
	virtual bool HasModifiers () { return false; }

	virtual MoonModifier GetModifiers ()
	{
		g_assert_not_reached ();
	}

	virtual bool IsEnter ()
	{
		return NO;
	}

private:
	MLEvent *event;
};

class MoonButtonEventCocoa : public MoonButtonEvent {
public:
	MoonButtonEventCocoa (MLEvent *event)
	{
		this->event = [event retain];
	}

	virtual ~MoonButtonEventCocoa ()
	{
		[event release];
	}

	virtual MoonEvent* Clone ()
	{
		return new MoonButtonEventCocoa (event);
	}

	virtual gpointer GetPlatformEvent ()
	{
		return event;
	}

	virtual Moonlight::Point GetPosition ()
	{
		NSPoint loc = [event.view convertPoint: [event.event locationInWindow] fromView: nil];
		loc.y = event.view.frame.size.height - loc.y;
		return Moonlight::Point (loc.x, loc.y);
	}

	virtual double GetPressure ()
	{
		return [event.event pressure];
	}

	virtual void GetStylusInfo (TabletDeviceType *type, bool *is_inverted)
	{
		g_assert_not_reached ();
	}

	// FIXME: this needs to return true...
	virtual bool HasModifiers () { return false; }

	virtual MoonModifier GetModifiers ()
	{
		g_assert_not_reached ();
	}

	bool IsRelease ()
	{
		if ([event.event type] == NSLeftMouseUp || [event.event type] == NSRightMouseUp || [event.event type] == NSOtherMouseUp)
			return YES;

		return NO;
	}

	int GetButton ()
	{
		return [event.event buttonNumber] + 1;
	}

	virtual int GetNumberOfClicks ()
	{
		return [event.event clickCount];
	}

private:
	MLEvent *event;
};

class MoonMotionEventCocoa : public MoonMotionEvent {
public:
	MoonMotionEventCocoa (MLEvent *event)
	{
		this->event = [event retain];
	}

	virtual ~MoonMotionEventCocoa ()
	{
		[event release];
	}

	virtual MoonEvent* Clone ()
	{
		return new MoonMotionEventCocoa (event);
	}

	virtual gpointer GetPlatformEvent ()
	{
		return event;
	}

	virtual Moonlight::Point GetPosition ()
	{
		NSPoint loc = [event.view convertPoint: [event.event locationInWindow] fromView: nil];
		loc.y = event.view.frame.size.height - loc.y;
		return Moonlight::Point (loc.x, loc.y);
	}

	virtual double GetPressure ()
	{
		return [event.event pressure];
	}

	virtual void GetStylusInfo (TabletDeviceType *type, bool *is_inverted)
	{
		g_assert_not_reached ();
	}

	// FIXME: this needs to return true...
	virtual bool HasModifiers () { return false; }

	virtual MoonModifier GetModifiers ()
	{
		g_assert_not_reached ();
	}

private:
	MLEvent *event;
};

/// our windowing system

MoonWindowingSystemCocoa::MoonWindowingSystemCocoa (bool out_of_browser)
{
	ProcessSerialNumber psn = { 0, kCurrentProcess };
	pool = [[NSAutoreleasePool alloc] init];

	TransformProcessType (&psn, kProcessTransformToForegroundApplication);

	[[NSApplication sharedApplication] finishLaunching];
}

MoonWindowingSystemCocoa::~MoonWindowingSystemCocoa ()
{
	[((NSAutoreleasePool *)pool) release];
}


cairo_surface_t *
MoonWindowingSystemCocoa::CreateSurface ()
{
	// FIXME...
	g_assert_not_reached ();
}

MoonWindow *
MoonWindowingSystemCocoa::CreateWindow (MoonWindowType windowType, int width, int height, MoonWindow *parentWindow, Surface *surface)
{
	MoonWindowCocoa *cocoawindow = new MoonWindowCocoa (windowType, width, height, parentWindow, surface);
	return cocoawindow;
}

MoonWindow *
MoonWindowingSystemCocoa::CreateWindowless (int width, int height, PluginInstance *forPlugin)
{
	MoonWindowCocoa *cocoawindow = (MoonWindowCocoa*)MoonWindowingSystem::CreateWindowless (width, height, forPlugin);
	return cocoawindow;
}

MoonMessageBoxResult
MoonWindowingSystemCocoa::ShowMessageBox (MoonMessageBoxType message_type, const char *caption, const char *text, MoonMessageBoxButton button)
{
	g_assert_not_reached ();
}

char**
MoonWindowingSystemCocoa::ShowOpenFileDialog (const char *title, bool multsel, const char *filter, int idx)
{
	g_assert_not_reached ();
}

char*
MoonWindowingSystemCocoa::ShowSaveFileDialog (const char *title, const char *filter, int idx)
{
	g_assert_not_reached ();
}


bool
MoonWindowingSystemCocoa::ShowConsentDialog (const char *question, const char *detail, const char *website, bool *remember)
{
	g_assert_not_reached ();
}

Color *
MoonWindowingSystemCocoa::GetSystemColor (SystemColor id)
{
	g_assert_not_reached ();
}

guint
MoonWindowingSystemCocoa::AddTimeout (gint priority, gint ms, MoonSourceFunc timeout, gpointer data)
{
	MLTimer *mtimer = [[MLTimer alloc] init];
	NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval: (ms/1000.0) target: mtimer selector: SEL("onTick:") userInfo: mtimer repeats: YES];

	mtimer.timeout = timeout;
	mtimer.userInfo = data;

	[[NSRunLoop mainRunLoop] addTimer: timer forMode: NSRunLoopCommonModes];
	// FIXME: 64-bit evil
	return (guint) timer;
}

void
MoonWindowingSystemCocoa::RemoveTimeout (guint timeoutId)
{
	// FIXME: 64-bit evil
	NSTimer *timer = (NSTimer *) timeoutId;

	[[timer userInfo] autorelease];
	[timer invalidate];
	[timer autorelease];
}

guint
MoonWindowingSystemCocoa::AddIdle (MoonSourceFunc idle, gpointer data)
{
	/* This is horrible, what we probably want is 1 timer we run at a low resolution that will pump some idle events we track in a seperate queue */
	MLTimer *mtimer = [[MLTimer alloc] init];
	NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval: (500/1000.0) target: mtimer selector: SEL("onTick:") userInfo: mtimer repeats: YES];

	mtimer.timeout = idle;
	mtimer.userInfo = data;

	[[NSRunLoop mainRunLoop] addTimer: timer forMode: NSRunLoopCommonModes];

	// FIXME: 64-bit evil
	return (guint) timer;
}

void
MoonWindowingSystemCocoa::RemoveIdle (guint idle_id)
{
	// FIXME: 64-bit evil
	NSTimer *timer = (NSTimer *) idle_id;

	[[timer userInfo] autorelease];
	[timer invalidate];
	[timer autorelease];
}

MoonIMContext*
MoonWindowingSystemCocoa::CreateIMContext ()
{
	return new MoonIMContextCocoa ();
}

MoonEvent*
MoonWindowingSystemCocoa::CreateEventFromPlatformEvent (gpointer platformEvent)
{
	MLEvent *evt = (MLEvent *) platformEvent;

	switch ([evt.event type]) {
		case NSMouseEntered:
			return new MoonCrossingEventEnteredCocoa (evt);
		case NSMouseExited:
			return new MoonCrossingEventExitedCocoa (evt);
			break;
		case NSMouseMoved:
			return new MoonMotionEventCocoa (evt);
		case NSLeftMouseUp:
		case NSRightMouseUp:
		case NSOtherMouseUp:
		case NSLeftMouseDown:
		case NSRightMouseDown:
		case NSOtherMouseDown:
			return new MoonButtonEventCocoa (evt);
		default:
			g_assert_not_reached ();
	}

	return NULL;
}

guint
MoonWindowingSystemCocoa::GetCursorBlinkTimeout (MoonWindow *moon_window)
{
	return 500;
}


MoonPixbufLoader*
MoonWindowingSystemCocoa::CreatePixbufLoader (const char *imageType)
{
	return new MoonPixbufLoaderCocoa (imageType);
}

void
MoonWindowingSystemCocoa::RunMainLoop (MoonWindow *window, bool quit_on_window_close)
{
	if (window) {
		window->Show ();
	}

	[NSApp run];
}


MoonInstallerServiceCocoa::MoonInstallerServiceCocoa ()
{
	base_install_dir = g_build_filename (g_get_home_dir (), ".local", "share", "moonlight", "applications", NULL);
}

MoonInstallerServiceCocoa::~MoonInstallerServiceCocoa ()
{
}

const char *
MoonInstallerServiceCocoa::GetBaseInstallDir ()
{
	return base_install_dir;
}

bool
MoonInstallerServiceCocoa::Install (Deployment *deployment, bool unattended)
{
	g_assert_not_reached ();
}

bool
MoonInstallerServiceCocoa::Uninstall (Deployment *deployment)
{
	g_assert_not_reached ();
}
