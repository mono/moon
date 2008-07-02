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
public:
	WindowlessSurface (int width, int height, PluginInstance *plugin);

	virtual void SetCursor (GdkCursor *cursor);
	virtual void Invalidate (Rect r);
	virtual void ProcessUpdates ();

 private:
	PluginInstance *plugin;
};

#endif /* __MOON_PLUGIN_WINDOWLESS__ */

