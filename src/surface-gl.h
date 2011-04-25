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
#elif defined(USE_EGL)
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/gl.h>
#endif

namespace Moonlight {

typedef void (*__GLFuncPtr) ();

class MOON_API GLSurface : public MoonSurface {
public:
	GLSurface ();
	GLSurface (GLsizei width, GLsizei height);
	virtual ~GLSurface ();

	cairo_surface_t *Cairo ();

	virtual GLuint Texture ();

	// Planar YUV
	bool IsPlanar ();
	virtual GLuint TextureYUV (int i);

	GLsizei Width ();
	GLsizei Height ();

	virtual __GLFuncPtr GetProcAddress (const char *procname);

protected:
	GLsizei size[2];

	GLuint texture;
	GLuint textureYUV[3];

	unsigned char *data;
};

};

#endif /* __MOON_SURFACE_GL_H__ */
