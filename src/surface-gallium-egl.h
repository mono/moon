/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * surface-gallium-egl.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __MOON_SURFACE_GALLIUM_EGL_H__
#define __MOON_SURFACE_GALLIUM_EGL_H__

#include "surface-gallium.h"

#ifdef __MOON_GALLIUM__

#include "common/native.h"

namespace Moonlight {

class EGLGalliumSurface : public GalliumSurface {
public:
	EGLGalliumSurface (native_display *ndpy,
			   EGLNativeWindowType win,
			   const native_config *nconf,
			   int att);
	virtual ~EGLGalliumSurface ();

	pipe_resource *Texture ();
	pipe_sampler_view *SamplerView ();
	void Invalidate ();
	void Flush ();
	void SwapBuffers ();

	static void InvalidSurface (native_display *ndpy,
				    native_surface *nsurf,
				    unsigned int   seq_num);

private:
	void Validate ();
	void MaybeValidate ();

	native_surface *nsurface;
	int attachment;
	bool need_validate;
};

};

#endif /* __MOON_GALLIUM__ */

#endif /* __MOON_SURFACE_GALLIUM_EGL_H__ */
