/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-glx.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <string.h>

#include "surface-cgl.h"

namespace Moonlight {

CGLSurface::CGLSurface (CGLContextObj context, GLsizei w, GLsizei h) : OpenGLSurface ()
{
	this->context = context;

	size[0] = w;
	size[1] = h;
}

};
