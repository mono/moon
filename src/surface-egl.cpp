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

#define __MOON_EGL__

#include "surface-egl.h"

namespace Moonlight {

MoonEGLSurface::MoonEGLSurface (EGLDisplay display, EGLSurface surface) : GLSurface ()
{
	int w, h;

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	this->display = display;
	this->surface = surface;
	size[0] = w;
	size[1] = h;
}

MoonEGLSurface::MoonEGLSurface (GLsizei w, GLsizei h) : GLSurface (w, h)
{
	this->display = EGL_NO_DISPLAY;
	this->surface = EGL_NO_SURFACE;
}

void
MoonEGLSurface::SwapBuffers ()
{
	g_assert (surface);
	eglSwapBuffers (display, surface);
}

void
MoonEGLSurface::Reshape (int width, int height)
{
	size[0] = width;
	size[1] = height;

	if (texture) {
		glDeleteTextures (1, &texture);
		texture = 0;
	}

	if (data) {
		g_free (data);
		data = NULL;
	}
}

cairo_surface_t *
MoonEGLSurface::Cairo ()
{
	g_assert (surface == EGL_NO_SURFACE);

	return GLSurface::Cairo ();
}

bool
MoonEGLSurface::HasTexture ()
{
	return texture != 0;
}

GLuint
MoonEGLSurface::Texture ()
{
	GLuint name = texture;

	if (!texture)
		glGenTextures (1, &texture);

	if (name != texture || data) {
		glBindTexture (GL_TEXTURE_2D, texture);
		glTexImage2D (GL_TEXTURE_2D,
			      0,
			      GL_RGBA,
			      size[0],
			      size[1],
			      0,
			      GL_RGBA,
			      GL_UNSIGNED_BYTE,
			      data);
		glBindTexture (GL_TEXTURE_2D, 0);
	}

	if (data) {
		g_free (data);
		data = NULL;
	}

	return texture;
}

};
