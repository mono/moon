/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context-gl.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include "projection.h"
#include "effect.h"
#include "context-gl.h"

#include <string.h>

namespace Moonlight {

GLContext::GLContext (MoonSurface *surface) : Context (surface)
{
	unsigned i, j;

	framebuffer = 0;
	vs = 0;

	/* blend fragment shader */
	blend_program = 0;

	/* perspective transform fragment shaders */
	for (i = 0; i < 2; i++)
		for (j = 0; j < 2; j++)
			project_program[i][j] = 0;

	/* convolve fragment shaders */
	for (i = 0; i <= MAX_CONVOLVE_SIZE; i++)
		convolve_program[i] = 0;

	/* drop shadow fragment shaders */
	for (i = 0; i <= MAX_CONVOLVE_SIZE; i++)
		dropshadow_program[i] = 0;

	effect_program = g_hash_table_new (g_direct_hash,
					   g_direct_equal);
}

GLContext::~GLContext ()
{
	unsigned i, j;

	for (i = 0; i <= MAX_CONVOLVE_SIZE; i++)
		if (convolve_program[i])
			glDeleteProgram (convolve_program[i]);

	for (i = 0; i < 2; i++)
		for (j = 0; j < 2; j++)
			if (project_program[i][j])
				glDeleteProgram (project_program[i][j]);

	if (blend_program)
		glDeleteProgram (blend_program);

	if (framebuffer)
		glDeleteFramebuffers (1, &framebuffer);

	if (vs)
		glDeleteShader (vs);

	g_hash_table_destroy (effect_program);
}

void
GLContext::SetFramebuffer ()
{
	Context::Target *target = Top ()->GetTarget ();
	MoonSurface     *ms;
	Rect            r = target->GetData (&ms);
	GLSurface       *dst = (GLSurface *) ms;
	GLuint          texture = dst->Texture ();
	GLenum          status;

	if (!framebuffer)
		glGenFramebuffers (1, &framebuffer);

	glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D (GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D,
				texture,
				0);
	status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		g_warning ("GLContext::SetFramebuffer status not complete");

	ms->unref ();
}

void
GLContext::SetScissor ()
{
	Context::Target *target = Top ()->GetTarget ();
	Rect            r = target->GetData (NULL);
	Rect            clip;

	Top ()->GetClip (&clip);

	clip.x -= r.x;
	clip.y -= r.y;

	glScissor (clip.x,
		   clip.y,
		   clip.width,
		   clip.height);
}

void
GLContext::SetViewport ()
{
	Context::Target *target = Top ()->GetTarget ();
	MoonSurface     *ms;
	Rect            r = target->GetData (&ms);
	GLSurface       *dst = (GLSurface *) ms;

	glViewport (0, 0, dst->Width (), dst->Height ());

	ms->unref ();
}

void
GLContext::SetupVertexData ()
{
	Context::Target *target = Top ()->GetTarget ();
	Rect            r = target->GetData (NULL);

	SetupVertexData (0.0, 0.0, r.width, r.height);
}

void
GLContext::SetupVertexData (double x,
			    double y,
			    double width,
			    double height)
{
	Context::Target *target = Top ()->GetTarget ();
	MoonSurface     *ms;
	Rect            r = target->GetData (&ms);
	GLSurface       *dst = (GLSurface *) ms;
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

	ms->unref ();
}

void
GLContext::SetupTexCoordData ()
{
	texcoords[0][0] = 0.0f;
	texcoords[0][1] = 0.0f;
	texcoords[0][2] = 0.0f;
	texcoords[0][3] = 1.0f;

	texcoords[1][0] = 1.0f;
	texcoords[1][1] = 0.0f;
	texcoords[1][2] = 0.0f;
	texcoords[1][3] = 1.0f;

	texcoords[2][0] = 1.0f;
	texcoords[2][1] = 1.0f;
	texcoords[2][2] = 0.0f;
	texcoords[2][3] = 1.0f;

	texcoords[3][0] = 0.0f;
	texcoords[3][1] = 1.0f;
	texcoords[3][2] = 0.0f;
	texcoords[3][3] = 1.0f;
}

void
GLContext::SetupTexCoordData (const double *matrix,
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
GLContext::Push (Group extents)
{
	g_warning ("GLContext::Push has been called. The derived class should have overridden it.");
	Context::Push (Clip ());
}

void
GLContext::Clear (Color *color)
{
	SetFramebuffer ();
	SetScissor ();

	glEnable (GL_SCISSOR_TEST);
	glClearColor (color->r, color->g, color->b, color->a);
	glClear (GL_COLOR_BUFFER_BIT);
	glDisable (GL_SCISSOR_TEST);

	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

void
GLContext::Blit (unsigned char *data,
		 int           stride)
{
	Context::Target *target = Top ()->GetTarget ();
	MoonSurface     *ms;
	Rect            r = target->GetData (&ms);
	GLSurface       *dst = (GLSurface *) ms;
	GLuint          texture = dst->Texture ();
	Rect            clip;

	Top ()->GetClip (&clip);

	// no support for clipping
	g_assert (r == clip);

	// need word alignment
	g_assert ((stride % 4) == 0);

	glPixelStorei (GL_UNPACK_ROW_LENGTH, stride / 4);
	glBindTexture (GL_TEXTURE_2D, texture);
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
	glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);

	ms->unref ();
}

void
GLContext::BlitYV12 (unsigned char *data[],
		     int           stride[])
{
	Context::Target *target = Top ()->GetTarget ();
	MoonSurface     *ms;
	Rect            r = target->GetData (&ms);
	GLSurface       *dst = (GLSurface *) ms;
	GLuint          texture[3];
	Rect            clip;
	int             i;

	Top ()->GetClip (&clip);

	// no support for clipping
	g_assert (r == clip);

	dst->AllocYUV ();

	texture[0] = dst->TextureY ();
	texture[1] = dst->TextureU ();
	texture[2] = dst->TextureV ();

	glPixelStorei (GL_UNPACK_ROW_LENGTH, stride[0]);
	glBindTexture (GL_TEXTURE_2D, texture[0]);
	glTexSubImage2D (GL_TEXTURE_2D,
			 0,
			 0,
			 0,
			 dst->Width (),
			 dst->Height (),
			 GL_LUMINANCE,
			 GL_UNSIGNED_BYTE,
			 data[0]);
	for (i = 1; i < 3; i++) {
		glPixelStorei (GL_UNPACK_ROW_LENGTH, stride[i]);
		glBindTexture (GL_TEXTURE_2D, texture[i]);
		glTexSubImage2D (GL_TEXTURE_2D,
				 0,
				 0,
				 0,
				 dst->Width () / 2,
				 dst->Height () / 2,
				 GL_LUMINANCE,
				 GL_UNSIGNED_BYTE,
				 data[i]);
	}
	glBindTexture (GL_TEXTURE_2D, 0);
	glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);

	ms->unref ();
}

GLuint
GLContext::CreateShader (GLenum       shaderType,
			 GLsizei      count,
			 const GLchar **str)
{
	GLuint shader;
	GLint  status;
	GLint  infoLen = 0;

	shader = glCreateShader (shaderType);
	glShaderSource (shader, count, str, NULL);
	glCompileShader (shader);
	glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
	glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &infoLen);
	if (infoLen > 1) {
		char *infoLog = (char *) g_malloc (infoLen);

		glGetShaderInfoLog (shader, infoLen, NULL, infoLog);
		g_warning ("glCompileShader:\n%s\n", infoLog);

		g_free (infoLog);
	}

