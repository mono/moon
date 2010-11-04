/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * window-cocoa.h: MoonWindow implementation using cocoa widgets.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_CONFIG_DIALOG_COCOA_H__
#define __MOON_CONFIG_DIALOG_COCOA_H__

#include "window-cocoa.h"
#include "moonlightconfiguration.h"

namespace Moonlight {

class MoonConfigDialogCocoa {
public:
	MoonConfigDialogCocoa (MoonWindowCocoa *window, Surface *surface, Deployment *deployment);
	~MoonConfigDialogCocoa ();

	void Show ();

	MoonWindowCocoa *GetWindow () { return window; }
	Surface *GetSurface() { return surface; }
	Deployment *GetDeployment() { return deployment; }

private:
	MoonWindowCocoa *window;
	Surface *surface;
	Deployment *deployment;
};

};
#endif /* __MOON_CONFIG_DIALOG_COCOA_H__ */

