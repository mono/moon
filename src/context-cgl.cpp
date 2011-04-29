/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context-glx.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "context-cgl.h"

namespace Moonlight {

CGLContext::CGLContext (CGLSurface *surface) : OpenGLContext (surface)
{
	this->context = surface->GetContext ();
}

CGLContext::~CGLContext ()
{
	if (context)
		CGLDestroyContext (context);
}

bool
CGLContext::Initialize ()
{
	CGLSetCurrentContext (context);

	return OpenGLContext::Initialize ();
}

void
CGLContext::BlitVUY2 (unsigned char *data)
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	OpenGLSurface *dst = (OpenGLSurface *) ms;
	int         size[] = { dst->Width (), dst->Height () };
	GLuint texture = dst->Texture ();

	// no support for clipping
	g_assert (GetClip () == r);

	// no support for blit to drawable at the moment
	g_assert (!dst->HasDrawable ());

	// mark target as initialized
	target->SetInit (ms);

	glBindTexture (GL_TEXTURE_2D, texture);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, size [0], size [1], 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, data);
	glBindTexture (GL_TEXTURE_2D, 0);

	ms->unref ();
}

};
