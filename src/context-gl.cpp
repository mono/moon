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

#define GL_GLEXT_PROTOTYPES

#include "projection.h"
#include "effect.h"
#include "context-gl.h"

namespace Moonlight {

GLContext::GLContext (MoonSurface *surface) : Context (surface)
{
	unsigned i;

	for (i = 0; i < 4; i++) {
		texcoords[i][2] = 0.0f; /* r */
		texcoords[i][3] = 1.0f; /* q */
	}

	framebuffer = 0;
	vs = 0;

	/* perspective transform fragment shaders */
	for (i = 0; i < 2; i++)
		project_fs[i] = 0;

	/* convolve fragment shaders */
	for (i = 0; i <= MAX_CONVOLVE_SIZE; i++)
		convolve_fs[i] = 0;

	/* drop shadow fragment shaders */
	for (i = 0; i <= MAX_CONVOLVE_SIZE; i++)
		dropshadow_fs[i] = 0;

	effect_fs = g_hash_table_new (g_direct_hash,
				      g_direct_equal);
}

GLContext::~GLContext ()
{
	unsigned i;

	for (i = 0; i <= MAX_CONVOLVE_SIZE; i++)
		if (convolve_fs[i])
			glDeleteProgram (convolve_fs[i]);

	for (i = 0; i < 2; i++)
		if (project_fs[i])
			glDeleteProgram (project_fs[i]);

	if (framebuffer)
		glDeleteFramebuffers (1, &framebuffer);

	if (vs)
		glDeleteShader (vs);

	g_hash_table_destroy (effect_fs);
}

void
GLContext::SetFramebuffer ()
{
	Context::Surface *cs = Top ()->GetSurface ();
	MoonSurface      *ms;
	Rect             r = cs->GetData (&ms);
	GLSurface        *dst = (GLSurface *) ms;
	GLuint           texture = dst->Texture ();
	GLenum           status;

	if (!framebuffer)
		glGenFramebuffers (1, &framebuffer);

	glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D (GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D,
				texture,
				0);
	status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
	g_assert (status == GL_FRAMEBUFFER_COMPLETE);

	Top ()->Sync ();

	ms->unref ();
}

void
GLContext::SetScissor ()
{
	Context::Surface *cs = Top ()->GetSurface ();
	Rect             r = cs->GetData (NULL);
	Rect             clip;

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
	Context::Surface *cs = Top ()->GetSurface ();
	MoonSurface      *ms;
	Rect             r = cs->GetData (&ms);
	GLSurface        *dst = (GLSurface *) ms;

	glViewport (0, 0, dst->Width (), dst->Height ());

	ms->unref ();
}

void
GLContext::SetupVertexData ()
{
	vertices[0][0] = -1.0f;
	vertices[0][1] = -1.0f;
	vertices[0][2] = 0.0f;
	vertices[0][3] = 1.0f;

	vertices[1][0] = 1.0f;
	vertices[1][1] = -1.0f;
	vertices[1][2] = 0.0f;
	vertices[1][3] = 1.0f;

	vertices[2][0] = 1.0f;
	vertices[2][1] = 1.0f;
	vertices[2][2] = 0.0f;
	vertices[2][3] = 1.0f;

	vertices[3][0] = -1.0f;
	vertices[3][1] = 1.0f;
	vertices[3][2] = 0.0f;
	vertices[3][3] = 1.0f;

	texcoords[0][0] = 0.0f;
	texcoords[0][1] = 0.0f;

	texcoords[1][0] = 1.0f;
	texcoords[1][1] = 0.0f;

	texcoords[2][0] = 1.0f;
	texcoords[2][1] = 1.0f;

	texcoords[3][0] = 0.0f;
	texcoords[3][1] = 1.0f;
}

void
GLContext::SetupVertexData (const double *matrix,
			    double       x,
			    double       y,
			    double       width,
			    double       height)
{
	Context::Surface *cs = Top ()->GetSurface ();
	MoonSurface      *ms;
	Rect             r = cs->GetData (&ms);
	GLSurface        *dst = (GLSurface *) ms;
	double           dx = 2.0 / dst->Width ();
	double           dy = 2.0 / dst->Height ();
	double           p[4][4];
	int              i;

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

	texcoords[0][0] = 0.0f;
	texcoords[0][1] = 0.0f;

	texcoords[1][0] = 1.0f;
	texcoords[1][1] = 0.0f;

	texcoords[2][0] = 1.0f;
	texcoords[2][1] = 1.0f;

	texcoords[3][0] = 0.0f;
	texcoords[3][1] = 1.0f;

	ms->unref ();
}

void
GLContext::InitMatrix (double *out)
{
	Context::Surface *cs = Top ()->GetSurface ();
	Rect             r = cs->GetData (NULL);
	cairo_matrix_t   ctm;
	double           viewport[16];
	double           m[16];

	Top ()->GetMatrix (&ctm);

	Matrix3D::Translate (viewport, -r.x, -r.y, 0.0);
	Matrix3D::Affine (m,
			  ctm.xx, ctm.xy,
			  ctm.yx, ctm.yy,
			  ctm.x0, ctm.y0);
	Matrix3D::Multiply (out, m, viewport);
}

void
GLContext::TransformMatrix (double *out, const double *matrix)
{
	double m[16];

	InitMatrix (m);

	Matrix3D::Multiply (out, matrix, m);
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

GLuint
GLContext::CreateShader (GLenum       shaderType,
			 GLsizei      count,
			 const GLchar **str)
{
	GLuint shader;
	GLint  status;

	shader = glCreateShader (shaderType);
	glShaderSource (shader, count, str, NULL);
	glCompileShader (shader);
	glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		GLint infoLen = 0;

		glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1) {
			char *infoLog = (char *) g_malloc (infoLen);

			glGetShaderInfoLog (shader, infoLen, NULL, infoLog);
			g_warning ("Error compiling shader:\n%s\n", infoLog);

			g_free (infoLog);
		}

		glDeleteShader (shader);
		return 0;
	}

