/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * debug.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

guint32 shocker_flags = 0;

void
shocker_debug_initialize ()
{
	char *env = getenv ("MOONLIGHT_SHOCKER_DEBUG");
	printf ("[shocker] parsing MOONLIGHT_SHOCKER_DEBUG=%s (valid values: all,harness,capture,input,plugin,shutdown,clipboard or a combination of those)\n", env);
	if (env == NULL || env [0] == 0)
		return;

	char *word = env;
	char *ptr = env;
	char *end = env + strlen (env);

	while (ptr++ <= end) {
		if (*ptr != ',' && *ptr != 0)
			continue;

		*ptr = 0;
		if (!strcmp (word, "all")) {
			shocker_flags = G_MAXUINT32;
		} else if (!strcmp (word, "harness")) {
			shocker_flags |= SHOCKER_DEBUG_HARNESS;
		} else if (!strcmp (word, "capture")) {
			shocker_flags |= SHOCKER_DEBUG_CAPTURE;
		} else if (!strcmp (word, "input")) {
			shocker_flags |= SHOCKER_DEBUG_INPUT;
		} else if (!strcmp (word, "plugin")) {
			shocker_flags |= SHOCKER_DEBUG_PLUGIN;
		} else if (!strcmp (word, "shutdown")) {
			shocker_flags |= SHOCKER_DEBUG_SHUTDOWN;
		} else if (!strcmp (word, "clipboard")) {
			shocker_flags |= SHOCKER_DEBUG_CLIPBOARD;
		} else {
			printf ("[shocker]: Unknown value '%s' in MOONLIGHT_SHOCKER_DEBUG.\n", word);
		}
			
		word = ptr + 1;
	}
}

