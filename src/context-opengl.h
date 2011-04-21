/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context-opengl.h
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_CONTEXT_OPENGL_H__
#define __MOON_CONTEXT_OPENGL_H__

#include "context-gl.h"
#include "surface-opengl.h"

namespace Moonlight {

class MOON_API OpenGLContext : public GLContext {
public:
	OpenGLContext (OpenGLSurface *surface);
	virtual ~OpenGLContext ();

	bool Initialize ();

	void Push (Group extents);
	cairo_t *Push (Cairo extents);
	Rect Pop (MoonSurface **ref);

	virtual void Blit (unsigned char *data,
			   int           stride);

	virtual void BlitYV12 (unsigned char *data[],
			       int           stride[]);

	virtual void Paint (Color *color);

	virtual void Paint (MoonSurface *src,
			    double      alpha,
			    double      x,
			    double      y);

	virtual void Project (MoonSurface  *src,
			      const double *matrix,
			      double       alpha,
			      double       x,
			      double       y);

	virtual void Flush ();

protected:
	void SetupVertexData (double x,
			      double y,
			      double width,
			      double height);
	void SetupVertexData (const double *matrix,
			      double       x,
			      double       y,
			      double       width,
			      double       height);
	void SetupTexCoordData (const double *matrix,
				double       du,
				double       dv);
	void SetupTexUnit (GLenum target, GLint texture);
	void SetFramebuffer ();
	void SetScissor ();
	gboolean HasDrawable ();
	void SyncDrawable ();
	Rect GroupBounds (Group extents);

	GLint maxTextureSize;
};

};

#endif /* __MOON_CONTEXT_OPENGL_H__ */
