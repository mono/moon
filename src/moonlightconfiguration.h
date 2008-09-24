/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * moonlightconfiguration.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MOON_MOONLIGHT_CONFIGURATION_H__
#define __MOON_MOONLIGHT_CONFIGURATION_H__

#include <glib.h>

class MoonlightConfiguration {
private:
	GKeyFile *data;
	gchar *filename;
	
public:
	MoonlightConfiguration ();
	~MoonlightConfiguration ();

	void Save ();

	void SetBooleanValue (const char *group, const char *key, gboolean value);
	void SetStringValue (const char *group, const char *key, const char *value);
	char *GetStringValue (const char *group, const char *key);
	gboolean GetBooleanValue (const char *group, const char *key);
};

#endif /* __MOON_MOONLIGHT_CONFIGURATION_H__ */
