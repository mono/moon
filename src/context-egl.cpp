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

#define __MOON_EGL__

#include "context-egl.h"
#include "projection.h"
#include "effect.h"
#include "region.h"

#define IS_TRANSLUCENT(x) (x * 255 < 254.5)

namespace Moonlight {

MoonEGLContext::MoonEGLContext (MoonEGLSurface *surface) : GLContext (surface)
{
	this->display = surface->GetEGLDisplay ();
	this->surface = surface->GetEGLSurface ();
}

MoonEGLContext::~MoonEGLContext ()
{
	if (context)
		eglDestroyContext (display, context);
}

bool
MoonEGLContext::Initialize ()
{
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLint numConfigs;
	EGLConfig config;

	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	context = eglCreateContext(display, config, NULL, context_attribs);

	if (eglMakeCurrent (display, surface, surface, context) == EGL_FALSE) {
		g_warning ("Failed to make MoonEGL context current");
		return false;
	}

	glGetIntegerv (GL_MAX_TEXTURE_SIZE, &maxTextureSize);

	if (maxTextureSize < 2048)
		g_warning ("OpenGL max texture size: %d", maxTextureSize);

	g_warning ("Moonlight: OpenGL vendor string: %s\n", glGetString (GL_VENDOR));
	g_warning ("Moonlight: OpenGL renderer string: %s\n", glGetString (GL_RENDERER));
	g_warning ("Moonlight: OpenGL version string: %s\n", glGetString (GL_VERSION));

	return true;
}

void
MoonEGLContext::MakeCurrent ()
{
	g_assert (context);
	eglMakeCurrent (display, surface, surface, context);
}

void
MoonEGLContext::ForceCurrent ()
{
	if (eglGetCurrentContext () != context)
		MakeCurrent ();
}

const char *
MoonEGLContext::ProgramPrecisionString ()
{
	return "precision mediump float;";
}

void
MoonEGLContext::SetupVertexData (const double *matrix,
				 double       x,
				 double       y,
				 double       width,
				 double       height)
{
	Target          *target = Top ()->GetTarget ();
	MoonSurface     *ms;
	Rect            r = target->GetData (&ms);
	MoonEGLSurface  *dst = (MoonEGLSurface *) ms;
	double          dx = 2.0 / dst->Width ();
	double          dy = 2.0 / dst->Height ();
	double          p[4][4];
	int             i;

	p[0][0] = x;
	p[0][1] = y;
	p[0][2] = 0.0;
	p[0][3] = 1.0;

	p[1][0] = x + width;
	p[1][1] = y;
	p[1][2] = 0.0;
	p[1][3] = 1.0;

	p[2][0] = x + width;
	p[2][1] = y + height;
	p[2][2] = 0.0;
	p[2][3] = 1.0;

	p[3][0] = x;
	p[3][1] = y + height;
	p[3][2] = 0.0;
	p[3][3] = 1.0;

	if (matrix) {
		for (i = 0; i < 4; i++)
			Matrix3D::TransformPoint (p[i], matrix, p[i]);
	}

	for (i = 0; i < 4; i++) {
		vertices[i][0] = p[i][0] * dx - p[i][3];
		vertices[i][1] = p[i][1] * dy - p[i][3];
		vertices[i][2] = -p[i][2];
		vertices[i][3] = p[i][3];
	}

	if (dst->GetEGLDisplay ()) {
		int i;

		for (i = 0; i < 4; i++) {
			GLfloat v = vertices[i][1] + vertices[i][3];

			vertices[i][1] = vertices[i][3] - v;
		}
	}

	ms->unref ();
}

gboolean
MoonEGLContext::HasDrawable ()
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	MoonEGLSurface  *dst = (MoonEGLSurface *) ms;
	gboolean    status = FALSE;

	if (dst->GetEGLDisplay () || dst->HasTexture ())
		status = TRUE;

	ms->unref ();

	return status;
}

