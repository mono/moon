/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context-glx.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_CONTEXT_CGL_H__
#define __MOON_CONTEXT_CGL_H__

#include "context-gl.h"
#include "surface-cgl.h"

#ifdef __MOON_CGL__

namespace Moonlight {

class MOON_API CGLContext : public GLContext {
public:
	CGLContext (CGLSurface *surface);
	virtual ~CGLContext ();

	bool Initialize ();

	void MakeCurrent ();

	void Push (Group extents);
	cairo_t *Push (Cairo extents);
	Rect Pop (MoonSurface **ref);

	void Clear (Color *color);

	void Blit (unsigned char *data,
		   int           stride);

	void BlitVUY2 (unsigned char *data);

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
	void SetupVertexData (double x,
			      double y,
			      double width,
			      double height);
	const char *ProgramPrecisionString ();
	void SetFramebuffer ();
	void SetScissor ();
	void ForceCurrent ();
	gboolean HasDrawable ();
	void SyncDrawable ();
	Rect GroupBounds (Group extents);

private:
	CGLContextObj context;
	GLint maxTextureSize;
};

};

#endif /* __MOON_CGL__ */

#endif /* __MOON_CONTEXT_GLX_H__ */
