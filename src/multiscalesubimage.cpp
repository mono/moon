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

#include "debug.h"
#include "runtime.h"
#include "multiscalesubimage.h"

MultiScaleSubImage::MultiScaleSubImage ()
{
	SetObjectType (Type::MULTISCALESUBIMAGE);
	source = NULL;	
}

MultiScaleSubImage::MultiScaleSubImage (const char* parent_uri, MultiScaleTileSource *tsource)
{
	LOG_MSI ("new MultiScaleSubImage ()\n");
	SetObjectType (Type::MULTISCALESUBIMAGE);
	source = tsource;
	//FIXME, set the new relative uri
}
