/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "runtime.h"
#include "effect.h"
#include "factory.h"

using namespace Moonlight;

class CustomEffect : public ShaderEffect {};

const int width = 256;
const int height = 256;

int frames = 0;

static void
effect_alarm_handler (int sig)
{
	printf ("%d frames in 5.0 seconds = %f FPS\n", frames, frames / 5.0);
	frames = 0;
	alarm (5);
}

int
main (int argc, char **argv)
{
	Context *ctx;
	MoonSurface *target;
	MoonSurface *surface;
	cairo_surface_t *dst, *src;
	CustomEffect *effect;
	PixelShader *shader;
	int stride = width * 4;
	Rect bounds = Rect (0, 0, width, height);
	gpointer data;
	bool status = true;
	int count = 1;

	if (argc < 2) {
		printf ("usage: %s SHADERFILE [COUNT]\n", argv[0]);
		return 1;
	}

	gtk_init (&argc, &argv);

	runtime_init_desktop ();

	data = g_malloc0 (height * stride);
	dst = cairo_image_surface_create_for_data ((unsigned char *) data,
						   CAIRO_FORMAT_ARGB32,
						   width, height, stride);
	target = new CairoSurface (dst);
	cairo_surface_destroy (dst);
	ctx = new Context ();
	ctx->Push (new Context::Node (target));
	target->unref ();

	src = cairo_surface_create_similar (dst,
					    CAIRO_CONTENT_COLOR_ALPHA,
					    width, height);
	surface = new CairoSurface (src);
	cairo_surface_destroy (src);

	effect = new CustomEffect ();
	shader = MoonUnmanagedFactory::CreatePixelShader ();

	shader->SetTokensFromPath (argv[1]);
	effect->SetPixelShader (shader);

	if (argc > 2) {
		count = atoi (argv[2]);
		if (count > 1) {
			signal (SIGALRM, effect_alarm_handler);
			alarm (5);
		}
	}

	while (status && count-- > 0) {
		status = effect->Render (ctx,
					 surface,
					 (double *) NULL,
					 0, 0, width, height);

		frames++;
	}

	effect->unref ();
	shader->unref ();

	surface->unref ();
	delete ctx;
	g_free (data);

	runtime_shutdown ();

	return status != true;
}
