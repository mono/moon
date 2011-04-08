/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-gl.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include "surface-gl.h"

namespace Moonlight {

GLSurface::GLSurface ()
{
	size[0]       = 0;
	size[1]       = 0;
	texture       = 0;
	textureYUV[0] = 0;
	textureYUV[1] = 0;
	textureYUV[2] = 0;
	data          = NULL;
}

GLSurface::GLSurface (GLsizei width, GLsizei height)
{
	size[0]       = width;
	size[1]       = height;
	texture       = 0;
	textureYUV[0] = 0;
	textureYUV[1] = 0;
	textureYUV[2] = 0;
	data          = NULL;
}

GLSurface::~GLSurface ()
{
	if (data)
		g_free (data);

	if (texture)
		glDeleteTextures (1, &texture);

	if (textureYUV[0])
		glDeleteTextures (3, textureYUV);
}

cairo_surface_t *
GLSurface::Cairo ()
{
	int stride = size[0] * 4;

	if (!data) {
		if (texture) {
#if !USE_EGL
			data = (unsigned char *) g_malloc (stride * size[1]);

			glBindTexture (GL_TEXTURE_2D, texture);
			glGetTexImage (GL_TEXTURE_2D,
				       0,
				       GL_BGRA,
				       GL_UNSIGNED_BYTE,
				       data);
			glBindTexture (GL_TEXTURE_2D, 0);
#endif
		}
		else {
			data = (unsigned char *) g_malloc0 (stride * size[1]);
		}
	}

	return cairo_image_surface_create_for_data (data,
						    CAIRO_FORMAT_ARGB32,
						    size[0],
						    size[1],
						    stride);
}

GLuint
GLSurface::Texture ()
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

#if !USE_EGL
	if (data) {
		g_free (data);
		data = NULL;
	}
#endif

	return texture;
}

void
GLSurface::AllocYUV ()
{
	if (!textureYUV[0]) {
		GLfloat borderY[] = { 0.0625f, 0.0625f, 0.0625f, 0.0625f };
		GLfloat borderUV[] = { 0.5f , 0.5f, 0.5f, 0.5f };
		int     i;

		glGenTextures (3, textureYUV);

		glBindTexture (GL_TEXTURE_2D, textureYUV[0]);
		glTexImage2D (GL_TEXTURE_2D,
			      0,
			      GL_LUMINANCE,
			      size[0],
			      size[1],
			      0,
			      GL_LUMINANCE,
			      GL_UNSIGNED_BYTE,
			      NULL);
		glTexParameterfv (GL_TEXTURE_2D,
				  GL_TEXTURE_BORDER_COLOR,
				  borderY);
		for (i = 1; i < 3; i++) {
			glBindTexture (GL_TEXTURE_2D, textureYUV[i]);
			glTexImage2D (GL_TEXTURE_2D,
				      0,
				      GL_LUMINANCE,
				      size[0] / 2,
				      size[1] / 2,
				      0,
				      GL_LUMINANCE,
				      GL_UNSIGNED_BYTE,
				      NULL);
			glTexParameterfv (GL_TEXTURE_2D,
					  GL_TEXTURE_BORDER_COLOR,
					  borderUV);
		}
		glBindTexture (GL_TEXTURE_2D, 0);
	}
}

GLuint
GLSurface::TextureY ()
{
	return textureYUV[0];
}

GLuint
GLSurface::TextureU ()
{
	return textureYUV[1];
}

GLuint
GLSurface::TextureV ()
{
	return textureYUV[2];
}

GLsizei
GLSurface::Width ()
{
	return size[0];
}

GLsizei
GLSurface::Height ()
{
	return size[1];
}

};
