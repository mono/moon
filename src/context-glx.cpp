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

#define __MOON_GLX__

#include "context-glx.h"
#include "projection.h"
#include "effect.h"
#include "region.h"

#define IS_TRANSLUCENT(x) (x * 255 < 254.5)

#define GETPROCADDR(type, name)						\
	(name) = (type) glXGetProcAddressARB ((const GLubyte*) # name)

namespace Moonlight {

GLXContext::GLXContext (GLXSurface *surface) : GLContext (surface)
{
	dpy = surface->GetDisplay ();
	drawable = surface->GetGLXDrawable ();
	vid = surface->GetVisualID ();
	ctx = (_XxGLXContext) 0;
}

GLXContext::~GLXContext ()
{
	if (ctx)
		glXDestroyContext (dpy, ctx);
}

bool
GLXContext::Initialize ()
{
	XVisualInfo templ, *visinfo;
	int         n;
	const char  *requiredExtension[] = {
		"GL_EXT_framebuffer_object",
		"GL_ARB_texture_non_power_of_two",
		"GL_ARB_shading_language_100"
	};

	templ.visualid = vid;
	visinfo = XGetVisualInfo (dpy, VisualIDMask, &templ, &n);
	if (visinfo == NULL) {
		g_warning ("Found no visuals matching VisualID 0x%x, "
			   "disabling GLX", (int) vid);
		return false;
	}

	GLXSurface::X11ErrorTrapPush (dpy);
	ctx = glXCreateContext (dpy, visinfo, 0, True);
	XFree (visinfo);

	if (GLXSurface::X11ErrorTrapPop (dpy) != Success) {
		g_warning ("Failed to create GLX context for VisualID: 0x%x",
			   (int) vid);
		return false;
	}

	GLXSurface::X11ErrorTrapPush (dpy);
	glXMakeCurrent (dpy, drawable, ctx);
	if (GLXSurface::X11ErrorTrapPop (dpy) != Success) {
		g_warning ("Failed to make GLX context current for drawable: "
			   "0x%x", (int) drawable);
		return false;
	}

	const char *version = (const char *) glGetString (GL_VERSION);

	if (!version) {
		g_warning ("glGetString returned NULL");
		return false;
	}
	
	if (atof (version) < MIN_GL_VERSION) {
		g_warning ("Insufficient OpenGL version: %s", version);
		return false;
	}

	const char *extensions = (const char *) glGetString (GL_EXTENSIONS);

	for (guint i = 0; i < G_N_ELEMENTS (requiredExtension); i++) {
		if (!strstr (extensions, requiredExtension[i])) {
			g_warning ("%s missing", requiredExtension[i]);
			return false;
		}
	}

	glGetIntegerv (GL_MAX_TEXTURE_SIZE, &maxTextureSize);

	if (maxTextureSize < 2048)
		g_warning ("OpenGL max texture size: %d", maxTextureSize);

	GETPROCADDR (PFNGLCREATESHADERPROC, glCreateShader);
	GETPROCADDR (PFNGLSHADERSOURCEPROC, glShaderSource);
	GETPROCADDR (PFNGLCOMPILESHADERPROC, glCompileShader);
	GETPROCADDR (PFNGLGETSHADERIVPROC, glGetShaderiv);
	GETPROCADDR (PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);
	GETPROCADDR (PFNGLDELETESHADERPROC, glDeleteShader);
	GETPROCADDR (PFNGLCREATEPROGRAMPROC, glCreateProgram);
	GETPROCADDR (PFNGLATTACHSHADERPROC, glAttachShader);
	GETPROCADDR (PFNGLBINDATTRIBLOCATIONPROC, glBindAttribLocation);
	GETPROCADDR (PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
	GETPROCADDR (PFNGLUNIFORM4FPROC, glUniform4f);
	GETPROCADDR (PFNGLUNIFORM4FVPROC, glUniform4fv);
	GETPROCADDR (PFNGLUNIFORM1IPROC, glUniform1i);
	GETPROCADDR (PFNGLLINKPROGRAMPROC, glLinkProgram);
	GETPROCADDR (PFNGLUSEPROGRAMPROC, glUseProgram);
	GETPROCADDR (PFNGLDELETEPROGRAMPROC, glDeleteProgram);
	GETPROCADDR (PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
	GETPROCADDR (PFNGLENABLEVERTEXATTRIBARRAYARBPROC,
		     glEnableVertexAttribArray);
	GETPROCADDR (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC,
		     glDisableVertexAttribArray);
	GETPROCADDR (PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers);
	GETPROCADDR (PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers);
	GETPROCADDR (PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);
	GETPROCADDR (PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);
	GETPROCADDR (PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers);
	GETPROCADDR (PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);
	GETPROCADDR (PFNGLCHECKFRAMEBUFFERSTATUSPROC,
		     glCheckFramebufferStatus);

	printf ("Moonlight: OpenGL vendor string: %s\n",
		glGetString (GL_VENDOR));
	printf ("Moonlight: OpenGL renderer string: %s\n",
		glGetString (GL_RENDERER));
	printf ("Moonlight: OpenGL version string: %s\n",
		glGetString (GL_VERSION));

	return true;
}

void
GLXContext::MakeCurrent ()
{
	g_assert (ctx);
	glXMakeCurrent (dpy, drawable, ctx);
}

void
GLXContext::ForceCurrent ()
{
	if (glXGetCurrentContext () != ctx)
		MakeCurrent ();
}

void
GLXContext::SetupVertexData (double x,
			     double y,
			     double width,
			     double height)
{
	Context::Target *target = Top ()->GetTarget ();
	MoonSurface     *ms;
	Rect            r = target->GetData (&ms);
	GLXSurface      *dst = (GLXSurface *) ms;
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

	for (i = 0; i < 4; i++) {
		vertices[i][0] = p[i][0] * dx - p[i][3];
		vertices[i][1] = p[i][1] * dy - p[i][3];
		vertices[i][2] = -p[i][2];
		vertices[i][3] = p[i][3];
	}

	if (dst->GetGLXDrawable ()) {
		int i;

		for (i = 0; i < 4; i++) {
			GLfloat v = vertices[i][1] + vertices[i][3];

			vertices[i][1] = vertices[i][3] - v;
		}
	}

	ms->unref ();
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
	double      dx = 2.0 / dst->Width ();
	double      dy = 2.0 / dst->Height ();
	double      p[4][4];
	int         i;

	p[0][0] = 0.0;
	p[0][1] = 0.0;
	p[0][2] = 0.0;
	p[0][3] = 1.0;

	p[1][0] = r.width;
	p[1][1] = 0.0;
	p[1][2] = 0.0;
	p[1][3] = 1.0;

	p[2][0] = r.width;
	p[2][1] = r.height;
	p[2][2] = 0.0;
	p[2][3] = 1.0;

	p[3][0] = 0.0;
	p[3][1] = r.height;
	p[3][2] = 0.0;
	p[3][3] = 1.0;

	for (i = 0; i < 4; i++) {
		vertices[i][0] = p[i][0] * dx - p[i][3];
		vertices[i][1] = p[i][1] * dy - p[i][3];
		vertices[i][2] = -p[i][2];
		vertices[i][3] = p[i][3];
	}

	if (dst->GetGLXDrawable ()) {
		int i;

		for (i = 0; i < 4; i++) {
			GLfloat v = vertices[i][1] + vertices[i][3];

			vertices[i][1] = vertices[i][3] - v;
		}
	}

	ms->unref ();
}

void
GLXContext::SetupTexCoordData (const double *matrix,
			       double       du,
			       double       dv)
{
	Context::Target *target = Top ()->GetTarget ();
	Rect            r = target->GetData (NULL);
	double          p[4][4];
	int             i;

	p[0][0] = 0.0;
	p[0][1] = 0.0;
	p[0][2] = 0.0;
	p[0][3] = 1.0;

	p[1][0] = r.width;
	p[1][1] = 0.0;
	p[1][2] = 0.0;
	p[1][3] = 1.0;

	p[2][0] = r.width;
	p[2][1] = r.height;
	p[2][2] = 0.0;
	p[2][3] = 1.0;

	p[3][0] = 0.0;
	p[3][1] = r.height;
	p[3][2] = 0.0;
	p[3][3] = 1.0;

	for (i = 0; i < 4; i++) {
		Matrix3D::TransformPoint (p[i], matrix, p[i]);

		texcoords[i][0] = p[i][0] * du;
		texcoords[i][1] = p[i][1] * dv;
		texcoords[i][2] = p[i][2];
		texcoords[i][3] = p[i][3];
	}
}

void
GLXContext::SetupTexUnit (GLenum target, GLint texture)
{
	glBindTexture (target, texture);
	glTexParameteri (target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri (target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
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
		GLuint     program = GetProjectProgram (1.0, 0);
		GLsizei    width0 = src->Width ();
		GLsizei    height0 = src->Height ();

		if (!dst->GetGLXDrawable ())
			GLContext::SetFramebuffer ();

		SetViewport ();

		glUseProgram (program);

		SetupVertexData (0, 0, width0, height0);
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
		GLXSurface  *src = (GLXSurface  *) mSrc;
		GLuint      texture0 = src->Texture ();
		GLuint      program = GetProjectProgram (1.0, 0);
		GLsizei     width0 = src->Width ();
		GLsizei     height0 = src->Height ();

		if (!dst->GetGLXDrawable ())
			GLContext::SetFramebuffer ();

		SetViewport ();

		glUseProgram (program);

		SetupVertexData (rSrc.x - r.x, rSrc.y - r.y, width0, height0);
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
GLXContext::GroupBounds (Group extents)
{
	if (extents.r.width  > maxTextureSize ||
	    extents.r.height > maxTextureSize)
		return Rect ();

	return extents.r;
}

void
GLXContext::Push (Group extents)
{
	Rect r = GroupBounds (extents);

	if (!r.IsEmpty ()) {
		cairo_matrix_t matrix;
		GLXSurface     *surface = new GLXSurface (r.width, r.height);
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
GLXContext::Push (Cairo extents)
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
GLXContext::Blit (unsigned char *data,
		  int           stride)
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	GLXSurface  *dst = (GLXSurface *) ms;

	ForceCurrent ();

	// no support for clipping
	g_assert (GetClip () == r);

	// no support for blit to drawable at the moment
	g_assert (!dst->GetGLXDrawable ());

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
GLXContext::BlitYV12 (unsigned char *data[],
		     int           stride[])
{
	Target      *target = Top ()->GetTarget ();
	MoonSurface *ms;
	Rect        r = target->GetData (&ms);
	GLXSurface  *dst = (GLXSurface *) ms;
	int         size[] = { dst->Width (), dst->Height () };
	int         width[] = { size[0], size[0] / 2, size[0] / 2 };
	int         height[] = { size[1], size[1] / 2, size[1] / 2 };
	int         i;

	ForceCurrent ();

	// no support for clipping
	g_assert (GetClip () == r);

	// no support for blit to drawable at the moment
	g_assert (!dst->GetGLXDrawable ());

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
GLXContext::Paint (Color *color)
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
GLXContext::Paint (MoonSurface *src,
		   double      alpha,
		   double      x,
		   double      y)
{
	GLXContext::Project (src, NULL, alpha, x, y);
}

void
GLXContext::Project (MoonSurface  *src,
		     const double *matrix,
		     double       alpha,
		     double       x,
		     double       y)
{
	GLXSurface *surface = (GLXSurface *) src;
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

	ForceCurrent ();

	GLContext::Paint (src, m, alpha, x, y);
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

};
