/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * windowless.h: windowless plugin's MoonWindow implementation
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_WINDOWLESS__
#define __MOON_WINDOWLESS__

#include "runtime.h"
#include "plugin.h"

class MoonWindowless : public MoonWindow {
	PluginInstance *plugin;
	
 public:
	MoonWindowless (int width, int height, PluginInstance *plugin);

	virtual void Resize (int width, int height);

	virtual void SetCursor (GdkCursor *cursor);
	virtual void Invalidate (Rect r);
	virtual void ProcessUpdates ();

	virtual gboolean HandleEvent (XEvent *event);

	virtual void Show ();
	virtual void Hide ();

	virtual void EnableEvents (bool first);
	virtual void DisableEvents ();
	
	virtual void GrabFocus ();
	virtual bool HasFocus ();

	virtual void SetSurface (Surface *s);

	virtual bool IsFullScreen () { return false; }
};

#endif /* __MOON_WINDOWLESS__ */