	if (status)
		return shader;

	glDeleteShader (shader);
	return 0;
}

GLuint
GLContext::GetVertexShader ()
{
	const GLchar *vShaderStr[] = {
		"attribute vec4 InVertex;",
		"attribute vec4 InTexCoord0;",
		"void main()",
		"{",
		"  gl_TexCoord[0] = InTexCoord0;",
		"  gl_Position = InVertex;",
		"}"
	};

	if (vs)
		return vs;

	vs = CreateShader (GL_VERTEX_SHADER,
			   G_N_ELEMENTS (vShaderStr),
			   vShaderStr);

	return vs;
}

GLuint
GLContext::GetBlendProgram ()
{
	GString *s;
	GLuint  fs;

	if (blend_program)
		return blend_program;

	s = g_string_new ("uniform vec4 color;");
	g_string_sprintfa (s, "void main()");
	g_string_sprintfa (s, "{");
	g_string_sprintfa (s, "gl_FragColor = color;");
	g_string_sprintfa (s, "}");

	fs = CreateShader (GL_FRAGMENT_SHADER, 1, (const GLchar **) &s->str);

	g_string_free (s, 1);

	blend_program = glCreateProgram ();
	glAttachShader (blend_program, GetVertexShader ());
	glAttachShader (blend_program, fs);
	glBindAttribLocation (blend_program, 0, "InVertex");
	glLinkProgram (blend_program);

	glDeleteShader (fs);

	return blend_program;
}

void
GLContext::Paint (Color *color)
{
	GLuint program = GetBlendProgram ();
	GLint  color_location;

	SetFramebuffer ();
	SetViewport ();
	SetScissor ();

	SetupVertexData ();

	glUseProgram (program);

	color_location = glGetUniformLocation (program, "color");
	if (color_location >= 0)
		glUniform4f (color_location,
			     color->r * color->a,
			     color->g * color->a,
			     color->b * color->a,
			     color->a);

	glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, vertices);

	glEnableVertexAttribArray (0);

	glEnable (GL_SCISSOR_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	glDisable (GL_BLEND);
	glDisable (GL_SCISSOR_TEST);

	glDisableVertexAttribArray (0);

	glUseProgram (0);

	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

void
GLContext::Paint (MoonSurface *src,
		  double      alpha,
		  double      x,
		  double      y)
{
	double m[16];

	Matrix3D::Identity (m);

	Project (src, m, alpha, x, y);
}

GLuint
GLContext::GetProjectProgram (double opacity, unsigned yuv)
{
	unsigned alpha = opacity < 1.0 ? 1 : 0;
	GString  *s;
	GLuint   fs;

	if (project_program[alpha][yuv])
		return project_program[alpha][yuv];

	s = g_string_new ("uniform sampler2D sampler0;");
	if (yuv) {
		g_string_sprintfa (s, "uniform sampler2D sampler1;");
		g_string_sprintfa (s, "uniform sampler2D sampler2;");
	}
	if (alpha)
		g_string_sprintfa (s, "uniform vec4 alpha;");
	g_string_sprintfa (s, "void main()");
	g_string_sprintfa (s, "{");
	if (yuv) {
		g_string_sprintfa (s, "float r, g, b, y, u, v;");
		g_string_sprintfa (s, "y = texture2DProj(sampler0, gl_TexCoord[0].xyzw).r;");
		g_string_sprintfa (s, "u = texture2DProj(sampler1, gl_TexCoord[0].xyzw).r;");
		g_string_sprintfa (s, "v = texture2DProj(sampler2, gl_TexCoord[0].xyzw).r;");
		g_string_sprintfa (s, "y = 1.1643 * (y - 0.0625);");
		g_string_sprintfa (s, "u = u - 0.5;");
		g_string_sprintfa (s, "v = v - 0.5;");
		g_string_sprintfa (s, "r = y + 1.5958 * v;");
		g_string_sprintfa (s, "g = y - 0.39173 * u - 0.81290 * v;");
		g_string_sprintfa (s, "b = y + 2.017 * u;");
		g_string_sprintfa (s, "gl_FragColor = vec4(r, g, b, 1.0)");
	}
	else {
		g_string_sprintfa (s, "gl_FragColor = texture2DProj(sampler0, gl_TexCoord[0].xyzw)");
	}
	if (alpha)
		g_string_sprintfa (s, " * alpha");
	g_string_sprintfa (s, ";");
	g_string_sprintfa (s, "}");

	fs = CreateShader (GL_FRAGMENT_SHADER, 1, (const GLchar **) &s->str);

	g_string_free (s, 1);
	
	project_program[alpha][yuv] = glCreateProgram ();
	glAttachShader (project_program[alpha][yuv], GetVertexShader ());
	glAttachShader (project_program[alpha][yuv], fs);
	glBindAttribLocation (project_program[alpha][yuv], 0, "InVertex");
	glBindAttribLocation (project_program[alpha][yuv], 1, "InTexCoord0");
	glLinkProgram (project_program[alpha][yuv]);

	glDeleteShader (fs);

	return project_program[alpha][yuv];
}

void
GLContext::Project (MoonSurface  *src,
		    const double *matrix,
		    double       alpha,
		    double       x,
		    double       y)
{
	GLSurface *surface = (GLSurface *) src;
	GLsizei   width0 = surface->Width ();
	GLsizei   height0 = surface->Height ();
	GLuint    texture[3];
	int       n_sampler, i;
	GLuint    program;
	GLint     alpha_location;
	double    m[16];

	if (surface->TextureY () &&
	    surface->TextureU () &&
	    surface->TextureV ()) {
		program = GetProjectProgram (alpha, 1);
		texture[0] = surface->TextureY ();
		texture[1] = surface->TextureU ();
		texture[2] = surface->TextureV ();
		n_sampler = 3;
	}
	else {
		program = GetProjectProgram (alpha, 0);
		texture[0] = surface->Texture ();
		n_sampler = 1;
	}

	GetDeviceMatrix (m);
	Matrix3D::Multiply (m, matrix, m);
	if (!GetSourceMatrix (m, m, x, y))
		return;

	SetFramebuffer ();
	SetViewport ();
	SetScissor ();

	SetupVertexData ();
	SetupTexCoordData (m, 1.0 / width0, 1.0 / height0);

	glUseProgram (program);

	alpha_location = glGetUniformLocation (program, "alpha");
	if (alpha_location >= 0)
		glUniform4f (alpha_location, alpha, alpha, alpha, alpha);

	for (i = 0; i < n_sampler; i++) {
		GLint sampler_location;
		char  name[256];

		sprintf (name, "sampler%d", i);
		sampler_location = glGetUniformLocation (program, name);

		if (sampler_location < 0)
			continue;

		glActiveTexture (GL_TEXTURE0 + i);
		glBindTexture (GL_TEXTURE_2D, texture[i]);

		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				 GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				 GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
				 GL_CLAMP_TO_BORDER);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				 GL_CLAMP_TO_BORDER);

		glUniform1i (sampler_location, i);
	}

	glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, 0, texcoords);

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);

	glEnable (GL_SCISSOR_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	glDisable (GL_BLEND);
	glDisable (GL_SCISSOR_TEST);

	glDisableVertexAttribArray (1);
	glDisableVertexAttribArray (0);

	for (i = 1; i < n_sampler; i++) {
		glActiveTexture (GL_TEXTURE0 + i);
		glBindTexture (GL_TEXTURE_2D, 0);
	}

	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, 0);

	glUseProgram (0);

	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

