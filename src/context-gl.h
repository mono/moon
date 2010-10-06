/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context-gl.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_CONTEXT_GL_H__
#define __MOON_CONTEXT_GL_H__

#include "context.h"
#include "surface-gl.h"

#define MAX_CONVOLVE_SIZE 32
#define MIN_GL_VERSION 2.0

namespace Moonlight {

class GLContext : public Context {
public:
	GLContext (MoonSurface *surface);
	virtual ~GLContext ();

	void Push (Group extents);

	void Clear (Color *color);

	void Project (MoonSurface  *src,
		      const double *matrix,
		      double       alpha,
		      double       x,
		      double       y);

	void Blur (MoonSurface *src,
		   double      radius,
		   double      x,
		   double      y);

	void DropShadow (MoonSurface *src,
			 double      dx,
			 double      dy,
			 double      radius,
			 Color       *color,
			 double      x,
			 double      y);

	void ShaderEffect (MoonSurface *src,
			   PixelShader *shader,
			   Brush       **sampler,
			   int         *sampler_mode,
			   int         n_sampler,
			   Color       *constant,
			   int         n_constant,
			   int         *ddxUvDdyUvPtr,
			   double      x,
			   double      y);

	void Flush ();

protected:
	virtual void SetFramebuffer ();
	virtual void SetScissor ();
	void SetViewport ();

	void SetupVertexData ();
	virtual void SetupVertexData (const double *matrix,
				      double       x,
				      double       y,
				      double       width,
				      double       height);

	void InitMatrix (double *out);
	void TransformMatrix (double *out, const double *matrix);
	static GLuint CreateShader (GLenum       shaderType,
				    GLsizei      count,
				    const GLchar **str);
	GLuint GetProjectShader (double alpha);
	GLuint GetConvolveShader (unsigned size);
	GLuint GetDropShadowShader (unsigned size);
	GLuint GetEffectShader (PixelShader *ps);

	float vertices[4][4];
	float texcoords[4][4];

	GLuint framebuffer;

	GLuint project_fs[2];
	GLuint convolve_fs[MAX_CONVOLVE_SIZE + 1];
	GLuint dropshadow_fs[MAX_CONVOLVE_SIZE + 1];

	GHashTable *effect_fs;
};

};

#endif /* __MOON_CONTEXT_GL_H__ */
