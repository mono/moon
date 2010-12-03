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

#define GL_GLEXT_PROTOTYPES
#define __MOON_GLX__

#include "context-glx.h"
#include "projection.h"
#include "effect.h"
#include "region.h"

#define IS_TRANSLUCENT(x) (x * 255 < 254.5)

namespace Moonlight {

GLXContext::GLXContext (GLXSurface *surface) : GLContext (surface)
{
	XVisualInfo templ, *visinfo;
	int         n;

	dpy = surface->GetDisplay ();
	drawable = surface->GetGLXDrawable ();
	templ.visualid = surface->GetVisualID ();
	visinfo = XGetVisualInfo (dpy, VisualIDMask, &templ, &n);

	g_assert (n == 1);

	ctx = glXCreateContext (dpy, visinfo, 0, True);
	glXMakeCurrent (dpy, drawable, ctx);

	XFree (visinfo);
}

GLXContext::~GLXContext ()
{
	glXDestroyContext (dpy, ctx);
}

void
GLXContext::ForceCurrent ()
{
	if (glXGetCurrentContext () != ctx)
		glXMakeCurrent (dpy, drawable, ctx);
}

void
GLXContext::SetupVertexData (const double *matrix,
			     double       x,
			     double       y,
			     double       width,
			     double       height)
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	GLXSurface  *dst = (GLXSurface *) ms;

	GLContext::SetupVertexData (matrix, x, y, width, height);

	if (dst->GetGLXDrawable ()) {
		int i;

		for (i = 0; i < 4; i++) {
			GLfloat v = vertices[i][1] + vertices[i][3];

			vertices[i][1] = vertices[i][3] - v;
		}
	}

	ms->unref ();
}

gboolean
GLXContext::HasDrawable ()
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	GLXSurface  *dst = (GLXSurface *) ms;
	gboolean    status = FALSE;

	if (dst->GetGLXDrawable () || dst->HasTexture ())
		status = TRUE;

	ms->unref ();

	return status;
}

void
GLXContext::SyncDrawable ()
{
	Target      *target = Top ()->GetTarget ();
	Target      *cairo = target->GetCairoTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	GLXSurface  *dst = (GLXSurface  *) ms;

	// clear target contents
	if (!target->GetInit ()) {
		if (!dst->GetGLXDrawable ())
			GLContext::SetFramebuffer ();

		glClearColor (0.0, 0.0, 0.0, 0.0);
		glClear (GL_COLOR_BUFFER_BIT);

		// mark target contents as initialized
		target->SetInit (ms);
	}

	// initialize target contents with surface
	if (target->GetInit () != ms) {
		GLXSurface *src = (GLXSurface  *) target->GetInit ();
		GLuint     texture0 = src->Texture ();
		GLuint     program = GetProjectProgram (1.0);
		GLsizei    width0 = src->Width ();
		GLsizei    height0 = src->Height ();

		if (!dst->GetGLXDrawable ())
			GLContext::SetFramebuffer ();

		SetViewport ();

		glUseProgram (program);

		SetupVertexData (NULL, 0, 0, width0, height0);

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
		GLXSurface  *src = (GLXSurface  *) mSrc;
		GLuint      texture0 = src->Texture ();
		GLuint      program = GetProjectProgram (1.0);
		GLsizei     width0 = src->Width ();
		GLsizei     height0 = src->Height ();

		if (!dst->GetGLXDrawable ())
			GLContext::SetFramebuffer ();

		SetViewport ();

		glUseProgram (program);

		SetupVertexData (NULL,
				 rSrc.x - r.x,
				 rSrc.y - r.y,
				 width0,
				 height0);

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

void
GLXContext::Push (Group extents)
{
	cairo_matrix_t matrix;
	Rect           r = extents.r.RoundOut ();
        GLXSurface     *surface = new GLXSurface (r.width, r.height);
        Target         *target = new Target (surface, extents.r);

	Top ()->GetMatrix (&matrix);

	// mark target contents as uninitialized
	target->SetInit (NULL);

	Stack::Push (new Context::Node (target, &matrix, &extents.r));

	target->unref ();
	surface->unref ();
}

cairo_t *
GLXContext::Push (Cairo extents)
{
	Target *target = Top ()->GetTarget ();
	Target *cairo = target->GetCairoTarget ();
	Rect   box;

	Top ()->GetClip (&box);
 
	box = box.Intersection (extents.r).RoundOut ();

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
			GLXSurface *surface = new GLXSurface (box.width,
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
GLXContext::Pop (MoonSurface **ref)
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
GLXContext::SetFramebuffer ()
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	GLXSurface  *dst = (GLXSurface *) ms;

	SyncDrawable ();

	if (!dst->GetGLXDrawable ())
		GLContext::SetFramebuffer ();

	ms->unref ();
}

void
GLXContext::SetScissor ()
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	GLXSurface  *dst = (GLXSurface *) ms;
	Rect        clip;

	Top ()->GetClip (&clip);

	clip.x -= r.x;
	clip.y -= r.y;

	if (dst->GetGLXDrawable ()) {
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
GLXContext::Clear (Color *color)
{
	ForceCurrent ();

	GLContext::Clear (color);
}

void
GLXContext::Blend (MoonSurface *src,
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
			GLXSurface *surface = (GLXSurface *) src;
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

	ForceCurrent ();

	GLContext::Blend (src, alpha, x, y);
}

void
GLXContext::Project (MoonSurface  *src,
		     const double *matrix,
		     double       alpha,
		     double       x,
		     double       y)
{
	GLXSurface *surface = (GLXSurface *) src;

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
GLXContext::Blur (MoonSurface *src,
		  double      radius,
		  double      x,
		  double      y)
{
	ForceCurrent ();

	GLContext::Blur (src, radius, x, y);
}

void
GLXContext::DropShadow (MoonSurface *src,
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
GLXContext::ShaderEffect (MoonSurface *src,
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
GLXContext::Flush ()
{
	ForceCurrent ();
	SyncDrawable ();

	GLContext::Flush ();
}

bool
GLXContext::CheckVersion ()
{
	ForceCurrent ();

	if (atof ((const char *) glGetString (GL_VERSION)) < MIN_GL_VERSION) {
		g_warning ("OpenGL version %s < %.1f",
			   glGetString (GL_VERSION),
			   MIN_GL_VERSION);
		return 0;
	}

	return 1;
}

};
