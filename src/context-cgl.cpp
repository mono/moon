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

#define __MOON_CGL__

#include "context-cgl.h"
#include "projection.h"
#include "effect.h"
#include "region.h"

#define IS_TRANSLUCENT(x) (x * 255 < 254.5)

namespace Moonlight {

CGLContext::CGLContext (CGLSurface *surface) : GLContext (surface)
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

	glGetIntegerv (GL_MAX_TEXTURE_SIZE, &maxTextureSize);

	if (maxTextureSize < 2048)
		g_warning ("OpenGL max texture size: %d", maxTextureSize);

	g_warning ("Moonlight: OpenGL vendor string: %s\n", glGetString (GL_VENDOR));
	g_warning ("Moonlight: OpenGL renderer string: %s\n", glGetString (GL_RENDERER));
	g_warning ("Moonlight: OpenGL version string: %s\n", glGetString (GL_VERSION));
	g_warning ("Moonlight: OpenGL version string: %s\n", glGetString (GL_EXTENSIONS));

	return true;
}

void
CGLContext::MakeCurrent ()
{
	g_assert (context);
	CGLSetCurrentContext (context);
}

void
CGLContext::ForceCurrent ()
{
	if (CGLGetCurrentContext () != context)
		MakeCurrent ();
}

const char *
CGLContext::ProgramPrecisionString ()
{
	return "";
}

void
CGLContext::SetupVertexData (double       x,
				 double       y,
				 double       width,
				 double       height)
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	CGLSurface  *dst = (CGLSurface *) ms;

	GLContext::SetupVertexData (x, y, width, height);

	if (dst->GetContext ()) {
		int i;

		for (i = 0; i < 4; i++) {
			GLfloat v = vertices[i][1] + vertices[i][3];

			vertices[i][1] = vertices[i][3] - v;
		}
	}

	ms->unref ();
}

gboolean
CGLContext::HasDrawable ()
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	CGLSurface  *dst = (CGLSurface *) ms;
	gboolean    status = FALSE;

	if (dst->GetContext () || dst->HasTexture ())
		status = TRUE;

	ms->unref ();

	return status;
}

void
CGLContext::SyncDrawable ()
{
	Target      *target = Top ()->GetTarget ();
	Target      *cairo = target->GetCairoTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	CGLSurface  *dst = (CGLSurface  *) ms;

	// clear target contents
	if (!target->GetInit ()) {
		if (!dst->GetContext ())
			GLContext::SetFramebuffer ();

		glClearColor (0.0, 0.0, 0.0, 0.0);
		glClear (GL_COLOR_BUFFER_BIT);

		// mark target contents as initialized
		target->SetInit (ms);
	}

	// initialize target contents with surface
	if (target->GetInit () != ms) {
		CGLSurface *src = (CGLSurface  *) target->GetInit ();
		GLuint     texture0 = src->Texture ();
		GLuint     program = GetProjectProgram (1.0, 0);
		GLsizei    width0 = src->Width ();
		GLsizei    height0 = src->Height ();

		if (!dst->GetContext ())
			GLContext::SetFramebuffer ();

		SetViewport ();

		glUseProgram (program);

		SetupVertexData (0, 0, width0, height0);
		SetupTexCoordData ();

		glVertexAttribPointer (0, 4,
				       GL_FLOAT, GL_FALSE, 0,
				       vertices);
		glVertexAttribPointer (1, 4,
				       GL_FLOAT, GL_FALSE, 0,
				       texcoords);

		glBindTexture (GL_TEXTURE_2D, texture0);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				 GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				 GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
				 GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				 GL_CLAMP_TO_EDGE);
		glUniform1i (glGetUniformLocation (program, "sampler0"), 0);

		glEnableVertexAttribArray (0);
		glEnableVertexAttribArray (1);

		glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

		glDisableVertexAttribArray (1);
		glDisableVertexAttribArray (0);

		glBindTexture (GL_TEXTURE_2D, 0);

		glUseProgram (0);

		glBindFramebuffer (GL_FRAMEBUFFER, 0);

		// mark target contents as initialized
		target->SetInit (ms);
	}

	// render any cairo contents onto target
	if (cairo) {
		MoonSurface *mSrc;
		Rect        rSrc = cairo->GetData (&mSrc);
		CGLSurface  *src = (CGLSurface  *) mSrc;
		GLuint      texture0 = src->Texture ();
		GLuint      program = GetProjectProgram (1.0, 0);
		GLsizei     width0 = src->Width ();
		GLsizei     height0 = src->Height ();

		if (!dst->GetContext ())
			GLContext::SetFramebuffer ();

		SetViewport ();

		glUseProgram (program);

		SetupVertexData (rSrc.x - r.x, rSrc.y - r.y, width0, height0);
		SetupTexCoordData ();

		glVertexAttribPointer (0, 4,
				       GL_FLOAT, GL_FALSE, 0,
				       vertices);
		glVertexAttribPointer (1, 4,
				       GL_FLOAT, GL_FALSE, 0,
				       texcoords);

		glBindTexture (GL_TEXTURE_2D, texture0);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				 GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				 GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
				 GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				 GL_CLAMP_TO_EDGE);
		glUniform1i (glGetUniformLocation (program, "sampler0"), 0);

		glEnableVertexAttribArray (0);
		glEnableVertexAttribArray (1);

		glEnable (GL_BLEND);
		glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

		glDisable (GL_BLEND);

		glDisableVertexAttribArray (1);
		glDisableVertexAttribArray (0);

		glBindTexture (GL_TEXTURE_2D, 0);

		glUseProgram (0);

		glBindFramebuffer (GL_FRAMEBUFFER, 0);

		mSrc->unref ();

		target->SetCairoTarget (NULL);
	}

	ms->unref ();
}

