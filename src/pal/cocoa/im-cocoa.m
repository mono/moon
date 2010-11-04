/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * im-cocoa.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>
#include "im-cocoa.h"
#include <cairo.h>

using namespace Moonlight;

MoonIMContextCocoa::MoonIMContextCocoa ()
{
	printf ("implement me: MoonIMContextCocoa\n");
}

MoonIMContextCocoa::~MoonIMContextCocoa ()
{
	printf ("implement me: MoonIMContextCocoa\n");
}

void
MoonIMContextCocoa::SetUsePreedit (bool flag)
{
	printf ("implement me: MoonIMContextCocoa\n");
}

void
MoonIMContextCocoa::SetClientWindow (MoonWindow* window)
{
	printf ("implement me: MoonIMContextCocoa\n");
}

bool
MoonIMContextCocoa::FilterKeyPress (MoonKeyEvent* event)
{
	printf ("implement me: MoonIMContextCocoa\n");
}

void
MoonIMContextCocoa::SetSurroundingText (const char *text, int offset, int length)
{
	printf ("implement me: MoonIMContextCocoa\n");
}

void
MoonIMContextCocoa::Reset ()
{
	printf ("implement me: MoonIMContextCocoa\n");
}


void
MoonIMContextCocoa::FocusIn ()
{
	printf ("implement me: MoonIMContextCocoa\n");
}

void
MoonIMContextCocoa::FocusOut ()
{
	printf ("implement me: MoonIMContextCocoa\n");
}

void
MoonIMContextCocoa::SetCursorLocation (Rect r)
{
	printf ("implement me: MoonIMContextCocoa\n");
}

void
MoonIMContextCocoa::SetRetrieveSurroundingCallback (MoonCallback cb, gpointer data)
{
	printf ("implement me: MoonIMContextCocoa\n");
}

void
MoonIMContextCocoa::SetDeleteSurroundingCallback (MoonCallback cb, gpointer data)
{
	printf ("implement me: MoonIMContextCocoa\n");
}

void
MoonIMContextCocoa::SetCommitCallback (MoonCallback cb, gpointer data)
{
	printf ("implement me: MoonIMContextCocoa\n");
}

gpointer
MoonIMContextCocoa::GetPlatformIMContext ()
{
	printf ("implement me: MoonIMContextCocoa\n");
	return NULL;
}

