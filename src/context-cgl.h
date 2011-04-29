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

#include "context-opengl.h"
#include "surface-cgl.h"

namespace Moonlight {

class MOON_API CGLContext : public OpenGLContext {
public:
	CGLContext (CGLSurface *surface);
	virtual ~CGLContext ();

	bool Initialize ();

	void BlitVUY2 (unsigned char *data);

private:
	CGLContextObj context;
};

};

#endif /* __MOON_CONTEXT_CGL_H__ */
