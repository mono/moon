/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-gallium-egl.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#define __MOON_GALLIUM__

#include "surface-gallium-egl.h"

#ifdef CLAMP
#undef CLAMP
#endif
#include "util/u_inlines.h"

namespace Moonlight {

EGLGalliumSurface::EGLGalliumSurface (native_display *ndpy,
				      EGLNativeWindowType win,
				      const native_config *nconf,
                                      int att) :
	GalliumSurface ()
{
	nsurface = (*ndpy->create_window_surface) (ndpy, win, nconf);
	g_assert (nsurface);
	nsurface->user_data = this;

	attachment = NATIVE_ATTACHMENT_BACK_LEFT;
	need_validate = true;
}

EGLGalliumSurface::~EGLGalliumSurface ()
{
	(*nsurface->destroy) (nsurface);
}

pipe_resource *
EGLGalliumSurface::Texture ()
{
	MaybeValidate ();

	return GalliumSurface::Texture ();
}

pipe_sampler_view *
EGLGalliumSurface::SamplerView ()
{
	MaybeValidate ();

	return GalliumSurface::SamplerView ();
}

void
EGLGalliumSurface::Validate ()
{
	struct pipe_resource *textures[NUM_NATIVE_ATTACHMENTS];

	memset (textures, 0, sizeof (textures));

	pipe_resource_reference (&resource, NULL);

	if (!(*nsurface->validate) (nsurface,
				    1 << attachment,
				    NULL,
				    textures,
				    NULL,
				    NULL))
		g_warning ("surface validation failed");

	pipe_resource_reference (&resource, textures[attachment]);
	pipe_resource_reference (&textures[attachment], NULL);
}

void
EGLGalliumSurface::MaybeValidate ()
{
	if (need_validate) {
		Validate ();
		need_validate = false;
	}
}

void
EGLGalliumSurface::Invalidate ()
{
	need_validate = true;
}

void
EGLGalliumSurface::Flush ()
{
	(*nsurface->flush_frontbuffer) (nsurface);
}

void
EGLGalliumSurface::SwapBuffers ()
{
	(*nsurface->swap_buffers) (nsurface);
}

void
EGLGalliumSurface::InvalidSurface (native_display *ndpy,
				   native_surface *nsurf,
				   unsigned int   seq_num)
{
	EGLGalliumSurface *esurf = (EGLGalliumSurface *) nsurf->user_data;

	esurf->Invalidate ();
}

};