void
MoonEGLContext::SyncDrawable ()
{
	Target      *target = Top ()->GetTarget ();
	Target      *cairo = target->GetCairoTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	MoonEGLSurface  *dst = (MoonEGLSurface  *) ms;

	// clear target contents
	if (!target->GetInit ()) {
		if (!dst->GetEGLDisplay ())
			GLContext::SetFramebuffer ();

		glClearColor (0.0, 0.0, 0.0, 0.0);
		glClear (GL_COLOR_BUFFER_BIT);

		// mark target contents as initialized
		target->SetInit (ms);
	}

	// initialize target contents with surface
	if (target->GetInit () != ms) {
		MoonEGLSurface *src = (MoonEGLSurface  *) target->GetInit ();
		GLuint     texture0 = src->Texture ();
		GLuint     program = GetProjectProgram (1.0, 0);
		GLsizei    width0 = src->Width ();
		GLsizei    height0 = src->Height ();

		if (!dst->GetEGLDisplay ())
			GLContext::SetFramebuffer ();

		SetViewport ();

		glUseProgram (program);

		SetupVertexData (NULL, 0, 0, width0, height0);
		GLContext::SetupTexCoordData ();

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
		MoonEGLSurface  *src = (MoonEGLSurface  *) mSrc;
		GLuint      texture0 = src->Texture ();
		GLuint      program = GetProjectProgram (1.0, 0);
		GLsizei     width0 = src->Width ();
		GLsizei     height0 = src->Height ();

		if (!dst->GetEGLDisplay ())
			GLContext::SetFramebuffer ();

		SetViewport ();

		glUseProgram (program);

		SetupVertexData (NULL,
				 rSrc.x - r.x,
				 rSrc.y - r.y,
				 width0,
				 height0);
		GLContext::SetupTexCoordData ();

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
MoonEGLContext::GroupBounds (Group extents)
{
	if (extents.r.width  > maxTextureSize ||
	    extents.r.height > maxTextureSize)
		return Rect ();

	return extents.r;
}

void
MoonEGLContext::Push (Group extents)
{
	Rect r = GroupBounds (extents);

	if (!r.IsEmpty ()) {
		cairo_matrix_t matrix;
		MoonEGLSurface     *surface = new MoonEGLSurface (r.width, r.height);
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
MoonEGLContext::Push (Cairo extents)
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
			MoonEGLSurface *surface = new MoonEGLSurface (box.width,
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
MoonEGLContext::Pop (MoonSurface **ref)
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
MoonEGLContext::SetFramebuffer ()
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	MoonEGLSurface  *dst = (MoonEGLSurface *) ms;

	SyncDrawable ();

	if (!dst->GetEGLDisplay ())
		GLContext::SetFramebuffer ();

	ms->unref ();
}

void
MoonEGLContext::SetScissor ()
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	MoonEGLSurface  *dst = (MoonEGLSurface *) ms;
	Rect        clip;

	Top ()->GetClip (&clip);

	clip.x -= r.x;
	clip.y -= r.y;

	if (dst->GetEGLDisplay ()) {
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
MoonEGLContext::Clear (Color *color)
{
	ForceCurrent ();

	GLContext::Clear (color);
}

void
MoonEGLContext::Blit (unsigned char *data,
		  int           stride)
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	MoonEGLSurface  *dst = (MoonEGLSurface *) ms;

	ForceCurrent ();

	// no support for blit to drawable at the moment
	g_assert (!dst->GetEGLDisplay ());

	// mark target as initialized
	target->SetInit (ms);

	GLContext::Blit (data, stride);

	ms->unref ();
}

void
MoonEGLContext::BlitYV12 (unsigned char *data[],
		     int           stride[])
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	MoonEGLSurface  *dst = (MoonEGLSurface *) ms;

	ForceCurrent ();

	// no support for blit to drawable at the moment
	g_assert (!dst->GetEGLDisplay ());

	// mark target as initialized
	target->SetInit (ms);

	GLContext::BlitYV12 (data, stride);

	ms->unref ();
}

void
MoonEGLContext::Paint (Color *color)
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	Rect        clip;

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
MoonEGLContext::Paint (MoonSurface *src,
		       double      alpha,
		       double      x,
		       double      y)
{
	Target *target = Top ()->GetTarget ();
	Rect   r = target->GetData (NULL);
	Rect   clip;

	Top ()->GetClip (&clip);

	if (!target->GetInit () && !IS_TRANSLUCENT (alpha) && r == clip) {
		double m[16];
		int    x0, y0;

		GetMatrix (m);

		if (Matrix3D::IsIntegerTranslation (m, &x0, &y0)) {
			MoonEGLSurface *surface = (MoonEGLSurface *) src;
			Rect           r = Rect (x + x0,
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

	ForceCurrent ();

	GLContext::Paint (src, alpha, x, y);
}

void
MoonEGLContext::Project (MoonSurface  *src,
		     const double *matrix,
		     double       alpha,
		     double       x,
		     double       y)
{
	MoonEGLSurface *surface = (MoonEGLSurface *) src;

	if (!HasDrawable () && !surface->HasTexture ()) {
		double m[16];
		int    x0, y0;

		GetMatrix (m);
		Matrix3D::Multiply (m, matrix, m);

		// avoid GL rendering to target without previously
		// allocated hardware drawable
		if (Matrix3D::IsIntegerTranslation (m, &x0, &y0)) {
			Target          *target = Top ()->GetTarget ();
			Rect            r = Rect (x + x0,
						  y + y0,
						  surface->Width (),
						  surface->Height ());
			cairo_surface_t *cs = src->Cairo ();
			cairo_t         *cr = Push (Cairo (r));

			cairo_identity_matrix (cr);
			cairo_translate (cr, x0, y0);
			cairo_set_operator (cr, target->GetInit () ?
					    CAIRO_OPERATOR_OVER :
					    CAIRO_OPERATOR_SOURCE);
			cairo_set_source_surface (cr, cs, x, y);
			cairo_paint_with_alpha (cr, alpha);
			cairo_surface_destroy (cs);

			Context::Pop ();
			return;
		}
	}

	ForceCurrent ();

	GLContext::Project (src, matrix, alpha, x, y);
}

void
MoonEGLContext::Blur (MoonSurface *src,
		  double      radius,
		  double      x,
		  double      y)
{
	ForceCurrent ();

	GLContext::Blur (src, radius, x, y);
}

void
MoonEGLContext::DropShadow (MoonSurface *src,
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
MoonEGLContext::ShaderEffect (MoonSurface *src,
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
MoonEGLContext::Flush ()
{
	ForceCurrent ();
	SyncDrawable ();

	GLContext::Flush ();
}

};
