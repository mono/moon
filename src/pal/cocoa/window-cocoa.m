/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * window-cocoa.cpp: MoonWindow implementation using cocoa widgets.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include <glib.h>

#include "window-cocoa.h"
#include "clipboard-cocoa.h"
#include "config-dialog-cocoa.h"
#include "pixbuf-cocoa.h"
#include "deployment.h"
#include "timemanager.h"
#include "enums.h"
#include "context-cairo.h"

#include "MLEvent.h"
#include "MLView.h"
#include <AppKit/AppKit.h>

#include <cairo.h>
#include "cairo-quartz.h"

#define PLUGIN_OURNAME      "Novell Moonlight"

// FIXME: We have a lot of horrible hacks here since we dont subclass NSWindow yet and proxy its events

using namespace Moonlight;

MoonWindowCocoa::MoonWindowCocoa (MoonWindowType windowType, int w, int h, MoonWindow *parent, Surface *surface)
	: MoonWindow (windowType, w, h, parent, surface)
{
	native = NULL;
	ctx = NULL;
	NSWindow *window = [[NSWindow alloc] initWithContentRect: NSMakeRect (0, 0, w, h) styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask backing: NSBackingStoreBuffered defer: YES];
	MLView *view = [[[MLView alloc] initWithFrame: NSMakeRect (0, 0, w, h)] autorelease];

	window.acceptsMouseMovedEvents = YES;
	window.contentView = view;
	view.moonwindow = this;

	[view setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];

	this->width = w;
	this->height = h;
	this->window = window;
	this->view = view;
}

MoonWindowCocoa::~MoonWindowCocoa ()
{
}

void
MoonWindowCocoa::ConnectToContainerPlatformWindow (gpointer container_window)
{
}

MoonClipboard*
MoonWindowCocoa::GetClipboard (MoonClipboardType clipboardType)
{
	g_assert_not_reached ();
}

gpointer
MoonWindowCocoa::GetPlatformWindow ()
{
	return this->view;
}

void
MoonWindowCocoa::Resize (int width, int height)
{
	NSRect frame = [window frame];

	frame.size.width = width;
	frame.size.height = height;

	[window setFrame: frame display: YES];

	this->width = width;
	this->height = height;
}

void
MoonWindowCocoa::SetBackgroundColor (Color *color)
{
}

void
MoonWindowCocoa::SetCursor (CursorType cursor)
{
}

void
MoonWindowCocoa::Invalidate (Rect r)
{
//	NSRect frame = [view frame];

//	[view setNeedsDisplayInRect: NSMakeRect (r.x, frame.size.height - r.y - r.height, r.width, r.height)];
	[view setNeedsDisplayInRect: [view frame]];
}

void
MoonWindowCocoa::ProcessUpdates ()
{
}

gboolean
MoonWindowCocoa::HandleEvent (gpointer platformEvent)
{
	// nothing to do here, since we don't pump events into the cocoa
	// window, cocoa calls our signal handlers directly.
	return TRUE;
}

void
MoonWindowCocoa::Show ()
{
	[window makeKeyAndOrderFront:nil];
	// FIXME: We need to subclass NSWindow as well to do this properly.
	this->surface->HandleUIWindowAvailable ();
	this->surface->HandleUIWindowAllocation (true);
}

void
MoonWindowCocoa::Hide ()
{
}

void
MoonWindowCocoa::EnableEvents (bool first)
{
	g_warning ("implement me");
}

void
MoonWindowCocoa::DisableEvents ()
{
	g_warning ("implement me");
}

void
MoonWindowCocoa::GrabFocus ()
{
	g_warning ("implement me");
}

bool
MoonWindowCocoa::HasFocus ()
{
	g_warning ("implement me");
}

void
MoonWindowCocoa::SetLeft (double left)
{
	NSRect frame = [window frame];

	frame.origin.x = left;

	[window setFrame: frame display: YES];
}

double
MoonWindowCocoa::GetLeft ()
{
	NSRect frame = [window frame];

	return frame.origin.x;
}

void
MoonWindowCocoa::SetTop (double top)
{
	NSRect frame = [window frame];

	frame.origin.y = top;

	[window setFrame: frame display: YES];
}

double
MoonWindowCocoa::GetTop ()
{
	NSRect frame = [window frame];

	return frame.origin.y;
}

void
MoonWindowCocoa::SetWidth (double width)
{
	NSRect frame = [window frame];

	frame.size.width = width;

	[window setFrame: frame display: YES];

	this->width = width;
}

