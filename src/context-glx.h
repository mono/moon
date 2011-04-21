/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context-glx.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_CONTEXT_GLX_H__
#define __MOON_CONTEXT_GLX_H__

#include "context-opengl.h"
#include "surface-glx.h"

#ifdef __MOON_GLX__

namespace Moonlight {

class MOON_API GLXContext : public OpenGLContext {
public:
	GLXContext (GLXSurface *surface);
	virtual ~GLXContext ();

	bool Initialize ();

	void MakeCurrent ();

private:
	Display       *dpy;
	GLXDrawable   drawable;
	VisualID      vid;
	_XxGLXContext ctx;
};

};

#endif /* __MOON_GLX__ */

#endif /* __MOON_CONTEXT_GLX_H__ */
