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

GLXSurface::GLXSurface (Display *dpy, XID win) : GLSurface ()
{
	XWindowAttributes attr;

	XGetWindowAttributes (dpy, win, &attr);

	display = dpy;
	window  = win;
	size[0] = attr.width;
	size[1] = attr.height;
	vid     = XVisualIDFromVisual (attr.visual);
}

GLXSurface::GLXSurface (GLsizei w, GLsizei h) : GLSurface (w, h)
{
	display = NULL;
	window  = 0;
	vid     = 0;
}

VisualID
GLXSurface::GetVisualID ()
{
	return vid;
}

void
GLXSurface::SwapBuffers ()
{
	g_assert (window);
	glXSwapBuffers (display, window);
}

void
GLXSurface::Reshape (int width, int height)
{
	size[0] = width;
	size[1] = height;

	if (texture) {
		glDeleteTextures (1, &texture);
		texture = 0;
	}

	if (surface) {
		cairo_surface_destroy (surface);
		surface = NULL;
	}

	if (data) {
		g_free (data);
		data = NULL;
	}
}

cairo_surface_t *
GLXSurface::Cairo ()
{
	if (window) {
		const cairo_format_t format = CAIRO_FORMAT_ARGB32;
		unsigned char        *tmp;
		int                  stride = size[0] * 4;
		int                  i;

		if (surface)
			return cairo_surface_reference (surface);

		tmp  = (unsigned char *) g_malloc (stride * size[1]);
		data = (unsigned char *) g_malloc (stride * size[1]);

		glReadPixels (0,
			      0,
			      size[0],
			      size[1],
			      GL_BGRA,
			      GL_UNSIGNED_BYTE,
			      tmp);

		for (i = 0; i < size[1]; i++)
			memcpy (data + (size[1] - i - 1) * stride,
				tmp + i * stride,
				stride);

		surface = cairo_image_surface_create_for_data (data,
							       format,
							       size[0],
							       size[1],
							       stride);

		g_free (tmp);

		return cairo_surface_reference (surface);
	}

	return GLSurface::Cairo ();
}

GLuint
GLXSurface::StealTexture ()
{
	GLuint name = texture;

	if (surface)
		name = Texture ();

	texture = 0;
	return name;
}

};
