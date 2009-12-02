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
	PropertyPath (DependencyProperty *property)
	{
		this->path = NULL;
		this->expanded_path = NULL;
		this->property = property;
	}
	
	PropertyPath (const char *path)
	{
		this->path = g_strdup (path);
		this->expanded_path = NULL;
		this->property = NULL;
	}

	PropertyPath (const PropertyPath &path)
	{
		this->path = g_strdup (path.path);
		this->expanded_path = g_strdup (path.expanded_path);
		this->property = path.property;
	}

	~PropertyPath ()
	{
		g_free (path);
		g_free (expanded_path);
	}

	bool operator== (const PropertyPath &v) const
	{
		if (path) {
			return v.path && !strcmp (v.path, path);
		}
		else {
			return v.property == property;
		}
	}

	bool operator!= (const PropertyPath &v) const
	{
		return !(*this == v);
	}

	char *path;
	char *expanded_path; // This is a path with namespaces expanded into full assembly names
	DependencyProperty *property;
};

#endif