GLuint
GLContext::GetConvolveProgram (unsigned size)
{
	GString  *s;
	GLuint   fs;
	unsigned i;

	g_assert (size <= MAX_CONVOLVE_SIZE);

	if (convolve_program[size])
		return convolve_program[size];

	s = g_string_new ("uniform sampler2D sampler0;");
	g_string_sprintfa (s, "uniform vec4 InFilter[%d];", (size + 1) * 2);
	g_string_sprintfa (s, "void main()");
	g_string_sprintfa (s, "{");
	g_string_sprintfa (s, "vec4 tex, sum, off;");
	g_string_sprintfa (s, "tex = gl_TexCoord[0] + InFilter[0];");

	if (size) {
		g_string_sprintfa (s, "off = tex + InFilter[2];");
		g_string_sprintfa (s, "sum = texture2D(sampler0, off.xy) * "
				   "InFilter[3];");
		g_string_sprintfa (s, "off = tex - InFilter[2];");
		g_string_sprintfa (s, "sum = sum + texture2D(sampler0, off.xy) * "
				   "InFilter[3];");
	}

	for (i = 2; i <= size; i++) {
		g_string_sprintfa (s, "off = tex + InFilter[%d];", i * 2);
		g_string_sprintfa (s, "sum = sum + texture2D(sampler0, off.xy) * "
				   "InFilter[%d];", i * 2 + 1);
		g_string_sprintfa (s, "off = tex - InFilter[%d];", i * 2);
		g_string_sprintfa (s, "sum = sum + texture2D(sampler0, off.xy) * "
				   "InFilter[%d];", i * 2 + 1);
	}

	g_string_sprintfa (s, "gl_FragColor = sum + texture2D(sampler0, tex.xy) * "
			   "InFilter[1];");
	g_string_sprintfa (s, "}");

	fs = CreateShader (GL_FRAGMENT_SHADER, 1, (const GLchar **) &s->str);

	g_string_free (s, 1);

	convolve_program[size] = glCreateProgram ();
	glAttachShader (convolve_program[size], GetVertexShader ());
	glAttachShader (convolve_program[size], fs);
	glBindAttribLocation (convolve_program[size], 0, "InVertex");
	glBindAttribLocation (convolve_program[size], 1, "InTexCoord0");
	glLinkProgram (convolve_program[size]);

	glDeleteShader (fs);

	return convolve_program[size];
}

void
GLContext::Blur (MoonSurface *src,
		 double      radius,
		 double      x,
		 double      y)
{
	GLSurface    *surface = (GLSurface *) src;
	GLuint       texture0 = surface->Texture ();
	GLuint       texture1;
	GLsizei      width0 = surface->Width ();
	GLsizei      height0 = surface->Height ();
	GLuint       program;
	const double precision = 1.0 / 256.0;
	double       values[MAX_CONVOLVE_SIZE + 1];
	GLfloat      cbuf[MAX_CONVOLVE_SIZE + 1][2][4];
	Rect         r = Rect (0, 0, width0, height0);
	int          size, i;
	double       m[16];

	size = ComputeGaussianSamples (radius, precision, values);
	if (size == 0) {
		Matrix3D::Identity (m);
		Project (src, m, 1.0, x, y);
		return;
	}

	program = GetConvolveProgram (size);

	GetDeviceMatrix (m);
	if (!GetSourceMatrix (m, m, x, y))
		return;

	glGenTextures (1, &texture1);
	glBindTexture (GL_TEXTURE_2D, texture1);
	glTexImage2D (GL_TEXTURE_2D,
		      0,
		      GL_RGBA,
		      width0,
		      height0,
		      0,
		      GL_BGRA,
		      GL_UNSIGNED_BYTE,
		      NULL);

	if (!framebuffer)
		glGenFramebuffers (1, &framebuffer);

	glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D (GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D,
				texture1,
				0);
	glViewport (0, 0, width0, height0);

	for (i = 0; i < 4; i++) {
		const float vx[] = { -1.0f, 1.0f, 1.0f, -1.0f };
		const float vy[] = { -1.0f, -1.0f, 1.0f, 1.0f };

		vertices[i][0] = vx[i];
		vertices[i][1] = vy[i];
		vertices[i][2] = 0.0f;
		vertices[i][3] = 1.0f;
	}
	SetupTexCoordData ();

	glUseProgram (program);

	for (i = 0; i <= size; i++) {
		cbuf[i][1][0] = values[i];
		cbuf[i][1][1] = values[i];
		cbuf[i][1][2] = values[i];
		cbuf[i][1][3] = values[i];
	}

	for (i = 0; i <= size; i++) {
		cbuf[i][0][0] = i / r.width;
		cbuf[i][0][1] = 0.0f;
		cbuf[i][0][2] = 0.0f;
		cbuf[i][0][3] = 1.0f;
	}

	glUniform4fv (glGetUniformLocation (program, "InFilter"),
		      (size + 1) * 2,
		      (const GLfloat *) cbuf);

	glBindTexture (GL_TEXTURE_2D, texture0);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glUniform1i (glGetUniformLocation (program, "sampler0"), 0);
	
	glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, 0, texcoords);

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);

	glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	SetFramebuffer ();
	SetViewport ();
	SetScissor ();

	SetupVertexData ();
	SetupTexCoordData (m, 1.0 / width0, 1.0 / height0);

	for (i = 0; i <= size; i++) {
		cbuf[i][0][0] = 0.0f;
		cbuf[i][0][1] = i / r.height;
		cbuf[i][0][2] = 0.0f;
		cbuf[i][0][3] = 1.0f;
	}

	glUniform4fv (glGetUniformLocation (program, "InFilter"),
		      (size + 1) * 2,
		      (const GLfloat *) cbuf);

	glBindTexture (GL_TEXTURE_2D, texture1);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glUniform1i (glGetUniformLocation (program, "sampler0"), 0);

	glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, 0, texcoords);

	glEnable (GL_SCISSOR_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	glDisable (GL_BLEND);
	glDisable (GL_SCISSOR_TEST);

	glDisableVertexAttribArray (1);
	glDisableVertexAttribArray (0);

	glBindTexture (GL_TEXTURE_2D, 0);

	glUseProgram (0);

	glBindFramebuffer (GL_FRAMEBUFFER, 0);

	glDeleteTextures (1, &texture1);
}

