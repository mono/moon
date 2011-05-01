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
}

bool
MoonClipboardCocoa::ContainsText ()
{
	NSArray *classes = [[NSArray alloc] initWithObjects:[NSString class], nil];
	NSDictionary *options = [NSDictionary dictionary];
	return [(NSPasteboard*)pasteboard canReadObjectsForClasses:classes options:options];
}

void
MoonClipboardCocoa::SetText (const char *text)
{
	NSString *string = [[NSString alloc] initWithBytes: text length: strlen (text) encoding: NSUTF8StringEncoding];

	NSArray *array = [[NSArray alloc] initWithObjects: string, nil];

	[(NSPasteboard*)pasteboard declareTypes:[NSArray arrayWithObjects:NSStringPboardType, nil] owner:nil];

	[(NSPasteboard*)pasteboard writeObjects: array];
}

void
MoonClipboardCocoa::AsyncGetText (MoonClipboardGetTextCallback cb, gpointer data)
{
	char *text = GetText ();

	cb (this, text, data);

	g_free (text);
}

char*
MoonClipboardCocoa::GetText ()
{
	NSArray *classes = [[NSArray alloc] initWithObjects:[NSString class], nil];
	NSDictionary *options = [NSDictionary dictionary];
	NSArray *copiedItems = [(NSPasteboard*)pasteboard readObjectsForClasses:classes options:options];
	if (copiedItems != nil) {
		NSString *str = [copiedItems objectAtIndex: 0];

		return g_strdup ([str UTF8String]);
	}

	return NULL;
}
