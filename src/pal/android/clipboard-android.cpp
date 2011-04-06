/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include "pal-android.h"

#include "clipboard-android.h"

#include "runtime.h"
#include "window-android.h"

using namespace Moonlight;

MoonClipboardAndroid::MoonClipboardAndroid (MoonWindowAndroid *win, MoonClipboardType clipboardType)
{
}

bool
MoonClipboardAndroid::ContainsText ()
{
	return false; // FIXME
}

void
MoonClipboardAndroid::SetText (const char *text)
{
	// FIXME
}

void
MoonClipboardAndroid::AsyncGetText (MoonClipboardGetTextCallback cb, gpointer data)
{
	// FIXME
}

char*
MoonClipboardAndroid::GetText ()
{
	// FIXME
	return NULL;
}
