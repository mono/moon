/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-opengl.h
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_SURFACE_OPENGL_H__
#define __MOON_SURFACE_OPENGL_H__

#include "surface-gl.h"

namespace Moonlight {

class MOON_API OpenGLSurface : public GLSurface {
public:
	OpenGLSurface ();
	OpenGLSurface (GLsizei w, GLsizei h);

	bool HasDrawable () { return drawable; }

	void Reshape (int width, int height);
	cairo_surface_t *Cairo ();
	GLuint Texture ();
	GLuint TextureYUV (int i);
	bool HasTexture ();

protected:
	bool drawable;
};

};

#endif /* __MOON_SURFACE_OPENGL_H__ */
