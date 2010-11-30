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
	size[0] = 0;
	size[1] = 0;
	texture = 0;
	data    = NULL;
}

GLSurface::GLSurface (GLsizei width, GLsizei height)
{
	size[0] = width;
	size[1] = height;
	texture = 0;
	data    = NULL;
}

GLSurface::~GLSurface ()
{
	if (data)
		g_free (data);

	if (texture)
		glDeleteTextures (1, &texture);
}

cairo_surface_t *
GLSurface::Cairo ()
{
	int stride = size[0] * 4;

	if (!data) {
		if (texture) {
			data = (unsigned char *) g_malloc (stride * size[1]);

			glBindTexture (GL_TEXTURE_2D, texture);
			glGetTexImage (GL_TEXTURE_2D,
				       0,
				       GL_BGRA,
				       GL_UNSIGNED_BYTE,
				       data);
			glBindTexture (GL_TEXTURE_2D, 0);
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

	if (data) {
		g_free (data);
		data = NULL;
	}

	return texture;
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
