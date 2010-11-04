/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include "pal-cocoa.h"

#include "clipboard-cocoa.h"

#include "runtime.h"
#include "window-cocoa.h"

#include <AppKit/AppKit.h>

using namespace Moonlight;

MoonClipboardCocoa::MoonClipboardCocoa (MoonWindowCocoa *win, MoonClipboardType clipboardType)
{
	pasteboard = [NSPasteboard generalPasteboard];

	// FIXME: We only handle text currently
	[pasteboard declareTypes:[NSArray arrayWithObjects:NSStringPboardType, nil] owner:nil];
}

bool
MoonClipboardCocoa::ContainsText ()
{
	g_assert_not_reached ();
}

void
MoonClipboardCocoa::SetText (const char *text, int length)
{
	NSString *string = [[NSString alloc] initWithBytes: text length: length encoding: NSUTF8StringEncoding];

	[pasteboard setString:string forType:NSStringPboardType];
}

void
MoonClipboardCocoa::AsyncGetText (MoonClipboardGetTextCallback cb, gpointer data)
{
	g_assert_not_reached ();
}

char*
MoonClipboardCocoa::GetText ()
{
	g_assert_not_reached ();
}
