/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-opengl.cpp
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <string.h>

#include "surface-opengl.h"

namespace Moonlight {

OpenGLSurface::OpenGLSurface () : GLSurface ()
{
	drawable = true;
}

OpenGLSurface::OpenGLSurface (GLsizei w, GLsizei h) : GLSurface (w, h)
{
	drawable = false;
}

void
OpenGLSurface::Reshape (int width, int height)
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
OpenGLSurface::Cairo ()
{
	g_assert (!drawable);

	if (!data && texture) {
		data = (unsigned char *) g_malloc (size[0] * size[1] * 4);

		glBindTexture (GL_TEXTURE_2D, texture);
		glGetTexImage (GL_TEXTURE_2D,
			       0,
			       GL_BGRA,
			       GL_UNSIGNED_BYTE,
			       data);
		glBindTexture (GL_TEXTURE_2D, 0);
	}

	return GLSurface::Cairo ();
}

GLuint
OpenGLSurface::Texture ()
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
			      GL_BGRA,
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


GLuint
OpenGLSurface::TextureYUV (int i)
{
	if (!textureYUV[i]) {
		int j;

		GLSurface::TextureYUV (i);

		for (j = 0; j < 3; j++) {
			GLfloat border[][4] = {
				{ 0.0625f, 0.0625f, 0.0625f, 0.0625f },
				{   0.5f ,    0.5f,    0.5f,    0.5f },
				{   0.5f ,    0.5f,    0.5f,    0.5f }
			};

			glBindTexture (GL_TEXTURE_2D, textureYUV[j]);
			glTexParameterfv (GL_TEXTURE_2D,
					  GL_TEXTURE_BORDER_COLOR,
					  border[j]);
		}
		glBindTexture (GL_TEXTURE_2D, 0);
	}

	return textureYUV[i];
}

bool
OpenGLSurface::HasTexture ()
{
	return texture != 0 || IsPlanar ();
}

};
