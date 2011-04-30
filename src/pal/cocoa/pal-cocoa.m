/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include "pal-cocoa.h"

#include "runtime.h"
#include "window-cocoa.h"
#include "pixbuf-cocoa.h"
#include "im-cocoa.h"
#include "debug.h"

#include <sys/time.h>

#include <glib.h>

#include <cairo.h>

#include "MLEvent.h"
#include "MLTimerContext.h"

#include <AppKit/AppKit.h>

using namespace Moonlight;

static Key
MapKeyCodeToKey (gunichar uc)
{
	if (uc >= 0x80)
		return KeyUNKNOWN;

	switch ((char)uc) {
	case '\t': return KeyTAB;
	case '\r':
	case '\n':
		return KeyENTER;
	case ' ':
		return KeySPACE;
	case 0x08:
		return KeyBACKSPACE;
	case 0x7f:
		return KeyBACKSPACE;
	// FIXME: lots more here

	default:
		return KeyUNKNOWN;
	}
}

// this class is kinda broken in that it doesn't/can't track the down/up state of modifier keys.
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
		if ([event.event type] == NSFlagsChanged) {
			unsigned int flags = [event.event modifierFlags];
			if (flags & NSShiftKeyMask)
				return KeySHIFT;
			else if (flags & NSAlphaShiftKeyMask)
				return KeyCAPSLOCK;
			else if (flags & NSAlternateKeyMask
				 /* do we want the command key to map to alt as well? i would guess not... */
				 /*|| flags & NSCommandKeyMask */)
				return KeyALT;
			else if (flags & NSControlKeyMask)
				return KeyALT;
			else
				return KeyUNKNOWN;
		}
		else {
			gunichar uc = GetUnicode();
			return MapKeyCodeToKey (uc);
		}
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
		return [event.event type] == NSKeyUp;
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

static gint32
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

/// our windowing system

MoonWindowingSystemCocoa::MoonWindowingSystemCocoa (bool out_of_browser)
{
	source_id = 1;
	timer = NULL;

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

class CocoaSource {
public:
	CocoaSource (int source_id, int priority, int interval, MoonSourceFunc source_func, gpointer data)
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
		const CocoaSource *source1 = (const CocoaSource*)p1;
		const CocoaSource *source2 = (const CocoaSource*)p2;

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

void
MoonWindowingSystemCocoa::RemoveSource (guint sourceId)
{
	sourceMutex.Lock ();

	for (GList *l = sources; l; l = l->next) {
		CocoaSource *s = (CocoaSource*)l->data;
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

guint
MoonWindowingSystemCocoa::AddTimeout (gint priority, gint ms, MoonSourceFunc timeout, gpointer data)
{
	sourceMutex.Lock ();

	int new_source_id = source_id;

	CocoaSource *new_source = new CocoaSource (new_source_id, priority, ms, timeout, data);

	sources = g_list_insert_sorted (sources, new_source, CocoaSource::Compare);

	source_id ++;

	sourceMutex.Unlock ();

	AddCocoaTimer ();

	return new_source_id;
}

void
MoonWindowingSystemCocoa::RemoveTimeout (guint timeoutId)
{
	RemoveSource (timeoutId);
}

guint
MoonWindowingSystemCocoa::AddIdle (MoonSourceFunc idle, gpointer data)
{
	sourceMutex.Lock ();

	int new_source_id = source_id;

	CocoaSource *new_source = new CocoaSource (new_source_id, MOON_PRIORITY_DEFAULT_IDLE, 100, idle, data);
	sources = g_list_insert_sorted (sources, new_source, CocoaSource::Compare);
	source_id ++;

	sourceMutex.Unlock ();
	return new_source_id;
}

void
MoonWindowingSystemCocoa::RemoveIdle (guint idleId)
{
	RemoveSource (idleId);
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
	case NSFlagsChanged:
	case NSKeyDown:
	case NSKeyUp:
		return new MoonKeyEventCocoa (evt);
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
MoonWindowingSystemCocoa::OnTick ()
{
	gint32 after = get_now_in_millis ();

	sourceMutex.Lock();

	[(NSTimer*)timer invalidate];
	timer = NULL;

	emitting_sources = true;

	GList *sources_to_dispatch = NULL;

	if (sources) {
		int max_priority = G_MAXINT;
		gint32 delta = before - after;
		GList *l = sources;

		while (l) {
			CocoaSource *s = (CocoaSource*)l->data;

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
		CocoaSource *s = (CocoaSource*)l->data;
		if (!s->pending_destroy) {
			bool pending_destroy = !s->InvokeSourceFunc ();
			if (!s->pending_destroy)
				s->pending_destroy = pending_destroy;
		}
	}

	g_list_free (sources_to_dispatch);

	sourceMutex.Lock ();
	for (GList *l = sources; l;) {
		CocoaSource *s = (CocoaSource*)l->data;
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

	emitting_sources = false;
	sourceMutex.Unlock();

	AddCocoaTimer ();
}

void
MoonWindowingSystemCocoa::AddCocoaTimer ()
{
	int timeout = -1;

	sourceMutex.Lock ();
	if (timer != NULL) {
		sourceMutex.Unlock ();
		return;
	}
	if (sources != NULL) {
		CocoaSource *s = (CocoaSource*)sources->data;
		timeout = s->time_remaining;
		if (timeout < 0)
			timeout = 0;
	}

	if (timeout >= 0) {
		MLTimerContext *timerContext = [[MLTimerContext alloc] initWithWindowingSystem: this];
		timer = [NSTimer scheduledTimerWithTimeInterval: (timeout/1000.0) target: timerContext selector: SEL("onTick:") userInfo: timerContext repeats: NO];

		[[NSRunLoop mainRunLoop] addTimer: (NSTimer*)timer forMode: NSRunLoopCommonModes];
		before = get_now_in_millis ();
	}

	sourceMutex.Unlock ();
}

void
MoonWindowingSystemCocoa::RunMainLoop (MoonWindow *window, bool quit_on_window_close)
{
	if (window) {
		window->Show ();
	}

	/* add the first Timer.  we'll add a new one each tick based on how much time we need to sleep */
	AddCocoaTimer ();

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

	return (gchar *) [folder UTF8String];

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
