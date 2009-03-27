/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * multiscalesubimage.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>

#include "debug.h"
#include "runtime.h"
#include "deepzoomimagetilesource.h"
#include "multiscalesubimage.h"

MultiScaleSubImage::MultiScaleSubImage ()
{
	SetObjectType (Type::MULTISCALESUBIMAGE);
	source = NULL;	
	parent = NULL;
}

MultiScaleSubImage::MultiScaleSubImage (const Uri *parent_uri, MultiScaleTileSource *tsource, int _id, int _n)
{
	LOG_MSI ("new MultiScaleSubImage ()\n");
	SetObjectType (Type::MULTISCALESUBIMAGE);
	source = tsource;
	parent = NULL;
	id = _id;
	n = _n;

	Uri *source_uri = ((DeepZoomImageTileSource*)source)->GetUriSource ();
	if (source_uri->isAbsolute)
		return;
	
	LOG_MSI ("MSSI: UriSource changed from %s", source_uri->ToString());
	Uri *original_uri = new Uri ();
	Uri::Copy (source_uri, original_uri);
	Uri::Copy (parent_uri, source_uri);
	source_uri->Combine (original_uri);
	delete original_uri;
	LOG_MSI (" to %s\n", source_uri->ToString());

}

double
MultiScaleSubImage::GetViewportHeight ()
{
	return GetAspectRatio () * GetViewportWidth ();
}

void
MultiScaleSubImage::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetId () == MultiScaleSubImage::ViewportOriginProperty) {
		Point *p = args->new_value->AsPoint();
		if (parent)
			parent->Invalidate ();
	}

	if (args->GetId () == MultiScaleSubImage::ViewportWidthProperty) {
		if (parent)
			parent->Invalidate ();
	}

	if (args->GetProperty ()->GetOwnerType () != Type::MULTISCALESUBIMAGE) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}
	
	NotifyListenersOfPropertyChange (args);
}