GLuint
GLContext::GetDropShadowProgram (unsigned size)
{
	GString  *s;
	GLuint   fs;
	unsigned i;

	g_assert (size <= MAX_CONVOLVE_SIZE);

	if (dropshadow_program[size])
		return dropshadow_program[size];

	s = g_string_new ("uniform sampler2D sampler0;");
	g_string_sprintfa (s, "uniform sampler2D sampler1;");
	g_string_sprintfa (s, "uniform vec4 InFilter[%d];", (size + 1) * 2);
	g_string_sprintfa (s, "void main()");
	g_string_sprintfa (s, "{");
	g_string_sprintfa (s, "vec4 tex, sum, off, img;");
	g_string_sprintfa (s, "img = texture2D(sampler0, gl_TexCoord[0].xy);");
	g_string_sprintfa (s, "tex = gl_TexCoord[0];");

	if (size > 0) { 
		g_string_sprintfa (s, "off = tex + InFilter[2];");
		g_string_sprintfa (s, "sum = texture2D(sampler1, off.xy) * "
				   "InFilter[3];");
		g_string_sprintfa (s, "off = tex - InFilter[2];");
		g_string_sprintfa (s, "sum = sum + texture2D(sampler1, off.xy) * "
				   "InFilter[3];");
	}

	for (i = 2; i <= size; i++) {
		g_string_sprintfa (s, "off = tex + InFilter[%d];", i * 2);
		g_string_sprintfa (s, "sum = sum + texture2D(sampler1, off.xy) * "
				   "InFilter[%d];", i * 2 + 1);
		g_string_sprintfa (s, "off = tex - InFilter[%d];", i * 2);
		g_string_sprintfa (s, "sum = sum + texture2D(sampler1, off.xy) * "
				   "InFilter[%d];", i * 2 + 1);
	}

	g_string_sprintfa (s, "sum = sum + texture2D(sampler1, tex.xy) * "
			   "InFilter[1];");
	g_string_sprintfa (s, "sum = sum.w * InFilter[0];");
	g_string_sprintfa (s, "gl_FragColor = (1.0 - img.w) * sum + img;");
	g_string_sprintfa (s, "}");

	fs = CreateShader (GL_FRAGMENT_SHADER, 1, (const GLchar **) &s->str);

	g_string_free (s, 1);

	dropshadow_program[size] = glCreateProgram ();
	glAttachShader (dropshadow_program[size], GetVertexShader ());
	glAttachShader (dropshadow_program[size], fs);
	glBindAttribLocation (dropshadow_program[size], 0, "InVertex");
	glBindAttribLocation (dropshadow_program[size], 1, "InTexCoord0");
	glLinkProgram (dropshadow_program[size]);

	glDeleteShader (fs);

	return dropshadow_program[size];
}

void
GLContext::DropShadow (MoonSurface *src,
		       double      dx,
		       double      dy,
		       double      radius,
		       Color       *color,
		       double      x,
		       double      y)
{
	GLSurface    *surface = (GLSurface *) src;
	GLuint       texture0 = surface->Texture ();
	GLuint       texture1;
	GLsizei      width0 = surface->Width ();
	GLsizei      height0 = surface->Height ();
	GLuint       program;
	const double precision = 1.0 / 256.0;
	double       values[MAX_CONVOLVE_SIZE + 1];
	GLfloat      cbuf[MAX_CONVOLVE_SIZE + 1][2][4];
	Rect         r = Rect (0, 0, width0, height0);
	int          size, i;
	double       m[16];

	size = ComputeGaussianSamples (radius, precision, values);

	program = GetConvolveProgram (size);

	GetDeviceMatrix (m);
	if (!GetSourceMatrix (m, m, x, y))
		return;

	glGenTextures (1, &texture1);
	glBindTexture (GL_TEXTURE_2D, texture1);
	glTexImage2D (GL_TEXTURE_2D,
		      0,
		      GL_RGBA,
		      width0,
		      height0,
		      0,
		      GL_BGRA,
		      GL_UNSIGNED_BYTE,
		      NULL);

	if (!framebuffer)
		glGenFramebuffers (1, &framebuffer);

	glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D (GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D,
				texture1,
				0);
	glViewport (0, 0, width0, height0);

	for (i = 0; i < 4; i++) {
		const float vx[] = { -1.0f, 1.0f, 1.0f, -1.0f };
		const float vy[] = { -1.0f, -1.0f, 1.0f, 1.0f };

		vertices[i][0] = vx[i];
		vertices[i][1] = vy[i];
		vertices[i][2] = 0.0f;
		vertices[i][3] = 1.0f;
	}
	SetupTexCoordData ();

	glUseProgram (program);

	for (i = 0; i <= size; i++) {
		cbuf[i][1][0] = values[i];
		cbuf[i][1][1] = values[i];
		cbuf[i][1][2] = values[i];
		cbuf[i][1][3] = values[i];
	}

	cbuf[0][0][0] = dx / r.width;
	cbuf[0][0][1] = dy / r.height;
	cbuf[0][0][2] = 0.0f;
	cbuf[0][0][3] = 0.0f;

	for (i = 1; i <= size; i++) {
		cbuf[i][0][0] = i / r.width;
		cbuf[i][0][1] = 0.0f;
		cbuf[i][0][2] = 0.0f;
		cbuf[i][0][3] = 1.0f;
	}

	glUniform4fv (glGetUniformLocation (program, "InFilter"),
		      (size + 1) * 2,
		      (const GLfloat *) cbuf);

	glBindTexture (GL_TEXTURE_2D, texture0);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glUniform1i (glGetUniformLocation (program, "sampler0"), 0);

	glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, 0, texcoords);

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);

	glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	program = GetDropShadowProgram (size);

	SetViewport ();
	SetFramebuffer ();
	SetScissor ();

	SetupVertexData ();
	SetupTexCoordData (m, 1.0 / width0, 1.0 / height0);

	glUseProgram (program);

	cbuf[0][0][0] = color->r;
	cbuf[0][0][1] = color->g;
	cbuf[0][0][2] = color->b;
	cbuf[0][0][3] = color->a;

	for (i = 1; i <= size; i++) {
		cbuf[i][0][0] = 0.0f;
		cbuf[i][0][1] = i / r.height;
		cbuf[i][0][2] = 0.0f;
		cbuf[i][0][3] = 1.0f;
	}

	glUniform4fv (glGetUniformLocation (program, "InFilter"),
		      (size + 1) * 2,
		      (const GLfloat *) cbuf);

	glBindTexture (GL_TEXTURE_2D, texture0);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glUniform1i (glGetUniformLocation (program, "sampler0"), 0);

	glActiveTexture (GL_TEXTURE1);
	glBindTexture (GL_TEXTURE_2D, texture1);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glUniform1i (glGetUniformLocation (program, "sampler1"), 1);

	glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, 0, texcoords);

	glEnable (GL_SCISSOR_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	glDisable (GL_BLEND);
	glDisable (GL_SCISSOR_TEST);

	glDisableVertexAttribArray (1);
	glDisableVertexAttribArray (0);

	glBindTexture (GL_TEXTURE_2D, 0);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, 0);

	glUseProgram (0);

	glBindFramebuffer (GL_FRAMEBUFFER, 0);

	glDeleteTextures (1, &texture1);
}

