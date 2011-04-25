/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include "pal-cocoa.h"

#include "runtime.h"
#include "window-cocoa.h"
#include "pixbuf-cocoa.h"
#include "im-cocoa.h"
#include "debug.h"

#include <glib.h>

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

	bool IsRelease ()
	{
		g_assert_not_reached ();
	}

	virtual MoonEventStatus DispatchToWindow (MoonWindow *window)
	{
		if (!window || !window->GetSurface())
			return MoonEventNotHandled;

		return IsRelease () ? window->GetSurface()->HandleUIKeyRelease (this) : window->GetSurface()->HandleUIKeyPress (this);
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

	virtual MoonEventStatus DispatchToWindow (MoonWindow *window)
	{
		if (!window || !window->GetSurface())
			return MoonEventNotHandled;

		return window->GetSurface()->HandleUICrossing (this);
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

	virtual MoonEventStatus DispatchToWindow (MoonWindow *window)
	{
		if (!window || !window->GetSurface())
			return MoonEventNotHandled;

		return window->GetSurface()->HandleUICrossing (this);
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

	virtual MoonEventStatus DispatchToWindow (MoonWindow *window)
	{
		if (!window || !window->GetSurface())
			return MoonEventNotHandled;

		return IsRelease() ? window->GetSurface()->HandleUIButtonRelease (this) : window->GetSurface()->HandleUIButtonPress (this);
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

	virtual MoonEventStatus DispatchToWindow (MoonWindow *window)
	{
		if (!window || !window->GetSurface())
			return MoonEventNotHandled;

		return window->GetSurface()->HandleUIMotion (this);
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

void
MoonWindowingSystemCocoa::ShowCodecsUnavailableMessage ()
{
	//g_assert_not_reached ();
}

cairo_surface_t *
MoonWindowingSystemCocoa::CreateSurface ()
{
	g_assert_not_reached ();
}

void
MoonWindowingSystemCocoa::ExitApplication ()
{
	// FIXME
	exit (1);
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
	NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval: (ms/2000.0) target: mtimer selector: SEL("onTick:") userInfo: mtimer repeats: YES];

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
}

guint
MoonWindowingSystemCocoa::AddIdle (MoonSourceFunc idle, gpointer data)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	/* This is horrible, what we probably want is 1 timer we run at a low resolution that will pump some idle events we track in a seperate queue */
	MLTimer *mtimer = [[MLTimer alloc] init];
	NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval: (500/2000.0) target: mtimer selector: SEL("onTick:") userInfo: mtimer repeats: YES];

	mtimer.timeout = idle;
	mtimer.userInfo = data;

	[[NSRunLoop mainRunLoop] addTimer: timer forMode: NSRunLoopCommonModes];

	[pool release];
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
		case NSLeftMouseDragged:
		case NSRightMouseDragged:
		case NSOtherMouseDragged:
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

gchar *
MoonWindowingSystemCocoa::GetTemporaryFolder ()
{
	return (gchar *) g_get_tmp_dir ();
}

gchar *
MoonWindowingSystemCocoa::GetUserConfigFolder ()
{
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSString *folder = @"~/Library/Application Support/Moonlight/";

	folder = [folder stringByExpandingTildeInPath];

	if ([fileManager fileExistsAtPath: folder] == NO) {
		[fileManager createDirectoryAtPath: folder attributes: nil];
	}

	return [folder UTF8String];

	// todo: make sure we have a NSAutoreleasePool on the thread we're calling this from too, otherwise wrap this in:

	// NSAutoreleasePool *pool = [NSAutoreleasePool new];

	// ...

	// [pool drain];
}

guint32
MoonWindowingSystemCocoa::GetScreenHeight (MoonWindow *moon_window)
{
	MLView *view = (MLView *) moon_window->GetPlatformWindow ();

	return view.frame.size.height;
}

guint32
MoonWindowingSystemCocoa::GetScreenWidth (MoonWindow *moon_window)
{
	MLView *view = (MLView *) moon_window->GetPlatformWindow ();

	return view.frame.size.width;
}

bool
MoonWindowingSystemCocoa::ConvertJPEGToBGRA (void *jpeg, guint32 jpeg_size, guint8 *buffer, guint32 buffer_stride, guint32 buffer_height)
{
	g_assert_not_reached ();
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
