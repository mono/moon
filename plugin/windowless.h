/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * windowless.h: windowless plugin's surface
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_PLUGIN_WINDOWLESS__
#define __MOON_PLUGIN_WINDOWLESS__

#include "runtime.h"
#include "plugin.h"

class WindowlessSurface : public Surface {
	PluginInstance *plugin;
	
 public:
	WindowlessSurface (int width, int height, PluginInstance *plugin);

	virtual void SetCursor (GdkCursor *cursor);
	virtual void Invalidate (Rect r);
	virtual void ProcessUpdates ();

	gboolean HandleEvent (XEvent *event);
};

#endif /* __MOON_PLUGIN_WINDOWLESS__ */