#define ERROR_IF(EXP)							\
	do { if (EXP) {							\
			ShaderEffect::ShaderError (ps,			\
				     "Shader error (" #EXP ") at "	\
				     "instruction %.2d", n);		\
			g_string_free (s, 1); return 0; }		\
	} while (0)

const char *
GLContext::WritemaskToType (const char *writemask)
{
	switch (strlen (writemask)) {
		case 1: return "float";
		case 2: return "vec2";
		case 3: return "vec3";
		case 4: return "vec4";
		default:
			g_assert (0);
	}
}

GLuint
GLContext::GetEffectProgram (PixelShader *ps)
{
	GString       *s;
	GLuint        fs;
	GLuint        program;
	gpointer      data;
	d3d_version_t version;
	d3d_op_t      op;
	int           index;
	GLfloat       imm[64][4];
	int           n_imm = 0;
	char          src_reg[D3DSPR_LAST][MAX_CONSTANTS][64];
	char          dst_reg[D3DSPR_LAST][MAX_CONSTANTS][64];
	int           n_sincos = 0;
	int           n = 0;

	// TODO: release effect shaders when destroyed
	data = g_hash_table_lookup (effect_program, ps);
	if (data)
		return GPOINTER_TO_UINT (data);

	if ((index = ps->GetVersion (0, &version)) < 0)
		return 0;

	if (version.type  != 0xffff ||
	    version.major != 2      ||
	    version.minor != 0) {
		ShaderEffect::ShaderError (ps, "Unsupported pixel shader");
		return 0;
	}

	for (int i = 0; i < D3DSPR_LAST; i++) {
		for (int j = 0; j < MAX_CONSTANTS; j++) {
			src_reg[i][j][0] = '\0';
			dst_reg[i][j][0] = '\0';
		}
	}
	
	s = g_string_new (NULL);

	sprintf (dst_reg[D3DSPR_COLOROUT][0], "gl_FragColor");

	/* validation and register allocation */
	for (int i = ps->GetOp (index, &op); i > 0; i = ps->GetOp (i, &op)) {
		d3d_destination_parameter_t reg;
		d3d_source_parameter_t      src;

		if (op.type == D3DSIO_COMMENT) {
			i += op.comment_length;
			continue;
		}

		if (op.type == D3DSIO_END)
			break;

		switch (op.type) {
				// case D3DSIO_DEFB:
				// case D3DSIO_DEFI:
			case D3DSIO_DEF: {
				d3d_def_instruction_t def;
				int                   j;

				i = ps->GetInstruction (i, &def);

				ERROR_IF (def.reg.writemask != 0xf);
				ERROR_IF (def.reg.dstmod != 0);
				ERROR_IF (def.reg.regnum >= MAX_CONSTANTS);

				sprintf (src_reg[def.reg.regtype][def.reg.regnum],
					 "imm[%d]", n_imm);

				for (j = 0; j < 4; j++)
					imm[n_imm][j] = def.v[j];

				n_imm++;
			} break;
			case D3DSIO_DCL: {
				d3d_dcl_instruction_t dcl;

				i = ps->GetInstruction (i, &dcl);

				ERROR_IF (dcl.reg.dstmod != 0);
				ERROR_IF (dcl.reg.regnum >= MAX_CONSTANTS);
				ERROR_IF (dcl.reg.regnum >= MAX_SAMPLERS);
				ERROR_IF (dcl.reg.regtype != D3DSPR_SAMPLER &&
					  dcl.reg.regtype != D3DSPR_TEXTURE);

				switch (dcl.reg.regtype) {
					case D3DSPR_SAMPLER:
						sprintf (src_reg[D3DSPR_SAMPLER][dcl.reg.regnum],
							 "sampler%d",
							 dcl.reg.regnum);

						g_string_sprintfa (s,
								   "uniform sampler2D %s;\n",
								   src_reg[D3DSPR_SAMPLER][dcl.reg.regnum]);
						break;
					case D3DSPR_TEXTURE:
						sprintf (src_reg[D3DSPR_TEXTURE][dcl.reg.regnum],
							 "gl_TexCoord[0]");
					default:
						break;
				}
			} break;
			default: {
				unsigned ndstparam = op.meta.ndstparam;
				unsigned nsrcparam = op.meta.nsrcparam;
				int      j = i;

				n++;

				while (ndstparam--) {
					j = ps->GetDestinationParameter (j, &reg);

					ERROR_IF (reg.regnum >= MAX_CONSTANTS);
					ERROR_IF (reg.dstmod != D3DSPD_NONE &&
						  reg.dstmod != D3DSPD_SATURATE);
					ERROR_IF (reg.regtype != D3DSPR_TEMP &&
						  reg.regtype != D3DSPR_COLOROUT);

					if (reg.regtype == D3DSPR_TEMP) {
						if (dst_reg[D3DSPR_TEMP][reg.regnum][0] == '\0') {
							sprintf (dst_reg[D3DSPR_TEMP][reg.regnum],
								 "tmp%d", reg.regnum);
							sprintf (src_reg[D3DSPR_TEMP][reg.regnum],
								 "tmp%d", reg.regnum);

							g_string_sprintfa (s,
									   "vec4 %s;\n",
									   dst_reg[D3DSPR_TEMP][reg.regnum]);
						}
					}

					ERROR_IF (dst_reg[reg.regtype][reg.regnum][0] == '\0');
					ERROR_IF (op.type == D3DSIO_SINCOS && (reg.writemask & ~0x3) != 0);
				}

				while (nsrcparam--) {
					j = ps->GetSourceParameter (j, &src);

					ERROR_IF (src.regnum >= MAX_CONSTANTS);
					ERROR_IF (src.srcmod != D3DSPS_NONE &&
						  src.srcmod != D3DSPS_NEGATE &&
						  src.srcmod != D3DSPS_ABS);
					ERROR_IF (src.regtype != D3DSPR_TEMP &&
						  src.regtype != D3DSPR_CONST &&
						  src.regtype != D3DSPR_SAMPLER &&
						  src.regtype != D3DSPR_TEXTURE);

					if (src.regtype == D3DSPR_CONST)
						if (src_reg[D3DSPR_CONST][src.regnum][0] == '\0')
							sprintf (src_reg[D3DSPR_CONST][src.regnum],
								 "InConstant[%d]", src.regnum);

					ERROR_IF (src_reg[src.regtype][src.regnum][0] == '\0');
				}

				if (!op.meta.name) {
					ShaderEffect::ShaderError (ps, "Unknown shader instruction %.2d", n);
					g_string_free (s, 1);
					return 0;
				}

				ERROR_IF (op.type == D3DSIO_SLT ||
					  op.type == D3DSIO_SGE ||
					  op.type == D3DSIO_LIT ||
					  op.type == D3DSIO_DST ||
					  op.type == D3DSIO_NRM ||
					  op.type == D3DSIO_CND);

				switch (op.type) {
					case D3DSIO_SINCOS:
						n_sincos++;
						break;
					default:
						break;
				}

				i += op.length;
			} break;
		}
	}

	g_string_sprintfa (s,
			   "uniform vec4 InConstant[%d];\n",
			   MAX_CONSTANTS);

	if (n_imm)
		g_string_sprintfa (s, "uniform vec4 imm[%d];\n", n_imm);

	if (n_sincos) {
		g_string_sprintfa (s, "vec4 sincos(in vec4 src0, "
				   "in vec4 src1, in vec4 src2)\n");
		g_string_sprintfa (s, "{\n");
		g_string_sprintfa (s, "vec4 v1, v2, v3, v;\n");
		
		g_string_sprintfa (s, "v1 = src0;\n");
		g_string_sprintfa (s, "v2 = src1;\n");
		g_string_sprintfa (s, "v3 = src2;\n");

		// x * x
		g_string_sprintfa (s, "v.z = v1.w * v1.w;\n");
		g_string_sprintfa (s, "v.xy = v.zz * v2.xy + v2.wz;\n");
		g_string_sprintfa (s, "v.xy = v.xy * v.zz + v3.xy;\n");

		// partial sin( x/2 ) and final cos( x/2 )
		g_string_sprintfa (s, "v.xy = v.xy * v.zz + v3.wz;\n");

		// sin( x/2 )
		g_string_sprintfa (s, "v.x = v.x * v1.w;\n");
	
		// compute sin( x/2 ) * sin( x/2 ) and sin( x/2 ) * cos( x/2 )
		g_string_sprintfa (s, "v1.xy = v.xy * v.xx;\n");
	
		// 2 * sin( x/2 ) * sin( x/2 ) and 2 * sin( x/2 ) * cos( x/2 )
		g_string_sprintfa (s, "v.xy = v1.xy + v1.xy;\n");
	
		// cos( x ) and sin( x )
		g_string_sprintfa (s, "v.x = v3.z - v.x;\n");

		g_string_sprintfa (s, "return v;\n");
		g_string_sprintfa (s, "}\n");
	}

	g_string_sprintfa (s, "void main()\n");
	g_string_sprintfa (s, "{\n");

	for (int i = ps->GetOp (index, &op); i > 0; i = ps->GetOp (i, &op)) {
		d3d_destination_parameter_t reg;
		d3d_source_parameter_t      source[8];
		char                        dstreg[64];
		char                        dstmod[64];
		char                        dstmodend[64];
		char                        writemask[64];
		char                        srcreg[8][64];
		char                        srcmod[8][64];
		char                        srcmodend[8][64];
		char                        swizzle[8][64];
		char                        src[8][64];
		char                        rvalue[256];
		int                         j = i;

		if (op.type == D3DSIO_COMMENT) {
			i += op.comment_length;
			continue;
		}

		ERROR_IF (op.meta.ndstparam > 1);

		if (op.meta.ndstparam) {
			j = ps->GetDestinationParameter (j, &reg);

			sprintf (dstreg, "%s",
				 dst_reg[reg.regtype][reg.regnum]);

			sprintf (writemask, "%s%s%s%s",
				 reg.writemask & 0x1 ? "x" : "",
				 reg.writemask & 0x2 ? "y" : "",
				 reg.writemask & 0x4 ? "z" : "",
				 reg.writemask & 0x8 ? "w" : "");

			ERROR_IF (reg.writemask == 0);

			switch (reg.dstmod) {
				case D3DSPD_SATURATE:
					sprintf (dstmod, "clamp(");
					sprintf (dstmodend, ", 0.0, 1.0)");
					break;
				default:
					dstmod[0] = dstmodend[0] = '\0';
			}
		}

		for (unsigned k = 0; k < op.meta.nsrcparam; k++) {
			const char *swizzle_str[] = { "x", "y", "z", "w" };

			j = ps->GetSourceParameter (j, &source[k]);

			sprintf (srcreg[k], "%s",
				 src_reg[source[k].regtype][source[k].regnum]);

			sprintf (swizzle[k], "%s%s%s%s",
				 swizzle_str[source[k].swizzle.x],
				 swizzle_str[source[k].swizzle.y],
				 swizzle_str[source[k].swizzle.z],
				 swizzle_str[source[k].swizzle.w]);

			switch (source[k].srcmod) {
				case D3DSPS_NEGATE:
					sprintf (srcmod[k], "(-");
					sprintf (srcmodend[k], ")");
					break;
				case D3DSPS_ABS:
					sprintf (srcmod[k], "abs(");
					sprintf (srcmodend[k], ")");
					break;
				default:
					srcmod[k][0] = srcmodend[k][0] = '\0';
					break;
			}

			if (op.meta.ndstparam)
				sprintf (src[k],
					 "%s%s.%s%s%s%s%s",
					 srcmod[k],
					 srcreg[k],
					 reg.writemask & 0x1 ?
					 swizzle_str[source[k].swizzle.x] : "",
					 reg.writemask & 0x2 ?
					 swizzle_str[source[k].swizzle.y] : "",
					 reg.writemask & 0x4 ?
					 swizzle_str[source[k].swizzle.z] : "",
					 reg.writemask & 0x8 ?
					 swizzle_str[source[k].swizzle.w] : "",
					 srcmodend[k]);
		}

		i += op.length;

		switch (op.type) {
			case D3DSIO_NOP:
				break;
				// case D3DSIO_BREAK: break;
				// case D3DSIO_BREAKC: break;
				// case D3DSIO_BREAKP: break;
				// case D3DSIO_CALL: break;
				// case D3DSIO_CALLNZ: break;
				// case D3DSIO_LOOP: break;
				// case D3DSIO_RET: break;
				// case D3DSIO_ENDLOOP: break;
				// case D3DSIO_LABEL: break;
				// case D3DSIO_REP: break;
				// case D3DSIO_ENDREP: break;
				// case D3DSIO_IF: break;
				// case D3DSIO_IFC: break;
				// case D3DSIO_ELSE: break;
				// case D3DSIO_ENDIF: break;
			case D3DSIO_MOV:
				sprintf (rvalue, "%s", src[0]);
				break;
			case D3DSIO_ADD:
				sprintf (rvalue, "%s + %s", src[0], src[1]);
				break;
			case D3DSIO_SUB:
				sprintf (rvalue, "%s - %s", src[0], src[1]);
				break;
			case D3DSIO_MAD:
				sprintf (rvalue, "%s * %s + %s", src[0], src[1], src[2]);
				break;
			case D3DSIO_MUL:
				sprintf (rvalue, "%s * %s", src[0], src[1]);
				break;
			case D3DSIO_RCP:
				sprintf (rvalue, "1.0 / %s", src[0]);
				break;
			case D3DSIO_RSQ:
				sprintf (rvalue, "inversesqrt(%s)", src[0]);
				break;
			case D3DSIO_DP3:
				sprintf (rvalue, "dot(%s%s.%c%c%c%s, %s%s.%c%c%c%s)",
					 srcmod[0], srcreg[0],
					 swizzle[0][0], swizzle[0][1], swizzle[0][2],
					 srcmodend[0],
					 srcmod[1], srcreg[1],
					 swizzle[1][0], swizzle[1][1], swizzle[1][2],
					 srcmodend[1]);
				break;
			case D3DSIO_DP4:
				sprintf (rvalue, "dot(%s, %s)", src[0], src[1]);
				break;
			case D3DSIO_MIN:
				sprintf (rvalue, "min(%s, %s)", src[0], src[1]);
				break;
			case D3DSIO_MAX:
				sprintf (rvalue, "max(%s, %s)", src[0], src[1]);
				break;
			case D3DSIO_SLT:
				sprintf (rvalue, "slt(%s, %s)", src[0], src[1]);
				break;
			case D3DSIO_SGE:
				sprintf (rvalue, "sgt(%s, %s)", src[0], src[1]);
				break;
			case D3DSIO_EXP:
				sprintf (rvalue, "exp(%s)", src[0]);
				break;
			case D3DSIO_LOG:
				sprintf (rvalue, "log(%s)", src[0]);
				break;
			case D3DSIO_LIT:
				sprintf (rvalue, "lit(%s)", src[0]);
				break;
			case D3DSIO_DST:
				sprintf (rvalue, "dst(%s, %s)", src[0], src[1]);
				break;
			case D3DSIO_LRP:
				sprintf (rvalue, "mix(%s, %s, %s)", src[2], src[1], src[0]);
				break;
			case D3DSIO_FRC:
				sprintf (rvalue, "fract(%s)", src[0]);
				break;
				// case D3DSIO_M4x4: break;
				// case D3DSIO_M4x3: break;
				// case D3DSIO_M3x4: break;
				// case D3DSIO_M3x3: break;
				// case D3DSIO_M3x2: break;
			case D3DSIO_POW:
				sprintf (rvalue, "pow(%s, %s)", src[0], src[1]);
				break;
				// case D3DSIO_CRS: break;
				// case D3DSIO_SGN: break;
			case D3DSIO_ABS:
				sprintf (rvalue, "abs(%s)", src[0]);
				break;
			case D3DSIO_NRM:
				sprintf (rvalue, "nrm(%s)", src[0]);
				break;
			case D3DSIO_SINCOS:
				sprintf (rvalue, "sincos(%s%s.%s%s, %s%s.%s%s, %s%s.%s%s).%s",
					 srcmod[0], srcreg[0], swizzle[0], srcmodend[0],
					 srcmod[1], srcreg[1], swizzle[1], srcmodend[1],
					 srcmod[2], srcreg[2], swizzle[2], srcmodend[2],
					 writemask);
				break;
				// case D3DSIO_MOVA: break;
				// case D3DSIO_TEXCOORD: break;
				// case D3DSIO_TEXKILL: break;
			case D3DSIO_TEX:
				sprintf (rvalue, "texture2D(%s, %s%s.%c%c%s).%s",
					 srcreg[1],
					 srcmod[0],
					 srcreg[0],
					 swizzle[0][0],
					 swizzle[0][1],
					 srcmodend[0],
					 writemask);
				break;
				// case D3DSIO_TEXBEM: break;
				// case D3DSIO_TEXBEML: break;
				// case D3DSIO_TEXREG2AR: break;
				// case D3DSIO_TEXREG2GB: break;
				// case D3DSIO_TEXM3x2PAD: break;
				// case D3DSIO_TEXM3x2TEX: break;
				// case D3DSIO_TEXM3x3PAD: break;
				// case D3DSIO_TEXM3x3TEX: break;
				// case D3DSIO_RESERVED0: break;
				// case D3DSIO_TEXM3x3SPEC: break;
				// case D3DSIO_TEXM3x3VSPEC: break;
				// case D3DSIO_EXPP: break;
				// case D3DSIO_LOGP: break;
			case D3DSIO_CND:
				sprintf (rvalue, "cnd(%s, %s, %s)", src[0], src[1], src[2]);
				break;
				// case D3DSIO_TEXREG2RGB: break;
				// case D3DSIO_TEXDP3TEX: break;
				// case D3DSIO_TEXM3x2DEPTH: break;
				// case D3DSIO_TEXDP3: break;
				// case D3DSIO_TEXM3x3: break;
				// case D3DSIO_TEXDEPTH: break;
			case D3DSIO_CMP:
				/* direct3d does src0 >= 0 */
				if (strlen (writemask) == 1)
					sprintf (rvalue, "mix(%s, %s, float(%s < 0.0))",
						 src[1], src[2], src[0]);
				else
					sprintf (rvalue, "mix(%s, %s, %s(lessThan(%s, %s(0.0))))",
						 src[1], src[2],
						 WritemaskToType (writemask),
						 src[0],
						 WritemaskToType (writemask));
				break;
				// case D3DSIO_BEM: break;
			case D3DSIO_DP2ADD:
				sprintf (rvalue, "dot(%s%s.%c%c%s, %s%s.%c%c%s) + %s%s.%c%s",
					 srcmod[0], srcreg[0], swizzle[0][0], swizzle[0][1],
					 srcmodend[0],
					 srcmod[1], srcreg[1], swizzle[1][0], swizzle[1][1],
					 srcmodend[1],
					 srcmod[2], srcreg[2], swizzle[2][0],
					 srcmodend[2]);
				break;
				// case D3DSIO_DSX: break;
				// case D3DSIO_DSY: break;
				// case D3DSIO_TEXLDD: break;
				// case D3DSIO_SETP: break;
				// case D3DSIO_TEXLDL: break;
			case D3DSIO_END:
				g_string_sprintfa (s, "}");

				fs = CreateShader (GL_FRAGMENT_SHADER,
						   1,
						   (const GLchar **) &s->str);

				g_string_free (s, 1);

				program = glCreateProgram ();
				glAttachShader (program, GetVertexShader ());
				glAttachShader (program, fs);
				glBindAttribLocation (program, 0, "InVertex");
				glBindAttribLocation (program, 1, "InTexCoord0");
				glLinkProgram (program);
				glDeleteShader (fs);

				if (n_imm) {
					glUseProgram (program);
					glUniform4fv (glGetUniformLocation (program, "imm"),
						      n_imm, (const GLfloat *) imm);
					glUseProgram (0);
				}

				g_hash_table_insert (effect_program,
						     ps,
						     GUINT_TO_POINTER (program));

				return program;
			default:
				break;
		}

		if (op.meta.ndstparam)
			g_string_sprintfa (s,
					   "%s.%s = %s%s%s;\n",
					   dstreg, writemask,
					   dstmod, rvalue, dstmodend);
	}
	
	g_string_free (s, 1);

	return 0;
}

