/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * propertypath.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __PROPERTY_PATH_H__
#define __PROPERTY_PATH_H__

#include <glib.h>
#include <string.h>

/* @IncludeInKinds */
struct PropertyPath {
public:
	PropertyPath (const char *path)
	{
		this->path = g_strdup (path);
	}

	~PropertyPath ()
	{
		g_free (path);
	}

	bool operator== (const PropertyPath &v) const
	{
		return !strcmp (v.path, path);
	}

	char *path;
};

#endif
