/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context-glx.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_CONTEXT_EGL_H__
#define __MOON_CONTEXT_EGL_H__

#include "context-gl.h"
#include "surface-egl.h"

#ifdef __MOON_EGL__

namespace Moonlight {

class MOON_API MoonEGLContext : public GLContext {
public:
	MoonEGLContext (MoonEGLSurface *surface);
	virtual ~MoonEGLContext ();

	bool Initialize ();

	void MakeCurrent ();

	void Push (Group extents);
	cairo_t *Push (Cairo extents);
	Rect Pop (MoonSurface **ref);

	void Clear (Color *color);

	void Blit (unsigned char *data,
		   int           stride);

	void BlitYV12 (unsigned char *data[],
		       int           stride[]);

	void Blend (Color *color);

	void Blend (MoonSurface *src,
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
	void SetupVertexData (const double *matrix,
			      double       x,
			      double       y,
			      double       width,
			      double       height);
	void SetFramebuffer ();
	void SetScissor ();
	void ForceCurrent ();
	gboolean HasDrawable ();
	void SyncDrawable ();
	Rect GroupBounds (Group extents);

private:
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	GLint maxTextureSize;
};

};

#endif /* __MOON_EGL__ */

#endif /* __MOON_CONTEXT_GLX_H__ */