void
GLContext::ShaderEffect (MoonSurface *src,
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
	GLSurface *surface = (GLSurface *) src;
	GLSurface *input[GL_TEXTURE30 - GL_TEXTURE0];
	GLuint    texture0 = surface->Texture ();
	GLuint    program = GetEffectProgram (shader);
	GLsizei   width0 = surface->Width ();
	GLsizei   height0 = surface->Height ();
	GLfloat   cbuf[MAX_CONSTANTS][4];
	GLint     constant_location;
	double    m[16];
	int       i;

	g_assert (n_constant <= MAX_CONSTANTS);
	g_assert (!ddxUvDdyUvPtr || *ddxUvDdyUvPtr < MAX_CONSTANTS);

	GetDeviceMatrix (m);
	if (!GetSourceMatrix (m, m, x, y))
		return;

	SetFramebuffer ();
	SetViewport ();
	SetScissor ();

	for (i = 0; i < n_constant; i++) {
		cbuf[i][0] = constant[i].r;
		cbuf[i][1] = constant[i].g;
		cbuf[i][2] = constant[i].b;
		cbuf[i][3] = constant[i].a;
	}

	if (ddxUvDdyUvPtr) {
		cbuf[*ddxUvDdyUvPtr][0] = 1.0f / width0;
		cbuf[*ddxUvDdyUvPtr][1] = 0.0f;
		cbuf[*ddxUvDdyUvPtr][2] = 0.0f;
		cbuf[*ddxUvDdyUvPtr][3] = 1.0f / height0;
	}

	SetupVertexData ();
	SetupTexCoordData (m, 1.0 / width0, 1.0 / height0);

	glUseProgram (program);

	for (i = 0; i < n_sampler; i++) {
		GLint  sampler_location;
		GLuint sampler_texture = 0;
		char   name[256];

		sprintf (name, "sampler%d", i);
		sampler_location = glGetUniformLocation (program, name);

		input[i] = NULL;

		if (sampler_location < 0)
			continue;

		glActiveTexture (GL_TEXTURE0 + i);

		if (sampler[i]) {
			input[i] = new GLSurface (width0, height0);
			if (input[i]) {
				cairo_surface_t *surface = input[i]->Cairo ();
				cairo_t         *cr = cairo_create (surface);
				Rect            area = Rect (0,
							     0,
							     width0,
							     height0);

				sampler[i]->SetupBrush (cr, area);

				cairo_paint (cr);
				cairo_destroy (cr);
				cairo_surface_destroy (surface);

				sampler_texture = input[i]->Texture ();
			}
		}
		else {
			sampler_texture = texture0;
		}

		if (!sampler_texture) {
			g_warning ("GLContext::ShaderEffect: failed to "
				   "generate input texture for sampler "
				   "register %d", i);
			sampler_texture = texture0;
		}

		glBindTexture (GL_TEXTURE_2D, sampler_texture);

		switch (sampler_mode[i]) {
			case 2:
				glTexParameteri (GL_TEXTURE_2D,
						 GL_TEXTURE_MAG_FILTER,
						 GL_LINEAR);
				glTexParameteri (GL_TEXTURE_2D,
						 GL_TEXTURE_MIN_FILTER,
						 GL_LINEAR);
				break;
			default:
				glTexParameteri (GL_TEXTURE_2D,
						 GL_TEXTURE_MAG_FILTER,
						 GL_NEAREST);
				glTexParameteri (GL_TEXTURE_2D,
						 GL_TEXTURE_MIN_FILTER,
						 GL_NEAREST);
				break;
		}

		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
				 GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				 GL_CLAMP_TO_EDGE);

		glUniform1i (sampler_location, i);
	}

	constant_location = glGetUniformLocation (program, "InConstant");
	if (constant_location >= 0)
		glUniform4fv (constant_location,
			      n_constant,
			      (const GLfloat *) cbuf);

	glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, 0, texcoords);

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);

	glEnable (GL_SCISSOR_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	glDisable (GL_BLEND);
	glDisable (GL_SCISSOR_TEST);

	glDisableVertexAttribArray (1);
	glDisableVertexAttribArray (0);

	for (i = 1; i < n_sampler; i++) {
		glActiveTexture (GL_TEXTURE0 + i);
		glBindTexture (GL_TEXTURE_2D, 0);

		if (input[i])
			input[i]->unref ();
	}

	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, 0);

	glUseProgram (0);

	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

void
GLContext::Flush ()
{
	glFlush ();
}

};
