/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-glx.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <string.h>

#define __MOON_GLX__

#include "surface-glx.h"

namespace Moonlight {

int GLXSurface::X11Error;
int (*GLXSurface::SavedX11ErrorHandler) (Display *, XErrorEvent *);

GLXSurface::GLXSurface (Display *dpy, XID win) : OpenGLSurface ()
{
	XWindowAttributes attr;
	Status            status;

	display = dpy;
	window  = 0;
	vid     = 0;

	X11ErrorTrapPush (dpy);
	status = XGetWindowAttributes (dpy, win, &attr);
	if (X11ErrorTrapPop (dpy) == Success)
		window = win;

	if (status) {
		size[0] = attr.width;
		size[1] = attr.height;
		vid     = XVisualIDFromVisual (attr.visual);
	}
}

int
GLXSurface::X11ErrorHandler (Display     *display,
			     XErrorEvent *event)
{
	X11Error = event->type;
	return False;
}

void
GLXSurface::X11ErrorTrapPush (Display *dpy)
{
	XSync (dpy, False);
	X11Error = Success;
	SavedX11ErrorHandler = XSetErrorHandler (X11ErrorHandler);
}

int
GLXSurface::X11ErrorTrapPop (Display *dpy)
{
	XSync (dpy, False);
	XSetErrorHandler (SavedX11ErrorHandler);
	return X11Error;
}

VisualID
GLXSurface::GetVisualID ()
{
	return vid;
}

__GLFuncPtr
GLXSurface::GetProcAddress (const char *procname)
{
	return glXGetProcAddressARB ((const GLubyte *) procname);
}

};
