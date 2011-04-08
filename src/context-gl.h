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

class MOON_API GLContext : public Context {
public:
	GLContext (MoonSurface *surface);
	virtual ~GLContext ();

	void Push (Group extents);

	void Clear (Color *color);

	void Blit (unsigned char *data,
		   int           stride);

	void BlitYV12 (unsigned char *data[],
		       int           stride[]);

	void Paint (Color *color);

	void Paint (MoonSurface *src,
		    double      alpha,
		    double      x,
		    double      y);

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
	PFNGLCREATESHADERPROC glCreateShader;
	PFNGLSHADERSOURCEPROC glShaderSource;
	PFNGLCOMPILESHADERPROC glCompileShader;
	PFNGLGETSHADERIVPROC glGetShaderiv;
	PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
	PFNGLDELETESHADERPROC glDeleteShader;
	PFNGLCREATEPROGRAMPROC glCreateProgram;
	PFNGLATTACHSHADERPROC glAttachShader;
	PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
	PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
	PFNGLUNIFORM4FPROC glUniform4f;
	PFNGLUNIFORM4FVPROC glUniform4fv;
	PFNGLUNIFORM1IPROC glUniform1i;
	PFNGLLINKPROGRAMPROC glLinkProgram;
	PFNGLUSEPROGRAMPROC glUseProgram;
	PFNGLDELETEPROGRAMPROC glDeleteProgram;
	PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
	PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArray;
	PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArray;
	PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
	PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
	PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
	PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
	PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
	PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
	PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;

	virtual void SetFramebuffer ();
	virtual void SetScissor ();
	void SetViewport ();

	void SetupVertexData ();
	virtual void SetupVertexData (double x,
				      double y,
				      double width,
				      double height);

	void SetupTexCoordData ();
	void SetupTexCoordData (const double *matrix,
				double       du,
				double       dv);

	GLuint CreateShader (GLenum       shaderType,
			     GLsizei      count,
			     const GLchar **str);
	GLuint GetVertexShader ();
	GLuint GetBlendProgram ();
	GLuint GetProjectProgram (double alpha, unsigned yuv);
	GLuint GetConvolveProgram (unsigned size);
	GLuint GetDropShadowProgram (unsigned size);
	GLuint GetEffectProgram (PixelShader *ps);
	static const char *WritemaskToType (const char *writemask);

	float vertices[4][4];
	float texcoords[4][4];

	GLuint framebuffer;

	GLuint vs;
	GLuint blend_program;
	GLuint project_program[2][2];
	GLuint convolve_program[MAX_CONVOLVE_SIZE + 1];
	GLuint dropshadow_program[MAX_CONVOLVE_SIZE + 1];

	GHashTable *effect_program;
};

};

#endif /* __MOON_CONTEXT_GL_H__ */
