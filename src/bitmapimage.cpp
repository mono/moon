/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bitmapimage.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <stdio.h>

#include "application.h"
#include "bitmapimage.h"

BitmapImage::BitmapImage ()
{
	SetObjectType (Type::BITMAPIMAGE);
	buffer = NULL;
	size = 0;
	SetUriSource ("");
}

BitmapImage::~BitmapImage ()
{
	CleanUp ();
}

void
BitmapImage::CleanUp ()
{
	if (buffer != NULL) {
		g_free (buffer);
		
		buffer = NULL;
		size = 0;
	}
}
void
BitmapImage::SetBuffer (gpointer buffer, int size)
{
	CleanUp ();

	this->buffer = buffer;
	this->size = size;
}

void
BitmapImage::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->GetId () == BitmapImage::UriSourceProperty) {
		const char *uri = args->new_value ? args->new_value->AsString () : NULL;

		CleanUp ();

		if (uri != NULL) {
			Application *current = Application::GetCurrent ();
			if (current)
				this->buffer = current->GetResource (uri, &size);
		}
	}
	
	NotifyListenersOfPropertyChange (args);
}