void
MoonWindowCocoa::SetHeight (double height)
{
	NSRect frame = [window frame];

	frame.size.height = height;

	[window setFrame: frame display: YES];

	this->height = height;
}

void
MoonWindowCocoa::SetTitle (const char *title)
{
}

void
MoonWindowCocoa::SetIconFromPixbuf (MoonPixbuf *pixbuf)
{
	g_assert_not_reached ();
}

void
MoonWindowCocoa::SetStyle (WindowStyle style)
{
	g_assert_not_reached ();
}

cairo_surface_t *
MoonWindowCocoa::CreateCairoSurface ()
{
	CGContextRef context = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
	int width = ((MLView *) view).frame.size.width;
	int height = ((MLView *) view).frame.size.height;

	CGContextTranslateCTM (context, 0.0, height);
	CGContextScaleCTM (context, 1.0, -1.0);

	return cairo_quartz_surface_create_for_cg_context (context, width, height);
}

void
MoonWindowCocoa::ButtonPressEvent (void *evt)
{
	MLEvent *event = [[MLEvent alloc] initWithEvent: (NSEvent *) evt view: (MLView *) this->view];
	SetCurrentDeployment ();

	if (surface) {
		MoonButtonEvent *mevent = (MoonButtonEvent*)Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		surface->HandleUIButtonPress (mevent);
		delete mevent;
	}
//	[event release];
}

void
MoonWindowCocoa::KeyDownEvent (void *evt)
{
	MLEvent *event = [[MLEvent alloc] initWithEvent: (NSEvent *) evt view: (MLView *) this->view];
	SetCurrentDeployment ();

	if (surface) {
		MoonKeyEvent *mevent = (MoonKeyEvent*)Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		surface->HandleUIKeyPress (mevent);
		delete mevent;
	}
//	[event release];
}

void
MoonWindowCocoa::KeyUpEvent (void *evt)
{
	MLEvent *event = [[MLEvent alloc] initWithEvent: (NSEvent *) evt view: (MLView *) this->view];
	SetCurrentDeployment ();

	if (surface) {
		MoonKeyEvent *mevent = (MoonKeyEvent*)Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		surface->HandleUIKeyRelease (mevent);
		delete mevent;
	}
//	[event release];
}

void
MoonWindowCocoa::ButtonReleaseEvent (void *evt)
{
	MLEvent *event = [[MLEvent alloc] initWithEvent: (NSEvent *) evt view: (MLView *) this->view];
	SetCurrentDeployment ();

	if (surface) {
		MoonButtonEvent *mevent = (MoonButtonEvent*)Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		surface->HandleUIButtonRelease (mevent);
		delete mevent;
	}
//	[event release];
}

void
MoonWindowCocoa::MotionEvent (void *evt)
{
	MLEvent *event = [[MLEvent alloc] initWithEvent: (NSEvent *) evt view: (MLView *) this->view];
	SetCurrentDeployment ();

	if (surface) {
		MoonMotionEvent *mevent = (MoonMotionEvent*)Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		surface->HandleUIMotion (mevent);
		delete mevent;
	}
//	[event release];
}

void
MoonWindowCocoa::MouseEnteredEvent (void *evt)
{
	MLEvent *event = [[MLEvent alloc] initWithEvent: (NSEvent *) evt view: (MLView *) this->view];
	SetCurrentDeployment ();

	if (surface) {
		MoonCrossingEvent *mevent = (MoonCrossingEvent*)Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		surface->HandleUICrossing (mevent);
		delete mevent;
	}
//	[event release];
}

void
MoonWindowCocoa::MouseExitedEvent (void *evt)
{
	MLEvent *event = [[MLEvent alloc] initWithEvent: (NSEvent *) evt view: (MLView *) this->view];
	SetCurrentDeployment ();

	if (surface) {
		MoonCrossingEvent *mevent = (MoonCrossingEvent*)Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (event);
		surface->HandleUICrossing (mevent);
		delete mevent;
	}
//	[event release];
}

void
MoonWindowCocoa::ExposeEvent (Rect r)
{
	SetCurrentDeployment ();

	Region *region = new Region (r);
	cairo_surface_t *native = CreateCairoSurface ();
	CairoSurface *target = new CairoSurface (width, height);
	Context *ctx = new CairoContext (target);

	surface->Paint (ctx, region, GetTransparent (), NO);

	cairo_surface_destroy (native);

	target->unref ();
	delete ctx;
	delete region;
}