Rect
CGLContext::GroupBounds (Group extents)
{
	if (extents.r.width  > maxTextureSize ||
	    extents.r.height > maxTextureSize)
		return Rect ();

	return extents.r;
}

void
CGLContext::Push (Group extents)
{
	Rect r = GroupBounds (extents);

	if (!r.IsEmpty ()) {
		cairo_matrix_t matrix;
		CGLSurface     *surface = new CGLSurface (r.width, r.height);
		Target         *target = new Target (surface, extents.r);

		Top ()->GetMatrix (&matrix);

		// mark target contents as uninitialized
		target->SetInit (NULL);

		Stack::Push (new Context::Node (target, &matrix, &extents.r));

		target->unref ();
		surface->unref ();
	}
	else {
		Context::Push (Clip ());
	}
}

cairo_t *
CGLContext::Push (Cairo extents)
{
	Target *target = Top ()->GetTarget ();
	Target *cairo = target->GetCairoTarget ();
	Rect   box;

	Top ()->GetClip (&box);

	box = box.Intersection (extents.r);

	if (box.IsEmpty ())
		return Context::Push (extents);

	if (cairo) {
		Rect   r = cairo->GetData (NULL);
		Region *region = new Region (r);

		if (region->RectIn (box) != CAIRO_REGION_OVERLAP_IN) {
			ForceCurrent ();
			SyncDrawable ();
		}

		delete region;
	}

	if (!target->GetCairoTarget ()) {
		MoonSurface *ms;
		Rect        r = target->GetData (&ms);

		if (HasDrawable ()) {
			CGLSurface *surface = new CGLSurface (box.width,
							      box.height);
			Target     *cairo = new Target (surface, box);

			target->SetCairoTarget (cairo);

			cairo->unref ();
			surface->unref ();
		}
		else {
			// mark target contents as initialized
			target->SetInit (ms);
		}

		ms->unref ();
	}

	return Context::Push (extents);
}

