/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-gl.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_SURFACE_GL_H__
#define __MOON_SURFACE_GL_H__

#include "surface.h"

#if defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

namespace Moonlight {

class GLSurface : public MoonSurface {
public:
	GLSurface ();
	GLSurface (GLsizei width, GLsizei height);
	virtual ~GLSurface ();

	cairo_surface_t *Cairo ();

	GLuint Texture ();

	GLsizei Width ();
	GLsizei Height ();

protected:
	GLsizei size[2];

	GLuint texture;

	cairo_surface_t *surface;
	unsigned char   *data;
};

};

#endif /* __MOON_SURFACE_GL_H__ */
