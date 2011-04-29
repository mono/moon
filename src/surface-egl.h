/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-glx.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_SURFACE_EGL_H__
#define __MOON_SURFACE_EGL_H__

#include "surface-gl.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>

namespace Moonlight {

class MOON_API MoonEGLSurface : public GLSurface {
public:
	MoonEGLSurface (EGLDisplay display, EGLSurface surface);
	MoonEGLSurface (GLsizei w, GLsizei h);

	EGLDisplay GetEGLDisplay () { return display; }
	EGLSurface GetEGLSurface () { return surface; }

	void SwapBuffers ();
	void Reshape (int width, int height);
	cairo_surface_t *Cairo ();
	GLuint Texture ();
	bool HasTexture ();

private:
	EGLDisplay display;
	EGLSurface surface;
};

};

#endif /* __MOON_SURFACE_EGL_H__ */
