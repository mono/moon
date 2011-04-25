/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * im-android.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>
#include "im-android.h"

using namespace Moonlight;

MoonIMContextAndroid::MoonIMContextAndroid ()
{
}

MoonIMContextAndroid::~MoonIMContextAndroid ()
{
}

void
MoonIMContextAndroid::SetUsePreedit (bool flag)
{
}

void
MoonIMContextAndroid::SetClientWindow (MoonWindow* window)
{
}

bool
MoonIMContextAndroid::FilterKeyPress (MoonKeyEvent* event)
{
	return false;
}

void
MoonIMContextAndroid::SetSurroundingText (const char *text, int offset, int length)
{
}

void
MoonIMContextAndroid::Reset ()
{
}


void
MoonIMContextAndroid::FocusIn ()
{
}

void
MoonIMContextAndroid::FocusOut ()
{
}

void
MoonIMContextAndroid::SetCursorLocation (Rect r)
{
}

void
MoonIMContextAndroid::SetRetrieveSurroundingCallback (MoonCallback cb, gpointer data)
{
}

void
MoonIMContextAndroid::SetDeleteSurroundingCallback (MoonCallback cb, gpointer data)
{
}

void
MoonIMContextAndroid::SetCommitCallback (MoonCallback cb, gpointer data)
{
}

gpointer
MoonIMContextAndroid::GetPlatformIMContext ()
{
	return NULL;
}