Rect
CGLContext::Pop (MoonSurface **ref)
{
	Context::Node *prev = (Context::Node *) Top ()->prev;

	g_assert (prev);

	if (Top ()->GetTarget () != prev->GetTarget ()) {
		Target      *target = Top ()->GetTarget ();
		MoonSurface *init = target->GetInit ();
		MoonSurface *ms;
		Rect        r = target->GetData (&ms);
		MoonSurface *data = init != ms ? init : NULL;

		ms->unref ();
		ForceCurrent ();

		// return reference to initial state surface instead
		// of the target surface itself
		if (data) {
			Node *node = (Node *) Stack::Pop ();

			*ref = data->ref ();
			delete node;
			return r;
		}
		else {
			SyncDrawable ();
		}
	}

	return GLContext::Pop (ref);
}

void
CGLContext::SetFramebuffer ()
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	CGLSurface  *dst = (CGLSurface *) ms;

	SyncDrawable ();

	if (!dst->GetContext ())
		GLContext::SetFramebuffer ();

	ms->unref ();
}

void
CGLContext::SetScissor ()
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	CGLSurface  *dst = (CGLSurface *) ms;
	Rect        clip;

	Top ()->GetClip (&clip);

	clip.x -= r.x;
	clip.y -= r.y;

	if (dst->GetContext ()) {
		glScissor (clip.x,
			   dst->Height () - (clip.y + clip.height),
			   clip.width,
			   clip.height);
	}
	else {
		GLContext::SetScissor ();
	}

	ms->unref ();
}

void
CGLContext::Clear (Color *color)
{
	ForceCurrent ();

	GLContext::Clear (color);
}

void
CGLContext::Blit (unsigned char *data,
		  int           stride)
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	CGLSurface  *dst = (CGLSurface *) ms;
	Rect        clip;

	ForceCurrent ();

	// no support for clipping
	g_assert (GetClip () == r);

	// no support for blit to drawable at the moment
	g_assert (!dst->GetContext ());

	// mark target as initialized
	target->SetInit (ms);

	glPixelStorei (GL_UNPACK_ROW_LENGTH,
			PixelRowLength (stride, dst->Width (), 4));
	glPixelStorei (GL_UNPACK_ALIGNMENT, PixelAlignment (stride));
	glBindTexture (GL_TEXTURE_2D, dst->Texture ());
	glTexSubImage2D (GL_TEXTURE_2D,
			0,
			0,
			0,
			dst->Width (),
			dst->Height (),
			GL_BGRA,
			GL_UNSIGNED_BYTE,
			data);
	glBindTexture (GL_TEXTURE_2D, 0);
	glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);

	ms->unref ();
}

void
CGLContext::BlitVUY2 (unsigned char *data)
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	CGLSurface  *dst = (CGLSurface *) ms;
	int         size[] = { dst->Width (), dst->Height () };
	GLuint texture = dst->Texture ();

	ForceCurrent ();

	// no support for clipping
	g_assert (GetClip () == r);

	// no support for blit to drawable at the moment
	g_assert (!dst->GetContext ());

	// mark target as initialized
	target->SetInit (ms);

	glBindTexture (GL_TEXTURE_2D, texture);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, size [0], size [1], 0, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE, data);
	glBindTexture (GL_TEXTURE_2D, 0);

	ms->unref ();
}

void
CGLContext::BlitYV12 (unsigned char *data[],
		     int           stride[])
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	CGLSurface  *dst = (CGLSurface *) ms;
	int         size[] = { dst->Width (), dst->Height () };
	int         width[] = { size[0], size[0] / 2, size[0] / 2 };
	int         height[] = { size[1], size[1] / 2, size[1] / 2 };
	int         i;

	ForceCurrent ();

	// no support for clipping
	g_assert (GetClip () == r);

	// no support for blit to drawable at the moment
	g_assert (!dst->GetContext ());

	// mark target as initialized
	target->SetInit (ms);

	for (i = 0; i < 3; i++) {
		glPixelStorei (GL_UNPACK_ROW_LENGTH,
		PixelRowLength (stride[i], width[i], 1));
		glPixelStorei (GL_UNPACK_ALIGNMENT, PixelAlignment (stride[i]));
		glBindTexture (GL_TEXTURE_2D, dst->TextureYUV (i));
		glTexSubImage2D (GL_TEXTURE_2D,
					0,
					0,
					0,
					width[i],
					height[i],
					GL_LUMINANCE,
					GL_UNSIGNED_BYTE,
					data[i]);
	}
	glBindTexture (GL_TEXTURE_2D, 0);
	glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);

	ms->unref ();
}

