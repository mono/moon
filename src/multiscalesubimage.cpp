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

#include <config.h>

#include <glib.h>

#include "debug.h"
#include "runtime.h"
#include "deepzoomimagetilesource.h"
#include "multiscalesubimage.h"
#include "deployment.h"

namespace Moonlight {

MultiScaleSubImage::MultiScaleSubImage ()
{
	SetObjectType (Type::MULTISCALESUBIMAGE);
	source = NULL;	
}

MultiScaleSubImage::MultiScaleSubImage (const Uri *parent_uri, MultiScaleTileSource *tsource, int _id, int _n)
{
	LOG_MSI ("new MultiScaleSubImage ()\n");
	SetObjectType (Type::MULTISCALESUBIMAGE);
	source = tsource;
	id = _id;
	n = _n;

	const Uri *source_uri = ((DeepZoomImageTileSource*)source)->GetUriSource ();
	if (!source_uri || source_uri->IsAbsolute ())
		return;

	Uri *new_uri;

	if (!parent_uri->IsAbsolute ()) {
		// See comment above the call to CombineWithSourceLocation in dzits.cpp
		new_uri = Uri::CombineWithSourceLocation (GetDeployment (), parent_uri, source_uri, (parent_uri == NULL || parent_uri->GetPath () == NULL || parent_uri->GetPath () [0] != '/'));
	} else {
		new_uri = Uri::Create (parent_uri, source_uri);
	}

	LOG_MSI ("MSSI: UriSource changed from %s to %s\n", source_uri->ToString (), new_uri->ToString ());

	((DeepZoomImageTileSource*) source)->SetUriSource (new_uri);
	delete new_uri;

	EnsureManagedPeer ();
}

double
MultiScaleSubImage::GetViewportHeight ()
{
	return GetAspectRatio () * GetViewportWidth ();
}

};
