/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-glx.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_SURFACE_GLX_H__
#define __MOON_SURFACE_GLX_H__

#include "surface-gl.h"

#ifdef __MOON_GLX__

#define GLXContext _XxGLXContext
#include <GL/glx.h>
#undef GLXContext

typedef int (*PFNX11ERRORHANDLERPROC) (Display *, XErrorEvent *);

namespace Moonlight {

class MOON_API GLXSurface : public GLSurface {
public:
	GLXSurface (Display *dpy, XID win);
	GLXSurface (GLsizei w, GLsizei h);

	Display     *GetDisplay () { return display; }
	GLXDrawable GetGLXDrawable () { return window; }
	VisualID    GetVisualID ();

	void SwapBuffers ();
	void Reshape (int width, int height);
	cairo_surface_t *Cairo ();
	GLuint Texture ();
	GLuint TextureYUV (int i);
	bool HasTexture ();

	static void X11ErrorTrapPush (Display *dpy);
	static int X11ErrorTrapPop (Display *dpy);

private:
	Display  *display;
	XID      window;
	VisualID vid;

	static int X11Error;
	static PFNX11ERRORHANDLERPROC SavedX11ErrorHandler;
	static int X11ErrorHandler (Display     *display,
				    XErrorEvent *event);
};

};

#endif /* __MOON_GLX__ */

#endif /* __MOON_SURFACE_GLX_H__ */