void
CGLContext::Paint (Color *color)
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	Rect        clip;

	// avoid GL rendering to target without previously
	// allocated hardware drawable	
	if (!HasDrawable ()) {
		Context::Paint (color);
		ms->unref ();
		return;
	}

	Top ()->GetClip (&clip);

	if (!target->GetInit () && r == clip) {
		// mark target as initialized
		target->SetInit (ms);
	}

	ForceCurrent ();

	GLContext::Paint (color);

	ms->unref ();
}

void
CGLContext::Paint (MoonSurface *src,
		       double      alpha,
		       double      x,
		       double      y)
{
	CGLContext::Project (src, NULL, alpha, x, y);
}

void
CGLContext::Project (MoonSurface  *src,
		     const double *matrix,
		     double       alpha,
		     double       x,
		     double       y)
{
	CGLSurface *surface = (CGLSurface *) src;
	Target     *target = Top ()->GetTarget ();
	Rect       r = target->GetData (NULL);
	Rect       clip = GetClip ();
	double     m[16];

	if (!target->GetInit () && !IS_TRANSLUCENT (alpha) && r == clip) {
		int x0, y0;

		GetMatrix (m);
		if (matrix)
			Matrix3D::Multiply (m, matrix, m);

		if (Matrix3D::IsIntegerTranslation (m, &x0, &y0)) {
			CGLSurface *surface = (CGLSurface *) src;
			Rect       r = Rect (x + x0,
						y + y0,
						surface->Width (),
						surface->Height ());

			// matching dimensions and no transformation allow us
			// to set source as initial state of target surface when
			// it is not already initialized.

			if (r == clip) {
				target->SetInit (src);
				return;
			}
		}
	}

	if (!HasDrawable () && !surface->HasTexture ()) {
		int x0, y0;

		GetMatrix (m);
		if (matrix)
			Matrix3D::Multiply (m, matrix, m);

		// avoid GL rendering to target without previously
		// allocated hardware drawable
		if (Matrix3D::IsIntegerTranslation (m, &x0, &y0)) {
			cairo_matrix_t m;

			cairo_matrix_init_translate (&m, x0, y0);

			Context::Push (Context::AbsoluteTransform (m));
			Context::Paint (src, alpha, x, y);
			Context::Pop ();
			return;
		}
	}

	GetDeviceMatrix (m);
	if (matrix)
		Matrix3D::Multiply (m, matrix, m);

	if (!GetSourceMatrix (m, m, x, y))
		return;

	ForceCurrent ();

	GLContext::Paint (src, m, alpha);
}

void
CGLContext::Blur (MoonSurface *src,
		  double      radius,
		  double      x,
		  double      y)
{
	ForceCurrent ();

	GLContext::Blur (src, radius, x, y);
}

void
CGLContext::DropShadow (MoonSurface *src,
			double      dx,
			double      dy,
			double      radius,
			Color       *color,
			double      x,
			double      y)
{
	ForceCurrent ();

	GLContext::DropShadow (src, dx, dy, radius, color, x, y);
}

void
CGLContext::ShaderEffect (MoonSurface *src,
			  PixelShader *shader,
			  Brush       **sampler,
			  int         *sampler_mode,
			  int         n_sampler,
			  Color       *constant,
			  int         n_constant,
			  int         *ddxUvDdyUvPtr,
			  double      x,
			  double      y)
{
	ForceCurrent ();

	GLContext::ShaderEffect (src,
				 shader,
				 sampler, sampler_mode, n_sampler,
				 constant, n_constant,
				 ddxUvDdyUvPtr,
				 x, y);
}

void
CGLContext::Flush ()
{
	ForceCurrent ();
	SyncDrawable ();

	GLContext::Flush ();
}

};