	return shader;
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
GLContext::GetProjectShader (double opacity)
{
	const GLchar *fShaderNoAlphaStr[] = {
		"uniform sampler2D sampler0;",
		"void main()",
		"{",
		"  gl_FragColor = texture2D(sampler0, gl_TexCoord[0].xy);",
		"}"
	};
	const GLchar *fShaderAlphaStr[] = {
		"uniform sampler2D sampler0;",
		"uniform vec4 alpha;",
		"void main()",
		"{",
		"  gl_FragColor = texture2D(sampler0, gl_TexCoord[0].xy) * alpha;",
		"}"
	};
	unsigned     index = opacity < 1.0 ? 1 : 0;
	GLuint       fs;

	if (project_fs[index])
		return project_fs[index];

	if (index)
		fs = CreateShader (GL_FRAGMENT_SHADER,
				   G_N_ELEMENTS (fShaderAlphaStr),
				   fShaderAlphaStr);
	else
		fs = CreateShader (GL_FRAGMENT_SHADER,
				   G_N_ELEMENTS (fShaderNoAlphaStr),
				   fShaderNoAlphaStr);

	project_fs[index] = glCreateProgram ();
	glAttachShader (project_fs[index], GetVertexShader ());
	glAttachShader (project_fs[index], fs);
	glBindAttribLocation (project_fs[index], 0, "InVertex");
	glBindAttribLocation (project_fs[index], 1, "InTexCoord0");
	glLinkProgram (project_fs[index]);

	return project_fs[index];
}

void
GLContext::Project (MoonSurface  *src,
		    const double *matrix,
		    double       alpha,
		    double       x,
		    double       y)
{
	GLSurface *surface = (GLSurface *) src;
	GLuint    texture0 = surface->Texture ();
	GLuint    program = GetProjectShader (alpha);
	GLsizei   width0 = surface->Width ();
	GLsizei   height0 = surface->Height ();
	GLint     alpha_location;
	double    m[16];

	TransformMatrix (m, matrix);

	SetFramebuffer ();
	SetViewport ();
	SetScissor ();

	SetupVertexData (m, x, y, width0, height0);

	glUseProgram (program);

	alpha_location = glGetUniformLocation (program, "alpha");
	if (alpha_location >= 0)
		glUniform4f (alpha_location, alpha, alpha, alpha, alpha);

	glBindTexture (GL_TEXTURE_2D, texture0);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glUniform1i (glGetUniformLocation (program, "sampler0"), 0);

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

	glBindTexture (GL_TEXTURE_2D, 0);

	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

GLuint
GLContext::GetConvolveShader (unsigned size)
{
	GString  *s;
	GLuint   fs;
	unsigned i;

	g_assert (size <= MAX_CONVOLVE_SIZE);

	if (convolve_fs[size])
		return convolve_fs[size];

	s = g_string_new ("uniform sampler2D sampler0;");
	g_string_sprintfa (s, "uniform vec4 InFilter[%d];", (size + 1) * 2);
	g_string_sprintfa (s, "void main()");
	g_string_sprintfa (s, "{");
	g_string_sprintfa (s, "vec4 tex, sum, off;");
	g_string_sprintfa (s, "tex = gl_TexCoord[0] + InFilter[0];");

	g_string_sprintfa (s, "off = tex + InFilter[2];");
	g_string_sprintfa (s, "sum = texture2D(sampler0, off.xy) * "
			   "InFilter[3];");
	g_string_sprintfa (s, "off = tex - InFilter[2];");
	g_string_sprintfa (s, "sum = sum + texture2D(sampler0, off.xy) * "
			   "InFilter[3];");

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

	convolve_fs[size] = glCreateProgram ();
	glAttachShader (convolve_fs[size], GetVertexShader ());
	glAttachShader (convolve_fs[size], fs);
	glBindAttribLocation (convolve_fs[size], 0, "InVertex");
	glBindAttribLocation (convolve_fs[size], 1, "InTexCoord0");
	glLinkProgram (convolve_fs[size]);

	return convolve_fs[size];
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

	size = Effect::ComputeGaussianSamples (radius, precision, values);
	if (size == 0) {
		Matrix3D::Identity (m);
		Project (src, m, 1.0, x, y);
		return;
	}

	program = GetConvolveShader (size);

	InitMatrix (m);

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

	SetupVertexData ();

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

	SetupVertexData (m, x, y, width0, height0);

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

	glBindFramebuffer (GL_FRAMEBUFFER, 0);

	glDeleteTextures (1, &texture1);
}

GLuint
GLContext::GetDropShadowShader (unsigned size)
{
	GString  *s;
	GLuint   fs;
	unsigned i;

	g_assert (size <= MAX_CONVOLVE_SIZE);

	if (dropshadow_fs[size])
		return dropshadow_fs[size];

	s = g_string_new ("uniform sampler2D sampler0;");
	g_string_sprintfa (s, "uniform sampler2D sampler1;");
	g_string_sprintfa (s, "uniform vec4 InFilter[%d];", (size + 1) * 2);
	g_string_sprintfa (s, "void main()");
	g_string_sprintfa (s, "{");
	g_string_sprintfa (s, "vec4 tex, sum, off, img;");
	g_string_sprintfa (s, "img = texture2D(sampler0, gl_TexCoord[0].xy);");
	g_string_sprintfa (s, "tex = gl_TexCoord[0];");

	g_string_sprintfa (s, "off = tex + InFilter[2];");
	g_string_sprintfa (s, "sum = texture2D(sampler1, off.xy) * "
			   "InFilter[3];");
	g_string_sprintfa (s, "off = tex - InFilter[2];");
	g_string_sprintfa (s, "sum = sum + texture2D(sampler1, off.xy) * "
			   "InFilter[3];");

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

	dropshadow_fs[size] = glCreateProgram ();
	glAttachShader (dropshadow_fs[size], GetVertexShader ());
	glAttachShader (dropshadow_fs[size], fs);
	glBindAttribLocation (dropshadow_fs[size], 0, "InVertex");
	glBindAttribLocation (dropshadow_fs[size], 1, "InTexCoord0");
	glLinkProgram (dropshadow_fs[size]);

	return dropshadow_fs[size];
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

	size = Effect::ComputeGaussianSamples (radius, precision, values);

	program = GetConvolveShader (size);

	InitMatrix (m);

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

	SetupVertexData ();

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

	program = GetDropShadowShader (size);

	SetViewport ();
	SetFramebuffer ();
	SetScissor ();

	SetupVertexData (m, x, y, width0, height0);

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

GLuint
GLContext::GetEffectShader (PixelShader *ps)
{
	GString       *s;
	GLuint        fs;
	GLuint        program;
	gpointer      data;
	d3d_version_t version;
	d3d_op_t      op;
	int           index;
	char          src_reg[D3DSPR_LAST][MAX_CONSTANTS][64];
	char          dst_reg[D3DSPR_LAST][MAX_CONSTANTS][64];
	int           last_constant = -1;
	int           n_sincos = 0;
	int           n_cmp = 0;
	int           n = 0;

	// TODO: release effect shaders when destroyed
	data = g_hash_table_lookup (effect_fs, ps);
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

				i = ps->GetInstruction (i, &def);

				ERROR_IF (def.reg.writemask != 0xf);
				ERROR_IF (def.reg.dstmod != 0);
				ERROR_IF (def.reg.regnum >= MAX_CONSTANTS);

				sprintf (src_reg[def.reg.regtype][def.reg.regnum],
					 "imm%d_%d",
					 def.reg.regtype,
					 def.reg.regnum);

				g_string_sprintfa (s,
						   "const vec4 %s = vec4(%.g, %.g, %.g, %.g);\n",
						   src_reg[def.reg.regtype][def.reg.regnum],
						   def.v[0], def.v[1], def.v[2], def.v[3]);
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

					if (src.regtype == D3DSPR_CONST) {
						if (src_reg[D3DSPR_CONST][src.regnum][0] == '\0') {
							sprintf (src_reg[D3DSPR_CONST][src.regnum],
								 "InConstant[%d]", src.regnum);

							last_constant = MAX (last_constant, (int) src.regnum);
						}
					}

					ERROR_IF (src_reg[src.regtype][src.regnum][0] == '\0');
				}

				if (!op.meta.name) {
					ShaderEffect::ShaderError (ps, "Unknown shader instruction %.2d", n);
					g_string_free (s, 1);
					return 0;
				}

				ERROR_IF (op.type == D3DSIO_DP3 ||
					  op.type == D3DSIO_SLT ||
					  op.type == D3DSIO_SGE ||
					  op.type == D3DSIO_LIT ||
					  op.type == D3DSIO_DST ||
					  op.type == D3DSIO_NRM ||
					  op.type == D3DSIO_CND ||
					  op.type == D3DSIO_DP2ADD);

				switch (op.type) {
					case D3DSIO_SINCOS:
						n_sincos++;
						break;
					case D3DSIO_CMP:
						n_cmp++;
					default:
						break;
				}

				i += op.length;
			} break;
		}
	}

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
		g_string_sprintfa (s, "v.xy = vec2(v.z) * vec2(v2) + "
				   "vec2(v2.wzzw);\n");
		g_string_sprintfa (s, "v.xy = vec2(v) * vec2(v.z) + "
				   "vec2(v3);\n");

		// partial sin( x/2 ) and final cos( x/2 )
		g_string_sprintfa (s, "v.xy = vec2(v) * vec2(v.z) + "
				   "vec2(v3.wzzw);\n");

		// sin( x/2 )
		g_string_sprintfa (s, "v.x = float(v) * float(v1.w);\n");
	
		// compute sin( x/2 ) * sin( x/2 ) and sin( x/2 ) * cos( x/2 )
		g_string_sprintfa (s, "v1.xy = vec2(v) * vec2(v.x);\n");
	
		// 2 * sin( x/2 ) * sin( x/2 ) and 2 * sin( x/2 ) * cos( x/2 )
		g_string_sprintfa (s, "v.xy = vec2(v1) + vec2(v1);\n");
	
		// cos( x ) and sin( x )
		g_string_sprintfa (s, "v.x = float(v3.z) - float(v);\n");

		g_string_sprintfa (s, "return v;\n");
		g_string_sprintfa (s, "}\n");
	}

	if (n_cmp) {
		g_string_sprintfa (s, "vec4 cmp(in vec4 src0, "
				   "in vec4 src1, in vec4 src2)\n");
		g_string_sprintfa (s, "{\n");
		g_string_sprintfa (s, "const vec4 zero = vec4(0.0);\n");
		g_string_sprintfa (s, "bvec4 b;\n");
		g_string_sprintfa (s, "b = lessThan(src0, zero);\n");
		g_string_sprintfa (s, "return mix(src2, src1, vec4 (b));\n");
		g_string_sprintfa (s, "}\n");
	}

	if (last_constant >= 0)
		g_string_sprintfa (s,
				   "uniform vec4 InConstant[%d];\n",
				   last_constant + 1);

	g_string_sprintfa (s, "void main()\n");
	g_string_sprintfa (s, "{\n");

	for (int i = ps->GetOp (index, &op); i > 0; i = ps->GetOp (i, &op)) {
		d3d_destination_parameter_t reg[8];
		d3d_source_parameter_t      source[8];
		char                        dst[8][64];
		char                        src[8][64];
		char                        cast[8][64];
		int                         j = i;

		if (op.type == D3DSIO_COMMENT) {
			i += op.comment_length;
			continue;
		}

		for (unsigned k = 0; k < op.meta.ndstparam; k++) {
			j = ps->GetDestinationParameter (j, &reg[k]);

			sprintf (dst[k], "%s.%s%s%s%s",
				 dst_reg[reg[k].regtype][reg[k].regnum],
				 reg[k].writemask & 0x1 ? "x" : "",
				 reg[k].writemask & 0x2 ? "y" : "",
				 reg[k].writemask & 0x4 ? "z" : "",
				 reg[k].writemask & 0x8 ? "w" : "");

			ERROR_IF (reg[k].dstmod == D3DSPD_SATURATE);

			switch (reg[k].dstmod) {
				case D3DSPD_SATURATE:
					break;
			}

			int bits = 0;

			if (reg[k].writemask & 0x1)
				bits++;
			if (reg[k].writemask & 0x2)
				bits++;
			if (reg[k].writemask & 0x4)
				bits++;
			if (reg[k].writemask & 0x8)
				bits++;
			
			switch (bits) {
				case 1:
					sprintf (cast[k], "float");
					break;
				case 2:
					sprintf (cast[k], "vec2");
					break;
				case 3:
					sprintf (cast[k], "vec3");
					break;
				case 4:
					sprintf (cast[k], "vec4");
					break;
			}
		}

		for (unsigned k = 0; k < op.meta.nsrcparam; k++) {
			const char *swizzle_str[] = { "x", "y", "z", "w" };
			char       tmp[64];

			j = ps->GetSourceParameter (j, &source[k]);

			if (source[k].swizzle.x != 0 ||
			    source[k].swizzle.y != 1 ||
			    source[k].swizzle.z != 2 ||
			    source[k].swizzle.w != 3)
				sprintf (tmp, "%s.%s%s%s%s",
					 src_reg[source[k].regtype][source[k].regnum],
					 swizzle_str[source[k].swizzle.x],
					 swizzle_str[source[k].swizzle.y],
					 swizzle_str[source[k].swizzle.z],
					 swizzle_str[source[k].swizzle.w]);
			else
				sprintf (tmp, "%s",
					 src_reg[source[k].regtype][source[k].regnum]);

			switch (source[k].srcmod) {
				case D3DSPS_NEGATE:
					sprintf (src[k], "-%s", tmp);
					break;
				case D3DSPS_ABS:
					sprintf (src[k], "abs(%s)", tmp);
					break;
				default:
					sprintf (src[k], "%s", tmp);
					break;
			}
		}

		i += op.length;

		switch (op.type) {
			case D3DSIO_NOP:
				g_string_sprintfa (s, ";\n");
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
				g_string_sprintfa (s, "%s = %s(%s);\n", dst[0], cast[0], src[0]);
				break;
			case D3DSIO_ADD:
				g_string_sprintfa (s, "%s = %s(%s + %s);\n", dst[0], cast[0], src[0], src[1]);
				break;
			case D3DSIO_SUB:
				g_string_sprintfa (s, "%s = %s(%s - %s);\n", dst[0], cast[0], src[0], src[1]);
				break;
			case D3DSIO_MAD:
				g_string_sprintfa (s, "%s = %s(%s * %s + %s);\n", dst[0], cast[0], src[0], src[1], src[2]);
				break;
			case D3DSIO_MUL:
				g_string_sprintfa (s, "%s = %s(%s * %s);\n", dst[0], cast[0], src[0], src[1]);
				break;
			case D3DSIO_RCP:
				g_string_sprintfa (s, "%s = %s(1 / %s);\n", dst[0], cast[0], src[0]);
				break;
			case D3DSIO_RSQ:
				g_string_sprintfa (s, "%s = %s(inversesqrt(%s));\n", dst[0], cast[0], src[0]);
				break;
			case D3DSIO_DP3:
				g_string_sprintfa (s, "%s = %s(dot3(%s, %s));\n", dst[0], cast[0], src[0], src[1]);
				break;
			case D3DSIO_DP4:
				g_string_sprintfa (s, "%s = %s(dot(%s, %s));\n", dst[0], cast[0], src[0], src[1]);
				break;
			case D3DSIO_MIN:
				g_string_sprintfa (s, "%s = %s(min(%s, %s));\n", dst[0], cast[0], src[0], src[1]);
				break;
			case D3DSIO_MAX:
				g_string_sprintfa (s, "%s = %s(max(%s, %s));\n", dst[0], cast[0], src[0], src[1]);
				break;
			case D3DSIO_SLT:
				g_string_sprintfa (s, "%s = %s(slt(%s, %s));\n", dst[0], cast[0], src[0], src[1]);
				break;
			case D3DSIO_SGE:
				g_string_sprintfa (s, "%s = %s(sgt(%s, %s));\n", dst[0], cast[0], src[0], src[1]);
				break;
			case D3DSIO_EXP:
				g_string_sprintfa (s, "%s = %s(exp(%s));\n", dst[0], cast[0], src[0]);
				break;
			case D3DSIO_LOG:
				g_string_sprintfa (s, "%s = %s(log(%s));\n", dst[0], cast[0], src[0]);
				break;
			case D3DSIO_LIT:
				g_string_sprintfa (s, "%s = %s(lit(%s));\n", dst[0], cast[0], src[0]);
				break;
			case D3DSIO_DST:
				g_string_sprintfa (s, "%s = %s(dst(%s, %s));\n", dst[0], cast[0], src[0], src[1]);
				break;
			case D3DSIO_LRP:
				g_string_sprintfa (s, "%s = %s(mix(%s, %s, %s));\n", dst[0], cast[0], src[2], src[1], src[0]);
				break;
			case D3DSIO_FRC:
				g_string_sprintfa (s, "%s = %s(fract(%s));\n", dst[0], cast[0], src[0]);
				break;
				// case D3DSIO_M4x4: break;
				// case D3DSIO_M4x3: break;
				// case D3DSIO_M3x4: break;
				// case D3DSIO_M3x3: break;
				// case D3DSIO_M3x2: break;
			case D3DSIO_POW:
				g_string_sprintfa (s, "%s = %s(pow(%s, %s));\n", dst[0], cast[0], src[0], src[1]);
				break;
				// case D3DSIO_CRS: break;
				// case D3DSIO_SGN: break;
			case D3DSIO_ABS:
				g_string_sprintfa (s, "%s = %s(abs(%s));\n", dst[0], cast[0], src[0]);
				break;
			case D3DSIO_NRM:
				g_string_sprintfa (s, "%s = %s(nrm(%s));\n", dst[0], cast[0], src[0]);
				break;
			case D3DSIO_SINCOS:
				g_string_sprintfa (s, "%s = %s(sincos(%s, %s, %s));\n", dst[0], cast[0], src[0], src[1], src[2]);
				break;
				// case D3DSIO_MOVA: break;
				// case D3DSIO_TEXCOORD: break;
				// case D3DSIO_TEXKILL: break;
			case D3DSIO_TEX:
				g_string_sprintfa (s, "%s = %s(texture2D(%s, vec2(%s)));\n", dst[0], cast[0], src[1], src[0]);
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
				g_string_sprintfa (s, "%s = %s(cnd(%s, %s, %s));\n", dst[0], cast[0], src[0], src[1], src[2]);
				break;
				// case D3DSIO_TEXREG2RGB: break;
				// case D3DSIO_TEXDP3TEX: break;
				// case D3DSIO_TEXM3x2DEPTH: break;
				// case D3DSIO_TEXDP3: break;
				// case D3DSIO_TEXM3x3: break;
				// case D3DSIO_TEXDEPTH: break;
			case D3DSIO_CMP:
				/* direct3d does src0 >= 0 */
				g_string_sprintfa (s, "%s = %s(cmp(%s, %s, %s));\n", dst[0], cast[0], src[0], src[2], src[1]);
				break;
				// case D3DSIO_BEM: break;
			case D3DSIO_DP2ADD:
				g_string_sprintfa (s, "%s = %s(dp2a(%s, %s, %s));\n", dst[0], cast[0], src[0], src[1], src[2]);
				break;
				// case D3DSIO_DSX: break;
				// case D3DSIO_DSY: break;
				// case D3DSIO_TEXLDD: break;
				// case D3DSIO_SETP: break;
				// case D3DSIO_TEXLDL: break;
			case D3DSIO_END:
				g_string_sprintfa (s, "}\n");

				g_warning ("shader: %s", s->str);

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

				g_hash_table_insert (effect_fs,
						     ps,
						     GUINT_TO_POINTER (program));

				return program;
			default:
				break;
		}
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
	GLuint    program = GetEffectShader (shader);
	GLsizei   width0 = surface->Width ();
	GLsizei   height0 = surface->Height ();
	GLfloat   cbuf[MAX_CONSTANTS][4];
	GLint     constant_location;
	double    m[16];
	int       i;

	g_assert (n_constant <= MAX_CONSTANTS);
	g_assert (!ddxUvDdyUvPtr || *ddxUvDdyUvPtr < MAX_CONSTANTS);

	InitMatrix (m);

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

	SetupVertexData (m, x, y, width0, height0);

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

	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

void
GLContext::Flush ()
{
	glFlush ();
}

};
